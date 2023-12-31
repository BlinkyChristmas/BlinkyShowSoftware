//Copyright © 2023 Charles Kerr. All rights reserved.

#include "playpacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
PlayPacket::PlayPacket():Packet() {
    this->resize(PACKETSIZE) ;
    this->setPacketID(PacketType::PLAY) ;
    this->setLength(PACKETSIZE) ;
}

//======================================================================
PlayPacket::PlayPacket(const std::vector<std::uint8_t> &data) : Packet(data) {
    
}

//======================================================================
auto PlayPacket::state() const -> bool {
    return (this->read<int>(STATEOFFSET) != 0) ;
}
//======================================================================
auto PlayPacket::setState(bool value) -> void {
    this->write((value? 1: 0) , STATEOFFSET) ;
}
//======================================================================
auto PlayPacket::frame() const -> std::uint32_t {
    return (this->read<std::uint32_t>(FRAMEOFFSET)) ;
}
//======================================================================
auto PlayPacket::setFrame(std::uint32_t value) -> void {
    this->write(value , FRAMEOFFSET) ;
}
