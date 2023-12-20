//Copyright © 2023 Charles Kerr. All rights reserved.

#include "blinkclient.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
// Foward delcares, where the real action occurs
auto clientLoad(const Packet &packet,BlinkClient *ptr) -> bool ;
auto clientNop(const Packet &packet,BlinkClient *ptr) -> bool ;
auto clientPlay(const Packet &packet,BlinkClient *ptr) -> bool ;
auto clientShow(const Packet &packet,BlinkClient *ptr) -> bool ;
auto clientSync(const Packet &packet,BlinkClient *ptr) -> bool ;

// ========================================================================================
auto BlinkClient::processLoadPacket(const Packet &packet) -> bool {
    return clientLoad(packet,this) ;
}
// ========================================================================================
auto BlinkClient::processNopPacket(const Packet &packet) -> bool {
    return clientNop(packet,this) ;
}
// ========================================================================================
auto BlinkClient::processPlayPacket(const Packet &packet) -> bool {
    return clientPlay(packet,this) ;
}
// ========================================================================================
auto BlinkClient::processShowPacket(const Packet &packet) -> bool {
    return clientShow(packet,this) ;
}
// ========================================================================================
auto BlinkClient::processSyncPacket(const Packet &packet) -> bool {
    return clientSync(packet,this) ;
}

// ========================================================================================
BlinkClient::BlinkClient(asio::io_context &context,std::uint32_t key ) : NameClient(context,key){
    
}
// ========================================================================================
BlinkClient::BlinkClient(asio::io_context &context,IdentPacket::ClientType clienttype, std::uint32_t key ): NameClient(context,clienttype,key) {
    
}
