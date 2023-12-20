//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef noppacket_hpp
#define noppacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"


/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        respond             4 bytes             8      (0/1)
 */

//======================================================================
class NopPacket : public Packet {
    
    static constexpr auto RESPONDOFFSET = 8 ;
    static constexpr auto PACKETSIZE = 12 ;
public:
    NopPacket() ;
    NopPacket(const std::vector<std::uint8_t> &data);

    auto respond() const -> bool ;
    auto setRespond(bool value) -> void ;
};

#endif /* noppacket_hpp */
