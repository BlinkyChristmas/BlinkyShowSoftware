//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef client_hpp
#define client_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "asio/asio.hpp"
#include "allpackets.hpp"
#include "utility/timeutil.hpp"
//======================================================================
class Client {
    
protected:
    asio::ip::tcp::socket netSocket ;
    
    Packet incomingPacket ;
    size_t bytesAsked ;
    
    std::string peer_address ;
    std::string peer_port ;
    
    util::ourclock::time_point connect_time ;
    util::ourclock::time_point receive_time ;
    util::ourclock::time_point send_time ;
    
    auto packetRead(const asio::error_code& err, size_t bytes_transferred) -> void ;
    virtual auto processPacket(const Packet &packet) -> bool ;

public:
    Client(asio::io_context &context) ;
    virtual ~Client() ;

    auto socket() -> asio::ip::tcp::socket& ;
    
    auto is_open() const -> bool ;
    auto shutdown() -> void ;
    auto close() -> void ;
    auto connect(asio::ip::tcp::endpoint &endpoint, int clientPort) -> bool ;
    auto bind(int port) -> bool ;
    static auto resolve(const std::string &ipaddress, int serverPort) -> asio::ip::tcp::endpoint ;
    
    [[maybe_unused]] auto sendPacket(const Packet &packet) -> bool ;
    
    auto isValid() const -> bool ;
    
    auto setPeerInformation() -> void ;
    auto address() const -> std::string ;
    
    auto setConnectTime(const util::ourclock::time_point &time = util::ourclock::now()) -> void ;
    auto connectTime() const -> util::ourclock::time_point ;
    
    auto lastReceiveTime() const -> util::ourclock::time_point ;
    auto millSinceReceive(const util::ourclock::time_point &time = util::ourclock::now()) -> size_t ;

    auto lastSendTime() const -> util::ourclock::time_point ;
    auto millSinceSend(const util::ourclock::time_point &time  = util::ourclock::now()) -> size_t ;
 
    
};


#endif /* client_hpp */
