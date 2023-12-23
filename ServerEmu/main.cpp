//Copyright © 2023 Charles Kerr. All rights reserved.

#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <string>
#include <thread>
#include <algorithm>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/dbgutil.hpp"

#include "network/allpackets.hpp"
#include "network/client.hpp"

#include "asio/asio.hpp"

using namespace std::string_literals ;

// Foward declares, where the real action occurs
auto clientLoad( const Packet &packet, Client *) -> bool;
auto clientNop( const Packet &packet, Client *) -> bool;
auto clientPlay( const Packet &packet, Client *) -> bool;
auto clientShow( const Packet &packet, Client *) -> bool;
auto clientSync( const Packet &packet, Client *) -> bool;
auto clientIdent( const Packet &packet, Client *) -> bool;

auto  process(const std::string &cmd, const std::string &value) -> bool ;

auto sendPacket(const Packet &packet) -> void ;

asio::io_context io_context ;
asio::ip::tcp::acceptor acceptor{io_context} ;
std::thread threadConnect;
auto isListening = false ;

asio::io_context client_context ;
std::thread threadClient;

constexpr std::uint32_t server_key = 0xDEADBEEF ;

std::vector<std::shared_ptr<Client> > connections ;
std::mutex connectionAccess ;
// ===============================================================================
// Support routines
// ===============================================================================

// ===============================================================================
auto runConnect() {
    io_context.run() ;
}
// ===============================================================================
auto runClient() {
    client_context.run() ;
}


// ===============================================================================
auto handleAccept(Client::ClientPointer connection,const asio::error_code& ec) -> void {
    // We got a connection, a few things we need to do
    if (ec) {
        // we got an error?
        std::cout << "Error on connection: "<<ec.message() << std::endl;
        io_context.stop() ;
        io_context.restart() ;
        return ;
    }
    connection->setIsConnected(true ) ;
    connection->setPeerInformation() ;
    std::cout << "Connection from " << connection->address() << std::endl;
    connection->initialRead() ;
    if (!threadClient.joinable()) {
        threadClient = std::thread(&runClient) ;
    }
    auto lock = std::lock_guard(connectionAccess) ;
    connections.push_back(connection) ;
    // Requeue another
    auto client = Client::createPointer(client_context) ;
    client->setPacketRoutine(Packet::LOAD, &clientLoad);
    client->setPacketRoutine(Packet::NOP, &clientNop);
    client->setPacketRoutine(Packet::PLAY, &clientPlay);
    client->setPacketRoutine(Packet::SHOW, &clientShow);
    client->setPacketRoutine(Packet::SYNC, &clientSync);
    client->setPacketRoutine(Packet::IDENT, &clientIdent);
    client->setServerKey(server_key) ;
    
    acceptor.async_accept(client->netSocket, std::bind(&handleAccept, client, std::placeholders::_1));
    
}


