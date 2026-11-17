# ///////////////////////////////////////////////////////////////////////////
# $Id$
# Author: mellinger@neurotechcenter.org
# Description: Python bindings for the BCI2000OperatorLib library.
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
"""Python bindings for the BCI2000OperatorLib library.

This module provides a BCI2000Operator class that allows control of
the BCI2000 Operator module.
It must reside in the same directory as the BCI2000OperatorLib library, which
is usually located the BCI2000/prog directory.
For documentation, see
   http://www.bci2000.org/mediawiki/index.php/Technical_Reference:Operator_Library
   http://www.bci2000.org/mediawiki/index.php/User_Reference:Operator_Module_Scripting
For a code example, see the end of this file.

"""

from __future__ import absolute_import
from enum import IntEnum
import os
import sys
import inspect
import string
import copy
import ctypes
import numpy as np

__all__ = [
	'BCI2000Operator',
	'BCI2000OperatorError', 'BCI2000OperatorRuntimeError',
]

class BCI2000OperatorError(Exception):
    pass
class BCI2000OperatorRuntimeError(RuntimeError, BCI2000OperatorError):
    pass

class BCI2000Operator(object):
    """BCI2000 Operator object.

    This class allows control of the BCI2000 Operator, and receiving of
    callback notifications on messages, and visualizations.
    For documentation, see
     http://doc.bci2000.org/index.php/Programming_Reference:BCI2000Remote_Class
     http://doc.bci2000.org/index.php/User_Reference:Operator_Module_Scripting
    """
    
    class CfgID(IntEnum):
        """An enum for configuration IDs.
        
        Configuration IDs are used to configure visualizations,
        and manage user settings.
        
        Visualization configuration is received through the
        Operator object's OnVisProperty and OnVisPropertyMessage
        callbacks.
        """

        None_ = -5

        Top = -4
        Left = -3
        Width = -2
        Height = -1
        WindowTitle = 1

        MinValue = 2 # in raw value units
        MaxValue = 3
        NumSamples = 4 # number of samples in x direction
        Reserved_1 = 5 # was XAxisLabel
        SignalIsStream = 6 # true: signal is a continuous stream of values, false: signal is discontinuous
        ChannelGroupSize = 7 # number of channels being displayed on the same YTick
        GraphType = 8
        # Graph types
        Polyline = 9
        # Polyline options
        ShowBaselines = 10 # true/false
        ChannelColors = 11 # ColorList object

        Field2d = 12

        # Units (unit gain with physical unit, e.g. "1ms")
        SampleUnit = 13
        ChannelUnit = 14
        ValueUnit = 15

        # Memo options
        NumLines = 16

        # Label lists (send LabelList objects)
        ChannelLabels = 17 # labels displayed with channels
        GroupLabels = 18 # labels displayed with channel groups if there are multiple channels in a group
        # Tick options (whether to draw ticks and labels)
        ShowXTicks = 19 # true/false
        ShowYTicks = 20 # true/false
        # Marker lists (send LabelList objects containing marker defs of the form "<label>|<color>|<style>|<width>")
        XAxisMarkers = 21 # <color> may be one of the color names defined at https://www.w3.org/TR/SVG/types.html#ColorKeywords
        YAxisMarkers = 22 # <style> may be composed of . and -, defining a line style; any of the elements may be omitted,
                      # <width> is relative to the marker's tick at the axis
        # Miscellaneous
        ShowSampleUnit = 23 # true/false
        ShowChannelUnit = 24 # true/false
        ShowValueUnit = 25 # true/false
        SampleOffset = 26 # number of samples to add when displaying x unit

        Visible = 27 # true/false
        InvertedDisplay = 28 # true/false
        # Set filter property to "off" to disable a filter
        HPFilter = 29 # e.g., "30Hz"
        LPFilter = 30
        NotchFilter = 31

        ShowNumericValues = 32 # whether to display the current value as a number
        AutoScale = 33 # auto scale option, true/false
        ChannelStyles = 34 # a string containing characters to plot at sample points, '-' for line
        FixedScale = 35 # true: user cannot adjust value scaling
        ValueColors = 36 # a string in the form "[ { <val1> 0xRRGGBB } ...]" with rgb values ranging from 0 to 255
        FontSize = 37 # in points
        WindowFrame = 38 # display a window frame, true/false

        ValueUnits = 39 # allow for different units in different channels (send a ValueList<std::string>, by channel index)
        MinValues = 40  # ValueList<double>, allow for different min values in different channels (by channel index)
        MaxValues = 41  # ValueList<double>, allow for different max values in different channels (by channel index)

        PlacementVis = 42  # put visualization into a container window, give ContainerVisID as a string argument
        PlacementRow = 43  # 1-based index of row in container window
        PlacementCol = 44  # 1-based index of column in container window
        PlacementHeight = 45 # number of rows to extend over
        PlacementWidth = 46 # number of columns to extend over
        PlacementMargin = 47 # margin between matrix entries
        PlacementColor = 48 # color of margin between matrix entries

        ShowStatusBar = 49 # in graph, true/false

        ShowChannelList = 50 # in graph, true/false
        ChannelsVisible = 51 # MappedProperty<bool> object (by channel name)
        ValueUnitsVisible = 53 # MappedProperty<bool> object (by channel name)
        ChannelUserScaling = 54 # MappedProperty<float> object (by channel name)
        CustomColors = 55 # ColorList object
        UserScaling = 56 # log2(magnificationFactor), applied globally
        UserSampleZoom = 57 # log2(magnificationFactor)

        Docked = 58 # whether the window is docked, true/false
        MarkerChannelStyles = 59 # for each marker channel, whether to be displayed by value or by change
        DisplayValues = 60,
        DisplayChanges = 61

        ShowToolBar = 62 # in graph, true/false
        ShowGrid = 63 # in polyline graph, true/false
        ColorTheme = 64 # in polyline graph, 0 = auto, 1 = light, 2 = dark

        XAxisLogScale = 65 # log scale factor on x axis, 0 for linear
        YAxisLogScale = 66 # log scale factor on y axis, 0 for linear

        ActivateBuffer = 67 # don't apply Cfgs immediately but buffer them, set to 0 to apply buffered Cfgs

        ClickingTraceSelectsChannel = 68 # in graph, true/false
        ActivateFilters = 69 # in graph, true/false
        FixedSampleCount = 70 # in graph, true/false

        DftType = 71 # in spectrum, DftLinear or DftConstantQ
        DftLinear = 72
        DftConstantQ = 73
        Reserved_2 = 74 # was DftQFactor
        DftDecimate = 75 # in spectrum
        DftNumBins = 76 # in spectrum, number of spectral bins

        Dependants = 77 # list of dependant visIDs

    
    class BCIEvent(IntEnum):
        BCI_None = -1

        BCI_OnSystemStateChange = 0
        BCI_OnCoreInput = 1

        BCI_OnStartup = 2
        BCI_OnConnect = 3
        BCI_OnSetConfig = 4
        BCI_OnStart = 5
        BCI_OnSuspend = 6
        BCI_OnStopRun = BCI_OnSuspend  # Alias
        BCI_OnResume = 7
        BCI_OnStartRun = 8
        BCI_OnShutdown = 9

        BCI_OnLogMessage = 10
        BCI_OnWarningMessage = 11
        BCI_OnErrorMessage = 12
        BCI_OnDebugMessage = 13
        BCI_OnNoteMessage = 14

        BCI_OnParameter = 15
        BCI_OnParameterJson = 16
        BCI_OnState = 17
        BCI_OnVisPropertyMessage = 18
        BCI_OnVisProperty = 19

        BCI_OnInitializeVis = 20
        BCI_OnVisMemo = 21
        BCI_OnVisSignal = 22
        BCI_OnVisSignalHandle = 23
        BCI_OnVisBitmap = 24
        BCI_OnVisVideoFrame = 25

        BCI_OnUnknownCommand = 26
        BCI_OnScriptError = 27
        BCI_OnScriptHelp = 28

        BCI_OnQuitRequest = 29

        BCI_OnNextFilePart = 30

        BCI_NumEvents = 31

    class BCIResult(IntEnum):
        BCI_Handled = 1
        BCI_NotHandled = 0
            
    # Constructor/destructor

    def __init__(self):
        pyfile = inspect.getfile(inspect.currentframe())
        bcidir = os.path.dirname(os.path.realpath(pyfile))
        bcilib = os.path.join(bcidir, "OperatorLib")
        LoadLibrary = ctypes.cdll.LoadLibrary
        if sys.platform.startswith('win'):
            dylib_extension = ".dll"
            LoadLibrary = ctypes.windll.LoadLibrary
        elif sys.platform.startswith('linux'):
            dylib_extension = ".so"
        elif sys.platform.startswith('darwin'):
            dylib_extension = ".dylib"
        else:
            dylib_extension = ""
        for platform_inflection in [ '64', '.64', '.64bit', '.arm64', '.x86_64', '.amd64', '32', '.32', '.32bit', '.x86' ]:
            try: self._lib = LoadLibrary(bcilib + platform_inflection + dylib_extension)
            except: pass
            else: break
        else: # this will run if the `for` loop did not `break`
            self._lib = LoadLibrary(bcilib + dylib_extension) # no exception-catching this time
            
        self._lib.BCI_Initialize.argtypes = []
        self._lib.BCI_Initialize.restype = ctypes.c_int
        self._lib.BCI_Dispose.argtypes = []
        self._lib.BCI_Dispose.restype = ctypes.c_int
        self._lib.BCI_AllocateString.argtypes = [ctypes.c_char_p]
        self._lib.BCI_AllocateString.restype = ctypes.c_char_p

        if sys.platform.startswith("win"):
            CALLBACK = ctypes.WINFUNCTYPE
        else:
            CALLBACK = ctypes.CFUNCTYPE
            
        self._VoidCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
        )

        self._IntCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_int,                     # value
        )

        self._StrCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # message
        )
        
        self._StrStrCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # visID
            ctypes.c_char_p,                  # message
        )
        
        self._PStrCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.POINTER(ctypes.c_char_p),  # result
        )
        
        self._StrPStrPStrCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # input
            ctypes.POINTER(ctypes.c_char_p),  # output
            ctypes.POINTER(ctypes.c_char_p),  # error
        )

        self._StrIntStrCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # visID
            ctypes.c_int,                     # cfgID
            ctypes.c_char_p,                  # value
        )

        self._StrIntIntPFloatCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # visID
            ctypes.c_int,                     # channels
            ctypes.c_int,                     # elements
            ctypes.POINTER(ctypes.c_float),   # signal data
        )

        self._StrIntIntPUShortCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # visID
            ctypes.c_int,                     # channels
            ctypes.c_int,                     # elements
            ctypes.POINTER(ctypes.c_ushort),  # signal data
        )

        self._StrIntIntPUIntCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_char_p,                  # visID
            ctypes.c_int,                     # channels
            ctypes.c_int,                     # elements
            ctypes.POINTER(ctypes.c_uint),    # signal data
        )

        self._WatchCallback = CALLBACK(
            ctypes.c_int,                     # return value
            ctypes.c_void_p,                  # refdata
            ctypes.c_int,                     # messages count
            ctypes.POINTER(ctypes.c_char_p),  # messages
        )

        self._func_refs = dict()
        self._trampoline_void_callback = self._VoidCallback(BCI2000Operator._trampoline_void)
        self._trampoline_int_callback = self._IntCallback(BCI2000Operator._trampoline_int)
        self._trampoline_str_callback = self._StrCallback(BCI2000Operator._trampoline_str)
        self._trampoline_str_str_callback = self._StrStrCallback(BCI2000Operator._trampoline_str_str)
        self._trampoline_pstr_callback = self._PStrCallback(BCI2000Operator._trampoline_pstr)
        self._trampoline_str_pstr_pstr_callback = self._StrPStrPStrCallback(BCI2000Operator._trampoline_str_pstr_pstr)
        self._trampoline_str_int_str_callback = self._StrIntStrCallback(BCI2000Operator._trampoline_str_int_str)
        self._trampoline_str_int_int_pfloat_callback = self._StrIntIntPFloatCallback(BCI2000Operator._trampoline_str_int_int_data)
        self._trampoline_str_int_int_pushort_callback = self._StrIntIntPUShortCallback(BCI2000Operator._trampoline_str_int_int_data)
        self._trampoline_str_int_int_puint_callback = self._StrIntIntPUIntCallback(BCI2000Operator._trampoline_str_int_int_data)
        self._trampoline_watch_callback = self._WatchCallback(BCI2000Operator._trampoline_watch)
            
        self._lib.BCI_SetCallback.argtypes = [
            ctypes.c_long,      # event
            ctypes.c_void_p,    # callback
            ctypes.c_void_p,    # refdata
        ]
        self._lib.BCI_SetCallback.restype = ctypes.c_int

        if not self._lib.BCI_Initialize():
            raise BCI2000OperatorRuntimeError
            
        self._register_callbacks()

    def __del__(self):
        self.halt()
            
    def halt(self):
        try:
            self._unregister_callbacks()
            self._lib.BCI_Shutdown()
            self._lib.BCI_Dispose()
        except:
            pass

    def _register_callbacks(self):

        callbacks = [
            'OnSystemStateChange',
            'OnCoreInput',
            'OnStartup',
            'OnConnect',
            'OnSetConfig',
            'OnStart',
            'OnSuspend',
            'OnResume',
            'OnStartRun',
            'OnShutdown',
        ]
        self.__register_callbacks(callbacks, self._trampoline_void_callback)
            
        callbacks = [
            'OnLogMessage',
            'OnWarningMessage',
            'OnErrorMessage',
            'OnDebugMessage',
            'OnNoteMessage',
            'OnScriptError',
            'OnStartRun',
            'OnParameter',
            'OnParameterJson',
            'OnState',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_callback)
        
        callbacks = [
            'OnUnknownCommand',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_pstr_pstr_callback)
                
        callbacks = [
            'OnScriptHelp',
            'OnQuitRequest',
        ]
        self.__register_callbacks(callbacks, self._trampoline_pstr_callback)

        callbacks = [
            'OnVisProperty',
            'OnVisPropertyMessage',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_int_str_callback)

        callbacks = [
            'OnInitializeVis',
            'OnVisMemo',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_str_callback)

        callbacks = [
            'OnVisSignal',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_int_int_pfloat_callback)

        callbacks = [
            'OnVisBitmap',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_int_int_pushort_callback)

        callbacks = [
            'OnVisVideoFrame',
        ]
        self.__register_callbacks(callbacks, self._trampoline_str_int_int_puint_callback)

        callbacks = [
            'OnNextFilePart',
        ]
        self.__register_callbacks(callbacks, self._trampoline_int_callback)

    def __register_callbacks(self, callbacks, trampoline_callback):
        for s in callbacks:
            enumval = self.BCIEvent['BCI_' + s]
            func = getattr(self, s)
            func = ctypes.py_object(func)
            self._func_refs[enumval] = func
            
            ok = self._lib.BCI_SetCallback(
                enumval,
                trampoline_callback,
                ctypes.cast(
                    ctypes.pointer(func),
                    ctypes.c_void_p,
                ),
            )
            if not ok:
                raise BCI2000OperatorRuntimeError("Could not register callback " + s)

    def _unregister_callbacks(self):
        for i in range(0, self.BCIEvent.BCI_NumEvents):
            self._lib.BCI_SetCallback(i, 0, 0)
            

    # --------------------------------------------------------------
    # Static callbacks
    #
    # These are the function pointers seen by the DLL.
    # --------------------------------------------------------------
    @staticmethod
    def _trampoline_void(refdata):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        try:
            fn()
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_int(refdata, value):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        try:
            fn(value)
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_str(refdata, message):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        message = message.decode("utf-8")
        try:
            fn(message)
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_str_str(refdata, visID, message):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        visID = visID.decode("utf-8")
        message = message.decode("utf-8")
        try:
            fn(visID, message)
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_str_int_str(refdata, visID, cfgID, value):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        visID = visID.decode("utf-8")
        value = value.decode("utf-8")
        try:
            fn(visID, cfgID, value)
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_str_int_int_data(refdata, visID, channels, elements, signal):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        visID = visID.decode("utf-8")

        n = channels * elements
        signal = np.ctypeslib.as_array(
            signal,
            shape=(n,),
        ).reshape(channels, elements)
        signal = signal.copy() # detach from library allocated memory

        try:
            fn(visID, signal)
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled

    @staticmethod
    def _trampoline_str_pstr_pstr(refdata, input, output, error):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        obj = fn.__self__
        input = input.decode("utf-8")
        try:
            outputStr = ''
            errorStr = ''
            [ok, outputStr, errorStr] = fn(input)
            if outputStr:
                output[0] = obj._lib.BCI_AllocateString(obj._string_arg(outputStr))
            if errorStr:
                error[0] = obj._lib.BCI_AllocateString(obj._string_arg(errorStr))
            if ok:
                return fn.__self__.BCIResult.BCI_Handled
            return fn.__self__.BCIResult.BCI_NotHandled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_pstr(refdata, output):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        obj = fn.__self__
        try:
            outputStr = fn()
            if outputStr:
                output[0] = obj._lib.BCI_AllocateString(obj._string_arg(outputStr))
            return fn.__self__.BCIResult.BCI_NotHandled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            
    @staticmethod
    def _trampoline_watch(refdata, count, messages):
        fn = ctypes.cast(
            refdata,
            ctypes.POINTER(ctypes.py_object),
        ).contents.value
        message_list = []
        for i in range(count):
            message = messages[i]
            message = message.decode("utf-8")
            message = message.rstrip()
            message = message.split("\t")
            message_list.append(message)
            
        try:
            fn(message_list)
            return fn.__self__.BCIResult.BCI_Handled

        except Exception:
            import traceback
            traceback.print_exc()
            return fn.__self__.BCIResult.BCI_NotHandled
            

    # --------------------------------------------------------------
    # Virtual callbacks, override in subclasses
    # --------------------------------------------------------------
    def OnSystemStateChange(self):
        pass

    def OnCoreInput(self):
        pass
    
    def OnStartup(self):
        pass
        
    def OnConnect(self):
        pass
    
    def OnSetConfig(self):
        pass
        
    def OnStart(self):
        pass

    def OnSuspend(self):
        pass

    def OnResume(self):
        pass

    def OnStartRun(self, filename:str):
        pass

    def OnShutdown(self):
        pass

    def OnLogMessage(self, message):
        print("Log: ", message)
        
    def OnWarningMessage(self, message):
        print("Warning: ", message)
        
    def OnErrorMessage(self, message):
        print("Error: ", message)
        
    def OnDebugMessage(self, message):
        print("Debug: ", message)
        
    def OnScriptError(self, message):
        print("ScriptError: ", message)

    def OnNoteMessage(self, message: str):
        pass

    def OnParameter(self, paramline: str):
        pass

    def OnParameterJson(self, paramdef: str):
        pass

    def OnState(self, paramline: str):
        pass
        
    def OnUnknownCommand(self, command: str):
        return [False, '', '']
        
    def OnScriptHelp(self):
        return ''

    def OnQuitRequest(self):
        return ''
        
    def OnVisProperty(self, visID: str, cfgID: int, value: str):
        pass
        
    def OnVisPropertyMessage(self, visID: str, cfgID: int, value: str):
        pass
        
    def OnInitializeVis(self, visID: str, kind: str):
        pass
        
    def OnVisMemo(self, visID: str, text: str):
        pass
        
    def OnVisSignal(self, visID: str, signal: np.ndarray):
        pass
        
    def OnVisBitmap(self, visID: str, bitmap: np.ndarray):
        pass
        
    def OnVisVideoFrame(self, visID: str, frame: np.ndarray):
        pass
        
    def OnNextFilePart(self, part):
        pass

    # --------------------------------------------------------------
    # Plain Methods
    # --------------------------------------------------------------
    def GetInfo(self):
        return self._get_string_result(self._lib.BCI_GetInfo)

    def PutParameter(self, line):
        return self._lib.BCI_PutParameter(self._string_arg(line))
        
    def GetParameter(self, index):
        return self._get_string_result(self._lib.BCI_GetParameter, index, mayfail=True)

    def PutParameterJson(self, data):
        return self._lib.BCI_PutParameterJson(self._string_arg(data))
        
    def GetParameterJson(self, index):
        return self._get_string_result(self._lib.BCI_GetParameterJson, index, mayfail=True)

    def GetSamplingRate(self):
        return self._lib.BCI_GetSamplingRate()
        
    def GetSampleBlockSize(self):
        return self._lib.BCI_GetSampleBlockSize()
        
    def GetStream(self, index):
        return self._get_string_result(self._lib.BCI_GetStream, index)
        
    def PutState(self, line):
        return self._lib.BCI_PutState(self._string_arg(line))
        
    def GetState(self, index):
        return self._get_string_result(self._lib.BCI_GetState, index, mayfail=True)
        
    def SetStateValue(self, name, value):
        return self._lib.BCI_SetStateValue(self._string_arg(name), value)
        
    def GetStateValue(self, name):
        return self._get_string_result(self._lib.BCI_GetStateValue, self._string_arg(name))

    def PutEvent(self, line):
        return self._lib.BCI_PutEvent(self._string_arg(line))
        
    def GetEvent(self, index):
        return self._get_string_result(self._lib.BCI_GetEvent, index, mayfail=True)
        
    def SetEvent(self, name, value):
        return self._lib.BCI_SetEvent(self._string_arg(name), value)
        
    def PulseEvent(self, name, value):
        return self._lib.BCI_PulseEvent(self._string_arg(name), value)
        
    def GetSignalChannels(self):
        return self._lib.BCI_GetSignalChannels()
        
    def GetSignalElements(self):
        return self._lib.BCI_GetSignalElements()
        
    def GetSignal(self, channel, element):
        return self._lib.BCI_SetSignal(channel, elements)
        
    def PutVisProperty(self, visID, cfgID, value):
        return self._lib.BCI_PutVisProperty(self._string_arg(visID), cfgID, self._string_arg(value))
        
    def GetVisProperty(self, visID, cfgID):
        return self._get_string_result(self._lib.BCI_GetVisProperty, self._string_arg(visID), cfgID)
        
    class BCI_State(IntEnum):
        BCI_StateUnavailable = 0
        BCI_StateStartup = 1
        BCI_StateConnected = 2
        BCI_StateInitialization = 2 # alias for StateConnected
        BCI_StateResting = 3
        BCI_StateSuspended = 4
        BCI_StateParamsModified = 5
        BCI_StateRunning = 6
        BCI_StateBusy = 7
        BCI_StateIdle = 8
        
    def GetStateOfOperation(self):
        return self._lib.BCI_GetStateOfOperation()
        
    def GetConnectionInfo(self, index):
        return self._get_string_result(self._lib.BCI_GetConnectionInfo, index)
        
    def GetCoreModuleStatus(self, index):
        return self._get_string_result(self._lib.BCI_GetCoreModuleStatus, index)
                

    def Startup(self, module_list):
        
        return self._lib.BCI_Startup(self._string_arg(module_list))
        
    def Shutdown(self):
        return self._lib.BCI_Shutdown()

    
    def PipeListen(self, name):
        return self._lib.BCI_PipeListen(self._string_arg(name))

    def PipeCloseOne(self, name):
        return self._lib.BCI_PipeCloseOne(self._string_arg(name))
        
    def PipeClose(self):
        return self._lib.BCI_PipeClose()


    def TelnetListen(self, address):
        return self._lib.BCI_TelnetListen(self._string_arg(address))

    def TelnetCloseOne(self, address):
        return self._lib.BCI_TelnetCloseOne(self._string_arg(address))
        
    def TelnetClose(self):
        return self._lib.BCI_TelnetClose()


    def WebsocketListen(self, address):
        return self._lib.BCI_WebsocketListen(self._string_arg(address))

    def WebsocketCloseOne(self, address):
        return self._lib.BCI_WebsocketCloseOne(self._string_arg(address))
        
    def WebsocketClose(self):
        return self._lib.BCI_WebsocketClose()


    def SetConfig(self):
        return self._lib.BCI_SetConfig()

    def StartRun(self):
        return self._lib.BCI_StartRun()

    def StopRun(self):
        return self._lib.BCI_StopRun()

    def ExecuteScript(self, script):
        return self._lib.BCI_ExecuteScript(self._string_arg(script))

    def ExecuteScriptWithResult(self, script):
        return self._get_string_result(self._lib.BCI_ExecuteScriptWithResult, self._string_arg(script))
        
    def GetPerThreadWD(self):
        return self._get_string_result(self._lib.BCI_GetPerThreadWD)

    def SetPerThreadWD(self, wd):
        return self._lib.BCI_SetPerThreadWD(self._string_arg(wd))
        
    def AddWatch(self, expr, cbfunc):
        id = self._lib.BCI_AddWatch(self._string_arg(expr))
        if id == self.BCIEvent.BCI_None:
            raise self._error('Call to AddWatch() failed')

        func = ctypes.py_object(cbfunc)
        self._func_refs[id] = func
            
        ok = self._lib.BCI_SetCallback(
            id,
            self._trampoline_watch_callback,
            ctypes.cast(
                ctypes.pointer(func),
                ctypes.c_void_p,
            ),
        )
        if not ok:
            raise BCI2000OperatorRuntimeError("Could not register watch callback")
        return id
        
    def SetWatchDecimation(self, id, dec):
        return self._lib.BCI_SetWatchDecimation(id, dec)
        
    def RemoveWatch(self, id):
        if self._lib.BCI_RemoveWatch(id):
            self._BCI_SetCallback(id, 0, 0)
            del self._func_refs[id]
            return 1
        return 0
        
    # Internal functions

    @staticmethod
    def _string_arg(s):
        return ctypes.c_char_p(s.encode('utf-8'))

    def _get_string_result(self, fn, *args, mayfail=False):
        fn.restype = ctypes.c_char_p
        s = fn(*args)
        if s is None:
            if mayfail:
                return ''
            else:
                raise self._error('%s() call failed' % fn.__name__)
        result = copy.deepcopy(s)
        result = result.decode()
        self._lib.BCI_ReleaseObject(s)
        return result

    def _error(self, msg='BCI2000OperatorLib API call failed'):
        cls = BCI2000OperatorRuntimeError
        return cls(msg)

