% a Template class to help implement a new stimulator
% see other stimulators for examples on each function
classdef Template < AbstractStimulator
  %these properties are defined by every stimulator in setConfig
  %they are used to plot the stimulation waveform
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
  % constant parameters that are always true for this stimulator
  properties (Constant)
    %BCI2000 section name. Of the form <Tab>:<Section> in BCI2000
    %must match with the BCI2000 module
    sectionName = ""
    %BCI2000 config table name. Commonly "StimulationConfigurations"
    configName = ""
    % indices of visible elements, defined as self.elements in
    % AbstractStimulator (line 47). Only show elements which are used
    visibleElements = [];
    %defined as [row #, column #]. Used to define number of possible
    %channels that can be chosen from
    electrodes = [];
    %label of the x-axis of the second graph. Commonly us or ms
    xBurstLabel = "";
  end
  %stimulator-specific functions
  methods
    % 1. Set-up bounds, labels, and units for the visible elements
    % 2. Define row labels for tables
    % 3. Define labels for plots
    function initialize(~, app)
      %1 (example)
      %app.Phase1ampuAEditFieldLabel.Text = "Pulse amplitude (uA)";
      %app.Phase1ampuAEditField.ValueDisplayFormat = "%11.4g uA";
      %app.Phase1ampuAEditField.Limits = [0 6120];

      %2
      %app.TableConfigurations.RowName = []
      %app.TableTriggers.RowName = []

      %3
      %xlabel(app.MultPulsesAxes, "Time (us)", 'Color', 'black')
      %ylabel(app.MultPulsesAxes, "Current (uA)")
      %xlabel(app.SinglePulseAxes, "Time (us)", 'Color', 'black')
      %ylabel(app.SinglePulseAxes, "Current (uA)")

    end
    % use the function self.shiftElements() to define which elements to
    % shift to make it visually appealing
    % shiftDown = normal space to shift an element by
    function shift(self, shiftDown)
    end
    % create config list, using same syntax as defined by app.TableConfigurations.RowName
    function config = buildConfig(~, app)
      config =  [];
    end
    % create trig list, using same syntax as app.TableTriggers.RowName
    function trig = buildTrig(self, app)
      trig = [];
    end
    function setConfig(self, config)
      %self.numPulses = 
      %self.amp1 = 
      %self.amp2 = 
      %self.dur1 = 
      %self.dur2 = 
      %self.del2 = 
      %self.durTrain = 
      %self.freqTrain = 
      %self.del1 = 
      %self.durBurst = 
      %self.freqBurst = 
    end
    %defines how to select channel
    function colorChannels(self, app)
      %example, default behavior
      circI = str2double(app.ChannelDropDown.Value);
      if ~isnan(circI)
        app.ChannelAxes.Children(circI).FaceColor = self.testColor;
      end
    end
    %defines how to sanity check trigger table. The desired behavior
    %heavily depends on the device, and how many expressions are used, or
    %how many channels, or any other desired behavior
    % set res to true if it passes or false if it fails
    function res = checkTrigTable(self, app, trigTable)
      res = true;
    end
    % defines how to handle the trigger table when the configuration is the same
    % 
    function [addChannelToList, sameExp, trigIndex] = handleSameConfig(~, ~, configCols)
      sameExp = true;
      addChannelToList = true;
      %trigIndex = configCols(1);
    end
  end
end