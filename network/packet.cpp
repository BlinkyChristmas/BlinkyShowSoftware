//Copyright © 2023 Charles Kerr. All rights reserved.

#include "packet.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"
using namespace std::string_literals ;

//======================================================================
Packet::Packet() : util::Buffer(8),packetTime(std::chrono::system_clock::now()) {
    extend = true ;
    this->setPacketID(PacketType::INVALID);
    this->setLength(8) ;
}
//======================================================================
Packet::Packet(const std::vector<std::uint8_t> &data) : util::Buffer(data),packetTime(std::chrono::system_clock::now()) {
    extend = true ;
}

//======================================================================
Packet::Packet( const Packet &value){
    this->data = value.data ;
    this->extend = value.extend ;
    this->offset = value.offset ;
}
//======================================================================
auto Packet::operator=(const Packet &value) -> Packet& {
    this->data = value.data ;
    this->extend = value.extend ;
    this->offset = value.offset ;

    return *this ;
}

//======================================================================
auto Packet::appendData(const std::vector<std::uint8_t> &value) -> void {
    data.insert(data.end(), value.begin(),value.end());
}

//======================================================================
auto Packet::packetID() const -> PacketType  {
    if (this->size() < 4) {
        return PacketType::INVALID ;
    }
    auto id = this->read<int>(IDOFFSET) ;
    if (id < 0 || id > static_cast<int>(PacketType::TYPECOUNT)){
        return PacketType::INVALID ;
    }
    return static_cast<PacketType>(id) ;
}
//======================================================================
auto Packet::setPacketID(PacketType value) -> void {
    this->write(static_cast<int>(value),IDOFFSET);
}

//======================================================================
auto Packet::length() const -> size_t {
    if (this->size() < 8) {
        return 0 ;
    }
    return static_cast<size_t>(this->read<std::uint32_t>(LENGTHOFFSET)) ;

}
//======================================================================
auto Packet::setLength(size_t value) -> void {
    this->write(static_cast<std::uint32_t>(value),IDOFFSET);
}

//======================================================================
auto Packet::stamp() -> void {
    this->packetTime = std::chrono::system_clock::now() ;
}

//======================================================================
auto Packet::deltaMicro(const std::chrono::system_clock::time_point &timepoint) -> size_t {
    return std::chrono::duration_cast<std::chrono::microseconds>(timepoint - packetTime).count() ;
}