// ===============================================================================
// Main
// ===============================================================================
int main(int argc, const char * argv[]) {
    auto statuscode = EXIT_SUCCESS ;
    auto buffer = std::vector<char>(2048,0) ;
    try {
        auto runForever = true ;
        while(runForever) {
            std::cin.getline(buffer.data(), buffer.size()-1) ;
            buffer[std::cin.gcount()] = 0 ;
            std::string line = buffer.data() ;
            auto [command, value] = util::split(util::trim(line)," ") ;
            if (!command.empty()){
                auto ucmd = util::upper(command) ;
                runForever = process(ucmd,value) ;
            }
            
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        statuscode = EXIT_FAILURE;
    }
    catch(...) {
        std::cerr << "Unknown error!" << std::endl;
        statuscode = EXIT_FAILURE;
    }
    if (threadClient.joinable()) {
        client_context.stop() ;
        threadClient.join() ;
    }
    if (threadConnect.joinable()) {
        io_context.stop() ;
        threadConnect.join() ;
    }
    return statuscode ;
}
// ===============================================================================
// Command routines
// ===============================================================================
auto  process(const std::string &cmd, const std::string &value) -> bool {
    if (cmd == "LISTEN") {
        try {
            auto [st,pt] = util::split(value," ") ;
            auto state = std::stoi(st,nullptr,0) != 0 ;
            if (state) {
                try {
                    if (acceptor.is_open()) {
                        std::cout << "All ready listening" << std::endl;
                    }
                    else {
                        asio::error_code ec ;
                        auto port = std::stoi(pt,nullptr,0) ;
                        
                        
                        acceptor.open(asio::ip::tcp::v4(),ec);
                        if (ec) {
                            std::cerr << "Error opening acceptor"<< ec.message() << std::endl;
                            return false ;
                        }
                        asio::socket_base::reuse_address option(true) ;
                        acceptor.set_option(option) ;
                        acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port ),ec) ;
                        if (ec) {
                            std::cerr << "Error binding acceptor: " << ec.message() << std::endl;
                            acceptor.close() ;
                            return false ;
                        }
                        
                        //                        acceptor = asio::ip::tcp::acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port ));
                        acceptor.listen(50, ec) ;
                        if (ec) {
                            std::cerr << "Error listening: " << ec.message() << std::endl;
                            acceptor.close() ;
                            return false ;
                            
                        }
                        auto connection = Client::createPointer(client_context);
                        acceptor.async_accept(connection->netSocket, std::bind(&handleAccept, connection, std::placeholders::_1));
                        threadConnect = std::thread(&runConnect) ;
                    }
                }
                catch(...) {
                    std::cerr << "Bad parameter" << std::endl;
                }
            }
            else {
                if (!acceptor.is_open()) {
                    std::cout << "All ready not listening" << std::endl;
                }
                else {
                    acceptor.cancel() ;
                    acceptor.close() ;
                    io_context.stop() ;
                    
                    auto lock = std::lock_guard(connectionAccess) ;
                    for (auto iter = connections.begin(); iter != connections.end();) {
                        (*iter)->close() ;
                        iter = connections.erase(iter) ;
                    }
                    io_context.restart() ;
                    // Frankly, we will stop all our clients as well
                    
                    
                    client_context.stop() ;
                    if (threadClient.joinable()){
                        threadClient.join() ;
                        threadClient = std::thread() ;
                    }
                    client_context.restart() ;
                    
                    if (threadConnect.joinable()){
                        threadConnect.join() ;
                        threadConnect = std::thread() ;
                    }
                    io_context.restart() ;
                    
                }
            }
        }
        catch(...) {
            std::cout << "Bad parameter" << std::endl;
        }
    }
    else if (cmd == "LOAD") {
        auto [music,light] = util::split(value," ") ;
        auto packet = LoadPacket() ;
        packet.setLight(light) ;
        packet.setMusic(music) ;
        sendPacket(packet) ;
        
    }
    else if (cmd == "PLAY") {
        auto packet = PlayPacket() ;
        if (!value.empty()) {
            try {
                auto frame = 0 ;
                auto [s,f] = util::split(value," ") ;
                auto state = std::stoi(s,nullptr,0) != 0;
                if (!f.empty()) {
                    frame = std::stoi(f,nullptr,0) ;
                }
                packet.setState(state) ;
                packet.setFrame(frame) ;
                sendPacket(packet) ;
                
            }
            catch(...) {
                std::cout <<"Bad parameter" << std::endl;
            }
        }
        
    }
    else if (cmd == "SHOW") {
        auto packet = ShowPacket() ;
        if (!value.empty()) {
            try {
                auto state = std::stoi(value,nullptr,0) != 0;
                packet.setState(state) ;
                sendPacket(packet) ;
                
            }
            catch(...) {
                std::cout <<"Bad parameter" << std::endl;
            }
        }
        
    }
    else if (cmd == "SYNC") {
        auto packet = SyncPacket() ;
        if (!value.empty()) {
            try {
                auto frame = std::stoi(value,nullptr,0) ;
                packet.setFrame(frame);
                sendPacket(packet) ;
                
            }
            catch(...) {
                std::cout <<"Bad parameter" << std::endl;
            }
        }
        
    }
    else if (cmd == "NOP") {
        auto packet = NopPacket() ;
        auto respond =  false ;
        if (!value.empty()) {
            try {
                respond = std::stoi(value,nullptr,0) != 0 ;
                packet.setRespond(respond) ;
                sendPacket(packet) ;
                
            }
            catch(...) {
                std::cout <<"Bad parameter" << std::endl;
            }
        }
    }
    else if (cmd == "STATUS") {
        auto lock = std::lock_guard(connectionAccess) ;
        
        for (auto iter = connections.begin(); iter != connections.end();) {
            if ( (*iter)->is_open()) {
                std::cout << (*iter)->address() << "\n" ;
                std::cout << "\tConnected: " << util::sysTimeToString((*iter)->connectTime()) << "\n";
                std::cout << "\tHandle: " << (*iter)->handle() << " Type: " << (*iter)->clientType() <<"n" ;
                iter++ ;
            }
            else {
                iter = connections.erase(iter) ;
            }
        }
        std::cout << "Total: " << connections.size() << std::endl;
        
    }
    else if (cmd == "EXIT") {
        acceptor.cancel() ;
        io_context.stop() ;
        acceptor.close() ;
        io_context.restart() ;
        auto lock = std::lock_guard(connectionAccess) ;
        for (auto iter = connections.begin(); iter != connections.end();) {
            (*iter)->close() ;
            iter = connections.erase(iter) ;
        }
        client_context.stop() ;
        client_context.restart() ;
        
        client_context.stop() ;
        if (threadClient.joinable()){
            threadClient.join() ;
            threadClient = std::thread() ;
        }
        
        if (threadConnect.joinable()){
            threadConnect.join() ;
            threadConnect = std::thread() ;
        }
        return false ;
    }
    return true ;
}

