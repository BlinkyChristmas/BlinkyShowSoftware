//Copyright © 2023 Charles Kerr. All rights reserved.

#include "loadpacket.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std::string_literals ;

//======================================================================
LoadPacket::LoadPacket():Packet() {
    this->resize(PACKETSIZE) ;
    this->setPacketID(PacketType::LOAD) ;
    this->setLength(PACKETSIZE) ;
}

//======================================================================
LoadPacket::LoadPacket(const std::vector<std::uint8_t> &data) : Packet(data) {
    
}

//======================================================================
auto LoadPacket::music() const -> std::string {
    return this->read<std::string>(MUSICSIZE,MUSICOFFSET) ;
}
//======================================================================
auto LoadPacket::setMusic(const std::string &value) -> void {
    this->write(value,MUSICOFFSET,MUSICSIZE) ;
}
//======================================================================
auto LoadPacket::light() const -> std::string {
    return this->read<std::string>(LIGHTSIZE,LIGHTOFFSET) ;

}
//======================================================================
auto LoadPacket::setLight(const std::string &value) -> void {
    this->write(value,LIGHTOFFSET,LIGHTSIZE) ;

}

