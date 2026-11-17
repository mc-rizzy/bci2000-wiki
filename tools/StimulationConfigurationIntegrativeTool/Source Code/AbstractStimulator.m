classdef (Abstract) AbstractStimulator < handle
  properties (Abstract)
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
  properties
    polarity = false
    stimBurstError = "DECREASE THE STIM DURATION OR THE TRAIN FREQUENCY";
    elements %holds all elements
  end
  properties (Constant)
    testColor = 0.2*ones(1,3); % color for selected electrodes
    dy = 33;
  end
  properties (Abstract, Constant)    
    sectionName
    configName
    xBurstLabel

    visibleElements %list of indices of elements to display
    electrodes %list of visible electrodes: [row, column]
  end

  methods
    initialize(self, app)
    shift(self, down)
    buildConfig(self, app)
    buildTrig(self, app)
    setConfig(self, app)
    colorChannels(self, app)
    checkTrigTable(self, app, trigTable)
    handleSameConfig(self, app, configCols)

    function setUp(self, app)
      %all elements
      self.elements = {
          app.NumberofpulsesSpinner, app.NumberofpulsesSpinnerLabel;                  %1
          app.Phase1delaymsEditField, app.Phase1delaymsEditFieldLabel;                %2
          app.Phase1ampuAEditField, app.Phase1ampuAEditFieldLabel;                    %3
          app.Phase2ampuAEditField, app.Phase2ampuAEditFieldLabel;                    %4
          app.Phase1durationusEditField, app.Phase1durationusEditFieldLabel;          %5
          app.InterphasedurationusEditField, app.InterphasedurationusEditFieldLabel;  %6
          app.Phase2durationusEditField, app.Phase2durationusEditFieldLabel;          %7
          app.FrequencyHzEditField, app.FrequencyHzEditFieldLabel;                    %8
          app.StimulationDurationsEditField, app.StimulationDurationsEditFieldLabel;  %9
          app.TrainDurationsEditField, app.TrainDurationsEditFieldLabel;              %10
          app.TrainFrequencyHzEditField, app.TrainFrequencyHzEditFieldLabel;          %11
          app.ChannelDropDown, app.ChannelDropDownLabel;                              %12
          app.OrderButtonGroup, '';                                                   %13
          app.ReturnChannelDropDown, app.ReturnChannelDropDownLabel;                  %14
          app.RecordingChannelsButtonGroup, '';                                       %15
          app.AbortDropDown, app.AbortExpressionEditField;
              app.AbortExpressionLabel, app.AbortSpinner;                             %16 - 17
          app.PolarityButtonGroup, '';                                                %18
          app.ModeButtonGroup, '';                                                    %19
          app.TriggerDropDown, app.TriggerExpressionEditField;
              app.TriggerExpressionLabel, app.TriggerSpinner;                         %20 - 21
          app.SaveConfigurationtoTablesButton, '';                                    %22
      };


      self.changeParameterVisibility(app);
      self.initialize(app);
      %self.dy = 33;%abs(app.TrainFrequencyHzEditField.Position(2) - app.TrainDurationsEditField.Position(2));
      %disp(self.dy)
      self.shift(1);
      
      cla(app.ChannelAxes)
      self.initializeChannels(app);
    end
    function initializeChannels(self, app)
      self.setNeuroOmegaChannels(app); %does nothing except for NeuroOmega

      r = self.electrodes(1);
      c = self.electrodes(2);
      xlim(app.ChannelAxes, [0 r])
      ylim(app.ChannelAxes, [0 c])
      axis(app.ChannelAxes,'tight')
      axis(app.ChannelAxes, 'equal')
      axis(app.ChannelAxes,'off')
      hold(app.ChannelAxes, "on")
      for i = 1:r
          for j = c:-1:1
              rect = rectangle(app.ChannelAxes,'Position',[j-1 i-1 1 1],'Curvature',[1 1], ...
                  'FaceColor', app.CenterPanel.BackgroundColor, 'ButtonDownFcn', @app.channelClickedCallback);
              rect.addprop('Channel');
              rect.Channel = (r-i)*c + j; %save channel number, can be overwritten in colorChannels()
          end
      end
      self.colorChannels(app);      
      
    end

    %redefined in NeuroOmega
    function setNeuroOmegaChannels(~, app)
    end
    %redefined in gEstim
    function param = formatParameterFile(self, app)
      configs = app.TableConfigurations.Data;
      numConfigs = size(configs,2);
      triggers = app.TableTriggers.Data;
      numTriggers = size(triggers,2);
      if numConfigs == 0
          errordlg('Please add data to the tables first, then save your parameter file. ', 'Parameter tables are empty');
          return
      end
      [configs, triggers, cRowLabels, tRowLabels] = self.uniqueParameterSaving(app.TableConfigurations, app.TableTriggers);
      columnLabels = app.TableConfigurations.ColumnName;
      rowSize = length(cRowLabels);

      param.(self.configName).Type = 'matrix';
      param.(self.configName).DefaultValue = '';
      param.(self.configName).LowRange = '';
      param.(self.configName).HighRange = '';
      param.(self.configName).Value = cell(rowSize, numConfigs);
      param.(self.configName).RowLabels = cell(rowSize,1);
      param.(self.configName).ColumnLabels = cell(1, numConfigs);
      param.(self.configName).Comment = 'Stimulation Configurations';
      param.StimulationTriggers.Comment = 'Stimulation Triggers';
      %get value set by each stimulator
      param.(self.configName).Section = self.sectionName;
      param.StimulationTriggers.Section = self.sectionName;
      
      for i=1:rowSize
          param.(self.configName).RowLabels{i} = char(cRowLabels(i));
      end
      for i=1:size(configs,2)
          if size(configs,2) > 1
              param.(self.configName).ColumnLabels{i} = char(columnLabels(i));
          else
              if iscell(columnLabels)
                  param.(self.configName).ColumnLabels = columnLabels;
              else
                  c{1} = columnLabels;
                  param.(self.configName).ColumnLabels = c;
              end
          end
      end
      for j=1:numConfigs
          for i=1:rowSize
              if numConfigs > 1
                  param.(self.configName).Value{i, j} = num2str(configs(i,j));
              else
                  param.(self.configName).Value{i, j} = sprintf('%d', configs(i,j));
              end
          end
      end
      
      %stimulation triggers
      columnLabels = app.TableTriggers.ColumnName;
      rowSize = length(tRowLabels);
      param.StimulationTriggers.Type = 'matrix';
      param.StimulationTriggers.DefaultValue = '';
      param.StimulationTriggers.LowRange = '';
      param.StimulationTriggers.HighRange = '';
      param.StimulationTriggers.Value = cell(rowSize, numTriggers);
      param.StimulationTriggers.RowLabels = cell(rowSize,1);
      param.StimulationTriggers.ColumnLabels = cell(1, numTriggers);
      
      for i=1:rowSize
          param.StimulationTriggers.RowLabels{i} = char(tRowLabels(i));
      end
      for i=1:numTriggers
          if numTriggers > 1
              param.StimulationTriggers.ColumnLabels{i} = char(columnLabels(i));
          else
              if iscell(columnLabels)
                  param.StimulationTriggers.ColumnLabels = columnLabels;
              else
                  c{1} = columnLabels;
                  param.StimulationTriggers.ColumnLabels = c;
              end
          end
      end
      for j=1:numTriggers
          %param.StimulationTriggers.Value{1, j} = sprintf('%s',triggers(1,j));
          %sprintf('%s',triggers(1,j))
          for i=1:rowSize
              a = triggers(i,j);
              param.StimulationTriggers.Value{i, j} = a{1};
              %convert ch to list
              if (i == 3 || i == 4)
                  if contains(triggers(i,j), ",")
                      chList = split(triggers(i,j), ", ");
                      %chList = erase(triggers(i,j), ",")
                      %f = "{ matrix " + string(nList) + " 1 " + chList + " }"
                      %param.StimulationTriggers.Value{i, j} = "{ matrix " + string(nList+1) + " 1 " + chList + " }";
