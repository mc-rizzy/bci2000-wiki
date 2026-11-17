# ///////////////////////////////////////////////////////////////////////////
# $Id$
# Author: mellinger@neurotechcenter.org
# Description: A demo Operator module in Python
#
# $BEGIN_BCI2000_LICENSE$
#
# This file is part of BCI2000, a platform for real-time bio-signal research.
# [ Copyright (C) 2000-2022: BCI2000 team and many external contributors ]
#
# BCI2000 is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# BCI2000 is distributed in the hope that it will be useful, but
#                         WITHOUT ANY WARRANTY
# - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#
# $END_BCI2000_LICENSE$
# ///////////////////////////////////////////////////////////////////////////
"""A demo Operator module using the OperatorLib bindings in 
   BCI2000Operator.py
"""

import sys
import os
import numpy as np
import json

from PyQt6 import QtCore, QtGui, QtWidgets
import pyqtgraph as pg

bci2000_dir = os.path.dirname(__file__) + '/../../../../..'
sys.path.append(bci2000_dir + '/tools/python')
sys.path.append(bci2000_dir + '/prog')

from BCI2000Operator import BCI2000Operator

class BCIOperatorLink(QtCore.QObject, BCI2000Operator):
    """ BCIOperatorLink class
        This class mainly translates callbacks from the BCI2000Operator class
        into Qt signals which can then be picked up by slots in the main window.
        Main window slots run in the GUI thread, which is required to perform actions
        on GUI elements.
    """
    # Instantiate signals as class objects
    signal_OnInitializeVis = QtCore.pyqtSignal(str, str)
    signal_OnVisSignal = QtCore.pyqtSignal(str, np.ndarray)
    signal_OnLog = QtCore.pyqtSignal(str)
    signal_OnSuspend = QtCore.pyqtSignal()
    signal_OnConfigChanged = QtCore.pyqtSignal(str)
    signal_OnQuit = QtCore.pyqtSignal()
    
    def __init__(self):
        super(BCIOperatorLink, self).__init__(None)
        self._visProperties = dict()
        self._parameters = dict()

    # Overridden callback methods defined in the superclass
    def OnParameterJson(self, json_data):
        param = json.loads(json_data)
        self._parameters[param['Name']] = param
        
    def OnLogMessage(self, message):
        self.signal_OnLog.emit("Log: " + message)
        
    def OnWarningMessage(self, message):
        self.signal_OnLog.emit("Warning: " + message)
        
    def OnErrorMessage(self, message):
        self.signal_OnLog.emit("Error: " + message)
        
    def OnDebugMessage(self, message):
        self.signal_OnLog.emit("Debug: " + message)
        
    def OnScriptError(self, message):
        self.signal_OnLog.emit("ScriptError: " + message)

    def OnSuspend(self):
        self.signal_OnSuspend.emit()
        
    def OnVisProperty(self, visID, cfgID, value):
        self._handle_vis_property(visID, cfgID, value, False)
        
    def OnVisPropertyMessage(self, visID, cfgID, value):
        self._handle_vis_property(visID, cfgID, value, True)
        
    def OnInitializeVis(self, visID, kind):
        self.signal_OnInitializeVis.emit(visID, kind)
    
    def OnVisSignal(self, visID, signal):
        self.signal_OnVisSignal.emit(visID, signal)
        
    def OnQuitRequest(self):
        self.signal_OnQuit.emit()
        return 'going to quit application'

    # Methods dealing with visualization properties
    def _handle_vis_property(self, visID, cfgID, value, is_message_defined):
        if not visID in self._visProperties:
            self._visProperties[visID] = dict()
        if not cfgID in self._visProperties[visID]:
            self._visProperties[visID][cfgID] = ("", False)
        entry = self._visProperties[visID][cfgID]
        if is_message_defined or not entry[1]:
            self._visProperties[visID][cfgID] = (value, is_message_defined)
        self.signal_OnConfigChanged.emit(visID)
    
    def get_vis_config_str(self, visID, cfgID):
        if not visID in self._visProperties:
            return ''
        if not cfgID in self._visProperties[visID]:
            return ''
        return self._visProperties[visID][cfgID][0]
        
    def get_vis_config_num(self, visID, cfgID):
        s = self.get_vis_config_str(visID, cfgID)
        try:
            return int(s)
        except ValueError:
            return float('nan')

