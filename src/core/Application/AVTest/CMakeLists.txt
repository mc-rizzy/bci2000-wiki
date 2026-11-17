###########################################################################
## Authors: mellinger@neurotechcenter.org
## Description: Build information for the AVTest application module
###########################################################################

bci2000_add_application_module(
  AVTest
  AVTestTask.cpp
  ${BCI2000_SRC_DIR}/core/Application/CursorTask/SimpleSynth.cpp
)
