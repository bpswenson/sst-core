#include "randomDropPortModule.h"

namespace SST::PortModules {

RandomDropPortModule::RandomDropPortModule(Params& params) : rng(7, 13) {
    // Restart the RNG to ensure completely consistent results
    // Seed with user-provided seed
    uint32_t z_seed = params.find<uint32_t>("rngseed", 7);
    rng.restart(z_seed, 13);

    m_drop_prob = params.find<double>("drop_prob", 0.01);
    m_verbose = params.find<bool>("verbose", false);
    m_drop_on_send = params.find<bool>("drop_on_send");

}

Event* RandomDropPortModule::eventSent(uintptr_t UNUSED(key), Event* ev) { 
    if(m_drop_on_send) {
        double pull = rng.nextUniform();

        if(pull < m_drop_prob) {
            if(m_verbose) { 
                std::cout << "Dropping event on send" << std::endl;
            }
            return nullptr;
        }
        else {
            if(m_verbose) {
                std::cout << "PULL: " << pull << " PROB: " << m_drop_prob << std::endl;
            }
        }
    }
    return ev;
}

Event* RandomDropPortModule::eventReceived(Event* ev) { 
    if(!m_drop_on_send) {
        double pull = rng.nextUniform();

        if(pull < m_drop_prob) {
            if(m_verbose) { 
                std::cout << "Dropping event on receive" << std::endl;
            }
            return nullptr;
        }
        else {
            if(m_verbose) {
                std::cout << "PULL: " << pull << " PROB: " << m_drop_prob << std::endl;
            }
        }
    }
    return ev;
}
}