if __name__ == '__main__':
    # Example code
    class MyOperator(BCI2000Operator):
        def __init__(self):
            BCI2000Operator.__init__(self)
            self._quit = False
    
        def OnParameter(self, paramline):
            print("Parameter: ", paramline)
            
        def OnWatch1(self, values):
            print("SourceTime: ", values[0][1])

        def OnWatch2(self, values):
            print("StimulusTime: ", values[0][1])
            
        def OnUnknownCommand(self, command):
            if command == 'foobar':
                print("Doing foobar")
                return [True, 'Executed ' + command, '']
            return [False, '', '']
            
        def OnScriptHelp(self):
            return 'foobar: test command'
            
        def OnQuitRequest(self):
            self._quit = True
            return 'going to quit application'
            
    bci = MyOperator()
    print(bci.GetInfo())
    bci.TelnetListen("localhost:3999")
    dir = os.path.dirname(__file__);
    bci.ExecuteScript("cd \"" + dir + "\"")
    bci.Startup("localhost SignalGenerator:4000 SignalProcessing:4001 Application:4002")
    bci.ExecuteScript("Start Executable SignalGenerator --local")
    bci.ExecuteScript("Start Executable ARSignalProcessing --local")
    bci.ExecuteScript("Start Executable CursorTask --local")
    bci.ExecuteScript("Wait for Connected 10")
    bci.SetConfig()
    bci.AddWatch("SourceTime", bci.OnWatch1)
    bci.AddWatch("StimulusTime", bci.OnWatch2)
    bci.ExecuteScript("Wait for Resting 10")
    bci.StartRun()
    bci.ExecuteScript("Wait for Suspended 10")
    bci.StopRun()
    bci.ExecuteScript("Wait for Suspended 10")

    bci.ExecuteScript("foobar") # this is handled by OnUnknownCommand above
    
    bci.ExecuteScript("echo Press Ctrl-c to quit")
    while not bci._quit:
        try:
            bci.ExecuteScript("Wait 1")
        except KeyboardInterrupt:
            bci._quit = True
    del bci
