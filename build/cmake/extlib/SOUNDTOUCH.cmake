###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including SoundTouch in a project

set(INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/soundtouch/soundtouch-2.4.1/include")
set(srcdir_ "${PROJECT_SRC_DIR}/extlib/soundtouch/soundtouch-2.4.1/source/SoundTouch")
set(SRC_EXTLIB
  ${srcdir_}/AAFilter.cpp
  ${srcdir_}/BPMDetect.cpp
  ${srcdir_}/cpu_detect_x86.cpp
  ${srcdir_}/FIFOSampleBuffer.cpp
  ${srcdir_}/FIRFilter.cpp
  ${srcdir_}/InterpolateCubic.cpp
  ${srcdir_}/InterpolateLinear.cpp
  ${srcdir_}/InterpolateShannon.cpp
  ${srcdir_}/mmx_optimized.cpp
  ${srcdir_}/PeakFinder.cpp
  ${srcdir_}/RateTransposer.cpp
  ${srcdir_}/SoundTouch.cpp
  ${srcdir_}/sse_optimized.cpp
  ${srcdir_}/TDStretch.cpp
)

set( EXTLIB_OK TRUE )

