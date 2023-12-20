//Copyright © 2023 Charles Kerr. All rights reserved.

#include "nameclient.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;


// ========================================================================================
auto NameClient::processPacket(const Packet &packet) -> bool {
    switch(packet.packetID()){
        case IdentPacket::IDENT:{
            return processIdentPacket(packet) ;
        }
        case IdentPacket::LOAD:{
            return processLoadPacket(packet) ;
        }
        case IdentPacket::NOP:{
            return processNopPacket(packet) ;
        }
        case IdentPacket::PLAY:{
            return processPlayPacket(packet) ;
        }
        case IdentPacket::SHOW:{
            return processShowPacket(packet) ;
        }
        case IdentPacket::SYNC:{
            return processSyncPacket(packet) ;
        }
        default:
            return true ;
    }
}

// ========================================================================================
auto NameClient::processIdentPacket(const Packet &packet) -> bool {
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
auto NameClient::processLoadPacket(const Packet &packet) -> bool {
    return true ;
}

// ========================================================================================
auto NameClient::processNopPacket(const Packet &packet) -> bool {
    return true ;
}

// ========================================================================================
auto NameClient::processPlayPacket(const Packet &packet) -> bool {
    return true ;
}

// ========================================================================================
auto NameClient::processShowPacket(const Packet &packet) -> bool {
    return true ;
}

// ========================================================================================
auto NameClient::processSyncPacket(const Packet &packet) -> bool {
    return true ;
}

// ========================================================================================
NameClient::NameClient(asio::io_context &context,std::uint32_t key):Client(context),type(IdentPacket::ClientType::UNKNOWN),server_key(key){
    
}

// ========================================================================================
NameClient::NameClient(asio::io_context &context,IdentPacket::ClientType clienttype, std::uint32_t key):Client(context),type(clienttype),server_key(key){
    
}
