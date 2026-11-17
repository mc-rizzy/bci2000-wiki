classdef SCIT_sourceCode < matlab.apps.AppBase

  % Properties that correspond to app components
  properties (Access = public)
    UIFigure                        matlab.ui.Figure
    FileMenu                        matlab.ui.container.Menu
    LoadParameterFileMenu           matlab.ui.container.Menu
    PreferencesMenu                 matlab.ui.container.Menu
    AutoincrementTriggerExpressionMenu  matlab.ui.container.Menu
    IncreaseFontSizeMenu            matlab.ui.container.Menu
    DecreaseFontSizeMenu            matlab.ui.container.Menu
    GridLayout                      matlab.ui.container.GridLayout
    LeftPanel                       matlab.ui.container.Panel
    ModeButtonGroup                 matlab.ui.container.ButtonGroup
    Asymmetric                      matlab.ui.control.RadioButton
    Monophasic                      matlab.ui.control.RadioButton
    Symmetric                       matlab.ui.control.RadioButton
    StimulationConfigurationLabel   matlab.ui.control.Label
    NeurostimulatorDropDown         matlab.ui.control.DropDown
    OrderButtonGroup                matlab.ui.container.ButtonGroup
    AnodeFirstButton                matlab.ui.control.RadioButton
    CathodeFirstButton              matlab.ui.control.RadioButton
    RecordingChannelsButtonGroup    matlab.ui.container.ButtonGroup
    LFPButton                       matlab.ui.control.RadioButton
    ECOGLFButton                    matlab.ui.control.RadioButton
    SEGButton                       matlab.ui.control.RadioButton
    SPKButton                       matlab.ui.control.RadioButton
    ECOGHFButton                    matlab.ui.control.RadioButton
    RAWButton                       matlab.ui.control.RadioButton
    NumberofpulsesSpinner           matlab.ui.control.Spinner
    PolarityButtonGroup             matlab.ui.container.ButtonGroup
    SteadyButton                    matlab.ui.control.RadioButton
    AlternatingButton               matlab.ui.control.RadioButton
    Phase1delaymsEditField          matlab.ui.control.NumericEditField
    Phase1ampuAEditField            matlab.ui.control.NumericEditField
    Phase2ampuAEditField            matlab.ui.control.NumericEditField
    Phase1durationusEditField       matlab.ui.control.NumericEditField
    InterphasedurationusEditField   matlab.ui.control.NumericEditField
    Phase2durationusEditField       matlab.ui.control.NumericEditField
    NumberofpulsesSpinnerLabel      matlab.ui.control.Label
    FrequencyHzEditField            matlab.ui.control.NumericEditField
    TrainDurationsEditField         matlab.ui.control.NumericEditField
    StimulationDurationsEditField   matlab.ui.control.NumericEditField
    TrainFrequencyHzEditField       matlab.ui.control.NumericEditField
    TriggerExpressionEditField      matlab.ui.control.EditField
    TriggerDropDown                 matlab.ui.control.DropDown
    TriggerSpinner                  matlab.ui.control.Spinner
    AbortExpressionLabel            matlab.ui.control.Label
    AbortExpressionEditField        matlab.ui.control.EditField
    AbortDropDown                   matlab.ui.control.DropDown
    AbortSpinner                    matlab.ui.control.Spinner
    TriggerExpressionLabel          matlab.ui.control.Label
    ReturnChannelDropDown           matlab.ui.control.DropDown
    ReturnChannelDropDownLabel      matlab.ui.control.Label
    StimulationDurationsEditFieldLabel  matlab.ui.control.Label
    Phase1delaymsEditFieldLabel     matlab.ui.control.Label
    InterphasedurationusEditFieldLabel  matlab.ui.control.Label
    NeurostimulatorDropDownLabel    matlab.ui.control.Label
    ChannelDropDown                 matlab.ui.control.DropDown
    ChannelDropDownLabel            matlab.ui.control.Label
    TrainFrequencyHzEditFieldLabel  matlab.ui.control.Label
    TrainDurationsEditFieldLabel    matlab.ui.control.Label
    FrequencyHzEditFieldLabel       matlab.ui.control.Label
    Phase2durationusEditFieldLabel  matlab.ui.control.Label
    Phase1durationusEditFieldLabel  matlab.ui.control.Label
    Phase2ampuAEditFieldLabel       matlab.ui.control.Label
    Phase1ampuAEditFieldLabel       matlab.ui.control.Label
    SaveConfigurationtoTablesButton  matlab.ui.control.Button
    CenterPanel                     matlab.ui.container.Panel
    ExperimentConditionsLabel       matlab.ui.control.Label
    ButtonGroup                     matlab.ui.container.ButtonGroup
    BUILDCONFIGButton               matlab.ui.control.ToggleButton
    TrainAxes                       matlab.ui.control.UIAxes
    ChannelAxes                     matlab.ui.control.UIAxes
    MultPulsesAxes                  matlab.ui.control.UIAxes
    SinglePulseAxes                 matlab.ui.control.UIAxes
    RightPanel                      matlab.ui.container.Panel
    ExporttoparameterfileButton     matlab.ui.control.Button
    PlayStimulationButton           matlab.ui.control.Button
    StimulationOnLamp               matlab.ui.control.Lamp
    StimulationOnLampLabel          matlab.ui.control.Label
    DeleteTriggerButton             matlab.ui.control.Button
    DeleteConfigurationButton       matlab.ui.control.Button
    DeleteTriggerDropDown           matlab.ui.control.DropDown
    DeleteChannelDropDown           matlab.ui.control.DropDown
    StimulationConfigurationsLabel  matlab.ui.control.Label
    StimulationTriggersLabel        matlab.ui.control.Label
    TableTriggers                   matlab.ui.control.Table
    TableConfigurations             matlab.ui.control.Table
    ColorContextMenu                matlab.ui.container.ContextMenu
    ChangeColorsMenu                matlab.ui.container.Menu
    BackgroundMenu                  matlab.ui.container.Menu
    Plot1Menu                       matlab.ui.container.Menu
    Plot2Menu                       matlab.ui.container.Menu
    Plot3Menu                       matlab.ui.container.Menu
    RestoreDefaultColorsMenu        matlab.ui.container.Menu
  end

  % Properties that correspond to apps with auto-reflow
  properties (Access = private)
    onePanelWidth = 576;
    twoPanelWidth = 768;
  end


    properties (Access = private)
        %define list of stimulation device names
        % stimDeviceName = ["Cerestim", "NeuroOmega", "g.Estim"];
        cereStim = "CereStim";
        neuroOmega = "NeuroOmega";
        gEstim = "g.Estim";
        cortec = "Cortec BIC";
        %mapping for channel names to numbers
        %if you change the order of the buttons, you must change this as well
        chMapping = {"LFP",     10000:10009;
            "ECOG LF", 10016:10047;
            "SEG",     10128:10132;
            "SPK",     10256:10260;
            "ECOG HF", 10272:10303;
            "RAW",     10384:10393};
        %for easy access to see if we should auto-update
        showTestConfig = false;
        lastStimName %to backtrack in case we don't want to change
        lastStimObj = false;
        dy; % initialize spacing between elements
        defaultBackgroundColor;
        defaultColors; % Default colors to restore
        testColor = 0.2*ones(1,3); % Description
        Stim = false; % stimulator, implemented in another file
    end

    properties (Access = public)
        firstButtonText % Description
        sourceChChanged = true; % changed on channel callback, source vs return
    end

    methods (Access = private)

        function deleteConfig(app, index)
            labels = app.TableConfigurations.ColumnName;
            data = app.TableConfigurations.Data;
            opts = app.DeleteChannelDropDown.Items;
            %allNodes = app.ButtonGroup.Children;
            %account for "test values" node, which should be 1
            %reverses order from channel drop down list
            %delete(allNodes(length(allNodes) - index));
            %newNodes = app.ButtonGroup.Children;
            updateGraphs(app)
            labels(index) = [];
            data(:,index) = [];
            opts(index) = [];
            if isempty(labels) || ischar(labels)
                labels = [];
                data = [];
                opts = {};
                app.TriggerSpinner.Value = 1;
            else
                for i2=index:size(labels,1)
                    %shift to the left by 1
                    newName = "Configuration"+i2;
                    labels(i2) = {newName};
                    opts(i2) = cellstr(newName);
                    %if i2 ~= index
                    %newNodes(i2+1).Text = newName;
                    %end
                end
            end
            app.TableConfigurations.ColumnName = labels;
            app.TableConfigurations.Data = data;
            app.DeleteChannelDropDown.Items = opts;
            %column width
            w = app.TableConfigurations.ColumnWidth;
            if iscell(w)
                w(1) = [];
                if isempty(w)
                    w = 'auto';
                elseif w{1} == 'auto'
                    w = 'auto';
                end
            else
                w = 'auto';
            end
            if isempty(app.TableConfigurations.Data)
                w = 'auto';
            end
            app.TableConfigurations.ColumnWidth = w;
        end

        function deleteTrigger(app, i, configShift)
            labels = app.TableTriggers.ColumnName;
            data = app.TableTriggers.Data;
            opts = app.DeleteTriggerDropDown.Items;

            %delete from condition list if no other equiv expressions
            allNodes = app.ButtonGroup.Buttons;
            sameExp = false;
            for j=1:size(data,2)
                if i == j
                    continue
                end
                if strcmp(data(1,i), data(1,j))
                    sameExp = true;
                    break
                end
            end
            if ~sameExp
                for j=1:length(allNodes)
                    if strcmp(data(1,i),allNodes(j).Text)
                        delete(allNodes(j));
                        positionButtons(app);
                        break
                    end
                end
            end

            %account for "test values" node, which should be 1
            %reverses order from channel drop down list


            labels(i) = [];
            data(:,i) = [];
            opts(i) = [];
            if isempty(labels) || ischar(labels)
                labels = [];
                data = [];
                opts = {};
                app.TriggerSpinner.Value = 1;
            else
                for i2=i:size(labels,1)
                    %shift to the left by 1
                    newName = "Trigger"+i2;
                    labels(i2) = {newName};
                    opts(i2) = cellstr(newName);
                    if configShift
                        oldI = str2double(data(2,i2));
                        if oldI > i
                            data(2,i2) = {string(oldI-1)};
                        end
                    end
                end
            end
            app.TableTriggers.ColumnName = labels;
            app.TableTriggers.Data = data;
            app.DeleteTriggerDropDown.Items = opts;
            %width
            w = app.TableTriggers.ColumnWidth;
            if iscell(w)
                w(1) = [];
                if isempty(w)
                    w = 'auto';
                elseif w{1} == 'auto'
                    w = 'auto';
                end
            else
                w = 'auto';
            end
            if isempty(app.TableTriggers.Data)
                w = 'auto';
            end
            app.TableTriggers.ColumnWidth = w;

            updateGraphs(app);
        end

        function createConfig(app, newData)
            %add visualize row
            oldData = app.TableConfigurations.Data;
            oldColumnNames = app.TableConfigurations.ColumnName;
            newConfigName = string.empty;
            app.TableConfigurations.Data = [oldData newData];
            for i=1:size(newData,2)
                newConfigName(end+1) = "Configuration"+(int8(size(oldColumnNames,1))+i);
            end
            if ~isempty(oldColumnNames)
                app.TableConfigurations.ColumnName = [oldColumnNames; newConfigName'];
            else
                app.TableConfigurations.ColumnName = newConfigName;
            end
            %add new random color if we have used all available colors
            while size(app.SinglePulseAxes.ColorOrder,1) < size(app.TableConfigurations.ColumnName,1)
                c = rand(1,3);
                updateColors(app, c)
            end

            %set delete button
            oldOpts = app.DeleteChannelDropDown.Items;
            app.DeleteChannelDropDown.Items = [oldOpts, newConfigName];

            %update width
            for i=1:size(newData,2)
                addWidth(app, app.TableConfigurations, 40);
            end
        end

        function createTrigger(app, newData)
            oldData = app.TableTriggers.Data;
            %column name
            oldColumnNames = app.TableTriggers.ColumnName;
            newConfigName = string.empty;
            app.TableTriggers.Data = [oldData newData];
            for i=1:size(newData,2)
                newConfigName(end+1) = "Trigger"+(int8(size(oldColumnNames,1))+i);
            end
            if ~isempty(oldColumnNames)
                app.TableTriggers.ColumnName = [oldColumnNames; newConfigName'];
            else
                app.TableTriggers.ColumnName = newConfigName;
            end

            %set delete button
            oldOpts = app.DeleteTriggerDropDown.Items;
            app.DeleteTriggerDropDown.Items = [oldOpts, newConfigName];

            %add to experiment conditions list
            for i=1:size(newData,2)
                %make sure it isn't in there already
                newTrigger = true;
                for j=1:size(oldData,2)
                    if strcmp(newData(1,i), oldData(1,j))
                        newTrigger = false;
                    end
                end
                %check if the new data has already added the trig
                for j=1:i-1
                    if strcmp(newData(1,i), newData(1,j))
                        newTrigger = false;
                    end
                end
                %add new button
                if newTrigger
                    uitogglebutton(app.ButtonGroup, 'Text', newData(1,i));
                end
            end
            %switch selected button to visualize new trigger
            %just do first column if there are multiple new triggers
            selectThisButton(app,newData(1,1));

            %button positioning
            positionButtons(app);

            %update width
            for i=1:size(newData,2)
                addWidth(app, app.TableTriggers, 40);
            end

            %perhaps increment
            if app.AutoincrementTriggerExpressionMenu.Checked
                app.TriggerSpinner.Value = app.TriggerSpinner.Value + 1;
            end
        end

        function addWidth(app, tableObj, LIMIT)
            %change width of each entry
            newLength = length(tableObj.ColumnName);
            w = tableObj.ColumnWidth;
            if ~iscell(w)
                if newLength ~= 1
                    w = {'auto', 'auto'};
                end
            else
                for i=1:length(w)
                    newW = w(i);
                    iWidth = newW{1};
                    if iWidth ~= 'auto'
                        if iWidth > LIMIT
                            newWidth = iWidth - newLength*iWidth/8;
                            if (newWidth > LIMIT)
                                w(i) = {newWidth};
                            else
                                w(i) = {LIMIT};
                            end
                        end
                    else
                        sWidth= 110 - newLength*10;
                        if sWidth > LIMIT
                            w(i) = {sWidth};
                        else
                            w(i) = {LIMIT};
                        end
                    end
                end
                while length(w) < newLength
                    w{end+1} = 'auto';
                end
            end
            tableObj.ColumnWidth = w;
        end

        function [rNames, cNames, lineArr] = getLabels(app, lineArr)
            lineArr = lineArr(4:end);
            [rNames, lineArr] = getLabel(app, lineArr);
            [cNames, lineArr] = getLabel(app, lineArr);
        end

        function [names, lineArr] = getLabel(app, lineArr)
            %assume beginning is start of label
            n=lineArr{1};
            lineArr = lineArr(2:end);
            names= string.empty;
            if strcmp(n, '{')
                %row names
                while ~strcmp(lineArr(1), '}')
                    oldString = string(lineArr{1});
                    names(end+1) = replace(oldString,"%20"," ");
                    lineArr = lineArr(2:end);
                end
                lineArr = lineArr(2:end);
            else
                %number of rows
                names = 1:str2double(n);
            end
        end

        function ret = clearAllData(app)
            %clear data from past stimulator
            app.TableConfigurations.Data = [];
            app.TableConfigurations.ColumnName = [];
            %app.TableConfigurations.RowName = [];
            app.TableConfigurations.ColumnWidth = 'auto';
            app.DeleteChannelDropDown.Items = {};
            app.TableTriggers.Data = [];
            app.TableTriggers.ColumnName = [];
            %app.TableTriggers.RowName = [];
            app.TableTriggers.ColumnWidth = 'auto';
            app.DeleteTriggerDropDown.Items = {};
            app.TriggerSpinner.Value = 1;
            %cla(app.ChannelAxes)
            %assume testing values is the first, which we keep
            delete(app.ButtonGroup.Children(1:end-1));
            ret = true;
        end

        function initializeStimulator(app)
            if ~clearAllData(app)
                return
            end
            if app.lastStimObj ~= false && ~isempty(app.lastStimObj.dy)
                app.lastStimObj.shift(-1);
            end
            %everything is reset, now change for stimulator
            value = app.NeurostimulatorDropDown.Value;
            if strcmp(value, app.cereStim)
                app.Stim = CereStim;
            elseif strcmp(value, app.cortec)
                app.Stim = Cortec;
            elseif strcmp(value,app.gEstim)
                app.Stim = GEstim;
            elseif strcmp(value, app.neuroOmega)
                app.Stim = NeuroOmega;
            else
                errordlg('This stimulator is not fully implemented. Please choose another stimulator','Stimulator Error');
                app.Stim = false;
            end
            app.Stim.setUp(app);
            app.lastStimObj = app.Stim;

            %updateGraphs
            updateGraphs(app)
            app.lastStimName = app.NeurostimulatorDropDown.Value;
        end

        function updateGraphs(app)
            nodeName = app.ButtonGroup.SelectedObject.Text;
            app.showTestConfig = false;
            if strcmp(nodeName, app.firstButtonText)
                app.showTestConfig = true;
            end
            count = 0;
            pulsesLegend = string.empty;
            trainPulses = string.empty;
            myLegend = string.empty;
            %use cell cause they will be different lengths
            d1 = {};
            d2 = {};
            d3 = {};
            lim = [];
            allLim = ones(2,2,3);
            allLim(:,1,:) = allLim(:,1,:)*Inf;
            allLim(:,2,:) = allLim(:,2,:)*-Inf;
            if app.showTestConfig
                count = 1;
                cIDs = 1;
                colors = app.testColor;

                [d1{count}, lim(:,:,1), d2{count}, lim(:,:,2), d3{count}, lim(:,:,3), numPulses, trainPulses(count)] = createGraphs(app, app.Stim.buildConfig(app));
                myLegend(end+1) = app.firstButtonText;
                pulsesLegend(end+1) = numPulses;
                allLim = [min(lim(:,1,:), allLim(:,1,:)), ...
                    max(lim(:,2,:), allLim(:,2,:))];
            else
                %configNames = app.TableConfigurations.ColumnName;
                triggerNames = app.TableTriggers.Data(1,:);
                configNames = app.TableTriggers.Data(1,:);
                %                 if ~iscell(triggerNames)
                %                     triggerNames = {triggerNames};
                %                 end
                colors = app.SinglePulseAxes.ColorOrder;
                cData = app.TableConfigurations.Data;
                cIDs = [];
                for i=1:length(configNames)
                    if strcmp(nodeName, triggerNames(i))
                        configID = str2double(app.TableTriggers.Data(2,i));
                        count = count + 1;
                        cIDs(end+1) = configID;
                        [d1{count}, lim(:,:,1), d2{count}, lim(:,:,2), d3{count}, lim(:,:,3), numPulses, trainPulses(count)] = createGraphs(app, cData(:,configID));
                        myLegend(end+1) = "Config " + app.TableTriggers.Data(2,i);
                        pulsesLegend(end+1) = numPulses;
                        allLim = [min(lim(:,1,:), allLim(:,1,:)), ...
                            max(lim(:,2,:), allLim(:,2,:))];
                    end
                end
            end
            %             if count == 0
            %                 cla(app.SinglePulseAxes)
            %                 cla(app.MultPulsesAxes)
            %                 cla(app.TrainAxes)
            %                 legend(app.SinglePulseAxes, 'hide')
            %                 legend(app.MultPulsesAxes, 'hide')
            %                 legend(app.TrainAxes, 'hide')
            %                 return
            %             end
            trainLegend = string.empty;
            useFig1 = false;
            lineWidth = 2;
            for i=1:count
                plot(app.SinglePulseAxes, d1{i}(:,1), d1{i}(:,2), 'LineWidth',lineWidth, 'Color',colors(cIDs(i),:))
                hold(app.SinglePulseAxes, "on")
                plot(app.MultPulsesAxes, d2{i}(:,1), d2{i}(:,2), 'LineWidth',lineWidth, 'Color',colors(cIDs(i),:))
                hold(app.MultPulsesAxes, "on")
                if ~isempty(d3{i})
                    trainLegend(end+1) = myLegend(i)+ ": "+ trainPulses(i);
                    plot(app.TrainAxes, d3{i}(:,1), d3{i}(:,2), 'LineWidth',lineWidth, 'Color',colors(cIDs(i),:))
                    hold(app.TrainAxes, "on")
                else
                    useFig1 = true;
                end
            end
            %if stim trains have diff number
            if useFig1
                legend(app.SinglePulseAxes, myLegend)
                %use 2 legends cause the colors will be different
                legend(app.TrainAxes, trainLegend)
            else
                legend(app.SinglePulseAxes, 'hide')
                legend(app.TrainAxes, trainLegend)
            end
            %no trains
            if all(allLim(1,:,3) == 0)
                cla(app.TrainAxes)
                allLim(1,:,3) = [0 1];
                legend(app.TrainAxes, 'hide')
            end
            %if no stim amplitude
            if all(allLim(2,:,1) == 0)
                allLim(2,:,1) = [-1 1];
                allLim(2,:,2) = [-1 1];
            end
            legend(app.MultPulsesAxes, pulsesLegend)
            xlim(app.SinglePulseAxes, allLim(1,:,1))
            ylim(app.SinglePulseAxes, allLim(2,:,1))
            xlim(app.MultPulsesAxes, allLim(1,:,2))
            ylim(app.MultPulsesAxes, allLim(2,:,2))
            xlim(app.TrainAxes, allLim(1,:,3))
            ylim(app.TrainAxes, allLim(2,:,3))
            hold(app.SinglePulseAxes, "off")
            hold(app.MultPulsesAxes, "off")
            hold(app.TrainAxes, "off")
        end

        function [d1, lim1, d2, lim2, d3, lim3, numPulses, numTrainPulses] = createGraphs(app, config)
            %for each device define:
            %amp1, dur1, del1
            %amp2, dur2, del2
            %durBurst, freqBurst (in units the other time scales use)
            %durTrain, freqTrain
            %numPulses
            app.Stim.setConfig(config)
            amp1 = app.Stim.amp1;
            amp2 = app.Stim.amp2;
            dur1 = app.Stim.dur1;
            dur2 = app.Stim.dur2;
            del1 = app.Stim.del1;
            del2 = app.Stim.del2;
            durBurst = app.Stim.durBurst;
            freqBurst = app.Stim.freqBurst;
            durTrain = app.Stim.durTrain;
            freqTrain = app.Stim.freqTrain;
            numPulses = app.Stim.numPulses;
            polarity = app.Stim.polarity; %only gEstim would set this to true

            %create piecewise fn
            xEnd = 2*dur1+del1+del2+dur2;
            xStart = 0-dur1;
            dX = (xEnd - xStart)/1000;
            x = xStart:dX:xEnd;
            y(x<=del1) = 0;
            y(del1 < x & x < dur1+del1) = amp1;
            y(del1+dur1 < x & x < del1+dur1+del2) = 0;
            y(del1+dur1+del2 < x & x < del1+dur1+del2+dur2) = amp2;
            y(x > del1+dur1+del2+dur2) = 0;

            %plot 1
            minY = 11/8*min([amp1,amp2,0]); %do 11/8 just to make it look nice with padding on top and bottom
            maxY = 11/8*max([amp1,amp2,0]);
            if (minY == 0)
                minY = -maxY/8; %again, to make it look nice
            end
            if (maxY == 0)
                maxY = -minY/8;
            end
            lim1 = [0-dur1, 2*dur1+del1+del2+dur2;
                minY, maxY];

            %create mult pulse plot
            %set bounds
            if numPulses > 1
                xStart = 0-1/freqBurst/2;
                xEnd1 = 1/freqBurst/2+(numPulses-1)/freqBurst+del1+dur1+del2+dur2;
            else
                xStart = 0-dur1;
                xEnd1 = 2*dur1+del1+del2+dur2;
            end
            timeArr = [del1, dur1, del2, dur2];
            %find smallest dx we need to use
            dX = min(timeArr(timeArr>0))/2;
            %dX = (xEnd1 - xStart)/1000000;
            x1 = xStart:dX:xEnd1;
            y1=zeros(size(x1));
            %loop thru each pulse
            for i=0:numPulses-1
                if polarity % would only be possibly set true with gEstim
                    alternate = 2*mod(i,2) - 1;
                    amp1 = -abs(amp1) * alternate;
                    amp2 = abs(amp2) * alternate;
                    minY = -maxY; %change bounds for plot 2
                end
                lastSt = (i-1)/freqBurst;
                st = i/freqBurst;
                y1(del1+dur1+del2+dur2+lastSt < x1 & x1 <= del1 + st) = 0;
                y1(st+del1 < x1 & x1 <= st+del1+dur1) = amp1;
                y1(del1+dur1+st < x1 & x1 <= del1+dur1+del2+st) = 0;
                y1(del1+dur1+del2+st < x1 & x1 <= del1+dur1+del2+dur2+st) = amp2;
                y1(x1 > del1+dur1+del2+dur2+st) = 0;
            end
            if (del1+dur1+del2+dur2) > 1/freqBurst
                xlabel(app.MultPulsesAxes, ["FREQ IS TOO FAST FOR THE"; "DESIRED STIMULATION DURATION"], ...
                    'FontWeight','bold', 'Color', 'r')
            else
                xlabel(app.MultPulsesAxes, "Time (" + app.Stim.xBurstLabel + ")", 'Color', 'black')
            end

            %plot 2
            lim2 = [xStart, xEnd1;
                minY, maxY];

            %create train plot
            spacing = durTrain/8;
            xEnd2 = durTrain+spacing;
            xStart = 0-spacing;
            dX = (xEnd2 - xStart)/10000;
            x2 = xStart:dX:xEnd2;

            numTrainPulses = freqTrain*durTrain;

            if (~isinteger(numTrainPulses))
                if (durBurst <= durTrain - floor(numTrainPulses)/freqTrain)
                    %enough time for the full train
                    numTrainPulses = ceil(numTrainPulses);
                else
                    %not enough time, no partial trains
                    numTrainPulses = floor(numTrainPulses);
                end
            end
            y2(x2 > 0) = 0;
            for i=0:numTrainPulses-1
                lastSt = 1/freqTrain*(i-1);
                st = 1/freqTrain*i;
                y2(lastSt+durBurst < x2 & x2 <= st) = 0;
                y2(st < x2 & x2 < st + durBurst) = 1;
                y2(x2 > st + durBurst) = 0;
            end
            if (durBurst >= 1/freqTrain)
                xlabel(app.TrainAxes, ["STIM BURSTS ARE TOO LONG"; app.Stim.stimBurstError], ...
                    'FontWeight','bold', 'Color', 'r')
            else
                xlabel(app.TrainAxes, "Time (s)", 'Color', 'black')
            end
            if (durTrain~=0 && freqTrain==0)||(durTrain==0 && freqTrain~=0)
                xlabel(app.TrainAxes, "Train duration and train frequency must both be zero, or both non-zero!", ...
                    'FontWeight','bold', 'Color', 'r')
            end

            %plot 3
            lim3 = [xStart, xEnd2;
                -0.4, 1.4];

            d1 = [x', y'];
            d2 = [x1', y1'];
            d3 = [x2', y2'];
        end


        function positionButtons(app)
            %first resize button group window to match displayed center panel if needed
            %             dispCenterPanelWidth = app.UIFigure.Position(3) - app.LeftPanel.Position(3) - app.RightPanel.Position(3);
            %                 app.ButtonGroup.Position(3) = dispCenterPanelWidth - 1;
            %             if dispCenterPanelWidth < app.ButtonGroup.Position(3)
            %             end
            buttons = app.ButtonGroup.Buttons;
            padding = 2;
            for i=2:length(buttons) %test values is first position
                startingPos = buttons(i-1).Position(1) + buttons(i-1).Position(3);
                %adjust width based on number of characters
                buttonWidth = length(buttons(i).Text)*9; %estimate
                buttons(i).Position(3) = buttonWidth;
                %y coord
                if app.ButtonGroup.Position(3) < startingPos + buttonWidth*9/8 %2x spacing in case we are making window smaller
                    %move to next row
                    startingPos = buttons(1).Position(1) + buttons(1).Position(3);
                    buttons(i).Position(2) = buttons(i-1).Position(2) - buttons(i).Position(4);
                else
                    %keep on same row
                    buttons(i).Position(2) = buttons(i-1).Position(2)+buttons(i-1).Position(4)-buttons(i).Position(4);
                end
                buttons(i).Position(1) = startingPos + padding; %adjust left location
            end
        end

        function selectThisButton(app, triggerExp)
            buts = app.ButtonGroup.Buttons;
            %find where expression matches button
            for j=1:length(buts)
                if strcmp(buts(j).Text, triggerExp)
                    buts(j).Value = true;
                    break %only should be true once
                end
            end
            updateChannelVisualization(app);
            updateGraphs(app);
        end

        function changeColor(app, index)
            plots = [app.SinglePulseAxes, app.MultPulsesAxes, app.TrainAxes];
            if index == 0
                %background color
                c = uisetcolor(plots(1).Color, "Background color");
                for i=1:length(plots)
                    plots(i).Color = c;
                end
            else
                c = uisetcolor(plots(1).ColorOrder(index,:), sprintf("Plot %d color",index));
                for i=1:length(plots)
                    plots(i).ColorOrder(index,:) = c;
                end
            end
        end

        %resets all circles
        function updateChannelVisualization(app)
            [chs, cIDs, retChs] = app.Stim.getActiveChannels(app);
            if isempty(chs) || isempty(cIDs) || isempty(retChs)
                return
            else
                %visualize channels
                %uses chs, cIDs, retChs
                i = 1;
                [oChs, I] = sort(str2double(chs));
                oCfgs = str2double(cIDs(I));
                oRetChs = str2double(retChs(I));
                ordChs = [oChs, oCfgs, oRetChs];
                colors = app.SinglePulseAxes.ColorOrder;
                for ch = 1:length(app.ChannelAxes.Children)
                    chColor = app.ChannelAxes.Children(ch).FaceColor;
                    chI = app.ChannelAxes.Children(ch).Channel;
                    if app.ChannelAxes.Children(ch).Curvature ~= [1 1]
                        app.ChannelAxes.Children(ch).Curvature = [1 1];
                    end
                    if i <= size(ordChs,1) && chI == ordChs(i,1)
                        if ~isequal(chColor, colors(ordChs(i,2),:))
                            chColor = colors(ordChs(i,2),:);
                        end
                        %change shape if return channel
                        if ordChs(i,3)
                            app.ChannelAxes.Children(ch).Curvature = [0 0];
                        end
                        %move counter to next channel
                        i = i + 1;
                    elseif ~strcmp(chColor,app.CenterPanel.BackgroundColor)
                        chColor = app.CenterPanel.BackgroundColor;
                    end
                    app.ChannelAxes.Children(ch).FaceColor = chColor;
                end
            end
        end

        function circI = getCircleIndexFromChannel(app, ch)
            for c = app.ChannelAxes.Children
                if c.Channel == ch
                    circI = ch;
                    return
                end
            end
        end

        function updateColors(app, color)
            plots = [app.SinglePulseAxes, app.MultPulsesAxes, app.TrainAxes];
            for i=1:length(plots)
                plots(i).ColorOrder = [plots(i).ColorOrder; color];
            end
        end

        function changeFontSize(app, el, diff)
            if isprop(el, "Children")
                %get all children
                elChildren = el.Children;
                for i = 1:length(elChildren)
                    changeFontSize(app, elChildren(i), diff); %recursive swag
                end
            elseif isprop(el, "FontSize")
                el.FontSize = el.FontSize + diff;
            end
        end

    end

    methods (Access = public)

        %should only deal with affected channels to maintain the quick speed
        function channelClickedCallback(app,src,~)
            app.Stim.channelClickedCallback(app, src)
        end


        function newChList = formatChList(~, chList)
            if isempty(chList)
                newChList = "";
                return
            end
            orderedChannels = string(sort(str2double(chList)));
            newChList = orderedChannels(1);
            for i=2:length(orderedChannels)
                if ~ismissing(orderedChannels(i))
                    newChList = newChList + ", " + orderedChannels(i);
                end
            end
        end
    
        function phaseModeChanged(app)
          %for use with gEstim
          if app.Asymmetric.Value
            app.Phase2ampuAEditField.Visible = true;
            app.Phase2durationusEditField.Visible = true;
          else
            app.Phase2ampuAEditField.Visible = false;
            app.Phase2durationusEditField.Visible = false;
          end
        end
    end


  % Callbacks that handle component events
  methods (Access = private)

    % Code that executes after component creation
    function startupFcn(app)
            app.lastStimName = 'none';
            app.dy = abs(app.TrainFrequencyHzEditField.Position(2) - app.TrainDurationsEditField.Position(2));
            app.defaultBackgroundColor = app.SinglePulseAxes.Color;
            app.defaultColors = app.SinglePulseAxes.ColorOrder;

            app.SinglePulseAxes.Toolbar.Visible = 'off';
            disableDefaultInteractivity(app.SinglePulseAxes)
            app.MultPulsesAxes.Toolbar.Visible = 'off';
            disableDefaultInteractivity(app.MultPulsesAxes)
            app.TrainAxes.Toolbar.Visible = 'off';
            disableDefaultInteractivity(app.TrainAxes)
            app.ChannelAxes.Toolbar.Visible = 'off';

            app.firstButtonText = string(app.BUILDCONFIGButton.Text);

            initializeStimulator(app);
            updateGraphs(app)

    end

    % Button pushed function: SaveConfigurationtoTablesButton
    function SaveConfigurationtoTablesButtonPushed(app, event)
            %define vars
            expres = app.Stim.createExpression(app, true);
            ch = app.ChannelDropDown.Value;
            retCh = app.ReturnChannelDropDown.Value;

            %stimulation configuration
            oldConfigData = app.TableConfigurations.Data;
            sizeConfigData = size(oldConfigData);
            %stimulation triggers
            oldTrigData = app.TableTriggers.Data;
            sizeOldTrigData = size(oldTrigData);

            %must define newConfigData and newTrigData for each device
            %current required syntax for each device:
            % Trigger table rows:
            % 1 = trigger expression
            % 2 = configuration ID
            % 3 = source channel (except gEstim currently doesn't have channels)
            newConfigData = app.Stim.buildConfig(app);
            newTrigData = app.Stim.buildTrig(app);
            %check to make sure channels aren't being triggered at same time
            if ~app.Stim.checkTrigTable(app, oldTrigData)
              return
            end

            %check if configuration already exists
            diffConfig = true;
            colIndex = 0;
            for j=1:sizeConfigData(2)
                %go across columns first
                diffConfig = false;
                for i=1:sizeConfigData(1)
                    if (oldConfigData(i,j) ~= newConfigData(i))
                        diffConfig = true;
                        break
                    end
                end
                if (diffConfig == false)
                    colIndex = j;
                    break
                end
            end

            %if config is new, adding trigger is simple
            if diffConfig
                newTrigData(2) = sizeConfigData(2) + 1;
                createConfig(app, newConfigData);
                createTrigger(app, newTrigData);
                return
            end

            %config is same. don't add new config, but might need to add trig
            %store which trigs have the id of the config
            sameConfigCols = [];
            sameExp = false;
            for i=1:sizeOldTrigData(2)
                %expression and config ID are the same
                if strcmp(expres, oldTrigData(1,i)) && colIndex == str2double(oldTrigData(2,i))
                    sameConfigCols(end+1) = i;
                    sameExp = true;
                end
            end

            addChannelToList = false;
            if sameExp
              [addChannelToList, sameExp, trigIndex] = app.Stim.handleSameConfig(app, sameConfigCols);
            end
            
            %same config, same trig
            %add channel to list, don't add new config or trig
            if addChannelToList
                channels = oldTrigData(3,trigIndex);
                chList = split(channels,", ");
                oldTrigData(3,trigIndex) = formatChList(app,[chList; ch]);
                if isprop(app.Stim, "returnChName") && app.Stim.returnChName==""
                  %cortec, or any device with return ch list
                  retChannels = oldTrigData(4,trigIndex);
                  retChList = split(retChannels,", ");
                  oldTrigData(4,trigIndex) = formatChList(app,[retChList; retCh]);
                end
                app.TableTriggers.Data = oldTrigData;

                %update button selection
                selectThisButton(app, oldTrigData(1,trigIndex));
                return
            end

            %same config, new trigger
            if ~sameExp
                %add new trigger, not new config
                newTrigData(2) = colIndex;
                createTrigger(app, newTrigData);
                return
            end

            errordlg('Unexpected software error. Please restart','Software Error');

    end

    % Button pushed function: ExporttoparameterfileButton
    function ExporttoparameterfileButtonPushed(app, event)
            %create parameter file
            [file, path] = uiputfile('*.prm');
            fPath = fullfile(path, file);
            if file ~=0
                settings.parm_filename = fPath;
            else
                return
            end

            %get parameter format for device
            param = app.Stim.formatParameterFile(app);

            %write the param struct to a bci2000 parameter file
            try
                parameter_lines = convert_bciprm( param );
                fid = fopen(settings.parm_filename, 'w');

                for i=1:length(parameter_lines)
                    fprintf( fid, '%s', parameter_lines{i} );
                    fprintf( fid, '\r\n' );
                end
                fclose(fid);
            catch ME
                report = getReport(ME);
                fig = uifigure;
                uialert(fig,report,'Error Message');
            end
    end

    % Button pushed function: DeleteConfigurationButton
    function DeleteConfigurationButtonPushed(app, event)
            badConfig = app.DeleteChannelDropDown.Value;

            labels = app.TableConfigurations.ColumnName;
            for i=1:length(labels)
                if iscell(labels)
                    l = labels(i);
                    newL = l{1};
                else
                    newL = labels;
                end
                if badConfig == newL
                    %delete config
                    deleteConfig(app, i);
                    %might want to delete multiple triggers
                    trigData = app.TableTriggers.Data;
                    deleteThese = [];
                    for j=1:size(trigData,2)
                        if (i == str2double(trigData(2,j)))
                            deleteThese(end+1) = j;
                        end
                    end
                    sortedA = sort(deleteThese, 'descend');
                    for j=1:length(sortedA)
                        if j==length(sortedA) %only shift on last deletion
                            deleteTrigger(app, sortedA(j), true);
                        else
                            deleteTrigger(app, sortedA(j), false);
                        end
                    end
                    break
                end
            end
            updateChannelVisualization(app);
    end

    % Button pushed function: DeleteTriggerButton
    function DeleteTriggerButtonPushed(app, event)
            badConfig = app.DeleteTriggerDropDown.Value;

            labels = app.TableTriggers.ColumnName;
            for i=1:length(labels)
                if iscell(labels)
                    l = labels(i);
                    newL = l{1};
                else
                    newL = labels;
                end
                if strcmp(badConfig, newL)
                    %make sure no other ids are correlated
                    trigData = app.TableTriggers.Data;
                    canDelete = true;
                    for j=1:size(trigData,2)
                        if ((i ~= j) && strcmp(trigData(2,i),trigData(2,j)))
                            canDelete = false;
                        end
                    end
                    if canDelete
                        deleteConfig(app, str2double(trigData(2,i)));
                    end
                    deleteTrigger(app, i, canDelete);
                    break
                end
            end
            updateChannelVisualization(app);
    end

    % Value changed function: NeurostimulatorDropDown
    function NeurostimulatorDropDownValueChanged(app, event)

            if ~isempty(app.TableConfigurations.Data)
                fig = uifigure;
                selection = uiconfirm(fig,'Continuing will delete the current values in the table. Proceed?', ...
                    'Confirm Switch','Icon','warning');
                delete(fig)
                if selection ~= "OK"
                    app.NeurostimulatorDropDown.Value = app.lastStimName;
                    return
                end
            end
            disp("drop down changed")
            initializeStimulator(app);
    end

    % Selection changed function: RecordingChannelsButtonGroup
    function RecordingChannelsButtonGroupSelectionChanged(app, event)
            app.Stim.setNeuroOmegaChannels(app)
            updateChannelVisualization(app);
            selectThisButton(app, app.firstButtonText);
    end

    % Menu selected function: LoadParameterFileMenu
    function LoadParameterFileMenuSelected(app, event)
            [file, path] = uigetfile('*.prm');
            updatedTables = false;
            if file~=0
                fileID = fopen(fullfile(path, file));
                cTable = [];
                tTable = string.empty;
                app.lastStimObj = app.Stim;
                while ~feof(fileID)
                    line = fgetl(fileID);
                    lineArr = strsplit(line);
                    if length(lineArr) < 3
                        continue
                    end
                    %hopefully follows parameter syntax
                    if contains(lineArr(1), "gEstim")
                        app.NeurostimulatorDropDown.Value = app.gEstim;
                        app.Stim = GEstim;
                    elseif contains(lineArr(1), "NeuroOmega")
                        app.NeurostimulatorDropDown.Value = app.neuroOmega;
                        app.Stim = NeuroOmega;
                    elseif contains(lineArr(3), "StimulationPulses=")
                        app.NeurostimulatorDropDown.Value = app.cortec;
                        app.Stim = Cortec;
                    elseif contains(lineArr(1), "CereStim")
                        app.NeurostimulatorDropDown.Value = app.cereStim;
                        app.Stim = CereStim;
                    end
                    [updatedTables, cTable, tTable] = app.Stim.loadParameterFile(lineArr, cTable, tTable, updatedTables);
                end
                fclose(fileID);

                if ~updatedTables
                    errordlg('No StimulationConfigurations or StimulationTriggers were found.','Loading Parameter File Error');
                    return
                end

                %currently only does something for Cortec
                [cTable, tTable] = app.Stim.uniqueParameterProcessing(cTable, tTable);

                %check to see if there are any repeat configurations
                repeatedC = [];
                %only check first half, second half will be repeats
                for c=1:ceil(size(cTable,2)/2)
                    for i=1:size(cTable,2)
                        if i ~= c && all(cTable(:,c) == cTable(:,i))
                            repeatedC(end+1,:) = [c i];
                        end
                    end
                end
                if ~isempty(repeatedC)
                    fig = uifigure;
                    selection = uiconfirm(fig,['Repeated configurations are not supported. ' ...
                        'Would you like to automatically convert the Config IDs?'],'Confirm Load',...
                        'Icon','warning');
                    delete(fig)
                    if selection ~= "OK"
                        return
                    else
                        %convert config ids in trigger table, then delete repeat configs
                        for num=1:size(repeatedC,1)
                            %remove i, keep c
                            for j=1:size(tTable,2)
                                tI = str2double(tTable(2,j));
                                if tI == repeatedC(num,2)
                                    tTable(2,j) = string(repeatedC(num,1));
                                    %gotta move it down cause we delete a config
                                elseif tI > repeatedC(num,2)
                                    tTable(2,j) = string(tI - 1);
                                end
                            end
                        end
                        for num=size(repeatedC,1):-1:1
                            %now delete from end to front
                            cTable(:,repeatedC(num,2)) = [];
                        end
                    end
                end

                %if there are no train specified
                if size(cTable,1) == 8
                    cTable(9:10,:) = zeros(size(cTable(1:2,:)));
                end

                if ~clearAllData(app)
                    return
                end

                %init layout
                initializeStimulator(app);

                %update tables
                if ~isempty(cTable)
                    createConfig(app, cTable);
                end
                if ~isempty(tTable)
                    createTrigger(app, tTable);
                end
            end
    end

    % Value changed function: Phase1ampuAEditField
    function Phase1ampuAEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: Phase2ampuAEditField
    function Phase2ampuAEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: Phase1durationusEditField
    function Phase1durationusEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: Phase2durationusEditField
    function Phase2durationusEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: FrequencyHzEditField
    function FrequencyHzEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: TrainDurationsEditField
    function TrainDurationsEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: TrainFrequencyHzEditField
    function TrainFrequencyHzEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: NumberofpulsesSpinner
    function NumberofpulsesSpinnerValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: InterphasedurationusEditField
    function InterphasedurationusEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: Phase1delaymsEditField
    function Phase1delaymsEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Selection changed function: OrderButtonGroup
    function OrderButtonGroupSelectionChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: StimulationDurationsEditField
    function StimulationDurationsEditFieldValueChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Menu selected function: AutoincrementTriggerExpressionMenu
    function AutoincrementTriggerExpressionMenuSelected(app, event)
            app.AutoincrementTriggerExpressionMenu.Checked = ~app.AutoincrementTriggerExpressionMenu.Checked;
    end

    % Display data changed function: TableConfigurations
    function TableConfigurationsDisplayDataChanged(app, event)
            updateGraphs(app)
    end

    % Selection changed function: PolarityButtonGroup
    function PolarityButtonGroupSelectionChanged(app, event)
            selectThisButton(app, app.firstButtonText);
    end

    % Selection changed function: ButtonGroup
    function ButtonGroupSelectionChanged(app, event)
            updateGraphs(app)
            updateChannelVisualization(app);
    end

    % Size changed function: ButtonGroup
    function ButtonGroupSizeChanged(app, event)
            positionButtons(app);
    end

    % Menu selected function: BackgroundMenu
    function BackgroundMenuSelected(app, event)
            changeColor(app,0)
    end

    % Menu selected function: Plot1Menu
    function Plot1MenuSelected(app, event)
            changeColor(app,1);
    end

    % Menu selected function: Plot2Menu
    function Plot2MenuSelected(app, event)
            changeColor(app,2);
    end

    % Menu selected function: Plot3Menu
    function Plot3MenuSelected(app, event)
            changeColor(app,3);
    end

    % Menu selected function: RestoreDefaultColorsMenu
    function RestoreDefaultColorsMenuSelected(app, event)
            plots = [app.SinglePulseAxes, app.MultPulsesAxes, app.TrainAxes];
            for i=1:length(plots)
                plots(i).Color = app.defaultBackgroundColor;
                plots(i).ColorOrder = app.defaultColors;
            end
    end

    % Button pushed function: PlayStimulationButton
    function PlayStimulationButtonPushed(app, event)
            for i=1:length(app.TrainAxes.Children)
                if strcmp(app.TrainAxes.Children(i).Type,'line')
                    %grab plot
                    p = app.TrainAxes.Children(i);

                    %init marker
                    hold(app.TrainAxes, "on")
                    c = xline(app.TrainAxes, 0, 'LineWidth',3, 'Color', [1 1 1]);
                    hold(app.TrainAxes, "off")

                    %init lamp
                    app.StimulationOnLamp.Color = [1 1 0];
                    app.SinglePulseAxes.Color = [0 0 0];
                    app.MultPulsesAxes.Color = [0 0 0];

                    %get data points of existing train
                    times = zeros(500,1);
                    count = 0;
                    for j = 1:20:length(p.XData)
                        count = count + 1;
                        tic
                        c.Value =  p.XData(j);
                        drawnow

                        %update lamp
                        %                         if toc >= p.XData(j) %time has passed
                        if p.YData(j)==1 && app.StimulationOnLamp.Color(1)
                            app.StimulationOnLamp.Color = [0 1 0];
                            app.SinglePulseAxes.Color = [1 1 1];
                            app.MultPulsesAxes.Color = [1 1 1];
                        elseif p.YData(j)==0 && ~app.StimulationOnLamp.Color(1)
                            app.StimulationOnLamp.Color = [1 1 0];
                            app.SinglePulseAxes.Color = [0 0 0];
                            app.MultPulsesAxes.Color = [0 0 0];
                        end
                        %                         else
                        %                             'skipping'
                        %                         end
                        times(count) = toc;
                    end
                    delete(c);
                    histogram(times);

                    %turn off lamp
                    app.StimulationOnLamp.Color = [1 0 0];
                end
            end
    end

    % Value changed function: ChannelDropDown
    function ChannelDropDownValueChanged(app, event)
            app.sourceChChanged = true;
            selectThisButton(app, app.firstButtonText);
    end

    % Value changed function: ReturnChannelDropDown
    function ReturnChannelDropDownValueChanged(app, event)
            app.sourceChChanged = false;
            selectThisButton(app, app.firstButtonText);
    end

    % Cell selection callback: TableTriggers
    function TableTriggersCellSelection(app, event)
            indices = event.Indices;
            %find the selected column
            if ~isempty(indices)
                c = indices(1,2);
                if all(indices(:,2)==c)
                    exp = app.TableTriggers.Data(1,c);
                    %change if we are not there already
                    if ~strcmp(exp,app.ButtonGroup.SelectedObject.Text)
                        selectThisButton(app, exp);
                    end
                end
            end
    end

    % Menu selected function: IncreaseFontSizeMenu
    function IncreaseFontSizeMenuSelected(app, event)
            changeFontSize(app, app.UIFigure, 1)
    end

    % Menu selected function: DecreaseFontSizeMenu
    function DecreaseFontSizeMenuSelected(app, event)
            changeFontSize(app, app.UIFigure, -1)
    end

    % Selection changed function: ModeButtonGroup
    function ModeButtonGroupSelectionChanged(app, event)
      phaseModeChanged(app);
      selectThisButton(app, app.firstButtonText);
    end

    % Changes arrangement of the app based on UIFigure width
    function updateAppLayout(app, event)
      currentFigureWidth = app.UIFigure.Position(3);
      if(currentFigureWidth <= app.onePanelWidth)
          % Change to a 3x1 grid
          app.GridLayout.RowHeight = {608, 608, 608};
          app.GridLayout.ColumnWidth = {'1x'};
          app.CenterPanel.Layout.Row = 1;
          app.CenterPanel.Layout.Column = 1;
          app.LeftPanel.Layout.Row = 2;
          app.LeftPanel.Layout.Column = 1;
          app.RightPanel.Layout.Row = 3;
          app.RightPanel.Layout.Column = 1;
      elseif (currentFigureWidth > app.onePanelWidth && currentFigureWidth <= app.twoPanelWidth)
          % Change to a 2x2 grid
          app.GridLayout.RowHeight = {608, 608};
          app.GridLayout.ColumnWidth = {'1x', '1x'};
          app.CenterPanel.Layout.Row = 1;
          app.CenterPanel.Layout.Column = [1,2];
          app.LeftPanel.Layout.Row = 2;
          app.LeftPanel.Layout.Column = 1;
          app.RightPanel.Layout.Row = 2;
          app.RightPanel.Layout.Column = 2;
      else
          % Change to a 1x3 grid
          app.GridLayout.RowHeight = {'1x'};
          app.GridLayout.ColumnWidth = {283, '1x', 396};
          app.LeftPanel.Layout.Row = 1;
          app.LeftPanel.Layout.Column = 1;
          app.CenterPanel.Layout.Row = 1;
          app.CenterPanel.Layout.Column = 2;
          app.RightPanel.Layout.Row = 1;
          app.RightPanel.Layout.Column = 3;
      end
    end
  end

  % Component initialization
  methods (Access = private)

    % Create UIFigure and components
    function createComponents(app)

      % Create UIFigure and hide until all components are created
      app.UIFigure = uifigure('Visible', 'off');
      app.UIFigure.AutoResizeChildren = 'off';
      app.UIFigure.Color = [0.0588 0 0.702];
      colormap(app.UIFigure, 'parula');
      app.UIFigure.Position = [100 100 1166 608];
      app.UIFigure.Name = 'MATLAB App';
      app.UIFigure.SizeChangedFcn = createCallbackFcn(app, @updateAppLayout, true);

      % Create FileMenu
      app.FileMenu = uimenu(app.UIFigure);
      app.FileMenu.Text = 'File';

      % Create LoadParameterFileMenu
      app.LoadParameterFileMenu = uimenu(app.FileMenu);
      app.LoadParameterFileMenu.MenuSelectedFcn = createCallbackFcn(app, @LoadParameterFileMenuSelected, true);
      app.LoadParameterFileMenu.Separator = 'on';
      app.LoadParameterFileMenu.Text = 'Load Parameter File';

      % Create PreferencesMenu
      app.PreferencesMenu = uimenu(app.UIFigure);
      app.PreferencesMenu.Text = 'Preferences';

      % Create AutoincrementTriggerExpressionMenu
      app.AutoincrementTriggerExpressionMenu = uimenu(app.PreferencesMenu);
      app.AutoincrementTriggerExpressionMenu.MenuSelectedFcn = createCallbackFcn(app, @AutoincrementTriggerExpressionMenuSelected, true);
      app.AutoincrementTriggerExpressionMenu.Text = 'Auto-increment Trigger Expression?';

      % Create IncreaseFontSizeMenu
      app.IncreaseFontSizeMenu = uimenu(app.PreferencesMenu);
      app.IncreaseFontSizeMenu.MenuSelectedFcn = createCallbackFcn(app, @IncreaseFontSizeMenuSelected, true);
      app.IncreaseFontSizeMenu.Accelerator = '=';
      app.IncreaseFontSizeMenu.Text = 'Increase Font Size';

      % Create DecreaseFontSizeMenu
      app.DecreaseFontSizeMenu = uimenu(app.PreferencesMenu);
      app.DecreaseFontSizeMenu.MenuSelectedFcn = createCallbackFcn(app, @DecreaseFontSizeMenuSelected, true);
      app.DecreaseFontSizeMenu.Accelerator = '-';
      app.DecreaseFontSizeMenu.Text = 'Decrease Font Size';

      % Create GridLayout
      app.GridLayout = uigridlayout(app.UIFigure);
      app.GridLayout.ColumnWidth = {283, '1x', 396};
      app.GridLayout.RowHeight = {'1x'};
      app.GridLayout.ColumnSpacing = 0;
      app.GridLayout.RowSpacing = 0;
      app.GridLayout.Padding = [0 0 0 0];
      app.GridLayout.Scrollable = 'on';

      % Create LeftPanel
      app.LeftPanel = uipanel(app.GridLayout);
      app.LeftPanel.BackgroundColor = [0.949 0.9804 1];
      app.LeftPanel.Layout.Row = 1;
      app.LeftPanel.Layout.Column = 1;
      app.LeftPanel.FontName = 'Montserrat';

      % Create SaveConfigurationtoTablesButton
      app.SaveConfigurationtoTablesButton = uibutton(app.LeftPanel, 'push');
      app.SaveConfigurationtoTablesButton.ButtonPushedFcn = createCallbackFcn(app, @SaveConfigurationtoTablesButtonPushed, true);
      app.SaveConfigurationtoTablesButton.BackgroundColor = [0.9882 0.8745 0.6627];
      app.SaveConfigurationtoTablesButton.FontName = 'Montserrat SemiBold';
      app.SaveConfigurationtoTablesButton.FontSize = 14;
      app.SaveConfigurationtoTablesButton.Position = [29 38 225 26];
      app.SaveConfigurationtoTablesButton.Text = 'Save Configuration to Tables';

      % Create Phase1ampuAEditFieldLabel
      app.Phase1ampuAEditFieldLabel = uilabel(app.LeftPanel);
      app.Phase1ampuAEditFieldLabel.HorizontalAlignment = 'right';
      app.Phase1ampuAEditFieldLabel.FontName = 'Montserrat';
      app.Phase1ampuAEditFieldLabel.Position = [6 427 149 22];
      app.Phase1ampuAEditFieldLabel.Text = 'Phase 1 amp (uA)';

      % Create Phase2ampuAEditFieldLabel
      app.Phase2ampuAEditFieldLabel = uilabel(app.LeftPanel);
      app.Phase2ampuAEditFieldLabel.HorizontalAlignment = 'right';
      app.Phase2ampuAEditFieldLabel.FontName = 'Montserrat';
      app.Phase2ampuAEditFieldLabel.Position = [6 394 149 22];
      app.Phase2ampuAEditFieldLabel.Text = 'Phase 2 amp (uA)';

      % Create Phase1durationusEditFieldLabel
      app.Phase1durationusEditFieldLabel = uilabel(app.LeftPanel);
      app.Phase1durationusEditFieldLabel.HorizontalAlignment = 'right';
      app.Phase1durationusEditFieldLabel.FontName = 'Montserrat';
      app.Phase1durationusEditFieldLabel.Position = [6 360 149 22];
      app.Phase1durationusEditFieldLabel.Text = 'Phase 1 duration (us)';

      % Create Phase2durationusEditFieldLabel
      app.Phase2durationusEditFieldLabel = uilabel(app.LeftPanel);
      app.Phase2durationusEditFieldLabel.HorizontalAlignment = 'right';
      app.Phase2durationusEditFieldLabel.FontName = 'Montserrat';
      app.Phase2durationusEditFieldLabel.Position = [6 289 149 22];
      app.Phase2durationusEditFieldLabel.Text = 'Phase 2 duration (us)';

      % Create FrequencyHzEditFieldLabel
      app.FrequencyHzEditFieldLabel = uilabel(app.LeftPanel);
      app.FrequencyHzEditFieldLabel.HorizontalAlignment = 'right';
      app.FrequencyHzEditFieldLabel.FontName = 'Montserrat';
      app.FrequencyHzEditFieldLabel.Position = [6 254 149 22];
      app.FrequencyHzEditFieldLabel.Text = 'Frequency (Hz)';

      % Create TrainDurationsEditFieldLabel
      app.TrainDurationsEditFieldLabel = uilabel(app.LeftPanel);
      app.TrainDurationsEditFieldLabel.HorizontalAlignment = 'right';
      app.TrainDurationsEditFieldLabel.FontName = 'Montserrat';
      app.TrainDurationsEditFieldLabel.Position = [46 219 109 22];
      app.TrainDurationsEditFieldLabel.Text = 'Train Duration (s)';

      % Create TrainFrequencyHzEditFieldLabel
      app.TrainFrequencyHzEditFieldLabel = uilabel(app.LeftPanel);
      app.TrainFrequencyHzEditFieldLabel.HorizontalAlignment = 'right';
      app.TrainFrequencyHzEditFieldLabel.FontName = 'Montserrat';
      app.TrainFrequencyHzEditFieldLabel.Position = [6 186 149 22];
      app.TrainFrequencyHzEditFieldLabel.Text = 'Train Frequency (Hz)';

      % Create ChannelDropDownLabel
      app.ChannelDropDownLabel = uilabel(app.LeftPanel);
      app.ChannelDropDownLabel.HorizontalAlignment = 'right';
      app.ChannelDropDownLabel.FontName = 'Montserrat';
      app.ChannelDropDownLabel.Position = [62 97 56 22];
      app.ChannelDropDownLabel.Text = 'Channel';

      % Create ChannelDropDown
      app.ChannelDropDown = uidropdown(app.LeftPanel);
      app.ChannelDropDown.Items = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', ''};
      app.ChannelDropDown.ValueChangedFcn = createCallbackFcn(app, @ChannelDropDownValueChanged, true);
      app.ChannelDropDown.FontName = 'Montserrat';
      app.ChannelDropDown.Position = [133 97 100 22];
      app.ChannelDropDown.Value = '1';

      % Create NeurostimulatorDropDownLabel
      app.NeurostimulatorDropDownLabel = uilabel(app.LeftPanel);
      app.NeurostimulatorDropDownLabel.HorizontalAlignment = 'right';
      app.NeurostimulatorDropDownLabel.FontName = 'Montserrat SemiBold';
      app.NeurostimulatorDropDownLabel.Position = [8 555 107 22];
      app.NeurostimulatorDropDownLabel.Text = 'Neurostimulator';

      % Create InterphasedurationusEditFieldLabel
      app.InterphasedurationusEditFieldLabel = uilabel(app.LeftPanel);
      app.InterphasedurationusEditFieldLabel.HorizontalAlignment = 'right';
      app.InterphasedurationusEditFieldLabel.FontName = 'Montserrat';
      app.InterphasedurationusEditFieldLabel.Position = [6 324 150 22];
      app.InterphasedurationusEditFieldLabel.Text = 'Interphase duration (us)';

      % Create Phase1delaymsEditFieldLabel
      app.Phase1delaymsEditFieldLabel = uilabel(app.LeftPanel);
      app.Phase1delaymsEditFieldLabel.HorizontalAlignment = 'right';
      app.Phase1delaymsEditFieldLabel.FontName = 'Montserrat';
      app.Phase1delaymsEditFieldLabel.Visible = 'off';
      app.Phase1delaymsEditFieldLabel.Position = [40 458 115 22];
      app.Phase1delaymsEditFieldLabel.Text = 'Phase 1 delay (ms)';

      % Create StimulationDurationsEditFieldLabel
      app.StimulationDurationsEditFieldLabel = uilabel(app.LeftPanel);
      app.StimulationDurationsEditFieldLabel.HorizontalAlignment = 'right';
      app.StimulationDurationsEditFieldLabel.FontName = 'Montserrat';
      app.StimulationDurationsEditFieldLabel.Visible = 'off';
      app.StimulationDurationsEditFieldLabel.Position = [6 219 149 22];
      app.StimulationDurationsEditFieldLabel.Text = 'Stimulation Duration (s)';

      % Create ReturnChannelDropDownLabel
      app.ReturnChannelDropDownLabel = uilabel(app.LeftPanel);
      app.ReturnChannelDropDownLabel.HorizontalAlignment = 'right';
      app.ReturnChannelDropDownLabel.FontName = 'Montserrat';
      app.ReturnChannelDropDownLabel.Visible = 'off';
      app.ReturnChannelDropDownLabel.Position = [16 68 101 22];
      app.ReturnChannelDropDownLabel.Text = 'Return Channel';

      % Create ReturnChannelDropDown
      app.ReturnChannelDropDown = uidropdown(app.LeftPanel);
      app.ReturnChannelDropDown.Items = {'-1', '10272', '10273', '10274', '10275', '10276', '10277', '10278', '10279', '10280', '10281', '10282', '10283', '10284', '10285', '10286', '10287'};
      app.ReturnChannelDropDown.ValueChangedFcn = createCallbackFcn(app, @ReturnChannelDropDownValueChanged, true);
      app.ReturnChannelDropDown.Visible = 'off';
      app.ReturnChannelDropDown.FontName = 'Montserrat';
      app.ReturnChannelDropDown.Position = [132 68 100 22];
      app.ReturnChannelDropDown.Value = '-1';

      % Create TriggerExpressionLabel
      app.TriggerExpressionLabel = uilabel(app.LeftPanel);
      app.TriggerExpressionLabel.HorizontalAlignment = 'center';
      app.TriggerExpressionLabel.FontName = 'Montserrat';
      app.TriggerExpressionLabel.Position = [1 116 85 38];
      app.TriggerExpressionLabel.Text = {'Trigger'; 'Expression'};

      % Create AbortSpinner
      app.AbortSpinner = uispinner(app.LeftPanel);
      app.AbortSpinner.FontName = 'Montserrat';
      app.AbortSpinner.Visible = 'off';
      app.AbortSpinner.Position = [232 89 47 22];

      % Create AbortDropDown
      app.AbortDropDown = uidropdown(app.LeftPanel);
      app.AbortDropDown.Items = {'==', '>=', '<=', '!='};
      app.AbortDropDown.Visible = 'off';
      app.AbortDropDown.FontName = 'Montserrat';
      app.AbortDropDown.Position = [184 89 47 22];
      app.AbortDropDown.Value = '==';

      % Create AbortExpressionEditField
      app.AbortExpressionEditField = uieditfield(app.LeftPanel, 'text');
      app.AbortExpressionEditField.FontName = 'Montserrat';
      app.AbortExpressionEditField.FontSize = 11;
      app.AbortExpressionEditField.Visible = 'off';
      app.AbortExpressionEditField.Placeholder = 'Valid BCI2000 Exp';
      app.AbortExpressionEditField.Position = [92 89 89 22];
      app.AbortExpressionEditField.Value = 'StimulusCode';

      % Create AbortExpressionLabel
      app.AbortExpressionLabel = uilabel(app.LeftPanel);
      app.AbortExpressionLabel.HorizontalAlignment = 'center';
      app.AbortExpressionLabel.FontName = 'Montserrat';
      app.AbortExpressionLabel.Visible = 'off';
      app.AbortExpressionLabel.Position = [2 81 85 38];
      app.AbortExpressionLabel.Text = {'Abort'; 'Expression'};

      % Create TriggerSpinner
      app.TriggerSpinner = uispinner(app.LeftPanel);
      app.TriggerSpinner.FontName = 'Montserrat';
      app.TriggerSpinner.Position = [231 124 47 22];
      app.TriggerSpinner.Value = 1;

      % Create TriggerDropDown
      app.TriggerDropDown = uidropdown(app.LeftPanel);
      app.TriggerDropDown.Items = {'==', '>=', '<=', '!='};
      app.TriggerDropDown.FontName = 'Montserrat';
      app.TriggerDropDown.Position = [183 124 47 22];
      app.TriggerDropDown.Value = '==';

      % Create TriggerExpressionEditField
      app.TriggerExpressionEditField = uieditfield(app.LeftPanel, 'text');
      app.TriggerExpressionEditField.FontName = 'Montserrat';
      app.TriggerExpressionEditField.FontSize = 11;
      app.TriggerExpressionEditField.Placeholder = 'Valid BCI2000 Exp';
      app.TriggerExpressionEditField.Position = [91 124 89 22];
      app.TriggerExpressionEditField.Value = 'StimulusCode';

      % Create TrainFrequencyHzEditField
      app.TrainFrequencyHzEditField = uieditfield(app.LeftPanel, 'numeric');
      app.TrainFrequencyHzEditField.Limits = [0 Inf];
      app.TrainFrequencyHzEditField.ValueDisplayFormat = '%11.4g Hz';
      app.TrainFrequencyHzEditField.ValueChangedFcn = createCallbackFcn(app, @TrainFrequencyHzEditFieldValueChanged, true);
      app.TrainFrequencyHzEditField.FontName = 'Montserrat';
      app.TrainFrequencyHzEditField.Position = [162 186 105 22];
      app.TrainFrequencyHzEditField.Value = 2;

      % Create StimulationDurationsEditField
      app.StimulationDurationsEditField = uieditfield(app.LeftPanel, 'numeric');
      app.StimulationDurationsEditField.Limits = [0 Inf];
      app.StimulationDurationsEditField.ValueDisplayFormat = '%11.4g s';
      app.StimulationDurationsEditField.ValueChangedFcn = createCallbackFcn(app, @StimulationDurationsEditFieldValueChanged, true);
      app.StimulationDurationsEditField.FontName = 'Montserrat';
      app.StimulationDurationsEditField.Visible = 'off';
      app.StimulationDurationsEditField.Position = [162 219 104 22];
      app.StimulationDurationsEditField.Value = 0.01;

      % Create TrainDurationsEditField
      app.TrainDurationsEditField = uieditfield(app.LeftPanel, 'numeric');
      app.TrainDurationsEditField.Limits = [0 Inf];
      app.TrainDurationsEditField.ValueDisplayFormat = '%11.4g s';
      app.TrainDurationsEditField.ValueChangedFcn = createCallbackFcn(app, @TrainDurationsEditFieldValueChanged, true);
      app.TrainDurationsEditField.FontName = 'Montserrat';
      app.TrainDurationsEditField.Position = [162 219 105 22];
      app.TrainDurationsEditField.Value = 2;

      % Create FrequencyHzEditField
      app.FrequencyHzEditField = uieditfield(app.LeftPanel, 'numeric');
      app.FrequencyHzEditField.Limits = [4 5000];
      app.FrequencyHzEditField.ValueDisplayFormat = '%11.4g Hz';
      app.FrequencyHzEditField.ValueChangedFcn = createCallbackFcn(app, @FrequencyHzEditFieldValueChanged, true);
      app.FrequencyHzEditField.FontName = 'Montserrat';
      app.FrequencyHzEditField.Position = [162 254 105 22];
      app.FrequencyHzEditField.Value = 100;

      % Create NumberofpulsesSpinnerLabel
      app.NumberofpulsesSpinnerLabel = uilabel(app.LeftPanel);
      app.NumberofpulsesSpinnerLabel.HorizontalAlignment = 'right';
      app.NumberofpulsesSpinnerLabel.FontName = 'Montserrat';
      app.NumberofpulsesSpinnerLabel.Position = [42 458 113 22];
      app.NumberofpulsesSpinnerLabel.Text = 'Number of pulses';

      % Create Phase2durationusEditField
      app.Phase2durationusEditField = uieditfield(app.LeftPanel, 'numeric');
      app.Phase2durationusEditField.Limits = [1 65535];
      app.Phase2durationusEditField.ValueDisplayFormat = '%11.4g us';
      app.Phase2durationusEditField.ValueChangedFcn = createCallbackFcn(app, @Phase2durationusEditFieldValueChanged, true);
      app.Phase2durationusEditField.FontName = 'Montserrat';
      app.Phase2durationusEditField.Position = [162 289 105 22];
      app.Phase2durationusEditField.Value = 500;

      % Create InterphasedurationusEditField
      app.InterphasedurationusEditField = uieditfield(app.LeftPanel, 'numeric');
      app.InterphasedurationusEditField.Limits = [53 65535];
      app.InterphasedurationusEditField.ValueDisplayFormat = '%11.4g us';
      app.InterphasedurationusEditField.ValueChangedFcn = createCallbackFcn(app, @InterphasedurationusEditFieldValueChanged, true);
      app.InterphasedurationusEditField.FontName = 'Montserrat';
      app.InterphasedurationusEditField.Position = [162 324 105 22];
      app.InterphasedurationusEditField.Value = 53;

      % Create Phase1durationusEditField
      app.Phase1durationusEditField = uieditfield(app.LeftPanel, 'numeric');
      app.Phase1durationusEditField.Limits = [1 65535];
      app.Phase1durationusEditField.ValueDisplayFormat = '%11.4g us';
      app.Phase1durationusEditField.ValueChangedFcn = createCallbackFcn(app, @Phase1durationusEditFieldValueChanged, true);
      app.Phase1durationusEditField.FontName = 'Montserrat';
      app.Phase1durationusEditField.Position = [162 360 105 22];
      app.Phase1durationusEditField.Value = 250;

      % Create Phase2ampuAEditField
      app.Phase2ampuAEditField = uieditfield(app.LeftPanel, 'numeric');
      app.Phase2ampuAEditField.Limits = [100 10000];
      app.Phase2ampuAEditField.ValueDisplayFormat = '%11.4g uA';
      app.Phase2ampuAEditField.ValueChangedFcn = createCallbackFcn(app, @Phase2ampuAEditFieldValueChanged, true);
      app.Phase2ampuAEditField.FontName = 'Montserrat';
      app.Phase2ampuAEditField.Position = [162 394 105 22];
      app.Phase2ampuAEditField.Value = 100;

      % Create Phase1ampuAEditField
      app.Phase1ampuAEditField = uieditfield(app.LeftPanel, 'numeric');
      app.Phase1ampuAEditField.Limits = [100 10000];
      app.Phase1ampuAEditField.ValueDisplayFormat = '%11.4g uA';
      app.Phase1ampuAEditField.ValueChangedFcn = createCallbackFcn(app, @Phase1ampuAEditFieldValueChanged, true);
      app.Phase1ampuAEditField.FontName = 'Montserrat';
      app.Phase1ampuAEditField.Position = [162 427 105 22];
      app.Phase1ampuAEditField.Value = 200;

      % Create Phase1delaymsEditField
      app.Phase1delaymsEditField = uieditfield(app.LeftPanel, 'numeric');
      app.Phase1delaymsEditField.Limits = [0 99];
      app.Phase1delaymsEditField.ValueDisplayFormat = '%11.4g ms';
      app.Phase1delaymsEditField.ValueChangedFcn = createCallbackFcn(app, @Phase1delaymsEditFieldValueChanged, true);
      app.Phase1delaymsEditField.FontName = 'Montserrat';
      app.Phase1delaymsEditField.Visible = 'off';
      app.Phase1delaymsEditField.Position = [162 458 105 22];

      % Create PolarityButtonGroup
      app.PolarityButtonGroup = uibuttongroup(app.LeftPanel);
      app.PolarityButtonGroup.SelectionChangedFcn = createCallbackFcn(app, @PolarityButtonGroupSelectionChanged, true);
      app.PolarityButtonGroup.Title = 'Polarity';
      app.PolarityButtonGroup.Visible = 'off';
      app.PolarityButtonGroup.BackgroundColor = [1 0.9686 0.902];
      app.PolarityButtonGroup.FontName = 'Montserrat';
      app.PolarityButtonGroup.Position = [28 432 223 54];

      % Create AlternatingButton
      app.AlternatingButton = uiradiobutton(app.PolarityButtonGroup);
      app.AlternatingButton.Text = 'Alternating';
      app.AlternatingButton.FontName = 'Montserrat';
      app.AlternatingButton.Position = [11 7 94 22];
      app.AlternatingButton.Value = true;

      % Create SteadyButton
      app.SteadyButton = uiradiobutton(app.PolarityButtonGroup);
      app.SteadyButton.Text = 'Steady';
      app.SteadyButton.FontName = 'Montserrat';
      app.SteadyButton.Position = [126 7 83 22];

      % Create NumberofpulsesSpinner
      app.NumberofpulsesSpinner = uispinner(app.LeftPanel);
      app.NumberofpulsesSpinner.Limits = [1 255];
      app.NumberofpulsesSpinner.ValueChangedFcn = createCallbackFcn(app, @NumberofpulsesSpinnerValueChanged, true);
      app.NumberofpulsesSpinner.FontName = 'Montserrat';
      app.NumberofpulsesSpinner.Position = [185 458 59 22];
      app.NumberofpulsesSpinner.Value = 10;

      % Create RecordingChannelsButtonGroup
      app.RecordingChannelsButtonGroup = uibuttongroup(app.LeftPanel);
      app.RecordingChannelsButtonGroup.SelectionChangedFcn = createCallbackFcn(app, @RecordingChannelsButtonGroupSelectionChanged, true);
      app.RecordingChannelsButtonGroup.BorderType = 'none';
      app.RecordingChannelsButtonGroup.TitlePosition = 'centertop';
      app.RecordingChannelsButtonGroup.Title = 'Recording Channels';
      app.RecordingChannelsButtonGroup.Visible = 'off';
      app.RecordingChannelsButtonGroup.BackgroundColor = [1 0.9686 0.902];
      app.RecordingChannelsButtonGroup.FontName = 'Montserrat';
      app.RecordingChannelsButtonGroup.Position = [6 487 273 65];

      % Create RAWButton
      app.RAWButton = uiradiobutton(app.RecordingChannelsButtonGroup);
      app.RAWButton.Text = 'RAW';
      app.RAWButton.Position = [11 20 58 22];

      % Create ECOGHFButton
      app.ECOGHFButton = uiradiobutton(app.RecordingChannelsButtonGroup);
      app.ECOGHFButton.Text = 'ECOG HF';
      app.ECOGHFButton.Position = [92 20 77 22];
      app.ECOGHFButton.Value = true;

      % Create SPKButton
      app.SPKButton = uiradiobutton(app.RecordingChannelsButtonGroup);
      app.SPKButton.Text = 'SPK';
      app.SPKButton.Position = [192 20 65 22];

      % Create SEGButton
      app.SEGButton = uiradiobutton(app.RecordingChannelsButtonGroup);
      app.SEGButton.Text = 'SEG';
      app.SEGButton.Position = [11 -1 65 22];

      % Create ECOGLFButton
      app.ECOGLFButton = uiradiobutton(app.RecordingChannelsButtonGroup);
      app.ECOGLFButton.Text = 'ECOG LF';
      app.ECOGLFButton.Position = [92 -1 75 22];

      % Create LFPButton
      app.LFPButton = uiradiobutton(app.RecordingChannelsButtonGroup);
      app.LFPButton.Text = 'LFP';
      app.LFPButton.Position = [192 -1 58 22];

      % Create OrderButtonGroup
      app.OrderButtonGroup = uibuttongroup(app.LeftPanel);
      app.OrderButtonGroup.SelectionChangedFcn = createCallbackFcn(app, @OrderButtonGroupSelectionChanged, true);
      app.OrderButtonGroup.Title = 'Order';
      app.OrderButtonGroup.BackgroundColor = [1 0.9686 0.902];
      app.OrderButtonGroup.FontName = 'Montserrat';
      app.OrderButtonGroup.Position = [28 494 222 54];

      % Create CathodeFirstButton
      app.CathodeFirstButton = uiradiobutton(app.OrderButtonGroup);
      app.CathodeFirstButton.Text = 'Cathode First';
      app.CathodeFirstButton.FontName = 'Montserrat';
      app.CathodeFirstButton.Position = [11 7 103 22];
      app.CathodeFirstButton.Value = true;

      % Create AnodeFirstButton
      app.AnodeFirstButton = uiradiobutton(app.OrderButtonGroup);
      app.AnodeFirstButton.Text = 'Anode First';
      app.AnodeFirstButton.FontName = 'Montserrat';
      app.AnodeFirstButton.Position = [114 7 91 22];

      % Create NeurostimulatorDropDown
      app.NeurostimulatorDropDown = uidropdown(app.LeftPanel);
      app.NeurostimulatorDropDown.Items = {'CereStim', 'NeuroOmega', 'g.Estim', 'Cortec BIC'};
      app.NeurostimulatorDropDown.ValueChangedFcn = createCallbackFcn(app, @NeurostimulatorDropDownValueChanged, true);
      app.NeurostimulatorDropDown.FontName = 'Montserrat ExtraBold';
      app.NeurostimulatorDropDown.BackgroundColor = [1 0.9333 0.6902];
      app.NeurostimulatorDropDown.Position = [130 555 131 22];
      app.NeurostimulatorDropDown.Value = 'CereStim';

      % Create StimulationConfigurationLabel
      app.StimulationConfigurationLabel = uilabel(app.LeftPanel);
      app.StimulationConfigurationLabel.FontName = 'Montserrat';
      app.StimulationConfigurationLabel.FontWeight = 'bold';
      app.StimulationConfigurationLabel.Position = [68 581 170 22];
      app.StimulationConfigurationLabel.Text = 'Stimulation Configuration';

      % Create ModeButtonGroup
      app.ModeButtonGroup = uibuttongroup(app.LeftPanel);
      app.ModeButtonGroup.SelectionChangedFcn = createCallbackFcn(app, @ModeButtonGroupSelectionChanged, true);
      app.ModeButtonGroup.Title = 'Mode';
      app.ModeButtonGroup.Visible = 'off';
      app.ModeButtonGroup.BackgroundColor = [1 0.9686 0.902];
      app.ModeButtonGroup.FontName = 'Montserrat';
      app.ModeButtonGroup.Position = [5 494 269 54];

      % Create Symmetric
      app.Symmetric = uiradiobutton(app.ModeButtonGroup);
      app.Symmetric.Text = 'Symmetric';
      app.Symmetric.FontName = 'Montserrat';
      app.Symmetric.Position = [3 7 88 22];
      app.Symmetric.Value = true;

      % Create Monophasic
      app.Monophasic = uiradiobutton(app.ModeButtonGroup);
      app.Monophasic.Text = 'Monophasic';
      app.Monophasic.FontName = 'Montserrat';
      app.Monophasic.Position = [78 7 93 22];

      % Create Asymmetric
      app.Asymmetric = uiradiobutton(app.ModeButtonGroup);
      app.Asymmetric.Text = 'Asymmetric';
      app.Asymmetric.FontName = 'Montserrat';
      app.Asymmetric.Position = [175 7 95 22];

      % Create CenterPanel
      app.CenterPanel = uipanel(app.GridLayout);
      app.CenterPanel.BackgroundColor = [0.949 0.9804 1];
      app.CenterPanel.Layout.Row = 1;
      app.CenterPanel.Layout.Column = 2;
      app.CenterPanel.FontName = 'Montserrat';

      % Create SinglePulseAxes
      app.SinglePulseAxes = uiaxes(app.CenterPanel);
      title(app.SinglePulseAxes, 'Single pulse')
      xlabel(app.SinglePulseAxes, 'Time (us)')
      ylabel(app.SinglePulseAxes, 'Current (uA)')
      app.SinglePulseAxes.FontName = 'Montserrat';
      app.SinglePulseAxes.LineWidth = 1;
      app.SinglePulseAxes.GridColor = [0 1 0];
      app.SinglePulseAxes.MinorGridColor = [0.1294 0.1216 0.1216];
      app.SinglePulseAxes.Box = 'on';
      app.SinglePulseAxes.ColorOrder = [0.502 0.6902 0.8314;0.9804 0.502 0.451;0.549 0.8314 0.7804;0.9882 0.7098 0.3804;0.702 0.8706 0.4118;0.749 0.7294 0.851;0.502 0.502 0.502];
      app.SinglePulseAxes.Position = [3 245 233 250];

      % Create MultPulsesAxes
      app.MultPulsesAxes = uiaxes(app.CenterPanel);
      title(app.MultPulsesAxes, 'Burst of pulses')
      xlabel(app.MultPulsesAxes, 'Time (us)')
      ylabel(app.MultPulsesAxes, 'Current (uA)')
      app.MultPulsesAxes.FontName = 'Montserrat';
      app.MultPulsesAxes.LineWidth = 1;
      app.MultPulsesAxes.Box = 'on';
      app.MultPulsesAxes.ColorOrder = [0.502 0.6902 0.8314;0.9804 0.502 0.451;0.549 0.8314 0.7804;0.9882 0.7098 0.3804;0.702 0.8706 0.4118;0.749 0.7294 0.851;0.502 0.502 0.502];
      app.MultPulsesAxes.Position = [248 243 231 250];

      % Create ChannelAxes
      app.ChannelAxes = uiaxes(app.CenterPanel);
      title(app.ChannelAxes, 'Channels')
      zlabel(app.ChannelAxes, 'Z')
      app.ChannelAxes.FontName = 'Montserrat';
      app.ChannelAxes.Color = [0.949 0.9804 1];
      app.ChannelAxes.TitleHorizontalAlignment = 'right';
      app.ChannelAxes.GridColor = [1 1 1];
      app.ChannelAxes.Box = 'on';
      app.ChannelAxes.Position = [4 1 476 114];

      % Create TrainAxes
      app.TrainAxes = uiaxes(app.CenterPanel);
      title(app.TrainAxes, 'Complete stimulation train')
      xlabel(app.TrainAxes, 'Time (s)')
      ylabel(app.TrainAxes, 'On / Off')
      app.TrainAxes.FontName = 'Montserrat';
      app.TrainAxes.LineWidth = 1;
      app.TrainAxes.TitleHorizontalAlignment = 'right';
      app.TrainAxes.Box = 'on';
      app.TrainAxes.ColorOrder = [0.502 0.6902 0.8314;0.9804 0.502 0.451;0.549 0.8314 0.7804;0.9882 0.7098 0.3804;0.702 0.8706 0.4118;0.749 0.7294 0.851;0.502 0.502 0.502];
      app.TrainAxes.Position = [9 105 475 131];

      % Create ButtonGroup
      app.ButtonGroup = uibuttongroup(app.CenterPanel);
      app.ButtonGroup.AutoResizeChildren = 'off';
      app.ButtonGroup.SelectionChangedFcn = createCallbackFcn(app, @ButtonGroupSelectionChanged, true);
      app.ButtonGroup.SizeChangedFcn = createCallbackFcn(app, @ButtonGroupSizeChanged, true);
      app.ButtonGroup.FontName = 'Montserrat';
      app.ButtonGroup.Scrollable = 'on';
      app.ButtonGroup.Position = [9 502 470 84];

      % Create BUILDCONFIGButton
      app.BUILDCONFIGButton = uitogglebutton(app.ButtonGroup);
      app.BUILDCONFIGButton.Text = 'BUILD CONFIG';
      app.BUILDCONFIGButton.WordWrap = 'on';
      app.BUILDCONFIGButton.FontName = 'Montserrat';
      app.BUILDCONFIGButton.Position = [9 7 61 70];
      app.BUILDCONFIGButton.Value = true;

      % Create ExperimentConditionsLabel
      app.ExperimentConditionsLabel = uilabel(app.CenterPanel);
      app.ExperimentConditionsLabel.HorizontalAlignment = 'right';
      app.ExperimentConditionsLabel.WordWrap = 'on';
      app.ExperimentConditionsLabel.FontName = 'Montserrat Medium';
      app.ExperimentConditionsLabel.FontSize = 13.5;
      app.ExperimentConditionsLabel.FontWeight = 'bold';
      app.ExperimentConditionsLabel.Position = [9 585 471 22];
      app.ExperimentConditionsLabel.Text = 'Experiment Conditions';

      % Create RightPanel
      app.RightPanel = uipanel(app.GridLayout);
      app.RightPanel.BackgroundColor = [0.949 0.9804 1];
      app.RightPanel.Layout.Row = 1;
      app.RightPanel.Layout.Column = 3;
      app.RightPanel.FontName = 'Montserrat';

      % Create TableConfigurations
      app.TableConfigurations = uitable(app.RightPanel);
      app.TableConfigurations.ColumnName = '';
      app.TableConfigurations.RowName = {'Cathode First'; 'Number of pulses'; 'Phase 1 amp (uA)'; 'Phase 2 amp (uA)'; 'Phase 1 duration (us)'; 'Phase 2 duration (us)'; 'Frequency (Hz)'; 'Interphase duration (us)'; 'Train Duration (s)'; 'Train Frequency (Hz)'};
      app.TableConfigurations.DisplayDataChangedFcn = createCallbackFcn(app, @TableConfigurationsDisplayDataChanged, true);
      app.TableConfigurations.Position = [12 286 364 296];

      % Create TableTriggers
      app.TableTriggers = uitable(app.RightPanel);
      app.TableTriggers.ColumnName = '';
      app.TableTriggers.RowName = {'Expression'; 'Config ID'; 'Electrode(s)'};
      app.TableTriggers.CellSelectionCallback = createCallbackFcn(app, @TableTriggersCellSelection, true);
      app.TableTriggers.Position = [12 110 364 156];

      % Create StimulationTriggersLabel
      app.StimulationTriggersLabel = uilabel(app.RightPanel);
      app.StimulationTriggersLabel.HorizontalAlignment = 'center';
      app.StimulationTriggersLabel.FontName = 'Montserrat SemiBold';
      app.StimulationTriggersLabel.Position = [7 265 384 22];
      app.StimulationTriggersLabel.Text = 'StimulationTriggers';

      % Create StimulationConfigurationsLabel
      app.StimulationConfigurationsLabel = uilabel(app.RightPanel);
      app.StimulationConfigurationsLabel.HorizontalAlignment = 'center';
      app.StimulationConfigurationsLabel.FontName = 'Montserrat SemiBold';
      app.StimulationConfigurationsLabel.Position = [7 582 384 20];
      app.StimulationConfigurationsLabel.Text = 'StimulationConfigurations';

      % Create DeleteChannelDropDown
      app.DeleteChannelDropDown = uidropdown(app.RightPanel);
      app.DeleteChannelDropDown.Items = {};
      app.DeleteChannelDropDown.FontName = 'Montserrat';
      app.DeleteChannelDropDown.Position = [187 79 142 22];
      app.DeleteChannelDropDown.Value = {};

      % Create DeleteTriggerDropDown
      app.DeleteTriggerDropDown = uidropdown(app.RightPanel);
      app.DeleteTriggerDropDown.Items = {};
      app.DeleteTriggerDropDown.FontName = 'Montserrat';
      app.DeleteTriggerDropDown.Position = [187 50 141 22];
      app.DeleteTriggerDropDown.Value = {};

      % Create DeleteConfigurationButton
      app.DeleteConfigurationButton = uibutton(app.RightPanel, 'push');
      app.DeleteConfigurationButton.ButtonPushedFcn = createCallbackFcn(app, @DeleteConfigurationButtonPushed, true);
      app.DeleteConfigurationButton.FontName = 'Montserrat';
      app.DeleteConfigurationButton.Position = [43 78 142 23];
      app.DeleteConfigurationButton.Text = 'Delete Configuration';

      % Create DeleteTriggerButton
      app.DeleteTriggerButton = uibutton(app.RightPanel, 'push');
      app.DeleteTriggerButton.ButtonPushedFcn = createCallbackFcn(app, @DeleteTriggerButtonPushed, true);
      app.DeleteTriggerButton.FontName = 'Montserrat';
      app.DeleteTriggerButton.Position = [84 50 101 23];
      app.DeleteTriggerButton.Text = 'Delete Trigger';

      % Create StimulationOnLampLabel
      app.StimulationOnLampLabel = uilabel(app.RightPanel);
      app.StimulationOnLampLabel.HorizontalAlignment = 'right';
      app.StimulationOnLampLabel.FontName = 'Montserrat Black';
      app.StimulationOnLampLabel.Visible = 'off';
      app.StimulationOnLampLabel.Position = [243 306 73 38];
      app.StimulationOnLampLabel.Text = 'Stimulation On';

      % Create StimulationOnLamp
      app.StimulationOnLamp = uilamp(app.RightPanel);
      app.StimulationOnLamp.Visible = 'off';
      app.StimulationOnLamp.Position = [315 289 62 62];
      app.StimulationOnLamp.Color = [1 0 0];

      % Create PlayStimulationButton
      app.PlayStimulationButton = uibutton(app.RightPanel, 'push');
      app.PlayStimulationButton.ButtonPushedFcn = createCallbackFcn(app, @PlayStimulationButtonPushed, true);
      app.PlayStimulationButton.WordWrap = 'on';
      app.PlayStimulationButton.FontName = 'Montserrat Black';
      app.PlayStimulationButton.FontSize = 22;
      app.PlayStimulationButton.Visible = 'off';
      app.PlayStimulationButton.Position = [203 63 174 67];
      app.PlayStimulationButton.Text = 'Play Stimulation';

      % Create ExporttoparameterfileButton
      app.ExporttoparameterfileButton = uibutton(app.RightPanel, 'push');
      app.ExporttoparameterfileButton.ButtonPushedFcn = createCallbackFcn(app, @ExporttoparameterfileButtonPushed, true);
      app.ExporttoparameterfileButton.BackgroundColor = [0 0.0549 0.251];
      app.ExporttoparameterfileButton.FontName = 'Montserrat Black';
      app.ExporttoparameterfileButton.FontSize = 18;
      app.ExporttoparameterfileButton.FontColor = [1 1 1];
      app.ExporttoparameterfileButton.Position = [73 9 254 30];
      app.ExporttoparameterfileButton.Text = 'Export to parameter file';

      % Create ColorContextMenu
      app.ColorContextMenu = uicontextmenu(app.UIFigure);

      % Create ChangeColorsMenu
      app.ChangeColorsMenu = uimenu(app.ColorContextMenu);
      app.ChangeColorsMenu.Text = 'Change Colors';

      % Create BackgroundMenu
      app.BackgroundMenu = uimenu(app.ChangeColorsMenu);
      app.BackgroundMenu.MenuSelectedFcn = createCallbackFcn(app, @BackgroundMenuSelected, true);
      app.BackgroundMenu.Text = 'Background';

      % Create Plot1Menu
      app.Plot1Menu = uimenu(app.ChangeColorsMenu);
      app.Plot1Menu.MenuSelectedFcn = createCallbackFcn(app, @Plot1MenuSelected, true);
      app.Plot1Menu.Text = 'Plot 1';

      % Create Plot2Menu
      app.Plot2Menu = uimenu(app.ChangeColorsMenu);
      app.Plot2Menu.MenuSelectedFcn = createCallbackFcn(app, @Plot2MenuSelected, true);
      app.Plot2Menu.Text = 'Plot 2';

      % Create Plot3Menu
      app.Plot3Menu = uimenu(app.ChangeColorsMenu);
      app.Plot3Menu.MenuSelectedFcn = createCallbackFcn(app, @Plot3MenuSelected, true);
      app.Plot3Menu.Text = 'Plot 3';

      % Create RestoreDefaultColorsMenu
      app.RestoreDefaultColorsMenu = uimenu(app.ColorContextMenu);
      app.RestoreDefaultColorsMenu.MenuSelectedFcn = createCallbackFcn(app, @RestoreDefaultColorsMenuSelected, true);
      app.RestoreDefaultColorsMenu.Text = 'Restore Default Colors';
      
      % Assign app.ColorContextMenu
      app.SinglePulseAxes.ContextMenu = app.ColorContextMenu;
      app.MultPulsesAxes.ContextMenu = app.ColorContextMenu;
      app.TrainAxes.ContextMenu = app.ColorContextMenu;

      % Show the figure after all components are created
      app.UIFigure.Visible = 'on';
    end
  end

  % App creation and deletion
  methods (Access = public)

    % Construct app
    function app = SCIT_sourceCode

      % Create UIFigure and components
      createComponents(app)

      % Register the app with App Designer
      registerApp(app, app.UIFigure)

      % Execute the startup function
      runStartupFcn(app, @startupFcn)

      if nargout == 0
        clear app
      end
    end

    % Code that executes before app deletion
    function delete(app)

      % Delete UIFigure when app is deleted
      delete(app.UIFigure)
    end
  end
end