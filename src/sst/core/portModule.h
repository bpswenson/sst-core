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

#ifndef SST_CORE_PORTMODULE_H
#define SST_CORE_PORTMODULE_H

#include "sst/core/sst_types.h"
#include "sst/core/ssthandler.h"

namespace SST {
    class Event;

    class PortModule : public HandlerProfileToolAPI {
    public:
        SST_ELI_REGISTER_PROFILETOOL_API(SST::PortModule, Params&)    
    protected:
        PortModule(const std::string& name, Params& params): HandlerProfileToolAPI(name) {}
        ~PortModule(){ }
    public:
        virtual bool eventSent(uintptr_t UNUSED(key), Event* UNUSED(ev)) { return true; }
        virtual bool eventReceived(Event* UNUSED(ev)) { return true; }
    };
}

#endif


