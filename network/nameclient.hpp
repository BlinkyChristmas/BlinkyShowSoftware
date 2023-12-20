//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef nameclient_hpp
#define nameclient_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "client.hpp"

//======================================================================
class NameClient: public Client {
    
protected:
    std::string name ;
    IdentPacket::ClientType type ;
    std::uint32_t server_key ;
    
    auto processPacket(const Packet &packet) -> bool final  ;
    
    auto processIdentPacket(const Packet &packet) -> bool ;
    virtual auto processLoadPacket(const Packet &packet) -> bool ;
    virtual auto processNopPacket(const Packet &packet) -> bool ;
    virtual auto processPlayPacket(const Packet &packet) -> bool ;
    virtual auto processShowPacket(const Packet &packet) -> bool ;
    virtual auto processSyncPacket(const Packet &packet) -> bool ;

public:
    NameClient(asio::io_context &context,std::uint32_t key = 0xDEADBEEF ) ;
    NameClient(asio::io_context &context,IdentPacket::ClientType clienttype, std::uint32_t key ) ;
    virtual ~NameClient() = default;
};

#endif /* nameclient_hpp */
