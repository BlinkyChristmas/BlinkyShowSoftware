//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef identpacket_hpp
#define identpacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"


/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        client type         4 bytes             8
        server key          4 bytes             12
        handle              20 bytes            16
 */

//======================================================================
class IdentPacket : public Packet {
    static constexpr auto TYPEOFFSET = 8 ;
    static constexpr auto KEYOFFSET = TYPEOFFSET + 4 ;
    static constexpr auto HANDLEOFFSET = KEYOFFSET + 4 ;
    static constexpr auto PACKETSIZE = HANDLEOFFSET + 4 ;
    static constexpr auto HANDLESIZE = 20 ;
    static const std::vector<std::string> CLIENTNAMES ;
public:
    enum ClientType {
        UNKNOWN,CLIENT,STATUS,MASTER,TYPECOUNT
    };
    static auto nameForClient(ClientType type) -> const std::string & ;
    static auto clientTypeFor(int clientid) -> ClientType ;
    
    IdentPacket()  ;
    IdentPacket(const std::vector<std::uint8_t> &data)  ;

    auto handle() const -> std::string ;
    auto setHandle(const std::string &value) -> void ;

    auto clientType() const -> ClientType ;
    auto setClientType(ClientType type) -> void ;
    
    auto key() const -> std::uint32_t  ;
    auto setKey(std::uint32_t value) -> void ;

};

#endif /* identpacket_hpp */
