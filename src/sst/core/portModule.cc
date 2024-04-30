#include "portModule.h"


namespace SST {

SST_ELI_DEFINE_INFO_EXTERN(PortModule)
SST_ELI_DEFINE_CTOR_EXTERN(PortModule)


PortModule::PortModule(const std::string& name) : name(name) {}

} // namespace SST
