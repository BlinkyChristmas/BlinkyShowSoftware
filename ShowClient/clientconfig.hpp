//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef clientconfig_hpp
#define clientconfig_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <array>

#include "utility/timeutil.hpp"
#include "controllers/pruconfig.hpp"



//======================================================================
struct ClientConfig {
    int clientPort ;
    std::string serverIP ;
    int serverPort ;
    std::array<PRUConfig,2> pruSetting ;
    std::string name ;
    
    std::uint32_t serverKey ;
    
    std::filesystem::path musicPath ;
    std::filesystem::path lightPath ;
    std::string musicExtension ;
    std::string lightExtension ;
    
    util::HourRange connectTime ;
    util::MonthRange runSpan ;
    
    bool useAudio ;
    bool useLights ;
    bool masterSync ;
    int syncCount ;
    
    ClientConfig() ;
    ClientConfig(const std::filesystem::path &configpath) ;
    auto load(const std::filesystem::path &configpath) -> bool ;
    auto processKeyValue(const std::string &ukey, const std::string &value) -> bool ;
};

#endif /* clientconfig_hpp */
