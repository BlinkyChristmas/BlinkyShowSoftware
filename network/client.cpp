//Copyright © 2023 Charles Kerr. All rights reserved.

#include "client.hpp"

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <utility>

#include "utility/dbgutil.hpp"
#include "utility/timeutil.hpp"

using namespace std::string_literals ;


// =======================================================================================
// Client interrupt handlers for read/writing
// =======================================================================================
// ========================================================================================
auto Client::packetRead(const asio::error_code& ec, size_t bytes_transferred) -> void {
    //DBGMSG(std::cout, "Client Read: "s + ec.message() + " Bytes: "s + std::to_string(bytes_transferred));
    if (ec) {
        // Ok, if we get an error in our read, it is probably something wrong on the other end
        // So we just close it and bail
        DBGMSG(std::cerr, "Error on read: "s + ec.message());
        try {
            if (ec.value() != asio::error::operation_aborted){
                netSocket.close() ;
            }
            // we should clear out queued output
            auto empty = std::queue<Packet>() ;
            sendInProgress = false ;
        }
        catch(...) {
            // Do nothing, we are still closing
            DBGMSG(std::cerr,"Error closing our socket in read handler");
        }
        return ;
    }
    if (inBytes != bytes_transferred){
        auto amount = inBytes - bytes_transferred ;
        auto offset = incomingPacket.bufferData().size() - amount ;
        inBytes = amount ;
        netSocket.async_read_some(asio::buffer(incomingPacket.bufferData().data()+offset,inBytes),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    if (incomingPacket.size() != incomingPacket.length()) {
        inBytes = incomingPacket.length() - incomingPacket.size() ;
        incomingPacket.resize(incomingPacket.length()) ;
        auto offset = incomingPacket.size() - inBytes ;
        netSocket.async_read_some(asio::buffer(incomingPacket.bufferData().data()+offset,inBytes),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    incomingPacket.stamp() ;  // Timestamp the packet
    receive_time = util::ourclock::now() ;  // indicate the last time we got a packet
    DBGMSG(std::cout, "Received packet: "s + Packet::nameForPacket(incomingPacket.packetID()));

    if (processPacket(incomingPacket)) {
        incomingPacket = Packet() ;
        inBytes = incomingPacket.size() ;
        netSocket.async_read_some(asio::buffer(incomingPacket.bufferData().data(),inBytes),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    DBGMSG(std::cout, "Client Read: did not requeue read!");

}

// ========================================================================================
auto Client::packetWrite(const asio::error_code& ec, size_t bytes_transferred) -> void {
    if (ec) {
        // We had an error writing
        DBGMSG(std::cerr, "We had error writing packet: "s + ec.message()) ;
        // So what I think we are suppose to do here, is just shutdown the socket
        try {
            sendInProgress = false ;
            auto empty = std::queue<Packet>() ;
            sendInProgress = false ;
            DBGMSG(std::cerr, "Clearing our write packets "s ) ;
            {
                auto lock = std::lock_guard(outAccess);
                std::swap(empty,outPackets) ;
            }
            if (ec.value() != asio::error::connection_aborted && ec.value() != asio::error::broken_pipe){
                DBGMSG(std::cerr, "Shutting down"s ) ;
                netSocket.shutdown( asio::ip::tcp::socket::shutdown_type::shutdown_both );
            }
            else {
                DBGMSG(std::cerr, "Wasnt a connection abort, closing instead"s ) ;
                
                netSocket.close() ;
            }
        }
        catch(...) {
            DBGMSG(std::cerr, "We had error shutdowing our socket, but where sending: "s + ec.message()) ;
            // Should we close?
            netSocket.close() ;
        }
        return ;
    }
    if (bytesOut != bytes_transferred) {
        // we need to send some more
        auto amount = bytesOut - bytes_transferred ;
        auto offset = outgoingPacket.size() - amount ;
        bytesOut = amount ;
        netSocket.async_write_some(asio::buffer(outgoingPacket.bufferData().data()+offset,bytesOut),std::bind(&Client::packetWrite,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    // We sent it! do we have anyother data
    send_time = util::ourclock::now() ;
    DBGMSG(std::cout, "We completed writing packet: "s + Packet::nameForPacket(outgoingPacket.packetID())) ;
    auto lock = std::lock_guard(outAccess) ;
    if (!outPackets.empty()) {
        outgoingPacket = outPackets.front() ;
        outPackets.pop() ;
        bytesOut = outgoingPacket.size() ;
        netSocket.async_write_some(asio::buffer(outgoingPacket.bufferData().data(),bytesOut),std::bind(&Client::packetWrite,this,std::placeholders::_1,std::placeholders::_2));
        sendInProgress = true ;
    }
    else {
        sendInProgress = false ;
    }
    
}

// ========================================================================================
auto Client::processPacket(const Packet &packet) -> bool {
    auto id = packet.packetID() ;
    auto iter = packetRoutines.find(id) ;
    if (iter != packetRoutines.end()) {
        if (iter->second != nullptr){
//            DBGMSG(std::cerr, "Packet registered,for: "s + Packet::nameForPacket(id)) ;
            return iter->second(packet,this) ;
        }
        DBGMSG(std::cerr, "Packet registered, but null routine for: "s + Packet::nameForPacket(id)) ;
        return true ;
        
    }
    DBGMSG(std::cerr, "No packet routine for: "s + Packet::nameForPacket(id)) ;
    
    return true ;
}
// =============================================================================================
// Static methods
// =============================================================================================

// ========================================================================================
auto Client::createPointer(asio::io_context &io_context) -> ClientPointer {
    return ClientPointer(new Client(io_context));
}

// ========================================================================================
auto Client::resolve(const std::string &ipaddress, int serverPort) -> asio::ip::tcp::endpoint {
    asio::io_context io_context ;
    asio::ip::tcp::resolver resolver(io_context) ;
    asio::ip::tcp::resolver::query query(ipaddress.c_str(),"") ;
    auto iter = resolver.resolve(query) ;
    if (iter != asio::ip::tcp::resolver::iterator()) {
        auto serverEndpoint = iter->endpoint() ;
        // Now we need to add the port
        return asio::ip::tcp::endpoint(serverEndpoint.address(),serverPort) ;
    }
    else {
        return asio::ip::tcp::endpoint();
    }
}

// =============================================================================================
// Public methods
// =============================================================================================

// ========================================================================================
Client::Client(asio::io_context &context): netSocket(context), inBytes(0), receive_time(util::ourclock::now()), send_time(util::ourclock::now()), connect_time(util::ourclock::now()), client_type(IdentPacket::UNKNOWN){
}

// ========================================================================================
Client::Client(asio::io_context &context, IdentPacket::ClientType clienttype ):Client(context){
    this->client_type = clienttype ;
}

// ========================================================================================
Client::~Client() {
    if (netSocket.is_open()) {
        netSocket.cancel() ;
        netSocket.close() ;
    }
}

// ========================================================================================
auto Client::is_open() const -> bool {
    return netSocket.is_open() ;
}

// ========================================================================================
auto Client::close() -> void {
    if (netSocket.is_open()) {
        netSocket.cancel() ;
        try {
            netSocket.shutdown( asio::ip::tcp::socket::shutdown_type::shutdown_both );
        }
        catch (...) {
            
        }
        netSocket.close() ;
    }
}

// ========================================================================================
auto Client::connect(asio::ip::tcp::endpoint &endpoint) -> bool {
    try {
        
        asio::error_code ec ;
        if (!netSocket.is_open()) {
            netSocket.open(asio::ip::tcp::v4(), ec);
            if (ec) {
                // we had an error on the open?
                DBGMSG(std::cerr, "Error opening the socket: "s + ec.message());
                return false ;
            }
        }
        // Just to be sure, lets clear out any send packets
        auto empty = std::queue<Packet>() ;
        std::swap(empty,outPackets);
        sendInProgress = false ;
        
        netSocket.connect(endpoint,ec) ;
        if (ec) {
            DBGMSG(std::cerr, "Error on connecting socket: "s + ec.message());
            if (netSocket.is_open()) {
                netSocket.close() ;
            }
            return false ;
        }
        this->setPeerInformation() ;
        return true ;
    }
    catch(...) {
        DBGMSG(std::cerr, "Uknown error on connect") ;
        if (netSocket.is_open()) {
            netSocket.close() ;
        }
        return false ;
    }
    
}

// ========================================================================================
auto Client::bind(int port) -> bool {
    asio::error_code ec ;
    try {
        if (!netSocket.is_open()){
            netSocket.open(asio::ip::tcp::v4(),ec) ;
            if (ec) {
                DBGMSG(std::cerr, "Error on opening socket: "s + ec.message());
                netSocket.close() ;
                return false ;
            }
        }
        asio::socket_base::reuse_address option(true) ;
        netSocket.set_option(option) ;
        
        if (port > 0) {
            auto endpoint = asio::ip::tcp::endpoint(asio::ip::address_v4::any(),port) ;
            netSocket.bind(endpoint,ec) ;
            if (ec) {
                DBGMSG(std::cerr, "Error on binding socket: "s + ec.message());
                netSocket.close() ;
                return false ;
            }
        }
        return true ;
    }
    catch(...) {
        DBGMSG(std::cerr, "Unknown error on bind") ;
        netSocket.close() ;
        return false ;
    }
}

// ========================================================================================
auto Client::initialRead() -> void {
    if (netSocket.is_open()){
        incomingPacket = Packet();
        inBytes = incomingPacket.size() ;
        netSocket.async_read_some(asio::buffer(incomingPacket.data.data(),inBytes),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
    }
}
// ========================================================================================
auto Client::sendPacket(const Packet &packet) -> bool {
    DBGMSG(std::cout, "Queuing packet for sending: "s + Packet::nameForPacket(packet.packetID()));
    auto lock = std::lock_guard(outAccess);
    if (sendInProgress) {
        outPackets.push(packet) ;
    }
    else {
        outgoingPacket = packet ;
        bytesOut = packet.size() ;
        netSocket.async_write_some(asio::buffer(outgoingPacket.bufferData().data(),bytesOut),std::bind(&Client::packetWrite,this,std::placeholders::_1,std::placeholders::_2));
        sendInProgress = true ;
    }
    return true ;
}


// ========================================================================================
auto Client::setPeerInformation() -> void {
    try {
        if (this->is_open()){
            peer_address = netSocket.remote_endpoint().address().to_string() ;
            peer_port = std::to_string(netSocket.remote_endpoint().port()) ;
        }
    }
    catch(...){
        // We couldn't do it, probably not connected, so we skip
        DBGMSG(std::cerr, "Exception on getting peer information");
    }
}

// ========================================================================================
auto Client::address() const -> std::string {
    return peer_address + ":"s + peer_port ;
}

// ========================================================================================
auto Client::connectTime() const -> util::ourclock::time_point {
    return connect_time ;
}

// ========================================================================================
auto Client::lastReceiveTime() const -> util::ourclock::time_point {
    return receive_time ;
}
// ========================================================================================
auto Client::minutesSinceReceive(const util::ourclock::time_point &time ) -> size_t {
    return std::chrono::duration_cast<std::chrono::minutes>(time - receive_time).count() ;
}

// ========================================================================================
auto Client::lastSendTime() const -> util::ourclock::time_point {
    return send_time ;
}

// ========================================================================================
auto Client::milliSinceSend(const util::ourclock::time_point &time) -> size_t {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time - send_time).count() ;
}

// ========================================================================================
auto Client::setPacketRoutine(Packet::PacketType type, PacketProcessing routine) -> void {
    this->packetRoutines.insert_or_assign(type,routine) ;
}

// ========================================================================================
auto Client::setClientType(IdentPacket::ClientType clienttype) -> void {
    this->client_type = clienttype ;
}


// ========================================================================================
auto Client::clientType() const -> const std::string&  {
    return IdentPacket::nameForClient(client_type) ;
}

// ========================================================================================
auto Client::type() const -> IdentPacket::ClientType {
    return client_type ;
}

// ========================================================================================
auto Client::information() const -> std::string {
    const auto format = "%b %d %H:%M:%S"s ;
    return util::sysTimeToString(this->connect_time,format) + " , "s +  this->address() + " , "  + IdentPacket::nameForClient(client_type) + " , " + util::sysTimeToString(this->receive_time,format) + " , "s + util::sysTimeToString(this->send_time,format) ;
}

// ========================================================================================
auto Client::clearSendBuffer() -> void {
    auto empty = std::queue<Packet>() ;
    auto lock = std::lock_guard(outAccess) ;
    std::swap(empty,outPackets) ;
    sendInProgress = false ;
}
