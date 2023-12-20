//Copyright © 2023 Charles Kerr. All rights reserved.

#include "wavfile.hpp"

#include <algorithm>
#include <stdexcept>
#include <cmath>

using namespace std::string_literals ;

//======================================================================
WAVFile::WAVFile() {
    
}

//======================================================================
WAVFile::WAVFile( const std::filesystem::path &filepath) : WAVFile() {
    if (!load(filepath) ) {
        throw std::runtime_error("Unable to open: "s + filepath.string()) ;
    }
}

//======================================================================
auto WAVFile::checkFileType(std::istream &input) -> bool {
    constexpr std::uint32_t riff = 0x46464952  ; // RIFF in big endian
    constexpr std::uint32_t wave = 0x45564157 ; // WAVE in big endian
    std::uint32_t signature ;
    std::uint32_t size ;
    input.read(reinterpret_cast<char*>(&signature), 4) ;
    if (input.gcount() != 4){
        return false ;
    }
    if (signature != riff) {
        std::cerr << "Invalid file type" << std::endl;
        return false ;
    }
    input.read(reinterpret_cast<char*>(&size), 4) ;
    if (input.gcount() != 4){
        return false ;
    }
    input.read(reinterpret_cast<char*>(&signature), 4) ;
    if (input.gcount() != 4){
        return false ;
    }
    if (signature != wave) {
        std::cerr << "Invalid file type" << std::endl;
        return false ;
    }
    return true;
}

//======================================================================
auto WAVFile::load(const std::filesystem::path &filepath) -> bool {
    this->clear() ;
    input = std::ifstream(filepath.string(),std::ios::binary) ;
    if (!input.is_open()){
        return false ;
    }
    try {
        if (checkFileType(input)) {
            
            // We opend the file, lets load our chunks
            fmtChunk.load(input) ;
            dataChunk.load(input) ;
            return true ;
        }
        else {
            return false ;
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Unable to process: "<< filepath.string() << "\n" ;
        std::cerr << e.what() << std::endl;
        return false ;
    }
}
//======================================================================
auto WAVFile::clear() -> void {
    if (input.is_open()) {
        input.close() ;
    }
    fmtChunk.clear() ;
    dataChunk.clear() ;
}

//======================================================================
auto  WAVFile::isLoaded() const -> bool {
    return input.is_open() ;
}

//======================================================================
auto WAVFile::timeForSample(std::uint32_t sample) const -> double {
    return double(sample) / double(fmtChunk.sampleRate) ;
}
//======================================================================
auto WAVFile::sampleForTime(double time) const -> std::uint32_t {
    return  static_cast<std::uint32_t>(std::round( time * double( fmtChunk.sampleRate ))) ;
}

//======================================================================
auto WAVFile::nextSample() const -> std::uint32_t {
    return static_cast<std::uint32_t>( ( static_cast<std::uint64_t>( input.tellg() ) -  dataChunk.offsetToData ) / static_cast<std::uint64_t>( fmtChunk.blockAlign ) ) ;
}
//======================================================================
auto WAVFile::nextTime() const -> double {
    return this->timeForSample(this->nextSample()) ;
}

//======================================================================
auto WAVFile::setNextTime(double time) -> bool {
    return this->setSample(this->sampleForTime(time)) ;
}

//======================================================================
auto WAVFile::setSample(std::uint32_t sample) -> bool {
   
    auto offset = sample * static_cast<std::uint32_t>(fmtChunk.blockAlign) ;
    if (offset >= dataChunk.size) {
        return false ;
    }
    input.seekg(dataChunk.offsetToData + static_cast<std::uint64_t>(offset), std::ios::beg) ;
    return true ;
}

//======================================================================
auto WAVFile::setFrame(std::uint32_t frame) -> bool {
    auto time = (SSDRATE * double(frame))  ;
    return setNextTime(time) ;
}
//======================================================================
auto WAVFile::loadBuffer(char *buffer, std::uint32_t samplecount ) -> std::uint32_t {
    auto location = static_cast<std::uint32_t>(input.tellg()) ;
    auto bytecount = samplecount * fmtChunk.blockAlign ;
    if (static_cast<std::uint32_t>(bytecount + location ) >= dataChunk.size) {
        // we need to reduce the size
        bytecount = dataChunk.size - location ;
        
    }
    input.read(buffer, bytecount) ;
    return static_cast<std::uint32_t>(input.gcount()/fmtChunk.blockAlign)   ;
}
