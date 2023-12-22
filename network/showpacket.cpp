//Copyright © 2023 Charles Kerr. All rights reserved.

#include "showpacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
ShowPacket::ShowPacket():Packet() {
    this->resize(PACKETSIZE) ;
    this->setPacketID(PacketType::SHOW) ;
    this->setLength(PACKETSIZE) ;
}

//======================================================================
ShowPacket::ShowPacket(const std::vector<std::uint8_t> &data) : Packet(data) {
    
}

//======================================================================
auto ShowPacket::state() const -> bool {
    return (this->read<int>(STATEOFFSET) != 0) ;
}
//======================================================================
auto ShowPacket::setState(bool value) -> void {
    this->write((value? 1: 0) , STATEOFFSET) ;
}
