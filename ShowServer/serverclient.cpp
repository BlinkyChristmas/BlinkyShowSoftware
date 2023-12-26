//Copyright © 2023 Charles Kerr. All rights reserved.

#include "serverclient.hpp"

#include <algorithm>
#include <stdexcept>
#include <functional>

using namespace std::string_literals ;

//======================================================================
ServerClient::ServerClient(asio::io_context &io_context) : Client(io_context),handle("Unknown"),identify(nullptr),server_key(0xdeadbeef){
    this->setPacketRoutine(Packet::IDENT, std::bind(&ServerClient::clientIdent,this,std::placeholders::_1,std::placeholders::_2)) ;
    this->setPacketRoutine(Packet::NOP, std::bind(&ServerClient::clientNop,this,std::placeholders::_1,std::placeholders::_2)) ;
}


//======================================================================
auto ServerClient::information() const -> std::string {
    auto rvalue = Client::information() ;
    rvalue +" , "s + handle ;
    return rvalue ;
}

// ============================================================================
auto ServerClient::clientIdent( const Packet &packet,Client *client) -> bool{
    auto ptr = static_cast<const IdentPacket*>(&packet) ;
    if (ptr->key() != server_key){
        // We need to get rid of this
        try {
            client->netSocket.shutdown(asio::ip::tcp::socket::shutdown_type::shutdown_both);
            client->netSocket.close();
        }
        catch(...) {
            client->netSocket.close() ;
        }
        return false ;
    }
    auto servClient = static_cast<ServerClient*>(client);
    servClient->setClientType(ptr->clientType());
    servClient->handle = ptr->handle() ;
    if (identify!=nullptr) {
        identify(this) ;
    }
    return true ;
}
// ============================================================================
auto ServerClient::clientNop( const Packet &packet, Client *client) -> bool{
    auto ptr = static_cast<const NopPacket*>(&packet) ;
    auto servClient = static_cast<ServerClient*>(client);
    if(ptr->respond()){
        auto packet = NopPacket() ;
        servClient->sendPacket(packet) ;
    }
    return true ;

}
