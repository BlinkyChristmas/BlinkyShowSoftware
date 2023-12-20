//Copyright © 2023 Charles Kerr. All rights reserved.

#include "wavfmtchunk.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
auto WAVFmtChunk::clear() -> void {
    signature = 0 ;
    size = 0 ;
    audioFormat = 0 ;
    channelCount = 0 ;
    sampleRate = 0 ;
    byteRate = 0 ;
    blockAlign  = 0 ;
    bitsPerSample = 0 ;
}

//======================================================================
auto WAVFmtChunk::load(std::istream &input) -> void {
    input.read(reinterpret_cast<char*>(&signature), sizeof(signature));
    if (signature != SIGNATURE){
        throw std::runtime_error("Wrong chunk type (looking for (fmt): "s + std::to_string(SIGNATURE)+" ): "s + std::to_string(signature)) ;
    }
    input.read(reinterpret_cast<char*>(&size),sizeof(size)) ;
    auto chunkStart = input.tellg() ;
    
    input.read(reinterpret_cast<char*>(&audioFormat),2);
    if (audioFormat != 1) {
        // THis is not PCM data!
        throw std::runtime_error("Incorrect data format, not PCM: " + std::to_string(audioFormat));
    }
    input.read(reinterpret_cast<char*>(&channelCount),2);
    input.read(reinterpret_cast<char*>(&sampleRate),4);
    input.read(reinterpret_cast<char*>(&byteRate),4);
    input.read(reinterpret_cast<char*>(&blockAlign),2);
    input.read(reinterpret_cast<char*>(&bitsPerSample),2);
    // Seek to the data chunk start
    input.seekg(static_cast<int>(chunkStart) + size, std::ios::beg) ;
}

//======================================================================
auto WAVFmtChunk::load(const std::uint8_t *ptr) -> const std::uint8_t * {
    if (ptr == nullptr){
        throw std::runtime_error("Null ptr passed to load wav fmt chunk") ;
    }
    size_t offset = 0 ;
    std::copy(ptr,ptr + sizeof(signature),reinterpret_cast<std::uint8_t*>(&signature)) ;
    if (signature != SIGNATURE){
        throw std::runtime_error("Wrong chunk type (looking for (fmt): "s + std::to_string(SIGNATURE)+" ): "s + std::to_string(signature)) ;
    }
    offset += sizeof(signature) ;
    std::copy(ptr+offset,ptr + offset + sizeof(size),reinterpret_cast<std::uint8_t*>(&size)) ;
    offset += sizeof(size) ;
    auto chunkoffset = offset ;
    std::copy(ptr+offset,ptr + offset + sizeof(audioFormat),reinterpret_cast<std::uint8_t*>(&audioFormat)) ;
    if (audioFormat != 1) {
        // THis is not PCM data!
        throw std::runtime_error("Incorrect data format, not PCM: " + std::to_string(audioFormat));
    }
    offset += sizeof(audioFormat) ;
    std::copy(ptr+offset,ptr + offset + sizeof(channelCount),reinterpret_cast<std::uint8_t*>(&channelCount)) ;
    offset += sizeof(channelCount) ;
    std::copy(ptr+offset,ptr + offset + sizeof(sampleRate),reinterpret_cast<std::uint8_t*>(&sampleRate)) ;
    offset += sizeof(sampleRate) ;
    std::copy(ptr+offset,ptr + offset + sizeof(byteRate),reinterpret_cast<std::uint8_t*>(&byteRate)) ;
    offset += sizeof(byteRate) ;
    std::copy(ptr+offset,ptr + offset + sizeof(blockAlign),reinterpret_cast<std::uint8_t*>(&blockAlign)) ;
    offset += sizeof(blockAlign) ;
    std::copy(ptr+offset,ptr + offset + sizeof(bitsPerSample),reinterpret_cast<std::uint8_t*>(&bitsPerSample)) ;
    return ptr+chunkoffset ;
}
//======================================================================


//======================================================================
WAVFmtChunk::WAVFmtChunk():signature(0),size(0),audioFormat(0),channelCount(0),sampleRate(0),byteRate(0),blockAlign(0),bitsPerSample(0){
    
}
//======================================================================
WAVFmtChunk::WAVFmtChunk(std::istream &input):WAVFmtChunk() {
    load(input) ;
}
//======================================================================
WAVFmtChunk::WAVFmtChunk(const std::uint8_t *ptr) {
    load(ptr) ;
}
