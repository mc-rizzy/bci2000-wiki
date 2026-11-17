classdef GEstim < AbstractStimulator
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

    configNames
    asymmetric = false
  end
  properties (Constant)
    sectionName = 'gEstim:Stimulation'
    configName = "" %not used
    visibleElements = [1 3:8 10 11 16:22];
    electrodes = [0 0];
    xBurstLabel = "ms";
  end
  methods
    function self = GEstim()
      %define gEstim parameter names from parameter file
      self.configNames = ["Modularity", "Polarity", "PhaseDuration", ...
          "InterphaseDuration", "Magnitude", "PulseFrequency", ...
          "NumberOfPulses", "FrequencyOfTrains", "NumberOfTrains"];
      self.configNames = strcat(self.configNames, "="); %add "=" to parameter names
    end
    function initialize(~, app)
      %change parameter bounds
%       app.CathodeFirstButton.Text = "Biphasic";
%       app.AnodeFirstButton.Text = "Monophasic";
%       app.OrderButtonGroup.Title = "Modularity";
      app.Phase1ampuAEditFieldLabel.Text = "Magnitude 1 (mA)";
      app.Phase1ampuAEditField.ValueDisplayFormat = "%11.4g mA";
      app.Phase1ampuAEditField.Limits = [-0.2 15];
      app.Phase2ampuAEditFieldLabel.Text = "Magnitude 2 (mA)";
      app.Phase2ampuAEditField.ValueDisplayFormat = "%11.4g mA";
      app.Phase2ampuAEditField.Limits = [-0.2 15];
      app.Phase1durationusEditFieldLabel.Text = "Phase duration 1 (ms)";
      app.Phase1durationusEditField.ValueDisplayFormat = "%11.4g ms";
      app.Phase1durationusEditField.Limits = [0.1 1.0];
      app.Phase2durationusEditFieldLabel.Text = "Phase duration 2 (ms)";
      app.Phase2durationusEditField.ValueDisplayFormat = "%11.4g ms";
      app.Phase2durationusEditField.Limits = [0.1 1.0];
      app.InterphasedurationusEditFieldLabel.Text = "Interphase duration (ms)";
      app.InterphasedurationusEditField.ValueDisplayFormat = "%11.4g ms";
      app.InterphasedurationusEditField.Limits = [0 inf]; %these are unknown
      app.InterphasedurationusEditField.Value = 0.1;
      app.FrequencyHzEditField.Limits = [2 100];
      app.FrequencyHzEditField.Value = 20;
      app.TrainDurationsEditFieldLabel.Text = "Number of Trains";
      app.TrainDurationsEditField.ValueDisplayFormat = "%11.4g";
      app.ChannelAxes.Title.String = "";
      app.phaseModeChanged() %change visibility based on moade selected
      %table
      app.TableConfigurations.RowName = ["Modularity", "Polarity", "Phase duration 1 (ms)", ...
                                          "Interphase duration (ms)", "Magnitude 1 (mA)", "Frequency (Hz)", ...
                                          "Number Of Pulses", "Train Frequency (Hz)", "Number Of Trains",...
                                          "Phase Duration 2 (ms)", "Magnitude 2 (mA)"];
      app.TableTriggers.RowName = ["Trigger Expression", "Config ID", "Abort Expression"];
  
      %figure
      xlabel(app.MultPulsesAxes, "Time (ms)", 'Color', 'black')
      ylabel(app.MultPulsesAxes, "Current (mA)")
      xlabel(app.SinglePulseAxes, "Time (ms)", 'Color', 'black')
      ylabel(app.SinglePulseAxes, "Current (mA)")
    end
    function shift(self, shiftDown)
      doubleShift = [1 3 4 5 6 7 8 10 11];
      els = [16 17 20:22];
