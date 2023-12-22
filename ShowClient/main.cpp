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
#include <algorithm>

#include "asio/asio.hpp"

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"

#include "network/client.hpp"
#include "network/allpackets.hpp"

#include "controllers/mediacontroller.hpp"
#include "controllers/frameclock.hpp"
#include "controllers/statusled.hpp"
#include "controllers/lightcontroller.hpp"

#include "utility/dbgutil.hpp"

#include "clientconfig.hpp"

using namespace std::string_literals ;

// Foward declares, where the real action occurs
auto clientLoad( const Packet &packet,Client *) -> bool;
auto clientNop( const Packet &packet, Client *) -> bool;
auto clientPlay( const Packet &packet, Client *) -> bool;
auto clientShow( const Packet &packet, Client *) -> bool;
auto clientSync( const Packet &packet, Client *) -> bool;

auto clockUpdate(std::uint32_t frame) -> void ;

// Now some stuff we use

auto runLoop() -> void ;
auto cleanup() -> void ;
auto connect( const ClientConfig &config) -> bool ;
auto usage() -> void ;
auto initialize() -> void ;
// Globals
FrameClock frameClock ;
MediaController mediaController ;
ClientConfig config ;
StatusLed status ;
LightController lights ;

asio::io_context io_context ;
Client myClient(io_context) ;
std::thread runThread ;

bool useAudio = false ;

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
        initialize() ;
        // Ok, we are "ready" in theory
        while (config.runSpan.inRange()){
            status.setLed(StatusLed::RUNSTATUS, StatusLed::FLASH);
            while(config.connectTime.inRange()) {
                status.setLed(StatusLed::RUNSTATUS, StatusLed::ON);
                status.setLed(StatusLed::CONNECTSTATUS, StatusLed::FLASH) ;
                // First bind
                // We should bind right now
                if (!myClient.bind(config.clientPort)){
                    // we have a problem
                    throw std::runtime_error("Error binding to port: "s + std::to_string(config.clientPort) ) ;
                }
                myClient.setServerKey(config.serverKey) ;
                if (connect(config)){
                    // we connected!
                    // Give it our routines
                    
                    status.setLed(StatusLed::CONNECTSTATUS, StatusLed::ON) ;
                    // we need to start a read, and start our thread
                    myClient.initialRead() ;
                    runThread = std::thread(&runLoop) ;
                }
                while (myClient.is_open() && config.connectTime.inRange() && config.runSpan.inRange()){
                    // we dont do anything, but just hang around
                    std::this_thread::sleep_for(std::chrono::milliseconds(250)) ;
                    //std::cout << "out of sleep"<< std::endl;
                }
                io_context.stop() ;
                io_context.restart() ;
                if (runThread.joinable()){
                    runThread.join() ;
                    runThread = std::thread() ;
                }
                // If we are here, we either: Disconnected, outside connect hours, outside running hours
                // Should we force it if we are playing something?
                frameClock.play(false) ;
                mediaController.play(false);
                lights.play(false) ;
                mediaController.setShow(false);
                lights.setShow(false) ;
                
                
                // What about status?
                status.setLed(StatusLed::SHOWSTATUS, StatusLed::OFF) ; // The issue, is we would wipe out error status?
                // Maybe show error status should exit, but just that error indicator
                status.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF) ;
                
            }
            status.setLed(StatusLed::CONNECTSTATUS, StatusLed::OFF) ;
            
            
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
auto clientLoad( const Packet &packet, Client *client) -> bool {
    auto ptr = static_cast<const LoadPacket*>(&packet) ;
    auto media = ptr->music() ;
    auto lights = ptr->light() ;
    if (useAudio) {
        if (!mediaController.load(media) ) {
            // Now, what status light to blink. Play hasnt happend, so will overloaded
            // I am not sure here
            // But in theory, we should get the error on the play (not loaded), so probably ok
        }
    }
    
    // Load the light file
    if (::lights.load(lights)){
        // WE had an issue loading?
        
    }
    
    
    return true;
}

//======================================================================
auto clientNop( const Packet &packet, Client *client) -> bool {
    auto ptr = static_cast<const NopPacket*>(&packet) ;
    if (ptr->respond()){
        // We need to send a response
        myClient.sendPacket(NopPacket()) ;
    }
    return true;
}

