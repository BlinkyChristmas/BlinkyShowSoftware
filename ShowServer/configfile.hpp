//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef configfile_hpp
#define configfile_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>

#include "utility/timeutil.hpp"

//======================================================================
struct ConfigFile {
    std::filesystem::path playlist ;
    util::HourRange showTime ;
    util::HourRange listenTime ;
    util::MonthRange runTime ;
    int delay ;
    int port ;
    std::uint32_t key ;
    std::uint32_t frameupdate;
    
    std::filesystem::file_time_type lastRead ;
    auto reset() -> void ;
    ConfigFile() ;
    auto load(const std::filesystem::path &path) -> bool ;
    auto hasBeenUpdated(const std::filesystem::path &path) const -> bool ;
};

#endif /* configfile_hpp */
