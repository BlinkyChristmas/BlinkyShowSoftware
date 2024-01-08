//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef showserver_hpp
#define showserver_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

#include "asio/asio.hpp"

#include "network/allpackets.hpp"
#include "controllers/frameclock.hpp"

#include "serverclient.hpp"


//======================================================================
class ShowServer  {
public:

    using StopCallback= std::function<void()> ;

private:
    FrameClock clock ;
    
    
    asio::io_context connectContext ;
    auto runConnect() -> void ;
    std::thread threadConnect ;
    bool connectIsRunning ;
    asio::ip::tcp::acceptor acceptor{connectContext} ;
    asio::executor_work_guard<asio::io_context::executor_type> connectguard{asio::make_work_guard(connectContext)} ;

    
    asio::io_context clientContext;
    auto runClient() -> void ;
    std::thread threadClient ;
    bool clientIsRunning ;
    mutable std::mutex clientAccess ;
    std::vector<ServerClient::Pointer> connections ;
    mutable std::mutex statusAccess;
    std::vector<ServerClient::Pointer> statusConnections;
    
    auto clearConnections() -> void ;
    auto clearStatus()->void ;

    asio::executor_work_guard<asio::io_context::executor_type> clientguard{asio::make_work_guard(clientContext)} ;
    
    std::thread serverThread ;
    std::atomic<bool> serverShouldRun ;
    auto runServer() -> void ;
    
    
    std::vector<StatusEntry> connectionInformation ;
    auto checkBroken() -> std::vector<StatusEntry> ;

    StopCallback  stopPlayingCallback ;
    
    auto sendAll(const Packet &packet) -> void ;
    
    bool inShow ;
    bool isPlaying;
    std::string music ;
    std::string lights ;
    std::uint32_t framecount ;
    
    std::uint32_t server_key ;
    
    auto createClient() -> ServerClient::Pointer ;
    
    auto handleConnect( ServerClient::Pointer client ,const asio::error_code& ec) -> void ;
    
public:
    std::uint32_t frameUpdate ;
    ShowServer() ;
    ~ShowServer() ;
    auto setPlayStopCallback( const StopCallback &callback) -> void ;
    
    auto setServerKey(std::uint32_t key) -> void ;
    auto informationOnConnections()  -> std::vector<StatusEntry> ;
    
    auto is_open() const -> bool ;
    // Clent callbacks ;
    auto identify(ServerClient *client) -> void ;
    
    // Setup our frame clock callback
    auto clockUpdate(std::uint32_t frame) -> void ;
    auto clockStop(std::uint32_t frame ) -> void ;
    
    auto run( int port) -> bool ;
    auto stop() -> void ;
    
    auto show(bool state) -> void ;
    
    auto load(std::uint32_t framecount, const std::string &music, const std::string &light) -> void ;
    auto play(bool state) -> bool ;
    auto clientStatus(const Packet &packet, Client *client) ->bool ;

};

#endif /* showserver_hpp */
