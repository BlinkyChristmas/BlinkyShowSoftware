//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef blinkclient_hpp
#define blinkclient_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "nameclient.hpp"
//======================================================================
class BlinkClient : public NameClient {
    
protected:
    auto processLoadPacket(const Packet &packet) -> bool final ;
    auto processNopPacket(const Packet &packet) -> bool final ;
    auto processPlayPacket(const Packet &packet) -> bool final ;
    auto processShowPacket(const Packet &packet) -> bool final ;
    auto processSyncPacket(const Packet &packet) -> bool final ;

public:
    BlinkClient(asio::io_context &context,std::uint32_t key = 0xDEADBEEF ) ;
    BlinkClient(asio::io_context &context,IdentPacket::ClientType clienttype, std::uint32_t key = 0xDEADBEEF) ;
};

#endif /* blinkclient_hpp */
