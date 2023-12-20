//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef syncpacket_hpp
#define syncpacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"

/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        frame               4 bytes             8      (unsigned)
 */


//======================================================================
class SyncPacket : public Packet {
    
    static constexpr auto FRAMEOFFSET = 8 ;
    static constexpr auto PACKETSIZE = 12 ;
    
public:
    SyncPacket() ;
    SyncPacket(const std::vector<std::uint8_t> &data);

    auto frame() const -> std::uint32_t ;
    auto setFrame(std::uint32_t value) -> void ;
    
};

#endif /* syncpacket_hpp */
