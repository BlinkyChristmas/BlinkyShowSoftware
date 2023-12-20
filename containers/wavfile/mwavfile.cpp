//Copyright © 2023 Charles Kerr. All rights reserved.

#include "mwavfile.hpp"

#include <algorithm>
#include <stdexcept>
#include <cmath>


using namespace std::string_literals ;

//======================================================================

//======================================================================
MWAVFile::MWAVFile():currentOffset(0) {
    
}

//======================================================================
MWAVFile::MWAVFile( const std::filesystem::path &filepath) : MWAVFile() {
    if (!load(filepath) ) {
        throw std::runtime_error("Invalid file type: "s + filepath.string());
    }
}
//======================================================================
auto MWAVFile::load(const std::filesystem::path &filepath) -> bool {
    this->clear() ;
    
    this->memoryMap.map(filepath) ;
    if (memoryMap.ptr == nullptr) {
        throw std::runtime_error("Unable to map: "s + filepath.string());
    }
    if (memoryMap.length <= 8) {
        throw std::runtime_error("Invalid file type: "s + filepath.string());
    }
    try {
        auto ptr = reinterpret_cast<const std::uint8_t*>(memoryMap.ptr) ;
        if (!checkFileType(ptr)) {
            return false ;
        }
        fmtChunk.load(ptr+12) ;
        dataChunk.load(ptr+20+fmtChunk.size);
        return true ;
    }
    catch (const std::exception &e) {
        std::cerr << "Unable to process: "<< filepath.string() << "\n" ;
        std::cerr << e.what() << std::endl;
        return false ;
    }
    catch (...){
        return false ;
    }
}
//======================================================================
auto MWAVFile::checkFileType(const std::uint8_t *input) -> bool {
    constexpr std::uint32_t riff = 0x46464952  ; // RIFF in big endian
    constexpr std::uint32_t wave = 0x45564157 ; // WAVE in big endian
    std::uint32_t signature ;
    std::copy(input,input+4,reinterpret_cast<std::uint8_t*>(&signature));
    if (signature != riff) {
        //std::cerr << "Invalid file type" << std::endl;
        return false ;
    }
    std::copy(input+8,input+12,reinterpret_cast<std::uint8_t*>(&signature));
    if (signature != wave) {
        //std::cerr << "Invalid file type" << std::endl;
        return false ;
    }
    return true;
}


//======================================================================
auto MWAVFile::clear() -> void {
    memoryMap.unmap() ;
    fmtChunk.clear() ;
    dataChunk.clear() ;
}

//======================================================================
auto  MWAVFile::isLoaded() const -> bool {
    return memoryMap.ptr != nullptr ;
}


//======================================================================
auto MWAVFile::setFrame(std::uint32_t frame) -> bool {
    auto time = (SSDRATE * double(frame))  ;
    auto sample = std::round(double(fmtChunk.sampleRate) * time) ;
    auto offset = static_cast<std::uint32_t>(sample) * fmtChunk.blockAlign ;
    if (offset > dataChunk.size){
        currentOffset = dataChunk.size ;
        return false ;
    }
    currentOffset = offset ;
    return true ;
}
//======================================================================
auto MWAVFile::loadBuffer(char *buffer, std::uint32_t samplecount ) -> std::uint32_t {
    auto location = currentOffset ;
    auto bytecount = samplecount * fmtChunk.blockAlign ;
    if (location + bytecount >= dataChunk.size) {
        bytecount = static_cast<std::uint32_t>(dataChunk.size - location) ;
    }
    std::copy(dataChunk.ptrToData + location,dataChunk.ptrToData+location+bytecount,reinterpret_cast<std::uint8_t*>(buffer));
    currentOffset += bytecount ;
    
    return static_cast<std::uint32_t>(bytecount/fmtChunk.blockAlign)   ;
}
