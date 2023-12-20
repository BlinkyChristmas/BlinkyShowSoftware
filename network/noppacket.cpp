//Copyright © 2023 Charles Kerr. All rights reserved.

#include "noppacket.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std::string_literals ;

//======================================================================
NopPacket::NopPacket():Packet() {
    this->resize(PACKETSIZE) ;
    this->setPacketID(PacketType::NOP) ;
    this->setLength(PACKETSIZE) ;
}

//======================================================================
NopPacket::NopPacket(const std::vector<std::uint8_t> &data) : Packet(data) {
    
}

//======================================================================
auto NopPacket::respond() const -> bool {
    return this->read<int>(RESPONDOFFSET) != 0 ;
}
//======================================================================
auto NopPacket::setRespond(bool value) -> void {
    this->write((value ? 1:0),RESPONDOFFSET) ;
}
