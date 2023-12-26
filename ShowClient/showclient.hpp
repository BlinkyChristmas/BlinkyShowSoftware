//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef showclient_hpp
#define showclient_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include <functional>

#include "asio/asio.hpp"

#include "controllers/frameclock.hpp"
#include "controllers/mediacontroller.hpp"
#include "controllers/lightcontroller.hpp"
#include "controllers/statusled.hpp"

#include "network/client.hpp"
#include "network/packet.hpp"
#include "network/noppacket.hpp"

#include "clientconfig.hpp"
#include "utility/timeutil.hpp"

//======================================================================
class ShowClient {
    
    FrameClock clock ;
    MediaController mediaController ;
    LightController lightController ;
    StatusLed ledController ;
    
    asio::io_context io_context ;
    // We dont want our context to stop unless we say so
    asio::executor_work_guard<asio::io_context::executor_type> contextguard{asio::make_work_guard(io_context)} ;

    Client myClient{io_context} ;
    std::thread runThread ;
    
    ClientConfig configuration ;

    bool useAudio ;
    bool useLight ;
    bool showAudio  ;
    bool showLight ;

    bool isPlaying ;
    bool inShow  ;
    std::uint32_t endFrame ;

    bool run_forever ;
    
    bool threadRunning ;
    auto runLoop() ;
    
    util::ourclock::time_point nopRequestSent ;
    
    util::ourclock::time_point last_resolved ;
    asio::ip::tcp::endpoint server_endpoint ;
    std::uint32_t connect_failures ;
    auto minutesSinceLastResolved() -> int ;
    std::string handle ;
    std::uint32_t server_key ;
    
    auto initRoutines() -> void ;
    auto initialize(const ClientConfig &config) -> void ;
    
    auto connect(const std::string &ip, int port, int clientPort = 0) -> bool ;
    
    static NopPacket nack ;
public:
    
    // class setup
    
    ShowClient() ;
    ~ShowClient() ;
    
    auto setConfiguration(const std::filesystem::path &path) -> bool ;
    auto run() -> void ;

    // Clent callbacks ;
    auto clientLoad( const Packet &packet,Client *) -> bool;
    auto clientNop( const Packet &packet, Client *) -> bool;
    auto clientPlay( const Packet &packet, Client *) -> bool;
    auto clientShow( const Packet &packet, Client *) -> bool;
    auto clientSync( const Packet &packet, Client *) -> bool;
    
    // Setup our frame clock callback
    auto clockUpdate(std::uint32_t frame) -> void ;
    auto clockStop(std::uint32_t frame ) -> void ;
    
    
};

#endif /* showclient_hpp */
