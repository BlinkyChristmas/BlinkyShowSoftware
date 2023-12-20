//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef loadpacket_hpp
#define loadpacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"

/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        music name          30 bytes            8  (extension is not included)
        light name          30 bytes            38 (extension is not included)
 */


//======================================================================
class LoadPacket : public Packet {
    
    static constexpr auto MUSICOFFSET = 8 ;
    static constexpr auto MUSICSIZE = 30 ;
    static constexpr auto LIGHTOFFSET = MUSICOFFSET + MUSICSIZE ;
    static constexpr auto LIGHTSIZE = 30 ;
    static constexpr auto PACKETSIZE = LIGHTOFFSET + LIGHTSIZE ;
    
public:
    LoadPacket() ;
    LoadPacket(const std::vector<std::uint8_t> &data);
    auto music() const -> std::string ;
    auto setMusic(const std::string &value) -> void ;
    auto light() const -> std::string ;
    auto setLight(const std::string &value) -> void ;

};


#endif /* loadpacket_hpp */