//======================================================================
auto clientPlay( const Packet &packet,  Client * client) -> bool {
    auto ptr = static_cast<const PlayPacket*>(&packet) ;
    if (ptr->state()) {
        status.setLed(StatusLed::PLAYSTATUS, StatusLed::ON) ;
        auto frame = ptr->frame() ;
        if (useAudio) {
            if (!mediaController.play(true,frame)) {
                status.setLed(StatusLed::PLAYSTATUS, StatusLed::FLASH) ;
            }
        }
        // Set the light controller to play here
        lights.play(true, frame) ;
        // and the frame clock ;
        frameClock.play(true,frame);
    }
    else {
        frameClock.play(false);
        if (useAudio) {
            mediaController.play(false) ;
        }
        // Set the light controller to stop play here
        lights.play(false) ;
        
        status.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF) ;
        
    }
    return true;
}

//======================================================================
auto clientShow( const Packet &packet,  Client *client) -> bool {
    auto ptr = static_cast<const ShowPacket*>(&packet) ;
    if (ptr->state()){
        useAudio = config.useAudio ;
        status.setLed(StatusLed::SHOWSTATUS, StatusLed::ON) ;
        if (useAudio) {
            if (!mediaController.setShow(true)) {
                useAudio = false ;
                status.setLed(StatusLed::SHOWSTATUS, StatusLed::FLASH) ;
            }
        }
        // Do Light controller here
        lights.setShow(true ) ;
    }
    else {
        status.setLed(StatusLed::SHOWSTATUS, StatusLed::OFF) ;
        if (useAudio) {
            mediaController.setShow(false) ;
        }
        // Set the light controller here
        lights.setShow(false);
    }
    return true;
}

//======================================================================
auto clientSync( const Packet  &packet, Client * client) -> bool {
    // Just the frame clock
    auto ptr = static_cast<const SyncPacket*>(&packet) ;
    frameClock.sync(ptr->frame());
    return true;
}
//======================================================================
auto clockUpdate(std::uint32_t frame) -> void {
    lights.sync(frame) ;
    mediaController.sync(frame) ;
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
    myClient.setPacketRoutine(Packet::LOAD, &clientLoad);
    myClient.setPacketRoutine(Packet::NOP, &clientNop);
    myClient.setPacketRoutine(Packet::PLAY, &clientPlay);
    myClient.setPacketRoutine(Packet::SHOW, &clientShow);
    myClient.setPacketRoutine(Packet::SYNC, &clientSync);
}

//======================================================================
auto initialize() -> void {
    initClient() ;
    // We need to add our callback
    frameClock.setUpdateCallback(&clockUpdate) ;
}

//======================================================================
auto usage() -> void {
    std::cout << "Usage:\n";
    std::cout << "   ShowClient configfile\n";
    std::cout <<" Where:\n";
    std::cout <<"     configfile is a file path to the client configuration file" << std::endl;
    
}

// =========================================================================
util::ourclock::time_point lastResolved ;
std::uint64_t connectAttempts = 0;
asio::ip::tcp::endpoint serverEndpoint ;

auto connect( const ClientConfig &config) -> bool {
    // How many minutes since we last resolved our server address
    auto minutesSinceLast = std::chrono::duration_cast<std::chrono::minutes>(util::ourclock::now() - lastResolved).count() ;
    
    // If the server endpoint is "bad"(never initialized, failed resolution), it has been greater then 60 minutes, or it has been more then 10 failures in a row, we resolve again
    if (serverEndpoint == asio::ip::tcp::endpoint() || minutesSinceLast >60 || connectAttempts%10 == 0) {
        // we should resolve it!
        serverEndpoint = Client::resolve(config.serverIP, config.serverPort) ;
        lastResolved = util::ourclock::now() ;
    }
    // Do we have a good endpoint ?
    if (serverEndpoint != asio::ip::tcp::endpoint()) {
        // We can attempt to connect
        if (myClient.connect(serverEndpoint) ) {
            connectAttempts = 0 ;
            return true ;
        }
        // We failed, increase our failures
        connectAttempts += 1 ;
        // Now, we will pause this any minutes (this slowly builds up, but with a max
        auto minutesToPause = std::min(static_cast<std::uint64_t>(10),connectAttempts/10) ;
        std::this_thread::sleep_for(std::chrono::minutes(minutesToPause)) ;
    }
    else {
        // we have a bad endpoint, which is extremely bad. DNS, or somthing is failing
        // So what do we do?
        // we will treat as a connect failure, but perhaps with a greater max
        connectAttempts += 1 ;
        // Now, we will pause this any minutes (this slowly builds up, but with a max)
        auto minutesToPause = std::min(static_cast<std::uint64_t>(60),connectAttempts/10) ;
        std::this_thread::sleep_for(std::chrono::minutes(minutesToPause)) ;
    }
    return false ;
}
