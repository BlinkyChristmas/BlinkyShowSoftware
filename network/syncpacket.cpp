//Copyright © 2023 Charles Kerr. All rights reserved.

#include "syncpacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
SyncPacket::SyncPacket():Packet() {
    this->resize(PACKETSIZE) ;
    this->setPacketID(PacketType::SYNC) ;
    this->setLength(PACKETSIZE) ;
}

//======================================================================
SyncPacket::SyncPacket(const std::vector<std::uint8_t> &data) : Packet(data) {
    
}

//======================================================================
auto SyncPacket::frame() const -> std::uint32_t {
    return this->read<std::uint32_t>(FRAMEOFFSET) ;
}
//======================================================================
auto SyncPacket::setFrame(std::uint32_t value) -> void {
    this->write(value,FRAMEOFFSET) ;
}
