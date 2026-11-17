classdef Cortec < AbstractStimulator
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
    sectionName = 'Stimulation:Stimulation'
    configName = "StimulationPulses"
    visibleElements = [1 3 5 6 8 10:12 14 20:22];
    electrodes = [2, 16];
    returnChName = ""
    xBurstLabel = "us";
  end
  methods
    function initialize(~, app)
      %change parameter bounds
      app.Phase1ampuAEditFieldLabel.Text = "Pulse amplitude (uA)";
      app.Phase1ampuAEditField.ValueDisplayFormat = "%11.4g uA";
      app.Phase1ampuAEditField.Limits = [0 6120];
      app.Phase1durationusEditFieldLabel.Text = "Pulse duration (us)";
      app.Phase1durationusEditField.ValueDisplayFormat = "%11.4g us";
      app.Phase1durationusEditField.Limits = [10 2550];
      app.InterphasedurationusEditFieldLabel.Text = "Interphase duration (us)";
      app.InterphasedurationusEditField.ValueDisplayFormat = "%11.4g us";
      app.InterphasedurationusEditField.Limits = [10 2550];
      app.FrequencyHzEditField.Limits = [43.57 50000]; %limits calculated from dead zone 0 + dead zone 1 specs
      app.TrainDurationsEditFieldLabel.Text = "Number of Trains";
      app.TrainDurationsEditField.ValueDisplayFormat = "%11.4g";
      app.ChannelDropDown.Items = [string(1:32) ""];
      app.ReturnChannelDropDown.Items = [string(1:32) ""];
      app.ReturnChannelDropDown.Value = "2";
      app.ChannelAxes.Title.String = "Channels";

      %table
      app.TableConfigurations.RowName = ["Pulse amplitude (uA)", "Pulse duration (us)", ...
                                          "Dead zone 0 (us)", "Dead zone 1 (us)", ...
                                          "Number of Pulses", "Train Frequency (Hz)", "Number of Trains"];
      app.TableTriggers.RowName = ["Trigger", "PulseID", "Source ch", "Destination ch"];
      %figure
      xlabel(app.MultPulsesAxes, "Time (us)", 'Color', 'black')
      ylabel(app.MultPulsesAxes, "Current (uA)")
      xlabel(app.SinglePulseAxes, "Time (us)", 'Color', 'black')
      ylabel(app.SinglePulseAxes, "Current (uA)")
    end
    function shift(self, shiftDown)
      bigShift = [1 5 8];
      smallShift = [3 6 10];
      tinyShift = [11];
      %bigShift = [app.NumberofpulsesSpinner, app.NumberofpulsesSpinnerLabel, app.Phase1durationusEditField, app.Phase1durationusEditFieldLabel, app.FrequencyHzEditField, app.FrequencyHzEditFieldLabel];
      %smallShift = [app.Phase1ampuAEditField, app.Phase1ampuAEditFieldLabel, app.InterphasedurationusEditField, app.InterphasedurationusEditFieldLabel, app.TrainDurationsEditField, app.TrainDurationsEditFieldLabel];
      self.shiftElements(bigShift, shiftDown*-3/2);
      self.shiftElements(smallShift, shiftDown*-1);
      self.shiftElements(tinyShift, shiftDown*-1/2);
    end
    function config = buildConfig(~, app)
      p1Dur = app.Phase1durationusEditField.Value;
      freq = app.FrequencyHzEditField.Value;
      p2Del = app.InterphasedurationusEditField.Value;
      deadzone1 = (1/freq*1e6 - (5*p1Dur + 2*p2Del + 1));
      config =  [app.Phase1ampuAEditField.Value; p1Dur; p2Del; deadzone1;
                app.NumberofpulsesSpinner.Value;
                app.TrainFrequencyHzEditField.Value; app.TrainDurationsEditField.Value];
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
      self.amp1 = config(1);
      self.dur1 = config(2);
      self.del2 = config(3);
      self.numPulses = config(5);
      self.freqTrain = config(6);
      self.durTrain = config(7);

      self.del1 = 0;
      self.amp2 = -1/4*self.amp1;
      self.dur2 = 4*self.dur1;
      %translate from num of trains to actual duration
      self.durTrain = 1/self.freqTrain * self.durTrain;

      deadZone1 = config(4);
      self.freqBurst = 1/(self.del2 + deadZone1); %already in us
      self.durBurst = (self.numPulses-1)/self.freqBurst*1e-6 + (self.del1+self.dur1+self.del2+self.dur2)*1e-6; %s
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
    function [addChannelToList, sameExp, trigIndex] = handleSameConfig(~, ~, configCols)
      addChannelToList = true;
      trigIndex = configCols(1);
      sameExp = true;
    end
    function [cTable, tTable] = uniqueParameterProcessing(~, cTable, tTable)
      %add repetitions and train configuration to pulses in config table
      cID = 1;
      numLoadedIDs = size(cTable,2);
      for i=1:size(tTable, 2)
        pulseID = tTable(2,i);
        for j = 1:numLoadedIDs
          if cTable(1,j)== str2double(pulseID)
            %create new column
            for k=1:8
              if k < 6
                cTable(k,numLoadedIDs + cID) = cTable(k,j);
              else
                cTable(k,numLoadedIDs + cID) = tTable(k-1,i);
              end
            end
            tTable(2,i) = string(cID); %update pulseID
            cID = cID + 1;
            break
          end
        end
      end
      %get rid of extra table rows, all of them should be moved over to configuration
      tTable(5:7,:) = [];
      %get rid of original configuration columns
      cTable(:, 1:numLoadedIDs) = [];
      %get rid of "PulseID" column in Configuration table
      cTable(1,:) = [];
    end
    function [configs, triggers, cRowLabels, tRowLabels] = uniqueParameterSaving(~, cTable, tTable)
      configs = cTable.Data;
      triggers = tTable.Data;
      cRowLabels = cTable.RowName;
      tRowLabels = tTable.RowName;
      numTriggers = size(triggers,2);
      numConfigs = size(configs,2);
      %will have same number of triggers, might have diff num of configs
      for t = 1:numTriggers
          configID = str2double(triggers(2,t));
          %move data over
          triggers(5:7, t) = configs(5:7, configID); %reps, train freq, train reps
          %data that is left over
          pulse = configs(1:4, configID);
          for c = 1:numConfigs
              if all(isequal(pulse, configs(1:4,c)))
                  %use this config ID instead
                  triggers(2,t) = string(c);
                  break
              end
          end
      end
      %get rid of repeats and train configs
      configs(5:7, :) = [];
      %delete any extra configs, now that we moved IDs over
      i = 1;
      while (i <= numConfigs)
          j = i+1;
          while (j < numConfigs)
              if all(isequal(configs(:,i),configs(:,j)))
                  configs(:,j) = [];
                  numConfigs = numConfigs - 1;
              else
                  j = j+1;
              end
          end
          i = i+1;
      end
      pulseIDs = 1:numConfigs;
      configs = [pulseIDs; configs]; %add pulse IDs

      %update labels
      cTable.ColumnName = "Configuration" + string(1:(size(configs,2)+1));
      cTable.ColumnName(end) = [];
      tRowLabels(end+1:end+3) = cRowLabels(end-2:end);
      cRowLabels = ["PulseID"; cRowLabels];
      cRowLabels(end-2:end) = [];
      tTable.ColumnName = "Trigger" + string(1:(size(triggers,2)+1));
      tTable.ColumnName(end) = [];
    end
    function colorChannels(self, app)
      circI = str2double(app.ChannelDropDown.Value);
      if ~isnan(circI)
        app.ChannelAxes.Children(circI).FaceColor = self.testColor;
      end
      circI = str2double(app.ReturnChannelDropDown.Value);
      if ~isnan(circI)
        app.ChannelAxes.Children(circI).FaceColor = self.testColor;
        app.ChannelAxes.Children(circI).Curvature = [0 0];
      end
    end
  end
end