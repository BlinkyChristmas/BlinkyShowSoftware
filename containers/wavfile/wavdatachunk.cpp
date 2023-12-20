//Copyright © 2023 Charles Kerr. All rights reserved.

#include "wavdatachunk.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
auto WAVDataChunk::clear() -> void {
    signature = 0 ;
    size = 0 ;
    offsetToData = 0 ;
}
//======================================================================
auto WAVDataChunk::load(std::istream &input) -> void {
    input.read(reinterpret_cast<char*>(&signature), sizeof(signature));
    if (signature != SIGNATURE){
        throw std::runtime_error("Wrong wav chunk type (looking for (data): "s + std::to_string(SIGNATURE)+" ): "s + std::to_string(signature)) ;
    }
    input.read(reinterpret_cast<char*>(&size),sizeof(size)) ;
    offsetToData  = input.tellg() ;
}

//======================================================================
auto WAVDataChunk::load(const std::uint8_t *ptr) -> void {
    std::copy(ptr,ptr+4,reinterpret_cast<std::uint8_t*>(&signature));
    if (signature != SIGNATURE){
        throw std::runtime_error("Wrong wav chunk type (looking for (data): "s + std::to_string(SIGNATURE)+" ): "s + std::to_string(signature)) ;
    }
    std::copy(ptr+4,ptr+8,reinterpret_cast<std::uint8_t*>(&size));
    ptrToData = ptr + 8 ;
}
//======================================================================
WAVDataChunk::WAVDataChunk():signature(0),size(0),offsetToData(0),ptrToData(nullptr) {
    
}

//======================================================================
WAVDataChunk::WAVDataChunk(std::istream &input):WAVDataChunk() {
    load(input) ;
}

