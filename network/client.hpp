//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef client_hpp
#define client_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <queue>

#include "asio/asio.hpp"
#include "packet.hpp"
#include "identpacket.hpp"
#include "utility/timeutil.hpp"

struct StatusEntry {
    static constexpr auto HANDLESIZE = 20 ;
    static constexpr auto TIMESIZE = 16 ;
    static constexpr auto CLIENTSIZE = 7 ;
    static constexpr auto ADDRESSSIZE = 18 ;
    
    static constexpr auto ENTRYSIZE = (TIMESIZE * 3) + HANDLESIZE + CLIENTSIZE + ADDRESSSIZE ;
    
    std::string client; // 7 bytes
    std::string address ; //18
    std::string handle ; // 20 bytes
    std::string connectTime ; // 16 bytes
    std::string receiveTime ; // 16 bytes
    std::string sendTime ; // 16 bytes
    
    auto data() const -> std::vector<std::uint8_t> ;
    auto load(const std::vector<std::uint8_t> &data) -> void ;
    StatusEntry() = default ;
    StatusEntry(const std::vector<std::uint8_t> &data);
    auto toString() const -> std::string ;
};

//======================================================================
class Client : public std::enable_shared_from_this<Client> {
public:
    // This is what our packet process routines signatures look like
    using PacketProcessing = std::function<bool( const Packet &, Client *)>  ;
    // Just a define for a shard pointer of our client
    using ClientPointer = std::shared_ptr<Client> ;
    
protected:
    // Incoming packet stuff
    Packet incomingPacket ;
    size_t inBytes ;
    std::unordered_map<Packet::PacketType,Client::PacketProcessing > packetRoutines ;
    // this is for reading packets
    auto packetRead(const asio::error_code& ec, size_t bytes_transferred) -> void ;
    auto processPacket(const Packet &packet) -> bool ;
    
    // Outgoing packet stuff
    Packet outgoingPacket ;
    size_t bytesOut ;
    std::queue<Packet> outPackets ;
    mutable std::recursive_mutex outAccess ;
    std::atomic<bool> sendInProgress ;
    // And writing packets
    auto packetWrite(const asio::error_code& ec, size_t bytes_transferred) -> void ;
    
    
    // Peer information
    std::string peer_address ;
    std::string peer_port ;
    
    // Some time information
    util::ourclock::time_point connect_time ;
    util::ourclock::time_point receive_time ;
    util::ourclock::time_point send_time ;
    
    // Stuff that makes us a tad unique from a generic client
    IdentPacket::ClientType client_type ;
public:
    asio::ip::tcp::socket netSocket ; // We expose this just for convienence, if we ever need it (we shouldn't)
    
    // A simple way to get shared pointers of a client
    static auto createPointer(asio::io_context &io_context) -> ClientPointer ;
    // If you need to resolve an ip. We will put the port to the endpoint if it resolves
    static auto resolve(const std::string &ipaddress,int serverPort) -> asio::ip::tcp::endpoint ;
    
    
    Client(asio::io_context &context) ;
    Client(asio::io_context &context, IdentPacket::ClientType clienttype);
    virtual ~Client() ;
    
    // Open and close us
    auto is_open() const -> bool ;
    auto close() -> void ;
    // A way to connect to a server , and its port
    auto connect(asio::ip::tcp::endpoint &endpoint) -> bool ;
    
    // If one needs to bind our socket
    auto bind(int port) -> bool ;
    
    // Get our reads started
    auto initialRead() -> void ;
    
    [[maybe_unused]] auto sendPacket(const Packet &packet) -> bool ;
    
    // A few time related functions
    auto connectTime() const -> util::ourclock::time_point ;
    
    // Monitor our receive times
    auto lastReceiveTime() const -> util::ourclock::time_point ;
    auto minutesSinceReceive(const util::ourclock::time_point &time = util::ourclock::now()) -> size_t ;
    // same with our send times
    auto lastSendTime() const -> util::ourclock::time_point ;
    auto milliSinceSend(const util::ourclock::time_point &time  = util::ourclock::now()) -> size_t ;
    
    
    // if we are conneced, we can get/set our peer ip/port
    auto setPeerInformation() -> void ;
    
    // address and total information
    auto address() const -> std::string ;
    auto information() const -> StatusEntry;
    
    
    // This lets use set our callbacks for packets
    auto setPacketRoutine(Packet::PacketType type, PacketProcessing routine) -> void ;
    // Set our client key (unique to blnky)
    auto setClientType(IdentPacket::ClientType clienttype) -> void ;
    auto clientType() const -> const std::string& ;
    auto type() const -> IdentPacket::ClientType ;
    
    auto clearSendBuffer() -> void ;
};


#endif /* client_hpp */
