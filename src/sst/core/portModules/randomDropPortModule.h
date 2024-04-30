// Copyright 2009-2022 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2022, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef SST_CORE_PORTMODULE_RANDOMDROPPORTMODULE_H
#define SST_CORE_PORTMODULE_RANDOMDROPPORTMODULE_H

#include "sst/core/portModule.h"
#include "sst/core/eli/elementinfo.h"

namespace SST {

/*    SST_ELI_REGISTER_MODULE_API(SST::CoreTestModule::CoreTestModuleExample)

    SST_ELI_REGISTER_MODULE_DERIVED(
        CoreTestModuleExample, "coreTestElement", "CoreTestModule", SST_ELI_ELEMENT_VERSION(1, 0, 0),
        "CoreTest module to demonstrate interface.", SST::CoreTestModule::CoreTestModuleExample)
*/

namespace PortModules {
    class RandomDropPortModule : public PortModule {
    public:
        SST_ELI_REGISTER_PORTMODULE(
            RandomDropPortModule,
            SST::PortModule,
            "sst",
            "portmodules.random_drop_port_module",
            SST_ELI_ELEMENT_VERSION(0, 1, 0),
            "port module that will randomly drop events"
        )

        SST_ELI_DOCUMENT_PARAMS(
            { "level", "Level at which to track profile (global, type, component, subcomponent)", "type" },
            { "track_ports",  "Controls whether to track by individual ports", "false" },
            { "profile_sends",  "Controls whether sends are profiled (due to location of profiling point in the code, turning on send profiling will incur relatively high overhead)", "false" },
            { "profile_receives",  "Controls whether receives are profiled", "true" },
        )

        RandomDropPortModule(Params& params) {

        }
        Event* eventSent(uintptr_t UNUSED(key), Event* ev) override { 
            std::cout << "RandomDropPortModule::eventSent" << std::endl;    
            return ev; 
            
        }
        Event* eventReceived(Event* ev) override { 
            std::cout << "RandomDropPortModule::eventReceived" << std::endl;    
            return ev; 
        }
    private:
        double m_drop_prob;
    };
}
}

#endif