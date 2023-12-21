//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightfile.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
auto LightFile::isValidFile(const void *ptr) const -> bool {
    std::uint32_t signature ;
    std::uint32_t version ;
    const std::uint32_t *dataPtr = reinterpret_cast<const std::uint32_t *>(ptr) ;
    if (dataPtr == nullptr) {
        return false ;
    }
    std::copy(dataPtr,dataPtr+1,&signature) ;
    std::copy(dataPtr+1,dataPtr+2,&version) ;
    if (signature == SIGNATURE && version == 0) {
        return true ;
    }
    return false ;
}

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
        if (!isValidFile(ptr)) {
            lightData.unmap() ;
            return false ;
        }
        lightHeader.load(reinterpret_cast<const char *>(lightData.ptr)) ;
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
    if (frame >= lightHeader.frameCount) {
        return 0 ;
    }
    if (length == 0) {
        length = lightHeader.frameLength ;
    }
    if ( offset + length > lightHeader.frameLength) {
        length = lightHeader.frameLength - offset ;
    }
    auto dataoffset = std::uint32_t(12) + lightHeader.headerSize  + frame * lightHeader.frameLength + offset;
    std::copy( reinterpret_cast<const char *>(lightData.ptr) + dataoffset,reinterpret_cast<const char *>(lightData.ptr) + dataoffset + length, buffer ) ;
    return length ;
}

//======================================================================
auto LightFile::clear(bool nothrow) -> void  {
    lightHeader.clear() ;
    lightData.unmap(nothrow) ;
}