%                             channel = triggers(i,j);
                      chanMat = cell(length(chList),1);
                      for k=1:length(chanMat)
                          chanMat(k,1) = {num2str(chList(k))};
                      end
                      param.StimulationTriggers.Value{i, j} = chanMat;
%                             chanMat
                  end
              end                        
          end
      end
    end
    %redefined in gEstim
    function [updatedTables, cTable, tTable] = loadParameterFile(self, lineArr, cTable, tTable, updatedTables)
      if lineArr(3)==(self.configName + "=")
          %send full lineArr
          [rNames, cNames, lineArr] = self.getLabels(lineArr);
          for r=1:length(rNames)
              for count=1:length(cNames)
                  cTable(r, count) = str2double(lineArr{count});
              end
              lineArr = lineArr(length(cNames)+1:end);
          end
          updatedTables = true;
          %app.TableConfigurations.RowName = rNames;
          %app.TableConfigurations.ColumnName = cNames;
      elseif lineArr(3)=="StimulationTriggers="
          %send full lineArr
          [rNames, cNames, lineArr] = self.getLabels(lineArr);
          tTable = string.empty;
          for r=1:length(rNames)
              %for updating the table correctly
              count=1;
              for col=1:length(cNames)
                  if strcmp(lineArr{count}, '{')
                      if strcmp(lineArr{count+1}, 'matrix')
                          %matrix
                          rSize = str2double(lineArr{count+2});
                          cSize = str2double(lineArr{count+3});
                          cellString = "";
                          %cellString = string(lineArr{c+3});
                          for i=1:rSize
                              for j=1:cSize
                                  lineArr{count+3+cSize*(i-1)+j};
                                  if j*i == 1
                                      %don't want a comma b4 the first one
                                      cellString = lineArr{count+3+cSize*(i-1)+j};
                                  else
                                      cellString = cellString + ", " + lineArr{count+3+cSize*(i-1)+j};
                                  end
                              end
                          end
                          tTable(r,col) = cellString;
                          %gotta update the counter irregularly
                          count = count + cSize + rSize + 4;
                      elseif strcmp(lineArr{count+1}, 'list')
                          %list
                          cSize = str2double(lineArr{count+2});
                          for i=1:cSize
                              if i == 1
                                  %don't want a comma b4 the first one
                                  cellString = lineArr{count+2+i};
                              else
                                  cellString = cellString + ", " + lineArr{count+2+i};
                              end
                          end
                          tTable(r,col) = cellString;
                          %gotta account for '{', 'list', 'cSize', '}'
                          count = count + cSize + 4;
                      end
                      if r==2
                          errordlg(['Loading parameter files with Config IDs as a list is not supported. ' ...
                          'Please make all Config IDs in StimulationTriggers scalar values.'],'StimulationTriggers Error');
                          fclose(fileID);
                          return
                      end
                  else
                      tTable(r, col) = string(lineArr{count});
                      %update counter like normal
                      count = count + 1;
                  end
              end
              lineArr = lineArr(count:end);
          end
          updatedTables = true;
          %app.TableTriggers.RowName = rNames;
          %app.TableTriggers.ColumnName = cNames;
          %app.TableTriggers.Data = t;
      end
    end
    %redefined in Cortec
    function [cTable, tTable] = uniqueParameterProcessing(~, cTable, tTable)
    end
    function [configs, triggers, cRowLabels, tRowLabels] = uniqueParameterSaving(~, cTable, tTable)
      configs = cTable.Data;
      triggers = tTable.Data;
      cRowLabels = cTable.RowName;
      tRowLabels = tTable.RowName;
    end

    function [chs, cIDs, retChs] = getActiveChannels(self, app)
        chs = string.empty;
        cIDs = string.empty;
        retChs = string.empty;
            if app.ButtonGroup.Buttons(1).Value
                %test values
                for c = 1:length(app.ChannelAxes.Children)
                    %reset all circles
                    if ~strcmp(app.ChannelAxes.Children(c).FaceColor,app.CenterPanel.BackgroundColor)
                        app.ChannelAxes.Children(c).FaceColor = app.CenterPanel.BackgroundColor;
                    end
                    if app.ChannelAxes.Children(c).Curvature ~= [1 1]
                        app.ChannelAxes.Children(c).Curvature = [1 1];
                    end
                end
                ch = convertCharsToStrings(app.ChannelDropDown.Value);
                if ~strcmp(ch, "")
                    cI = self.getCircleIndexFromChannel(app,str2double(ch));
                    app.ChannelAxes.Children(cI).FaceColor = self.testColor;
                else
                    cI = 0; %no meaning
                end
                %return channel if it exists
                if app.ReturnChannelDropDown.Visible
                    retCh = convertCharsToStrings(app.ReturnChannelDropDown.Value);
                    if ~strcmp(retCh, "-1") && ~strcmp(retCh, "")
                        retI = self.getCircleIndexFromChannel(app,str2double(retCh));
                        %check if it is the same as source channel
                        if retI == cI
                            if app.sourceChChanged
                              app.ReturnChannelDropDown.Value = self.returnChName;
                            else
                                app.ChannelDropDown.Value = "";
                                app.ChannelAxes.Children(retI).Curvature = [0 0];
                            end
                        else
                            app.ChannelAxes.Children(retI).Curvature = [0 0];
                        end
                        app.ChannelAxes.Children(retI).FaceColor = self.testColor;
                    end
                end
                return
            else
                d = app.TableTriggers.Data;
                triggerExp = app.ButtonGroup.SelectedObject.Text;
                for i = 1:size(d,2)
                    if strcmp(triggerExp, d(1,i))
                        %grab channels
                        chList = split(d(3,i),", ");
                        chLength = length(chs);
                        chs(chLength+1:chLength+length(chList),1) = chList;
                        %store config id
                        cIDs(chLength+1:chLength+length(chList),1) = d(2,i);
                        %store if is a return channel
                        retChs(chLength+1:chLength+length(chList),1) = 0;

                        if isprop(self, "returnChName")
                          retChList = split(d(4,i), ", ");
                          for j = 1:length(retChList)
                            r = retChList(j);
                            if ~strcmp(r,self.returnChName)
                              chs(end+1,1) = r;
                              cIDs(end+1,1) = d(2,i);
                              retChs(end+1,1) = 1;
                            end
                          end
                        end
                    end
                end
            end
    end

    
    %should only deal with affected channels to maintain the quick speed
    function channelClickedCallback(self, app,src,~)
        expName = app.ButtonGroup.SelectedObject.Text;
        %get channel number
        for i = 1:length(app.ChannelAxes.Children)
            if isequal(app.ChannelAxes.Children(i), src)
                e = app.ChannelAxes.Children(i).Channel;
                break
            end
        end

        %if testing values, just color it
        if expName == app.firstButtonText
            %un-color the last selected value
            oldCh = str2double(app.ChannelDropDown.Value);
            oldRetCh = str2double(app.ReturnChannelDropDown.Value);

            %set recording channels group (for neuroomega)
            if app.RecordingChannelsButtonGroup.Visible
                recButI = src.Position(2) + 1;
                if ~isequal(app.RecordingChannelsButtonGroup.SelectedObject, app.RecordingChannelsButtonGroup.Children(recButI))
                    %clear old return channel
                    if oldRetCh ~= -1
                        oldRet = self.getCircleIndexFromChannel(app, str2double(app.ReturnChannelDropDown.Value));
                        app.ChannelAxes.Children(oldRet).FaceColor = app.CenterPanel.BackgroundColor;
                        app.ChannelAxes.Children(oldRet).Curvature = [1 1];
                    end

                    app.RecordingChannelsButtonGroup.SelectedObject = app.RecordingChannelsButtonGroup.Children(recButI);
                    self.setNeuroOmegaChannels(app);
                end
            end

            if  (oldRetCh == e) || (~isnan(oldCh) && oldCh == e)
                if app.ReturnChannelDropDown.Visible
                    %unless we want to make it return channel
                    if src.Curvature == [1 1]
                        %make return channel
                        src.Curvature = [0 0];
                        app.ChannelDropDown.Value = ""; %clear channel

                        %clear old return
                        if ~isnan(oldRetCh) && oldRetCh ~= -1
                            oldRet = self.getCircleIndexFromChannel(app, str2double(app.ReturnChannelDropDown.Value));
                            app.ChannelAxes.Children(oldRet).FaceColor = app.CenterPanel.BackgroundColor;
                            app.ChannelAxes.Children(oldRet).Curvature = [1 1];
                        end
                        app.ReturnChannelDropDown.Value = string(e);
                    else
                        %already a return channel
                        src.FaceColor = app.CenterPanel.BackgroundColor;
                        src.Curvature = [1 1];
                        if strcmp(app.NeurostimulatorDropDown.Value, app.neuroOmega)
                            app.ReturnChannelDropDown.Value = "-1";
                        else
                            %cortec
                            oldRet = self.getCircleIndexFromChannel(app, str2double(app.ReturnChannelDropDown.Value));
                            app.ChannelAxes.Children(oldRet).FaceColor = app.CenterPanel.BackgroundColor;
                            app.ChannelAxes.Children(oldRet).Curvature = [1 1];
                            app.ReturnChannelDropDown.Value = "";
                        end
                    end
                end
            else
                %reset old one
                if ~isnan(oldCh)
                    oldC = self.getCircleIndexFromChannel(app, oldCh);
                    app.ChannelAxes.Children(oldC).FaceColor = app.CenterPanel.BackgroundColor;
                end
                %color new one
                src.FaceColor = self.testColor;
                app.ChannelDropDown.Value = string(e);
            end
            return
        end

        %find config id from current color of channel
        configFound = false;
        for i = 1:length(app.SinglePulseAxes.ColorOrder)
            if isequal(app.SinglePulseAxes.ColorOrder(i,:), src.FaceColor)
                n = i;
                configFound = true;
                break
            end
        end
        d = app.TableTriggers.Data;
        expCols = find(strcmp(d(1,:),expName));
        returnCh = false;
        
        %available config ids for this expression
        cIDs = [];
        for i = 1:size(d,2)
            if strcmp(expName, d(1,i))
                cIDs(end+1) = str2double(d(2,i));
            end
        end
        cIDs = sort(cIDs);
        colCounter = 1;

        if configFound
            %find where all expressions are true
            nextIDExists = false;
            chFound = 0;
            %array locs of same configs
            repConfigs = find(cIDs==n);
            for i = 1:length(expCols)
                cID = str2double(d(2,expCols(i)));
                if cID == n && chFound==0
                    %return ch or normal ch?
                    if src.Curvature == [1 1]
                        %normal ch
                        %channel might be in there
                        chList = split(d(3,expCols(i)),", ");
                        for j = 1:length(chList)
                            if str2double(chList(j)) == e
                                chList(j) = []; %remove from list
                                app.TableTriggers.Data(3,expCols(i)) = app.formatChList(chList);
                                chFound = i;
                                break
                            end
                        end
                    else
                        %return ch
                        if isprop(self, "returnChName")
                          returnCh = true;
                          retChList = split(d(4,expCols(i)),", ");
                          for j = 1:length(retChList)
                              if str2double(retChList(j)) == e
                                  if length(retChList) > 1
                                    retChList(j) = []; %remove from list
                                    app.TableTriggers.Data(4,expCols(i)) = app.formatChList(retChList);
                                  else
                                    app.TableTriggers.Data(4,expCols(i)) = self.returnChName;
                                  end
                                  chFound = i;
                                  break
                              end
                          end
                        end                        
                    end
                elseif configFound && cID == n && colCounter < length(repConfigs)
                    %for multiple columns with same config (neuroomega)
                    colCounter = colCounter + 1;
                    nextIDExists = true;
                    newConfigID = cID;
                elseif cID == n+1 && ~nextIDExists
                    %color channel of next id
                    nextIDExists = true;
                    newConfigID = cID;
                end
            end
            if ~nextIDExists
                if isprop(self, "returnChName")
                    %toggle return channel
                    if returnCh
                        src.Curvature = [1 1];
                        src.FaceColor = app.CenterPanel.BackgroundColor;
                        return
                    else
                        src.Curvature = [0 0];
                        colCounter = 1;
                        newConfigID = cIDs(colCounter);
                    end
                    returnCh = ~returnCh;
                else
                    src.FaceColor = app.CenterPanel.BackgroundColor;
                    return
                end
            end
        else
            %want to add channel to table
            newConfigID = cIDs(1);
        end
        tempCounter = 1;
        for i = 1:length(expCols)
            if newConfigID == str2double(d(2,expCols(i)))
                if tempCounter < colCounter
                    %skip this column
                    tempCounter = tempCounter + 1;
                    continue;
                end
                if returnCh
                    %reset old return channel (only for neuro omega)
                    if self.returnChName == "-1" %assuming if -1, we want to only have one return ch (maybe very poor assumption)
                        oldChNum = str2double(d(4,expCols(i)));
                        if ~isnan(oldChNum) && oldChNum ~= -1
                            oldCircI = self.getCircleIndexFromChannel(app,str2double(d(4,expCols(i))));
                            app.ChannelAxes.Children(oldCircI).FaceColor = app.CenterPanel.BackgroundColor;
                            app.ChannelAxes.Children(oldCircI).Curvature = [1 1];
                        end
                        app.TableTriggers.Data(4,expCols(i)) = string(e);
                    else
                        oldChList = split(d(4,expCols(i)), ", ");
                        app.TableTriggers.Data(4,expCols(i)) = app.formatChList([oldChList; string(e)]);
                    end
                    %color new return channel
                    src.FaceColor = app.SinglePulseAxes.ColorOrder(newConfigID,:);
                else
                    %add to column
                    oldChList = split(d(3,expCols(i)), ", ");
                    chList = [oldChList; string(e)];
                    ch = expCols(i);
                    src.FaceColor = app.SinglePulseAxes.ColorOrder(newConfigID,:);
                    app.TableTriggers.Data(3,ch) = app.formatChList(chList);
                end
                break
            end
        end
    end
        

    %universal functions used by all stimulators
    function circI = getCircleIndexFromChannel(~, app, ch)
      for i = 1:length(app.ChannelAxes.Children)
        if app.ChannelAxes.Children(i).Channel == ch
          circI = ch;
          return
        end
      end
    end
    function changeParameterVisibility(self, app)
      elements = self.elements;
      count = 1;
      for i = self.visibleElements
          while i ~= count %count needs to catch up, and turn off things
              changeVisibility(count, 'off');
              count = count + 1;
          end
          changeVisibility(i, 'on');
          count = count + 1;
      end
      extraEls = length(elements) - self.visibleElements(end) - 1;
      if extraEls > 0
          startI = self.visibleElements(end) + 1;
          for i = startI: startI + extraEls
              changeVisibility(i, 'off');
          end
      end

      function changeVisibility(i, toggleStr)
        for j = 1:2
          if ~isempty(elements{i,j})
            if elements{i, j}.Visible ~= toggleStr
              elements{i, j}.Visible = toggleStr;
            end
          end
        end
      end
    end
    function shiftElements(self, elIndices, factor)
      y = self.dy * factor;
      for el = elIndices
        for j = 1:2
          if ~isempty(self.elements{el,j})
            self.elements{el,j}.Position(2) = self.elements{el,j}.Position(2) - y;
          end
        end
      end
    end
    function expres = createExpression(~, app, trig)
      if trig
        expresStart = convertCharsToStrings(app.TriggerExpressionEditField.Value);
        expresOp = convertCharsToStrings(app.TriggerDropDown.Value);
        expresEnd = int2str(app.TriggerSpinner.Value);
      else
        expresStart = convertCharsToStrings(app.AbortExpressionEditField.Value);
        expresOp = convertCharsToStrings(app.AbortDropDown.Value);
        expresEnd = int2str(app.AbortSpinner.Value);
      end
      expres = expresStart + expresOp + expresEnd;
    end
    function checkCh(~, ch)
      if strcmp(ch, "")
        errordlg('Specify a channel for stimulation.','Channel Error');
      end
    end
    function [rNames, cNames, lineArr] = getLabels(self, lineArr)
        lineArr = lineArr(4:end);
        [rNames, lineArr] = self.getLabel(lineArr);
        [cNames, lineArr] = self.getLabel(lineArr);
    end

    function [names, lineArr] = getLabel(~, lineArr)
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
  end
end