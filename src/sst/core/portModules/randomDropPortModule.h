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
#include <sst/core/rng/marsaglia.h>

namespace SST {

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
        //TODO Need stats
        SST_ELI_DOCUMENT_PARAMS(
            { "drop_prop", "Probably to drop event", "0.01" },
            { "drop_on_send",  "Controls whether to drop packetes during the send versus the default of on the receive", "false" },
            { "rngseed", "Set a seed for the random number generator used to control drops", "7" },
            { "verbose", "Debugging output", "false"}
        )

        RandomDropPortModule(Params& params);

        Event* eventSent(uintptr_t UNUSED(key), Event* ev) override;

        Event* eventReceived(Event* ev) override;

    private:
        double m_drop_prob;
        bool m_verbose;
        bool m_drop_on_send;
        SST::RNG::MarsagliaRNG rng;
    };
}
}

#endif