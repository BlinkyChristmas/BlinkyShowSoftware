//Copyright © 2023 Charles Kerr. All rights reserved.

#include "client.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;



// ========================================================================================
auto Client::packetRead(const asio::error_code& err, size_t bytes_transferred) -> void {
    if (err) {
        if (netSocket.is_open()) {
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
    return true ;
}

// ========================================================================================
Client::Client(asio::io_context &context):netSocket(context),bytesAsked(0),receive_time(util::ourclock::now()),send_time(util::ourclock::now()),connect_time(util::ourclock::now()) {
    setPeerInformation() ;
}

// ========================================================================================
Client::~Client() {
    this->close() ;
}

// ========================================================================================
auto Client::socket() -> asio::ip::tcp::socket& {
    return netSocket ;
}

// ========================================================================================
auto Client::is_open() const -> bool {
    return netSocket.is_open() ;
}

// ========================================================================================
auto Client::shutdown() -> void {
    netSocket.shutdown(asio::ip::tcp::socket::shutdown_type::shutdown_both) ;
}

// ========================================================================================
auto Client::close() -> void {
    if (netSocket.is_open()) {
        netSocket.shutdown(asio::socket_base::shutdown_both) ;
        netSocket.close() ;
    }
}

// ========================================================================================
auto Client::connect(asio::ip::tcp::endpoint &endpoint, int clientPort) -> bool {
    asio::error_code ec ;
    if (netSocket.is_open()) {
        netSocket.shutdown(asio::ip::tcp::socket::shutdown_type::shutdown_both) ;
        netSocket.close() ;
    }
    netSocket.open(asio::ip::tcp::v4(),ec) ;
    if (ec) {
        // We couldn't open, should we bail?
        throw std::runtime_error("Unable to open socket!");
    }
    if (clientPort > 0) {
        asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any() , clientPort);
        netSocket.bind(endpoint,ec) ;
        if (ec){
            // we couldn't bind, should we bail?
            netSocket.close() ;
            throw std::runtime_error("Unable to bind socket to: "s + std::to_string( clientPort) );
        }
    }
    if (endpoint == asio::ip::tcp::endpoint()) {
        netSocket.close();
        return false ;
    }
    netSocket.connect(endpoint,ec) ;
    if (ec) {
        netSocket.close() ;
        return false ;
    }
    this->setPeerInformation() ;
    return true ;

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

// ========================================================================================
auto Client::sendPacket(const Packet &packet) -> bool {
    auto rvalue = false ;
    if (netSocket.is_open()) {
        asio::error_code ec ;
        asio::write(netSocket,asio::buffer(packet.data.data(),packet.size()),ec) ;
        if (!ec) {
            send_time = util::ourclock::now() ;
            rvalue = true ;
        }
    }
    return rvalue ;
}

// ========================================================================================
auto Client::isValid() const -> bool {
    return netSocket.is_open() ;
}

// ========================================================================================
auto Client::setPeerInformation() -> void {
    peer_address = netSocket.remote_endpoint().address().to_string() ;
    peer_port = std::to_string(netSocket.remote_endpoint().port()) ;
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
