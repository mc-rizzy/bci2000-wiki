classdef NeuroOmega < AbstractStimulator
  properties
    amp1
    amp2
    dur1
    dur2
    del1
    del2
    durBurst
    freqBurst
    durTrain
    freqTrain
    numPulses
  end
  properties (Constant)
    sectionName = 'Application:NeuroOmegaStimulation'
    configName = "StimulationConfigurations"
    visibleElements = [2:11 12 14:15 20:22];
    xBurstLabel = "ms";
    
    returnChName = "-1"
    %neuro omega unique electrodes
    chMapping = {"LFP",     10000:10009;
                 "ECOG LF", 10016:10047;
                 "SEG",     10128:10132;
                 "SPK",     10256:10260;
                 "ECOG HF", 10272:10303;
                 "RAW",     10384:10393};
    electrodes = [6, 32]; %row number should be changed by chMapping
    
  end
  methods
    function initialize(~, app)
      %change
      app.Phase1ampuAEditFieldLabel.Text = "Phase 1 amp (mA)";
      app.Phase1ampuAEditField.ValueDisplayFormat = "%11.4g mA";
      app.Phase1ampuAEditField.Limits = [-15 15];
      app.Phase2ampuAEditFieldLabel.Text = "Phase 2 amp (mA)";
      app.Phase2ampuAEditField.ValueDisplayFormat = "%11.4g mA";
      app.Phase2ampuAEditField.Limits = [-15 15];
      app.Phase2ampuAEditField.Value = -15;
      app.Phase1durationusEditFieldLabel.Text = "Phase 1 duration (ms)";
      app.Phase1durationusEditField.ValueDisplayFormat = "%11.4g ms";
      app.Phase1durationusEditField.Limits = [0.02 0.5];
      app.InterphasedurationusEditFieldLabel.Text = "Phase 2 delay (ms)";
      app.InterphasedurationusEditField.ValueDisplayFormat = "%11.4g ms";
      app.InterphasedurationusEditField.Limits = [0 99];
      app.InterphasedurationusEditField.Value = 0.05;
      app.Phase2durationusEditFieldLabel.Text = "Phase 2 duration (ms)";
      app.Phase2durationusEditField.ValueDisplayFormat = "%11.4g ms";
      app.Phase2durationusEditField.Limits = [0.02 0.5];
      app.FrequencyHzEditField.Limits = [0 300];
      app.FrequencyHzEditField.Value = 3;
      app.TrainDurationsEditFieldLabel.Text = "Train Duration (s)";
      app.TrainDurationsEditField.ValueDisplayFormat = "%11.4g s";
      app.ChannelAxes.Title.String = "Channels";

      %table
      app.TableConfigurations.RowName = ["Phase 1 Delay (ms)", "Phase 1 Amp (mA)", "Phase 1 Width (ms)", ...
                                          "Phase 2 Delay (ms)", "Phase 2 Amp (mA)", "Phase 2 Width (ms)", ...
                                          "Frequency (Hz)","Stimulation Duration (s)","Train Duration (s)","Train Frequency (Hz)"];
      app.TableTriggers.RowName = ["Expression", "Config ID", "Channel(s)", "Return Channel"];
      
      %figure
      xlabel(app.MultPulsesAxes, "Time (ms)", 'Color', 'black')
      ylabel(app.MultPulsesAxes, "Current (mA)")
      xlabel(app.SinglePulseAxes, "Time (ms)", 'Color', 'black'),
      ylabel(app.SinglePulseAxes, "Current (mA)")
    end
    function shift(self, shiftDown)
      els = [10 11];
      %els = [app.TrainDurationsEditField, app.TrainDurationsEditFieldLabel, app.TrainFrequencyHzEditField, app.TrainFrequencyHzEditFieldLabel];
      self.shiftElements(els, shiftDown*1);
    end
    function config = buildConfig(~, app)
      p1Amp = app.Phase1ampuAEditField.Value;
      p2Amp = app.Phase2ampuAEditField.Value;
      p1Dur = app.Phase1durationusEditField.Value;
      p2Dur = app.Phase2durationusEditField.Value;
      freq = app.FrequencyHzEditField.Value;
      p2Del = app.InterphasedurationusEditField.Value;
      trainDur = app.TrainDurationsEditField.Value;
      trainFreq = app.TrainFrequencyHzEditField.Value;
      p1Del = app.Phase1delaymsEditField.Value;
      stimWidth = app.StimulationDurationsEditField.Value;
      config = [p1Del; p1Amp; p1Dur; p2Del; p2Amp; p2Dur; freq; stimWidth; trainDur; trainFreq];
    end
    function trig = buildTrig(self, app)
      expres = self.createExpression(app, true);
      ch = app.ChannelDropDown.Value;
      retCh = app.ReturnChannelDropDown.Value;
      self.checkCh(ch);
      self.checkCh(retCh);
      trig = [expres; -1; ch; retCh];
    end
    function setConfig(self, config)
      self.del1 = config(1);
      self.amp1 = config(2);
      self.dur1 = config(3);
      self.del2 = config(4);
      self.amp2 = config(5);
      self.dur2 = config(6);
      self.durBurst = config(8);
      self.durTrain = config(9);
      self.freqTrain = config(10);
  
      freq = config(7);
      self.numPulses = floor(freq*(self.durBurst-(self.del1+self.dur1+self.del2+self.dur2)*1e-3))+1;
      self.freqBurst = freq/1e3; %units in ms instead
    end
    function colorChannels(self, app)
      %delete extra channels
      for i = length(app.ChannelAxes.Children):-1:1
        pos = app.ChannelAxes.Children(i).Position(1:2);
        x = pos(1)+1;
        y = pos(2)+1;
        if x > size(self.chMapping{y,2},2)
          delete(app.ChannelAxes.Children(i));
        else
          %map channel to channel object
          app.ChannelAxes.Children(i).Channel = self.chMapping{y, 2}(x);
        end
      end
      circI = self.getCircleIndexFromChannel(app, str2double(app.ChannelDropDown.Value));
      if ~isnan(circI)
        app.ChannelAxes.Children(circI).FaceColor = self.testColor;
      end
      circI = self.getCircleIndexFromChannel(app, str2double(app.ReturnChannelDropDown.Value));
      if ~isnan(circI)
        app.ChannelAxes.Children(circI).FaceColor = self.testColor;
        app.ChannelAxes.Children(circI).Curvature = [0 0];
      end
    end
    
    function res = checkTrigTable(self, app, trigTable)
      res = true;
      expres = self.createExpression(app, true);
      ch = app.ChannelDropDown.Value;
      retCh = app.ReturnChannelDropDown.Value;
      %check to make sure channels aren't being triggered at same time
      for i=1:size(trigTable,2)
        if strcmp(expres, trigTable(1,i))
          chs = trigTable(3,i);
          retChs = trigTable(4,i);
          chList = [split(chs,", "), split(retChs,", ")] ;
          for j =1:length(chList)
            if strcmp(chList(j), ch) || strcmp(chList(j), retCh)
              errordlg(['The specified channel is already' ...
                  ' being stimulated with the same expression. ' ...
                  'Choose a different channel or trigger expression'],'Configuration Error');
              res = false;
              return 
            end
          end
        end
      end
    end
    function [addChannelToList, sameExp, trigIndex] = handleSameConfig(~, app, configCols)
      addChannelToList = false;
      sameExp = true;
      trigIndex = -1;
      %need to see if ch and return ch are in there
      %go thru each trig column where they have the config
      retCh = app.ReturnChannelDropDown.Value;
      for i=1:length(configCols)
        col = configCols(i);
        %column has same config
        if (retCh == app.TableTriggers.Data(4,col))
          %return ch is the same
          addChannelToList = true;
          trigIndex = col;
          break
        else
          %diff return ch, need new trigger
          sameExp = false;
        end
      end
    end
          

    %neuro omega specific functions
    function setNeuroOmegaChannels(self, app)
      selectedButton = app.RecordingChannelsButtonGroup.SelectedObject;
      for i=1:size(self.chMapping, 1)
          if self.chMapping{i,1} == selectedButton.Text
              app.ChannelDropDown.Items = [string(self.chMapping{i,2}) ""];
              app.ReturnChannelDropDown.Items = string([-1, self.chMapping{i,2}]);
              break
          end
      end
      %self.electrodes(1) = size(self.chMapping,1);
    end
    function circI = getCircleIndexFromChannel(self, app, ch)
        %make sure it is valid channel
      if isnan(ch) || ch == -1
          circI = nan;
          return
      end
      for i = 1:size(self.chMapping, 1) %6
          startVal = self.chMapping{i,2}(1);
          n = ch - startVal;
          if n >= 0 && n <= 32 %max length of channel group
              %make sure it doesn't belong in next group
              if i + 1 <= size(self.chMapping, 1)
                  if ch - self.chMapping{i+1,2}(1) < 0
                      %found group
                      g = i;
                      break
                  end
              else
                  %found group
                  g = i;
                  break
              end
          end
      end
      x = n;
      y = g - 1;
      for i = 1:length(app.ChannelAxes.Children)
          if app.ChannelAxes.Children(i).Position == [x y 1 1]
              circI = i;
              break
          end
      end
    end
  end
end