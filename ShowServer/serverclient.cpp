//Copyright © 2023 Charles Kerr. All rights reserved.

#include "serverclient.hpp"

#include <algorithm>
#include <stdexcept>
#include <functional>

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
using namespace std::string_literals ;

//======================================================================
ServerClient::ServerClient(asio::io_context &io_context) : Client(io_context),handle("Unknown"),identify(nullptr),server_key(0xdeadbeef){
    this->setPacketRoutine(Packet::IDENT, std::bind(&ServerClient::clientIdent,this,std::placeholders::_1,std::placeholders::_2)) ;
    this->setPacketRoutine(Packet::NOP, std::bind(&ServerClient::clientNop,this,std::placeholders::_1,std::placeholders::_2)) ;
}


//======================================================================
auto ServerClient::information() const -> StatusEntry {
    auto rvalue = Client::information() ;
    rvalue.handle = handle ;
    return rvalue ;
}

// ============================================================================
auto ServerClient::clientIdent( const Packet &packet,Client *client) -> bool{
    auto ptr = static_cast<const IdentPacket*>(&packet) ;
    //DBGMSG(std::cout,"Identification packet received") ;
    if (ptr->key() != server_key){
        // We need to get rid of this
        DBGMSG(std::cout,"Bad Server key: "s + util::ntos(ptr->key(),16,true,8) + " looking for "s + util::ntos(server_key,16,true,8)) ;
        
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
    //DBGMSG(std::cout, "Client type: "s + IdentPacket::nameForClient(this->type()));
    servClient->handle = ptr->handle() ;
    //DBGMSG(std::cout, "Client name: "s + ptr->handle());
    if (identify!=nullptr) {
        DBGMSG(std::cout, "Calling identify callback!") ;
        identify(this) ;
    }
    return true ;
}
// ============================================================================
auto ServerClient::clientNop( const Packet &packet, Client *client) -> bool{
    auto ptr = static_cast<const NopPacket*>(&packet) ;
    auto servClient = static_cast<ServerClient*>(client);
    DBGMSG(std::cout, "Received NOP packet!") ;
    if(ptr->respond()){
        DBGMSG(std::cout, "sending NOP response!") ;
        auto packet = NopPacket() ;
        servClient->sendPacket(packet) ;
    }
    return true ;
}
