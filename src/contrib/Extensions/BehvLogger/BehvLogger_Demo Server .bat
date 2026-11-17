#! ../prog/BCI2000Shell
@cls & ..\prog\BCI2000Shell %0 %* #! && exit /b 0 || exit /b 1\n
#######################################################################################
## $Id: FeedbackDemo_SignalGenerator.bat 8056 2024-04-22 19:54:35Z mellinger $
## Description: BCI2000 startup Operator module script. For an Operator scripting
##   reference, see
##   https://www.bci2000.org/mediawiki/index.php/User_Reference:Operator_Module_Scripting
##
## $BEGIN_BCI2000_LICENSE$
##
## This file is part of BCI2000, a platform for real-time bio-signal research.
## [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
##
## BCI2000 is free software: you can redistribute it and/or modify it under the
## terms of the GNU General Public License as published by the Free Software
## Foundation, either version 3 of the License, or (at your option) any later
## version.
##
## BCI2000 is distributed in the hope that it will be useful, but
##                         WITHOUT ANY WARRANTY
## - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
## A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along with
## this program.  If not, see <http://www.gnu.org/licenses/>.
##
## $END_BCI2000_LICENSE$
#######################################################################################
# CNEL Behv log and send data demo
# Created by Luciano Branco and Matt Hall - CNELab - for https://ieeexplore.ieee.org/document/8037190
# for latest updates or issues contact mjhall4@cougarnet.uh.edu
#######################################################################################
Change directory $BCI2000LAUNCHDIR
Show window; Set title ${Extract file base $0}
Reset system
Startup system localhost
Start executable SignalGenerator --local --LogBehv=1
Start executable DummySignalProcessing --local
Start executable DummyApplication --local
Wait for Connected

# Sig gen params
Set Parameter SamplingRate 2400Hz
Set Parameter SampleBlockSize 24
Set Parameter SineFrequency 1Hz
Set Parameter NoiseAmplitude 10muV

# Behv params
# Load in Demo Matrix for server sending
# if matrix is blank or no screenshot, server won't connect
# example found in ../src/contrib/Extensions/BehvLogger/
# Note Slashes ! / 
# Load parameterfile "../src/contrib/Extensions/BehvLogger/DemoServer_local.prm"

Set Parameter IPCnelBehv 127.0.0.1
# Set Parameter IPCnelBehv 169.254.x.x # example of a remote IP to receive from a different computer running behv
Set Parameter UDPPortCnelBehv 27002

Set Parameter IPCnelBehvServer 127.0.0.1
#Set Parameter IPCnelBehvServer 169.254.x.x
Set Parameter UDPPortCnelBehvServer 27100

#behav must be open for screenshot to work
Set Parameter doScreenshot 1 # for periodic screenshots
Set Parameter ScreenshotSeconds 10
Set Parameter ScreenshotDirectory "C:/RecordingTest/Screenshot/" # will create directory if nonexistent
visualize watch CnelBehv0
visualize watch CnelBehv1
visualize watch CnelBehv2
visualize watch CnelBehv3
visualize watch CnelBehv4
visualize watch CnelBehv5
visualize watch CnelBehv6
# watch channels up to 32


# Simulink params for example send to link
#Set Parameter % list SimulinkConnectorStates= 13 CnelBehv0 CnelBehv1 CnelBehv2 CnelBehv3 CnelBehv4 CnelBehv5 CnelBehv6

# server send command example (matches matrix code)
SET BUTTON 1  LED_on "SET STATE CnelServerState 1"
SET BUTTON 2 "Send Behv 2" "SET STATE CnelServerState 2"
SET BUTTON 3 "Clear Send" "SET STATE CnelServerState 3"
SET BUTTON 4 "State zero" "SET STATE CnelServerState 0"
visualize watch CnelServerState # default watch for code variable

# example for triggering with a different variable
# add state myAppVariable 32 0 0 0
# SET STATE myAppVariable 0
# Set Parameter CnelServerStateName myAppVariable # will watch this variable instead
# SET BUTTON 1  LED_on "SET STATE myAppVariable 1"
# SET BUTTON 2 "Send Behv 2" "SET STATE myAppVariable 2"
# SET BUTTON 3 "Clear Send" "SET STATE myAppVariable 3"
# SET BUTTON 4 "State zero" "SET STATE myAppVariable 0"
# visualize watch myAppVariable