class MainWindow(QtWidgets.QWidget):
    """ MainWindow class
        This is the demo application's main window, providing buttons for user interaction,
        and visualization of the source signal.
        MainWindow maintains an instance of DemoOperator as an attribute, and interacts
        with it through the Qt signal/slot mechanism.
    """
    def __init__(self):
        super().__init__()
        
        self._textEdit = QtWidgets.QTextEdit(self)
        layout1 = QtWidgets.QVBoxLayout()
        layout2 = QtWidgets.QHBoxLayout()
        
        self._loadParams = QtWidgets.QPushButton('Load')
        layout2.addWidget(self._loadParams)
        self._loadParams.clicked.connect(self.on_load_params)
        
        self._setConfig = QtWidgets.QPushButton('Set Config')
        layout2.addWidget(self._setConfig)
        self._setConfig.clicked.connect(self.on_set_config)
        
        self._start = QtWidgets.QPushButton('Start')
        layout2.addWidget(self._start)
        self._start.setEnabled(False)
        self._start.clicked.connect(self.on_start)
        
        self._quit = QtWidgets.QPushButton('Quit')
        layout2.addWidget(self._quit)
        self._quit.clicked.connect(self.on_quit)
        
        layout1.addLayout(layout2)
        layout1.addWidget(self._textEdit)
        self.setLayout(layout1)

        self.num_channels = 0
        self.n_samples = 0
        
        self.setWindowTitle('BCI2000 Python Example')
        self.resize(800,600)

        self.init_bci()

    def init_bci(self):
        '''Instantiate the operator link, and start up the BCI2000 system'''
        bci = BCIOperatorLink()
        self._bci = bci
        self.init_signals()
        bci.TelnetListen("localhost:3999")
        bci.ExecuteScript("cd \"" + bci2000_dir + "/prog\"")
        
        # Define core modules, their generic names, and their associated listening ports
        bci.Startup("localhost SignalSource:4000 SignalProcessing:4001 Application:4002")

        # Start up the actual core modules -- this is where you select which experiment to
        # run
        bci.ExecuteScript("Start Executable SignalGenerator --local")
        bci.ExecuteScript("Start Executable SpectralSignalProcessing --local")
        bci.ExecuteScript("Start Executable CursorTask --local")
        
        bci.ExecuteScript("Wait for Connected 10")

    def init_signals(self):
        ''' Connect the operator link's Qt signals to our Qt slots '''
        self._bci.signal_OnLog.connect(self.OnLog)
        self._bci.signal_OnInitializeVis.connect(self.OnInitializeVis)
        self._bci.signal_OnVisSignal.connect(self.OnVisSignal)
        self._bci.signal_OnSuspend.connect(self.OnSuspend)
        self._bci.signal_OnConfigChanged.connect(self.OnConfigChanged)
        self._bci.signal_OnQuit.connect(self.on_quit)

    def on_load_params(self):
        files, _filter = QtWidgets.QFileDialog.getOpenFileNames(
            self,
            'Choose parameter files to load',
            '../parms',
            '*.prm'
        )
        for filename in files:
            self._start.setEnabled(False)
            self._bci.ExecuteScript("Load Parameterfile \"" + filename + "\"")

    def on_set_config(self):
        if self._bci.SetConfig():
            self._start.setEnabled(True)
        
    def on_start(self):
        if self._bci.StartRun():
            self._start.setEnabled(False)
        
    def on_quit(self):
        self.close()
        
    def closeEvent(self, event):
        # Properly shutdown the BCI library and avoid dangling callbacks
        self._bci.halt()
        event.accept()

    def _check_signal_size(self, signal: np.ndarray):
        '''Here we apply any changes to the signal and plot object'''
        channels = signal.shape[0]
        
        if self.num_channels != channels:
            self.num_channels = channels
            # Initialize data buffer (channels x samples)
            self.data = np.zeros((self.num_channels, self.n_samples))
            # Create curves with vertical offsets for stacking channels
            if hasattr(self, 'plot_widget') and hasattr(self, 'curves'):
                for curve in self.curves:
                    self.plot_widget.removeItem(curve)
            self.curves = []
            colors = ['r', 'g', 'b', 'y']
            self.channel_spacing = 400
            
            for i in range(self.num_channels):
                color = colors[i % len(colors)]
                curve = self.plot_widget.plot(pen=pg.mkPen(color=color, width=1.5))
                self.curves.append(curve)

            if self.plot_title != '':
                self.plot_widget.setLabel('top', self.plot_title)

    # These are slots so they will be run in the GUI thread
    # but may be triggered from a secondary thread
    @QtCore.pyqtSlot(str)
    def OnLog(self, message):
        self._textEdit.append(message)

    @QtCore.pyqtSlot(str, str)
    def OnInitializeVis(self, visID, kind):
        if kind == 'Graph' and visID == 'SRCD':
            self.plot_widget = pg.PlotWidget()
            self.layout().addWidget(self.plot_widget)
            self.plot_widget.setLabel('bottom', 'Time', 's')
            self.plot_widget.setLabel('left', 'Channels (Stacked)')
            self.plot_widget.enableAutoRange(x=True, y=True)

    @QtCore.pyqtSlot(str)
    def OnConfigChanged(self, visID):
        if visID == 'SRCD':
            # Trigger reconfiguration in _check_signal_size()
            self.num_channels = 0
            # Read all configuration values into member variables, so they may be applied later
            self.n_samples = self._bci.get_vis_config_num(visID, BCI2000Operator.CfgID.NumSamples)
            sample_unit = self._bci.get_vis_config_str(visID, BCI2000Operator.CfgID.SampleUnit)
            if sample_unit != '':
                sample_unit = pg.siEval(sample_unit)
                self.time_axis = np.linspace(0, self.n_samples * sample_unit, self.n_samples)
            self.plot_title = self._bci.get_vis_config_str(visID, BCI2000Operator.CfgID.WindowTitle)

    @QtCore.pyqtSlot(str, np.ndarray)
    def OnVisSignal(self, visID, signal):
        if visID == 'SRCD':
            self._check_signal_size(signal)
            new_samples = signal.shape[1]

            # Roll data left and append new samples
            self.data = np.roll(self.data, -new_samples, axis=1)
            self.data[:, -new_samples:] = signal

            # Update each channel graph with a vertical stacking offset
            for i in range(self.num_channels):
                offset_y = (self.num_channels - 1 - i) * self.channel_spacing
                offset_y = self.data[i, :] + offset_y
                self.curves[i].setData(self.time_axis, offset_y)
        
    @QtCore.pyqtSlot()
    def OnSuspend(self):
        self._start.setEnabled(True)

app = QtWidgets.QApplication(sys.argv)
win = MainWindow()
win.show()
sys.exit(app.exec())

