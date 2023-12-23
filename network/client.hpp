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
    // This is what our packet process routines signatures look like
    using PacketProcessing = std::function<bool( const Packet &, Client *)>  ;
    
    using ClientPointer = std::shared_ptr<Client> ;

protected:
    
    Packet incomingPacket ;
    size_t bytesAsked ;
    
    std::string peer_address ;
    std::string peer_port ;
    
    util::ourclock::time_point connect_time ;
    util::ourclock::time_point receive_time ;
    util::ourclock::time_point send_time ;
    
    // We need to know when we should "shutdown" in addition to close
    bool is_connected ;
    
    // Stuff that makes us a tad unique from a generic client

    std::string name ;
    IdentPacket::ClientType type ;
    std::uint32_t server_key ;

    std::unordered_map<Packet::PacketType,Client::PacketProcessing > packetRoutines ;

    // this is for reading packets
    auto packetRead(const asio::error_code& err, size_t bytes_transferred) -> void ;
    auto processPacket(const Packet &packet) -> bool ;
    
 
public:
    auto processIdentPacket(const Packet &packet) -> bool ;

    asio::ip::tcp::socket netSocket ; // We expose this just for convienence, if we ever need it (we shouldn't)
    
    // A simple way to get shared pointers of a client
    static auto createPointer(asio::io_context &io_context) -> ClientPointer ;
    // If you need to resolve an ip. We will put the port to the endpoint if it resolves
    static auto resolve(const std::string &ipaddress,int serverPort) -> asio::ip::tcp::endpoint ;
 
    
    Client(asio::io_context &context) ;
    Client(asio::io_context &context, IdentPacket::ClientType clienttype, std::uint32_t key = 0xDEADBEEF);
     ~Client() ;

    // Open and close us
    auto is_open() const -> bool ;
    auto close() -> void ;
    // A way to connect to a server , and its port
    auto connect(asio::ip::tcp::endpoint &endpoint) -> bool ;
    
    // A way to set if connected if from an acceptor
    auto setIsConnected(bool state) -> void ;
    
    // If one needs to bind our socket
    auto bind(int port) -> bool ;
   
    // Get our reads started
    auto initialRead() -> void ;

    [[maybe_unused]] auto sendPacket(const Packet &packet) -> bool ;
    
    // A few time related functions
    auto setConnectTime(const util::ourclock::time_point &time = util::ourclock::now()) -> void ;
    auto connectTime() const -> util::ourclock::time_point ;
    // Monitor our receive times
    auto lastReceiveTime() const -> util::ourclock::time_point ;
    auto millSinceReceive(const util::ourclock::time_point &time = util::ourclock::now()) -> size_t ;
    // same with our send times
    auto lastSendTime() const -> util::ourclock::time_point ;
    auto millSinceSend(const util::ourclock::time_point &time  = util::ourclock::now()) -> size_t ;
 

    // if we are conneced, we can get/set our peer ip/port
    auto setPeerInformation() -> void ;
    auto address() const -> std::string ;
    

    // This lets use set our callbacks for packets
    auto setPacketRoutine(Packet::PacketType type, PacketProcessing routine) -> void ;
    // Set our server key (unique to blnky)
    auto setServerKey( std::uint32_t key) -> void ;
    // Set our client key (unique to blnky)
    auto setClientType(IdentPacket::ClientType clienttype) -> void ;
    
    // Get/set our handle (nickname) (unique to blnky) (
    auto handle() const -> const std::string& ;
    auto setHandle(const std::string &name) -> void ;
    
    auto clientType() const -> const std::string& ;
    auto setClientType() const -> void ;
    auto information() const -> std::string ;
};


#endif /* client_hpp */