%       doubleShift = [app.Phase1ampuAEditField, app.Phase1ampuAEditFieldLabel, app.NumberofpulsesSpinner, app.NumberofpulsesSpinnerLabel];
%       els = [app.InterphasedurationusEditField, app.InterphasedurationusEditFieldLabel, app.Phase1durationusEditField, app.Phase1durationusEditFieldLabel,...
%                     app.FrequencyHzEditField, app.FrequencyHzEditFieldLabel, ...
%                     app.TrainDurationsEditField, app.TrainDurationsEditFieldLabel,...
%                     app.TrainFrequencyHzEditField, app.TrainFrequencyHzEditFieldLabel];
      self.shiftElements(els, shiftDown*1);
      self.shiftElements(doubleShift, shiftDown*2);
    end
    function config = buildConfig(~, app)
      asym = app.Asymmetric.Value;
      
      p1Amp = app.Phase1ampuAEditField.Value;
      freq = app.FrequencyHzEditField.Value;
      p2Del = app.InterphasedurationusEditField.Value;
      trainDur = app.TrainDurationsEditField.Value;
      trainFreq = app.TrainFrequencyHzEditField.Value;
      pulses = app.NumberofpulsesSpinner.Value;
      p1Dur = app.Phase1durationusEditField.Value;
      polarity = app.AlternatingButton.Value;
      %modul = app.CathodeFirstButton.Value; %cathode = biphasic, anode = monophasic
      modul = app.Symmetric.Value || app.Asymmetric.Value;
      if asym
        %asymmetric
        p2Dur = app.Phase2durationusEditField.Value;
        p2Amp = app.Phase2ampuAEditField.Value;
      elseif modul
        %biphasic, symmetrical
        p2Dur = p1Dur;
        p2Amp = p1Amp;
      else
        %monophasic
        p2Dur = 0;
        p2Amp = 0;
      end
      config = [modul; polarity; p1Dur; p2Del; p1Amp; freq; pulses; trainFreq; trainDur; p2Dur; p2Amp];
    end
    function trig = buildTrig(self, app)
      expres = self.createExpression(app, true);
      abortExp = self.createExpression(app, false);
      trig = [expres; -1; abortExp];
    end
    function setConfig(self, config)
      self.polarity = config(2);
      self.dur1 = config(3);
      self.del2 = config(4);
      self.amp1 = config(5);
      self.numPulses = config(7);
      self.freqTrain = config(8);
      self.durTrain = config(9);

      self.dur2 = config(10);
      self.amp2 = -1 * config(11);
      self.del1 = 0;

      freq = config(6);
      self.durBurst = (self.numPulses-1)/freq + (self.del1+self.dur1+self.del2+self.dur2)*1e-3; %s
      self.durTrain = 1/self.freqTrain * self.durTrain; %+ durBurst; %translate from num of trains to actual duration
      self.freqBurst = freq/1e3; %ms units
    end
    
    function res = checkTrigTable(self, app, trigTable)
      res = true;
      expres = self.createExpression(app, true);
      %check to make sure channels aren't being triggered at same time
      for i=1:size(trigTable,2)
        if strcmp(expres, trigTable(1,i))
          %can't have same expression
          errordlg(['Choose a different trigger expression. g.Estim cannot excute multiple configurations' ...
              ' at the same time.'],'Trigger expression Error');
          res = false;
          return
        end
      end
    end
    function [addChannelToList, sameExp, trigIndex] = handleSameConfig(~, app, configCols)
      abortExp = self.createExpression(app, false);
      %need to see if abort exp is the same
      for i=1:length(configCols)
          col = configCols(i);
          if strcmp(abortExp, app.TableTriggers.Data(3,col))
              %abort exp is the same
              %don't want to add anything
              errordlg('Same exact configuration is already in tables, will not add a duplicate.', 'Configuration Error');
              %errorSameConfiguration(app);
              return
          end
      end
      sameExp = false;
      addChannelToList = false;
      trigIndex = -1; %won't be used
    end


    function param = formatParameterFile(~, app)
      configs = app.TableConfigurations.Data;
      numConfigs = size(configs,2);
      triggers = app.TableTriggers.Data;
      numTriggers = size(triggers,2);
      if numConfigs == 0
          errordlg('Please add data to the tables first, then save your parameter file. ', 'Parameter tables are empty');
          return
      end
      if numConfigs ~= 1 || numTriggers ~= 1
          errordlg(['BCI2000 can only load one configuration for g.Estim. ' ...
              'Please make your configuration and trigger tables one column before exporting. '], 'One configuration constraint');
          return
      end

      %add expressions
      configs = [triggers(1); triggers(3); configs];
      %format units for parameters
      configs(5) = configs(5) + " ms";
      configs(6) = configs(6) + " ms";
      configs(8) = configs(8) + " Hz";
      configs(10) = configs(10) + " Hz";
      %format of gEstim is different, separate each parameter
      fields = ["StimulationExpression", "AbortExpression", "Modularity", ...
                  "Polarity", "PhaseDuration", "InterphaseDuration", ...
                  "Magnitude", "PulseFrequency", "NumberOfPulses", ...
                  "FrequencyOfTrains", "NumberOfTrains"];
      numParams = length(fields);
      for i=1:numParams
          if i < 3
              param.(fields(i)).Type = 'string';
          else
              param.(fields(i)).Type = 'float';
          end
          param.(fields(i)).DefaultValue = '';
          param.(fields(i)).LowRange = '';
          param.(fields(i)).HighRange = '';
          param.(fields(i)).Value = cellstr(configs(i));
          param.(fields(i)).Section = 'gEstim:Stimulation';
          param.(fields(i)).Comment = 'gEstim Stim Config';
      end
    end
    function [updateTables, cTable, tTable] = loadParameterFile(self, lineArr, cTable, tTable, updatedTables)
      if contains(lineArr(1), "gEstim")
        parameterName = lineArr(3);
        value = convertCharsToStrings(lineArr{4});
        if strcmp(value, "%") %empty parameter
            value = "";
        end
        if strcmp(parameterName,"StimulationExpression=")
            tTable(1,1) = value;
        elseif strcmp(parameterName,"AbortExpression=")
            tTable(3,1) = value;
        else
            for i=1:length(self.configNames)
                if strcmp(parameterName, self.configNames(i))
                    cTable(i,1) = self.formatImportedParameter(value);
                    break
                end
            end
            tTable(2,1) = '1'; %set multiple times, shouldn't matter
        end
        updateTables = true;
      end
    end
    function initializeChannels(~, ~)
    end
    function colorChannels(~, ~)
    end
    function channelClickedCallback(~, ~, ~)
    end
    function [chs, cIDs, retChs] = getActiveChannels(~, ~)
      chs = string.empty;
      cIDs = chs;
      retChs = chs;
    end

    %gEstim specific functions
    function results = formatImportedParameter(self, p)
        results = p;
        results = self.removeStr(results, "%20"); %space operator
        [results, chang1] = self.removeStr(results, "mus");
        [results, chang2] = self.removeStr(results, "us");
        if any(isstrprop(results, 'alpha')) %any letters
            resChars = convertStringsToChars(results);
            results = convertCharsToStrings(resChars(~isstrprop(results, 'alpha'))); %take only numbers
        end
        if chang1 || chang2
            results = string(str2double(results)/1000); %convert to ms
        end
    end
    function [fullStr, changed] = removeStr(~, fullStr, subStr)
      if contains(fullStr, subStr)
          fullStr = erase(fullStr, subStr);
          changed = true;
      else
          changed = false;
      end
    end
  end
end