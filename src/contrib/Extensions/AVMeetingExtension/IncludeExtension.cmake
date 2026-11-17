###########################################################################
## Authors: mellinger@neurotechcenter.org
## Description: Build information for the AVMeetingExtension
###########################################################################

set(WEBCAMLOGGER_DIR ${PROJECT_SRC_DIR}/contrib/Extensions/WebcamLogger)

set(WEBCAMLOGGER_SRC
  ${WEBCAMLOGGER_DIR}/AudioMediaType.cpp
  ${WEBCAMLOGGER_DIR}/VideoMediaType.cpp
  ${WEBCAMLOGGER_DIR}/AVSourceEnumerator.cpp
  ${WEBCAMLOGGER_DIR}/ffmpeg/AVSourceLibAVDevice.cpp
  ${WEBCAMLOGGER_DIR}/ffmpeg/AVSourceEnumeratorLibAVDevice.cpp
)

if (WIN32)
   list(APPEND WEBCAMLOGGER_SRC
      ${BCI2000_SRC_DIR}/extlib/com/ComPtr.h
      ${BCI2000_SRC_DIR}/extlib/com/ComStrings.cpp
      ${WEBCAMLOGGER_DIR}/win32mf/MFInit.h
      ${WEBCAMLOGGER_DIR}/win32mf/MFVideoMediaType.cpp
      ${WEBCAMLOGGER_DIR}/win32mf/MFAudioMediaType.cpp
      ${WEBCAMLOGGER_DIR}/win32mf/AVSourceMF.cpp
      ${WEBCAMLOGGER_DIR}/win32mf/AVSourceEnumeratorMF.cpp
      ${WEBCAMLOGGER_DIR}/win32mf/WebcamPipeline.cpp
      ${WEBCAMLOGGER_DIR}/win32mf/GrabberTransform.cpp
   )
   foreach(lib mfplat.lib;mf.lib;mfreadwrite.lib;mfuuid.lib;strmiids.lib;propsys.lib)
     list(APPEND WEBCAMLOGGER_SRC LIBRARY ${lib})
   endforeach()

elseif (APPLE)

  list(APPEND WEBCAMLOGGER_SRC
     ${WEBCAMLOGGER_DIR}/ffmpeg/AVFoundationHelper.mm
  )
  list(APPEND WEBCAMLOGGER_SRC
    LIBRARY "-framework Foundation"
    LIBRARY "-framework CoreMedia"
    LIBRARY "-framework AVFoundation"
  )

else()

   list(APPEND WEBCAMLOGGER_SRC
     ${WEBCAMLOGGER_DIR}/ffmpeg/Video4LinuxHelper.cpp
  )

endif()

list(APPEND BCI2000_APP_FILES
  ${BCI2000_EXTENSION_DIR}/AVMeetingFilter.cpp
  ${BCI2000_EXTENSION_DIR}/ReceivingAVDisplay.cpp
  ${BCI2000_EXTENSION_DIR}/MirrorDisplay.cpp
  ${BCI2000_EXTENSION_DIR}/AVBidirectionalChannel.cpp
  ${BCI2000_EXTENSION_DIR}/AVMeetingSaveToFile.cpp
  ${BCI2000_EXTENSION_DIR}/AVMeetingSaveVideoToFile.cpp
  ${BCI2000_EXTENSION_DIR}/AVMeetingSaveAudioToFile.cpp
  ${BCI2000_EXTENSION_DIR}/AVMuxer.cpp
  ${PROJECT_SRC_DIR}/shared/utils/AV/AVError.cpp
  ${PROJECT_SRC_DIR}/shared/utils/AV/AVEncoder.cpp
  ${PROJECT_SRC_DIR}/shared/utils/AV/AVBufferedEncoder.cpp
  ${WEBCAMLOGGER_SRC}
)

