//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef packet_hpp
#define packet_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <chrono>
#include <ostream>

#include "utility/buffer.hpp"
/* ***************************************************************************************
 Format:
        Data                Size                Offset
 
        packet id           4 bytes             0
        packet size         4 bytes             4
 */



//======================================================================
class Packet : public util::Buffer {
    static constexpr auto IDOFFSET = 0 ;
    static constexpr auto LENGTHOFFSET = 4 ;
protected:
    static const std::vector<std::string> TYPENAMES ;

public:
    enum PacketType {
        INVALID,IDENT,SHOW,PLAY,SYNC,LOAD,NOP,STATUS,TYPECOUNT
    };
    static auto nameForPacket(PacketType type) -> const std::string & ;
    std::chrono::system_clock::time_point packetTime ;
    
    Packet() ;
    Packet(const std::vector<std::uint8_t> &data) ;
    Packet( const Packet &value) ;
    auto operator=(const Packet &value) -> Packet& ;
    auto appendData(const std::vector<std::uint8_t> &value) -> void ;
    
    auto packetID() const -> PacketType ;
    auto setPacketID(PacketType type) -> void ;
    
    auto length() const -> size_t ;
    auto setLength(size_t value) -> void ;
    
    auto stamp() -> void ;
    auto deltaMicro(const std::chrono::system_clock::time_point &timepoint) -> size_t ;
    
};

#endif /* packet_hpp */
