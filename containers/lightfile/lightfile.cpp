//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightfile.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/mapfile.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;


//======================================================================
LightFile::LightFile() {
    
}
//======================================================================
LightFile::LightFile(const std::filesystem::path &lightfile) {
    
}
//======================================================================
auto LightFile::loadFile(const std::filesystem::path &lightfile) -> bool {
    try {
        if (lightData.ptr  != nullptr) {
            clear() ;
        }
        auto ptr = lightData.map(lightfile) ;
        // we should check right here if the right type of file
        if (ptr == nullptr) {
            return false ;
        }
        try {
            lightHeader.load(reinterpret_cast<const char *>(lightData.ptr)) ;
        }
        catch (...) {
            lightData.unmap() ;
            return false ;

        }
        return true ;
    }
    catch (const std::exception &e){
        // we had some type of error
        DBGMSG(std::cerr, "Error loading: "s + lightfile.string()+"\n"s+e.what());
        return false ;
    }
}

//======================================================================
auto LightFile::isLoaded() const -> bool {
    return lightData.ptr != nullptr ;
}

//======================================================================
auto LightFile::frameCount() const -> std::uint32_t {
    return lightHeader.frameCount ;
}
//======================================================================
auto LightFile::frameLength() const -> std::uint32_t {
    return lightHeader.frameLength ;
}

//======================================================================
auto LightFile::copy(std::uint32_t frame,  char *buffer, int offset, int length )  -> int {
    if (lightData.ptr == nullptr) {
        return 0 ;
    }
    if (buffer == nullptr) {
        return 0 ;
    }
    if (frame >= lightHeader.frameCount) {
        return 0 ;
    }
    if (length == 0) {
        length = lightHeader.frameLength ;
    }
    if ( offset + length > lightHeader.frameLength) {
        length = lightHeader.frameLength - offset ;
    }
    auto dataoffset = lightHeader.offsetToData  + (frame * lightHeader.frameLength) + offset;
    std::copy( reinterpret_cast<const char *>(lightData.ptr) + dataoffset,reinterpret_cast<const char *>(lightData.ptr) + dataoffset + length, buffer ) ;
    return length ;
}

//======================================================================
auto LightFile::clear(bool nothrow) -> void  {
    lightHeader.clear() ;
    lightData.unmap() ;
}
