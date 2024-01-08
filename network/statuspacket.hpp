//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef statuspacket_hpp
#define statuspacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "packet.hpp"

struct StatusEntry ;

/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
        #entries            4 bytes             8      (unsigned)
        [entries]
 */
//======================================================================

class StatusPacket : public Packet {
    
    static constexpr auto ENTRYOFFSET = 8 ;
    static constexpr auto MINSIZE = 12 ;
    
public:
    StatusPacket() ;
    StatusPacket(const std::vector<std::uint8_t> &data);
    
    auto entryCount() const -> std::uint32_t ;
    auto setEntryCount(std::uint32_t value) -> void ;
    auto setEntries(const std::vector<StatusEntry> &entries) -> void ;
    auto allEntries() const -> std::vector<StatusEntry> ;
    
};
 
#endif /* statuspacket_hpp */
