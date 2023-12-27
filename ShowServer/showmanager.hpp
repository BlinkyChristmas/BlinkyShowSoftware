//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef showmanager_hpp
#define showmanager_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <atomic>
#include <chrono>

#include "configfile.hpp"
#include "showserver.hpp"
#include "playlist.hpp"
//======================================================================
class ShowManager {
    
    ShowServer server ;
    ConfigFile config ;
    bool inShow ;
    bool isListening ;
    std::atomic<bool> isPlaying ;
    
    auto waitToStop() -> void ;
    auto playPlaylist( const std::vector<ListEntry> &entries) -> void ;
public:
    auto eventStop() -> void ;
    ShowManager() ;
    auto run(const std::filesystem::path &path) -> bool ;
    
};

#endif /* showmanager_hpp */
