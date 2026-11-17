# $Id$

set(DIR_NAME "${DIR_NAME}/mex/")
set_crt_link_type(STATIC)

bci2000_add_mexfile(signal_sharing signal_sharing.cpp SignalSharingClient.cpp NO_COMPANIONS)

