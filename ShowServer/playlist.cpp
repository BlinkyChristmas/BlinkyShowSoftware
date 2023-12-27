//Copyright © 2023 Charles Kerr. All rights reserved.

#include "playlist.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "utility/strutil.hpp"

using namespace std::string_literals ;

//======================================================================
// ListEntry
//======================================================================

//======================================================================
ListEntry::ListEntry() : framecount(0){
    
}

//======================================================================
ListEntry::ListEntry(const std::string &line): ListEntry() {
    auto values = util::parse(line,",") ;
    switch(values.size()){
        default:
        case 3: {
            lightName = values[2] ;
            [[fallthrough]];
        }
        case 2:{
            musicName = values[1] ;
            [[fallthrough]] ;
        }
        case 1:{
            try {
                framecount = static_cast<std::uint32_t>(std::stoul(values[0],nullptr,0)) ;
            }
            catch(...) {
                // This is a bad number!
                
            }
            [[fallthrough]] ;
        }
        case 0:
            break;
    }
}

//======================================================================
auto ListEntry::valid() const ->bool {
    return framecount != 0 ;
}

//======================================================================
// PlayList
//======================================================================
//======================================================================
PlayList::PlayList(const std::filesystem::path &path) :PlayList() {
    if (!load(path)) {
        throw std::runtime_error("Error loading: "s + path.string());
    }
}

//======================================================================
auto PlayList::load(const std::filesystem::path &path) -> bool {
    epilogue.clear();
    prologue.clear();
    body.clear() ;
    
    auto input = std::ifstream(path.string()) ;
    if (!input.is_open()){
        return false ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    std::vector<ListEntry> *ptr = nullptr ;
    
    while (input.good() && !input.eof()) {
        input.getline(buffer.data(), buffer.size()-1) ;
        if (input.gcount() > 0){
            buffer[input.gcount()] = 0 ;
            std::string line = buffer.data() ;
            line = util::trim(util::strip(line,"#")) ;
            if (!line.empty()){
                if (line[0] == '[' && line[line.size()-1]==']') {
                    // We have a section header!
                    auto sec = util::upper(util::contentsOf(line, "[", "]")) ;
                    if (sec == "BODY"){
                        ptr = &body ;
                    }
                    else if (sec == "EPILOGUE"){
                        ptr = &epilogue ;
                    }
                    else if (sec == "PROLOGUE") {
                        ptr = &prologue ;
                    }
                    else {
                        ptr = nullptr ;
                    }
                }
                else if (line == "}") {
                    ptr = nullptr ;
                }
                else if (line != "{"){
                    auto [skey,value] = util::split(line,"=") ;
                    auto key = util::upper(skey) ;
                    if (key == "ENTRY" && ptr != nullptr) {
                        auto entry = ListEntry(value) ;
                        if (entry.valid()) {
                            ptr->push_back(entry) ;
                        }
                    }
                }
            }
        }
    }
    return true ;
}
