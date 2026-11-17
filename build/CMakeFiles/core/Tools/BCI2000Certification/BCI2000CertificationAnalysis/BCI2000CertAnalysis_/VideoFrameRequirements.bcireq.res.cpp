// auto-created file, do not edit
// created 2026-11-17 20:38:59Z
// created from: /Users/gooooooooooooner/Documents/bci2000-svn/src/core/Tools/BCI2000Certification/BCI2000CertificationAnalysis/Scripts/VideoFrameRequirements.bcireq
#define IS_RESOURCE_VideoFrameRequirements
#ifdef REGISTER_RESOURCE
REGISTER_RESOURCE( "VideoFrameRequirements.bcireq", VideoFrameRequirements )
#endif

#ifdef DECLARE_RESOURCES
namespace bci {
 namespace Resources {
  extern bci::Resource VideoFrameRequirements;
 }
}
#else // DECLARE_RESOURCES
#include "Resource.h"
namespace { static const unsigned char VideoFrameRequirements_data_[] = {
114,101,113,117,105,114,101,40,70,114,97,109,101,83,116,97,116,105,115,116,105,99,115,46,108,111,115,116,40,41,47,70,114,97,109,101,83,116,97,
116,105,115,116,105,99,115,46,116,111,116,97,108,40,41,60,48,46,48,53,41,59,13,10,
}; }
namespace bci {
 namespace Resources {
  bci::Resource VideoFrameRequirements = { "bcireq", sizeof( VideoFrameRequirements_data_), (const char*)VideoFrameRequirements_data_ };
 }
}
#endif // DECLARE_RESOURCES

