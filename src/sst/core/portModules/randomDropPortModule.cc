#include "randomDropPortModule.h"

namespace SST::PortModules {

RandomDropPortModule::RandomDropPortModule(Params& params) : rng(7, 13) {
    // Restart the RNG to ensure completely consistent results
    // Seed with user-provided seed
    uint32_t z_seed = params.find<uint32_t>("rngseed", 7);
    rng.restart(z_seed, 13);

    m_drop_prob = params.find<double>("drop_prop", 0.01);
    m_verbose = params.find<bool>("verbose", false);
    m_drop_on_send = params.find<bool>("drop_on_send");

}

Event* RandomDropPortModule::eventSent(uintptr_t UNUSED(key), Event* ev) { 
    if(m_drop_on_send) {
        if(rng.nextUniform() < m_drop_prob) {
            if(m_verbose) { 
                std::cout << "Dropping event" << std::endl;
            }
            return nullptr;
        }
    }
    return ev;
}

Event* RandomDropPortModule::eventReceived(Event* ev) { 
    if(!m_drop_on_send) {
        if(rng.nextUniform() < m_drop_prob) {
            if(m_verbose) { 
                std::cout << "Dropping event" << std::endl;
            }
            return nullptr;
        }
    }
    return ev;
}
}