//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef showpacket_hpp
#define showpacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"

/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        state               4 bytes             8  (0/1)
 */


//======================================================================
class ShowPacket : public Packet {
    
    static constexpr auto STATEOFFSET = 8 ;
    static constexpr auto PACKETSIZE = 12 ;
    
public:
    ShowPacket() ;
    ShowPacket(const std::vector<std::uint8_t> &data);

    auto state() const -> bool ;
    auto setState(bool value) -> void ;
};


#endif /* showpacket_hpp */
