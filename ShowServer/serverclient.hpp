//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef serverclient_hpp
#define serverclient_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <memory>

#include "asio/asio.hpp"

#include "network/allpackets.hpp"
#include "network/client.hpp"
//======================================================================
class ServerClient : public Client {
    
public:
    using IdentCallback = std::function<void(ServerClient*)> ;
    using Pointer = std::shared_ptr<ServerClient> ;
    ServerClient(asio::io_context &io_context) ;
    
    std::string handle ;
    std::uint32_t server_key ;
    
    auto information() const -> std::string ;
    
    auto clientIdent( const Packet &packet,Client *client) -> bool ;
    auto clientNop( const Packet &packet, Client *client) -> bool;
    IdentCallback identify ;
    

};

#endif /* serverclient_hpp */