// ===============================================================================
auto sendPacket(const Packet &packet) -> void {
    auto lock = std::lock_guard(connectionAccess) ;
    
    for (auto iter = connections.begin(); iter != connections.end();) {
        if ( (*iter)->is_open()) {
            (*iter)->sendPacket(packet) ;
            iter++ ;
        }
        else {
            iter = connections.erase(iter) ;
        }
    }
    
}


// ===============================================================================
// Client routines
// ===============================================================================
auto clientLoad( const Packet &packet, Client *client) -> bool{
    std::cout << "received load packet?" << std::endl;
    return true ;
}
auto clientNop( const Packet &packet, Client *client) -> bool{
    auto ptr = static_cast<const NopPacket*>(&packet) ;
    std::cout << "received nop packet with response: " << ptr->respond() << std::endl;
    return true ;
}
auto clientPlay( const Packet &packet, Client *client) -> bool{
    auto ptr = static_cast<const PlayPacket*>(&packet) ;
    std::cout << "received play packet with state: " << ptr->state() << " frame: " << ptr->frame() << std::endl;
    return true ;
    
}
auto clientShow( const Packet &packet, Client *client) -> bool{
    auto ptr = static_cast<const ShowPacket*>(&packet) ;
    std::cout << "received show packet with state: " << ptr->state() << std::endl;
    return true ;
    
}
auto clientSync( const Packet &packet, Client *client) -> bool{
    auto ptr = static_cast<const SyncPacket*>(&packet) ;
    std::cout << "received sync packet with frame: " << ptr->frame() << std::endl;
    return true ;
    
}
auto clientIdent( const Packet &packet, Client *client) -> bool{
    client->processIdentPacket(packet) ;
    std::cout << client->information() << std::endl ;
    return true ;
    
}

