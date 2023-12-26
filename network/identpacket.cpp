//Copyright © 2023 Charles Kerr. All rights reserved.

#include "identpacket.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std::string_literals ;

//======================================================================
auto IdentPacket::clientTypeFor(int clientid) -> ClientType {
    if (clientid < 0 || clientid >= static_cast<int>(ClientType::TYPECOUNT) ) {
        return ClientType::UNKNOWN ;
    }
    return static_cast<ClientType>(clientid);
}

//======================================================================
const std::vector<std::string> IdentPacket::CLIENTNAMES {
    "UNKNOWN"s,"CLIENT"s,"STATUS"s,"MASTER"s
};
//======================================================================
auto IdentPacket::nameForClient(ClientType type) -> const std::string & {
    if (static_cast<int>(type) >= static_cast<int>(CLIENTNAMES.size())){
        return CLIENTNAMES.at(0) ;
    }
    return CLIENTNAMES.at(static_cast<int>(type));
}

//======================================================================
IdentPacket::IdentPacket() : Packet() {
    this->resize(PACKETSIZE) ;
    this->setPacketID(PacketType::IDENT) ;
    this->setLength(PACKETSIZE) ;
}

//======================================================================
IdentPacket::IdentPacket(const std::vector<std::uint8_t> &data) : Packet(data) {
    
}

//======================================================================
auto IdentPacket::handle() const -> std::string {
    return this->read<std::string>(HANDLESIZE,HANDLEOFFSET) ;
}
//======================================================================
auto IdentPacket::setHandle(const std::string &value) -> void {
    this->write(value,HANDLESIZE,HANDLEOFFSET) ;
}

//======================================================================
auto IdentPacket::clientType() const -> ClientType  {
    auto value = this->read<int>(TYPEOFFSET) ;
    if (value < 0 || value >= ClientType::TYPECOUNT) {
        return ClientType::UNKNOWN ;
    }
    return static_cast<ClientType>(value) ;
}
//======================================================================
auto IdentPacket::setClientType(ClientType type) -> void {
    this->write(static_cast<int>(type),TYPEOFFSET) ;
}

//======================================================================
auto IdentPacket::setKey(std::uint32_t value) -> void {
    this->write(value,KEYOFFSET) ;
}

//======================================================================
auto IdentPacket::key() const -> std::uint32_t {
    return this->read<std::uint32_t>(KEYOFFSET) ;
}
