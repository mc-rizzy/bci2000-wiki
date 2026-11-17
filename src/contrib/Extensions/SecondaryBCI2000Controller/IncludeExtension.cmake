###########################################################################
## $Id: IncludeExtension.cmake 8931 2025-07-01 15:05:33Z mellinger $
## Authors: mellinger@neurotechcenter.org

list( APPEND BCI2000_SIGSRC_FILES
   ${BCI2000_EXTENSION_DIR}/SecondaryBCI2000Controller.cpp
   ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp
   ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
   SUPPORT ${BCI2000_EXTENSION_DIR}/*.bat=>/batch
)


