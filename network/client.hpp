//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef client_hpp
#define client_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "asio/asio.hpp"
#include "packet.hpp"
#include "identpacket.hpp"
#include "utility/timeutil.hpp"


//======================================================================
class Client : public std::enable_shared_from_this<Client> {
public:
    using ClientPointer = std::shared_ptr<Client> ;
    using PacketProcessing = std::function<bool( const Packet &)>  ;

protected:
    
    Packet incomingPacket ;
    size_t bytesAsked ;
    
    std::string peer_address ;
    std::string peer_port ;
    
    util::ourclock::time_point connect_time ;
    util::ourclock::time_point receive_time ;
    util::ourclock::time_point send_time ;

    std::string name ;
    IdentPacket::ClientType type ;
    std::uint32_t server_key ;

    std::unordered_map<Packet::PacketType,Client::PacketProcessing > packetRoutines ;


    auto packetRead(const asio::error_code& err, size_t bytes_transferred) -> void ;
    auto processPacket(const Packet &packet) -> bool ;
    auto processIdentPacket(const Packet &packet) -> bool ;

public:

    asio::ip::tcp::socket netSocket ;
    static auto createPointer(asio::io_context &io_context) -> ClientPointer ;

    Client(asio::io_context &context) ;
    Client(asio::io_context &context, IdentPacket::ClientType clienttype, std::uint32_t key = 0xDEADBEEF);
     ~Client() ;

    
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
 
    auto initialRead() -> void ;

    auto setPacketRoutine(Packet::PacketType type, PacketProcessing routine) -> void ;
    auto setServerKey( std::uint32_t key) -> void ;
    auto setClientType(IdentPacket::ClientType clienttype) -> void ;
    
    auto handle() const -> const std::string& ;
    auto clientType() const -> const std::string& ;

};


#endif /* client_hpp */
