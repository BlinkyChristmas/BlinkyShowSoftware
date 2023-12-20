//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef playpacket_hpp
#define playpacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"

/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        play                4 bytes             8  (0/1)
        frame               4 bytes             12
 */


//======================================================================
class PlayPacket : public Packet {
    
    static constexpr auto PLAYOFFSET = 8 ;
    static constexpr auto FRAMEOFFSET = 12;
    static constexpr auto PACKETSIZE = 16 ;
    
    
public:
    PlayPacket() ;
    PlayPacket(const std::vector<std::uint8_t> &data);

    auto play() const -> bool ;
    auto setPlay(bool value) -> void ;
    auto frame() const -> std::uint32_t ;
    auto setFrame(std::uint32_t value) -> void ;
};


#endif /* playpacket_hpp */
