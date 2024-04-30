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

#include "sst/core/eli/elementinfo.h"
#include "sst/core/sst_types.h"
#include "sst/core/warnmacros.h"

namespace SST {
    class Event;
    class Param;

    class PortModule {
    public:
        SST_ELI_DECLARE_BASE(PortModule)
        SST_ELI_DECLARE_CTORS(ELI_CTOR(SST::Params&))
        SST_ELI_DECLARE_INFO_EXTERN(ELI::ProvidesInterface, ELI::ProvidesParams)
        PortModule() { }
        virtual ~PortModule(){ }

        virtual Event* eventSent(uintptr_t UNUSED(key), Event* ev) { std::cout << "PortModule::eventSent" << std::endl; return ev; }
        virtual Event* eventReceived(Event* ev) { std::cout << "PortModule::eventReceived" << std::endl; return ev; }

    protected:
    };
}

// Register profile tools.  Must register an interface
// (API) first, then you can register a subcomponent that implements
// it
#define SST_ELI_REGISTER_PORTMODULE_API(cls, ...)            \
    SST_ELI_DECLARE_NEW_BASE(SST::PortModule,::cls) \
    SST_ELI_NEW_BASE_CTOR(SST::Params&,##__VA_ARGS__)

#define SST_ELI_REGISTER_PORTMODULE_DERIVED_API(cls, base, ...) \
    SST_ELI_DECLARE_NEW_BASE(::base,::cls)                       \
    SST_ELI_NEW_BASE_CTOR(SST::Params&,##__VA_ARGS__)

#define SST_ELI_REGISTER_PORTMODULE(cls, interface, lib, name, version, desc)          \
    SST_ELI_REGISTER_DERIVED(::interface,cls,lib,name,ELI_FORWARD_AS_ONE(version),desc) \
    SST_ELI_INTERFACE_INFO(#interface)
#endif


