classdef CereStim < AbstractStimulator
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
    sectionName = 'CereStim:Waveform definition'
    configName = "StimulationConfigurations"
    visibleElements = [1 3:8 10:13 20:22];
    electrodes = [4, 24];
    xBurstLabel = "us";
  end
  methods
    function self = CereStim()
      self.stimBurstError = "DECREASE THE NUM OF PULSES OR THE TRAIN FREQUENCY";
    end
    function initialize(~, app)
      %change parameter bounds
      app.CathodeFirstButton.Text = "Cathode First";
      app.AnodeFirstButton.Text = "Anode First";
      app.OrderButtonGroup.Title = "Order";
      app.Phase1ampuAEditFieldLabel.Text = "Phase 1 amp (uA)";
      app.Phase1ampuAEditField.ValueDisplayFormat = "%11.4g uA";
      app.Phase1ampuAEditField.Limits = [100, 10000];
      app.Phase1ampuAEditField.Value = 200;
      app.Phase2ampuAEditFieldLabel.Text = "Phase 2 amp (uA)";
      app.Phase2ampuAEditField.ValueDisplayFormat = "%11.4g uA";
      app.Phase2ampuAEditField.Limits = [100, 10000];
      app.Phase2ampuAEditField.Value = 100;
      app.Phase1durationusEditFieldLabel.Text = "Phase 1 duration (us)";
      app.Phase1durationusEditField.ValueDisplayFormat = "%11.4g us";
      app.Phase1durationusEditField.Limits = [1, 65535];
      app.Phase1durationusEditField.Value = 250;
      app.InterphasedurationusEditFieldLabel.Text = "Interphase duration (us)";
      app.InterphasedurationusEditField.ValueDisplayFormat = "%11.4g us";
      app.InterphasedurationusEditField.Limits = [53, 65535];
      app.InterphasedurationusEditField.Value = 53;
      app.Phase2durationusEditFieldLabel.Text = "Phase 2 duration (us)";
      app.Phase2durationusEditField.ValueDisplayFormat = "%11.4g us";
      app.Phase2durationusEditField.Limits = [1, 65535];
      app.Phase2durationusEditField.Value = 500;
      app.FrequencyHzEditField.Limits = [4, 5000];
      app.FrequencyHzEditField.Value = 30;
      app.ChannelDropDown.Items = [string(1:96) ""];
      app.TrainDurationsEditFieldLabel.Text = "Train Duration (s)";
      app.TrainDurationsEditField.ValueDisplayFormat = "%11.4g s";
      app.ChannelAxes.Title.String = "Channels";

      %table
      app.TableConfigurations.RowName = ["Cathode First", "Number of pulses", "Phase 1 amp (uA)", "Phase 2 amp (uA)", ...
                                          "Phase 1 duration (us)","Phase 2 duration (us)","Frequency (Hz)", ...
                                          "Interphase duration (us)", "Train Duration (s)", "Train Frequency (Hz)"];
      app.TableTriggers.RowName = ["Expression", "Config ID", "Electrode(s)"];

      %figure
      xlabel(app.MultPulsesAxes, "Time (us)", 'Color', 'black')
      ylabel(app.MultPulsesAxes, "Current (uA)")
      xlabel(app.SinglePulseAxes, "Time (us)", 'Color', 'black')
      ylabel(app.SinglePulseAxes, "Current (uA)")
    end
    function shift(~, ~)
      return %do nothing, this is the default configuration
    end
    function config = buildConfig(~, app)
      config =  [app.CathodeFirstButton.Value; 
                app.NumberofpulsesSpinner.Value; 
                app.Phase1ampuAEditField.Value;
                app.Phase2ampuAEditField.Value; 
                app.Phase1durationusEditField.Value;
                app.Phase2durationusEditField.Value;
                app.FrequencyHzEditField.Value;
                app.InterphasedurationusEditField.Value;
                app.TrainDurationsEditField.Value;
                app.TrainFrequencyHzEditField.Value];
    end
    function trig = buildTrig(self, app)
      expres = self.createExpression(app, true);
      ch = app.ChannelDropDown.Value;
      self.checkCh(ch);
      trig = [expres; -1; ch];
    end
    function setConfig(self, config)
      self.numPulses = config(2);
      self.amp1 = config(3);
      self.amp2 = config(4);
      self.dur1 = config(5);
      self.dur2 = config(6);
      freq = config(7);
      self.del2 = config(8); 
      self.durTrain = config(9);
      self.freqTrain = config(10);        
      
      %change based on cathFirst
      cathFirst = config(1);
      if (cathFirst == 1)
          self.amp1 = -self.amp1;
      else
          self.amp2 = -self.amp2;
      end
      self.del1 = 0;
      self.durBurst = (self.numPulses-1)/freq + (self.del1+self.dur1+self.del2+self.dur2)*1e-6; %s
      self.freqBurst = freq/1e6;   
    end
    function colorChannels(self, app)
      circI = str2double(app.ChannelDropDown.Value);
      if ~isnan(circI)
        app.ChannelAxes.Children(circI).FaceColor = self.testColor;
      end
    end
    function res = checkTrigTable(self, app, trigTable)
      res = true;
      expres = self.createExpression(app, true);
      ch = app.ChannelDropDown.Value;
      %check to make sure channels aren't being triggered at same time
      for i=1:size(trigTable,2)
        if strcmp(expres, trigTable(1,i))
          chs = trigTable(3,i);
          chList = split(chs,", ");
          for j =1:length(chList)
            if strcmp(chList(j), ch)
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
      sameExp = true;
      addChannelToList = true;
      %cerestim should only have 1 trig column with same expression
      trigIndex = configCols(1);
    end
  end
end