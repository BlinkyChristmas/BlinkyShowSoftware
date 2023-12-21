//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef blinkclient_hpp
#define blinkclient_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <functional>

#include <unordered_map>

#include "client.hpp"
#include "allpackets.hpp"

using PacketProcessing = std::function<bool( const Packet &)>  ;
//======================================================================
class BlinkClient: public Client {
    
protected:
    std::string name ;
    IdentPacket::ClientType type ;
    std::uint32_t server_key ;

    std::unordered_map<Packet::PacketType,PacketProcessing > packetRoutines ;

    auto processPacket(const Packet &packet) -> bool final  ;
    
    auto processIdentPacket(const Packet &packet) -> bool ;

public:
    BlinkClient(asio::io_context &context,std::uint32_t key = 0xDEADBEEF ) ;
    BlinkClient(asio::io_context &context,IdentPacket::ClientType clienttype, std::uint32_t key ) ;
    virtual ~BlinkClient() = default;
    
    auto setPacketRountine(Packet::PacketType type, PacketProcessing routine) -> void ;
    auto setServerKey( std::uint32_t key) -> void ;
};



#endif /* blinkclient_hpp */
