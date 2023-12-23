//Copyright © 2023 Charles Kerr. All rights reserved.

#include "client.hpp"

#include <algorithm>
#include <stdexcept>
#include <sstream>

#include "utility/dbgutil.hpp"
#include "utility/timeutil.hpp"

using namespace std::string_literals ;


// ========================================================================================
auto Client::packetRead(const asio::error_code& err, size_t bytes_transferred) -> void {
    if (err) {
        if (netSocket.is_open()) {
            DBGMSG(std::cerr, "Error on read: "s + err.message());
            this->close() ;
        }
        return ;
    }
    if (bytesAsked != bytes_transferred){
        auto amount = bytesAsked - bytes_transferred ;
        auto offset = incomingPacket.bufferData().size() - amount ;
        bytesAsked = amount ;
        netSocket.async_read_some(asio::buffer(incomingPacket.bufferData().data()+offset,bytesAsked),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    if (incomingPacket.size() != incomingPacket.length()) {
        bytesAsked = incomingPacket.length() - incomingPacket.size() ;
        incomingPacket.resize(incomingPacket.length()) ;
        auto offset = incomingPacket.size() - bytesAsked ;
        netSocket.async_read_some(asio::buffer(incomingPacket.bufferData().data()+offset,bytesAsked),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }
    incomingPacket.stamp() ;  // Timestamp the packet
    receive_time = util::ourclock::now() ;  // indicate the last time we got a packet
    if (processPacket(incomingPacket)) {
        incomingPacket = Packet() ;
        bytesAsked = incomingPacket.size() ;
        netSocket.async_read_some(asio::buffer(incomingPacket.bufferData().data(),bytesAsked),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
    }
}

// ========================================================================================
auto Client::processPacket(const Packet &packet) -> bool {
    auto id = packet.packetID() ;
    auto iter = packetRoutines.find(id) ;
    if (iter != packetRoutines.end()) {
        if (iter->second != nullptr){
            return iter->second(packet,this) ;
        }
        DBGMSG(std::cerr, "Packet registured, but null routine for: "s + Packet::nameForPacket(id)) ;
        return true ;
        
    }
    else if(id == IdentPacket::IDENT){
        // We will run our internal ident handler
        return processIdentPacket(packet) ;
    }
    DBGMSG(std::cerr, "No packet routine for: "s + Packet::nameForPacket(id)) ;
    
    return true ;
}
// ========================================================================================
auto Client::processIdentPacket(const Packet &packet) -> bool {
    auto ptr = static_cast<const IdentPacket*>(&packet) ;
    name = ptr->handle() ;
    type = ptr->clientType() ;
    if (server_key != ptr->key()){
        // This didn't match our key, lets get rid of this ;
        this->close() ;
        return false ; // Dont reissue the read
    }
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
Client::Client(asio::io_context &context): netSocket(context), bytesAsked(0), receive_time(util::ourclock::now()), send_time(util::ourclock::now()), connect_time(util::ourclock::now()), type(IdentPacket::CLIENT), server_key(0XDEADBEEF), is_connected(false){
}

// ========================================================================================
Client::Client(asio::io_context &context, IdentPacket::ClientType clienttype, std::uint32_t key ):Client(context){
    this->type = clienttype ;
    this->server_key = key ;
}

// ========================================================================================
Client::~Client() {
    this->close() ;
}


// ========================================================================================
auto Client::is_open() const -> bool {
    return netSocket.is_open() ;
}

// ========================================================================================
auto Client::close() -> void {
    if (netSocket.is_open()) {
        try {
            netSocket.cancel();
            if (is_connected){
                netSocket.shutdown( asio::ip::tcp::socket::shutdown_type::shutdown_both ) ;
                is_connected = false ;
            }
            netSocket.close() ;
        }
        catch(...) {
            DBGMSG(std::cerr, "Error closing client socket.");
            netSocket.close() ;
        }
    }
}

// ========================================================================================
auto Client::connect(asio::ip::tcp::endpoint &endpoint) -> bool {
    try {
        asio::error_code ec ;
        netSocket.connect(endpoint,ec) ;
        if (ec) {
            DBGMSG(std::cerr, "Error on connecting socket: "s + ec.message());
            this->close() ;
            return false ;
        }
        is_connected = true ;
        this->setPeerInformation() ;
        return true ;
    }
    catch(...) {
        DBGMSG(std::cerr, "Uknown error on connect") ;
        this->close() ;
        return false ;
    }
    
}

// ========================================================================================
auto Client::setIsConnected(bool state) -> void {
    is_connected = state ;
}
// ========================================================================================
auto Client::bind(int port) -> bool {
    asio::error_code ec ;
    try {
        if (!netSocket.is_open()){
            netSocket.open(asio::ip::tcp::v4(),ec) ;
            if (ec) {
                DBGMSG(std::cerr, "Error on opening socket: "s + ec.message());
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
                this->close() ;
                return false ;
            }
        }
        return true ;
    }
    catch(...) {
        DBGMSG(std::cerr, "Unknown error on bind") ;
        this->close() ;
        return false ;
    }
}

// ========================================================================================
auto Client::initialRead() -> void {
    if (netSocket.is_open()){
        incomingPacket = Packet();
        bytesAsked = incomingPacket.size() ;
        netSocket.async_read_some(asio::buffer(incomingPacket.data.data(),bytesAsked),std::bind(&Client::packetRead,this,std::placeholders::_1,std::placeholders::_2));
    }
}
// ========================================================================================
auto Client::sendPacket(const Packet &packet) -> bool {
    auto rvalue = false ;
    try {
        if (netSocket.is_open()) {
            asio::error_code ec ;
            asio::write(netSocket,asio::buffer(packet.data.data(),packet.size()),ec) ;
            if (!ec) {
                send_time = util::ourclock::now() ;
                rvalue = true ;
            }
            else {
                DBGMSG(std::cerr, "Error on writing socket: "s + ec.message());
                this->close() ;
            }
        }
        return rvalue ;
    }
    catch (...) {
        this->close() ;
        DBGMSG(std::cerr, "Unknown error on send packet"s );
        return false ;
    }
}


// ========================================================================================
auto Client::setPeerInformation() -> void {
    try {
        if (this->is_open() && is_connected){
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
auto Client::setConnectTime(const util::ourclock::time_point &time) -> void {
    connect_time = time ;
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
auto Client::millSinceReceive(const util::ourclock::time_point &time ) -> size_t {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time - receive_time).count() ;
}

// ========================================================================================
auto Client::lastSendTime() const -> util::ourclock::time_point {
    return send_time ;
}

// ========================================================================================
auto Client::millSinceSend(const util::ourclock::time_point &time) -> size_t {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time - send_time).count() ;
}

// ========================================================================================
auto Client::setPacketRoutine(Packet::PacketType type, PacketProcessing routine) -> void {
    this->packetRoutines.insert_or_assign(type,routine) ;
}

// ========================================================================================
auto Client::setServerKey( std::uint32_t key) -> void {
    this->server_key = key ;
}

// ========================================================================================
auto Client::setClientType(IdentPacket::ClientType clienttype) -> void {
    this->type = clienttype ;
}

// ========================================================================================
auto Client::handle() const -> const std::string& {
    return name ;
}
// ========================================================================================
auto Client::setHandle(const std::string &handle)  -> void {
    name = handle ;
}

// ========================================================================================
auto Client::clientType() const -> const std::string&  {
    return IdentPacket::nameForClient(type) ;
}

// ========================================================================================
auto Client::information() const -> std::string {
    const auto format = "%b %d %H:%M"s ;
    return util::sysTimeToString(this->connect_time,format) + " , "s +  this->address() + " , " + this->name + " , " + IdentPacket::nameForClient(type) + " , " + util::sysTimeToString(this->receive_time,format) + " , "s + util::sysTimeToString(this->send_time,format) ;
}
