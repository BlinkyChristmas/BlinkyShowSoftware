//Copyright © 2023 Charles Kerr. All rights reserved.

#if defined(_WIN32)
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <iostream>
#include <filesystem>
#include <chrono>
#include <string>
#include <thread>

#include "asio/asio.hpp"

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"

#include "network/blinkclient.hpp"
#include "network/allpackets.hpp"

#include "controllers/mediacontroller.hpp"
#include "controllers/frameclock.hpp"
#include "controllers/statusled.hpp"

#include "utility/dbgutil.hpp"

#include "clientconfig.hpp"

using namespace std::string_literals ;

// Foward declares, where the real action occurs
auto clientLoad( const Packet &packet) -> bool;
auto clientNop( const Packet &packet) -> bool;
auto clientPlay( const Packet &packet) -> bool;
auto clientShow( const Packet &packet) -> bool;
auto clientSync( const Packet &packet) -> bool;
auto runLoop() -> void ;
auto cleanup() -> void ;
auto initClient() -> void ;
auto connect() -> bool ;
auto usage() -> void ;
// Globals
FrameClock frameClock ;
MediaController mediaController ;
ClientConfig config ;
StatusLed status ;

asio::io_context io_context ;
BlinkClient myClient(io_context) ;
std::thread runThread ;

asio::ip::tcp::endpoint serverEndpoint ;
util::ourclock::time_point lastResolved ;


//======================================================================
// Main
//======================================================================
int main(int argc, const char * argv[]) {
    status.clearAll() ;
    try {
        if (argc != 2) {
            usage() ;
            throw std::runtime_error("Insufficient parameters.");
        }
        auto path = std::filesystem::path(argv[1]) ;
        if ( !config.load(path) ) {
            throw std::runtime_error("Unable to open: "s + path.string() ) ;
        }
        while (config.runSpan.inRange()){
            while(config.connectTime.inRange()) {
                
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        cleanup();
        status.flashAll();
        return 1;
    }
    catch (...) {
        std::cerr << "Unkown exception!" << std::endl;
        cleanup();
        status.flashAll() ;
        return 1;
    }
    cleanup();
    status.clearAll() ;
    return 0;
}


//======================================================================
// Client routines
//======================================================================

//======================================================================
auto clientLoad( const Packet &packet) -> bool {

    return true;
}

//======================================================================
auto clientNop( const Packet &packet) -> bool {

    return true;
}

//======================================================================
auto clientPlay( const Packet &packet) -> bool {

    return true;
}

//======================================================================
auto clientShow( const Packet &packet) -> bool {

    return true;
}

//======================================================================
auto clientSync( const Packet  &packet) -> bool {

    return true;
}

//======================================================================
auto runLoop() -> void {
    io_context.run() ;
}

//======================================================================
auto cleanup() -> void {
    io_context.stop() ;
    if (runThread.joinable()){
        runThread.join() ;
    }
    runThread = std::thread() ;
}

//======================================================================
auto initClient() -> void {
    myClient.setPacketRountine(Packet::LOAD, &clientLoad);
    myClient.setPacketRountine(Packet::NOP, &clientNop);
    myClient.setPacketRountine(Packet::PLAY, &clientPlay);
    myClient.setPacketRountine(Packet::SHOW, &clientShow);
    myClient.setPacketRountine(Packet::SYNC, &clientSync);
}

//======================================================================
auto connect() -> bool {
    
    return false ;
}

//======================================================================
auto usage() -> void {
    std::cout << "Usage:\n";
    std::cout << "   ShowClient configfile\n";
    std::cout <<" Where:\n";
    std::cout <<"     configfile is a file path to the client configuration file" << std::endl;
    
}
