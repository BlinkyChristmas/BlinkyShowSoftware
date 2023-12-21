//Copyright © 2023 Charles Kerr. All rights reserved.

#include "blinkclient.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"
using namespace std::string_literals ;


// ========================================================================================
auto BlinkClient::processPacket(const Packet &packet) -> bool {
    auto id = packet.packetID() ;
    switch(id){
        case IdentPacket::IDENT:{
            return processIdentPacket(packet) ;
        }
        default: {
            auto iter = packetRoutines.find(id) ;
            if (iter != packetRoutines.end()) {
                if (iter->second != nullptr){
                    return iter->second(packet) ;
                }
                return true ;
            }
            DBGMSG(std::cerr, "Unknown packet received: "s + std::to_string(id)) ;
            return true ;
        }
    }
}

// ========================================================================================
auto BlinkClient::processIdentPacket(const Packet &packet) -> bool {
    auto ptr = static_cast<const IdentPacket*>(&packet) ;
    name = ptr->handle() ;
    type = ptr->clientType() ;
    if (server_key != ptr->key()){
        this->close() ;
        return false ;
    }
    return true ;
}


// ========================================================================================
BlinkClient::BlinkClient(asio::io_context &context,std::uint32_t key):Client(context),type(IdentPacket::ClientType::UNKNOWN),server_key(key){
    packetRoutines.insert_or_assign(Packet::LOAD,nullptr) ;
    packetRoutines.insert_or_assign(Packet::NOP,nullptr) ;
    packetRoutines.insert_or_assign(Packet::PLAY,nullptr) ;
    packetRoutines.insert_or_assign(Packet::SHOW,nullptr) ;
    packetRoutines.insert_or_assign(Packet::SYNC,nullptr) ;

}

// ========================================================================================
BlinkClient::BlinkClient(asio::io_context &context,IdentPacket::ClientType clienttype, std::uint32_t key):Client(context),type(clienttype),server_key(key){
    packetRoutines.insert_or_assign(Packet::LOAD,nullptr) ;
    packetRoutines.insert_or_assign(Packet::NOP,nullptr) ;
    packetRoutines.insert_or_assign(Packet::PLAY,nullptr) ;
    packetRoutines.insert_or_assign(Packet::SHOW,nullptr) ;
    packetRoutines.insert_or_assign(Packet::SYNC,nullptr) ;

}
// ========================================================================================
auto BlinkClient::setPacketRountine(Packet::PacketType type, std::function<bool(const Packet&)> &routine) -> void {
    this->packetRoutines.insert_or_assign(type,routine) ;
}
