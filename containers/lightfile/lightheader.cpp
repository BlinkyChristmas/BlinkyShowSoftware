//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightheader.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

using namespace std::string_literals ;

//======================================================================
auto LightHeader::clear() -> void {
    headerSize = 0 ;
    sampleRate = 0 ;
    frameCount = 0 ;
    frameLength = 0 ;
    sourceName = "" ;
}
//======================================================================
LightHeader::LightHeader():sampleRate(0),frameCount(0),frameLength(0),headerSize(0){
    
}
//======================================================================
LightHeader::LightHeader( const char *ptr):LightHeader() {
    load(ptr) ;
}
//======================================================================
auto LightHeader::load(const char *ptr) -> void {
    std::copy(ptr,ptr+4,reinterpret_cast<char*>(&headerSize)) ;
    std::copy(ptr+4,ptr+8,reinterpret_cast<char*>(&sampleRate)) ;
    std::copy(ptr+8,ptr+12,reinterpret_cast<char*>(&frameCount)) ;
    std::copy(ptr+12,ptr+16,reinterpret_cast<char*>(&frameLength)) ;
    auto buffer = std::vector<char>(NAMESIZE+1,0) ;
    std::copy(ptr+16,ptr+16+NAMESIZE,buffer.data()) ;
    sourceName = buffer.data() ;
}

