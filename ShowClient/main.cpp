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
auto connect( const ClientConfig &config) -> bool ;
auto usage() -> void ;
// Globals
FrameClock frameClock ;
MediaController mediaController ;
ClientConfig config ;
StatusLed status ;

asio::io_context io_context ;
BlinkClient myClient(io_context) ;
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
                    status.setLed(StatusLed::CONNECTSTATUS, StatusLed::ON) ;

                }
                while (myClient.isValid() && config.connectTime.inRange() && config.runSpan.inRange()){
                    // we dont do anything, but just hang around
                    std::this_thread::sleep_for(std::chrono::milliseconds(250)) ;
                }
                // If we are here, we should not be running a show, or playing a song!
                // Should we force it?
                
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
auto clientLoad( const Packet &packet) -> bool {
    auto ptr = static_cast<const LoadPacket*>(&packet) ;
    auto media = ptr->music() ;
    auto lights = ptr->light() ;
    if (useAudio) {
        if (!mediaController.load(media) ) {
            // Now, what status light to blink. Play hasnt happend, so will overloaded
            // I am not sure here
            // But in theory, we should get the error on the play (not loaded), so probably ok
        }
        // Load the light file
        
    }

    return true;
}

//======================================================================
auto clientNop( const Packet &packet) -> bool {
    auto ptr = static_cast<const NopPacket*>(&packet) ;
    if (ptr->respond()){
        // We need to send a response
        myClient.sendPacket(NopPacket()) ;
    }
    return true;
}

//======================================================================
auto clientPlay( const Packet &packet) -> bool {
    auto ptr = static_cast<const PlayPacket*>(&packet) ;
    if (ptr->play()) {
        status.setLed(StatusLed::PLAYSTATUS, StatusLed::ON) ;
        auto frame = ptr->frame() ;
        if (useAudio) {
            if (!mediaController.play(true,frame)) {
                status.setLed(StatusLed::PLAYSTATUS, StatusLed::FLASH) ;
            }
        }
        // Set the light controller to play here
        
        // and the frame clock ;
    }
    else {
        status.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF) ;
        if (useAudio) {
            mediaController.play(false) ;
        }
        // Set the light controller to stop play here
        // and the frame clock ;
    }
    return true;
}

//======================================================================
auto clientShow( const Packet &packet) -> bool {
    auto ptr = static_cast<const ShowPacket*>(&packet) ;
    if (ptr->show()){
        useAudio = config.useAudio ;
        status.setLed(StatusLed::SHOWSTATUS, StatusLed::ON) ;
        if (useAudio) {
            if (!mediaController.setShow(true)) {
                useAudio = false ;
                status.setLed(StatusLed::SHOWSTATUS, StatusLed::FLASH) ;
            }
        }
        // Do Light controller here
    }
    else {
        status.setLed(StatusLed::SHOWSTATUS, StatusLed::OFF) ;
        if (useAudio) {
            mediaController.setShow(false) ;
        }
        // Set the light controller here
    }
    return true;
}

//======================================================================
auto clientSync( const Packet  &packet) -> bool {
    // Just the frame clock
    
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
        if (myClient.connect(serverEndpoint, config.serverPort) ) {
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
