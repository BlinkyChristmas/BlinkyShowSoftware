//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef playlist_hpp
#define playlist_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

//======================================================================
// ListEntry
//======================================================================
struct ListEntry {
    std::string musicName ;
    std::string lightName ;
    std::uint32_t framecount ;
    
    ListEntry() ;
    ListEntry(const std::string &line) ;
    auto valid() const -> bool ;
};

//======================================================================
// PlayList
//======================================================================
struct PlayList {
    std::vector<ListEntry> prologue ;
    std::vector<ListEntry> body ;
    std::vector<ListEntry> epilogue ;
    
    PlayList() = default ;
    PlayList(const std::filesystem::path &path) ;
    
    auto load(const std::filesystem::path &path) -> bool ;
};
#endif /* playlist_hpp */
