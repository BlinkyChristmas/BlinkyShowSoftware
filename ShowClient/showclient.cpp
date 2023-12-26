//Copyright © 2023 Charles Kerr. All rights reserved.

#include "showclient.hpp"

#include <algorithm>
#include <stdexcept>
#include <chrono>

#include "network/allpackets.hpp"
#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
using namespace std::string_literals ;



//======================================================================
NopPacket ShowClient::nack = NopPacket();

//======================================================================
auto ShowClient::runLoop() {
    threadRunning = true ;
    io_context.run();
    threadRunning = false ;
}

//======================================================================
auto ShowClient::minutesSinceLastResolved() -> int {
    return std::chrono::duration_cast<std::chrono::minutes>(util::ourclock::now() - last_resolved).count() ;
}
//======================================================================
auto ShowClient::initRoutines() -> void {
    myClient.setPacketRoutine(Packet::LOAD, std::bind(&ShowClient::clientLoad,this,std::placeholders::_1,std::placeholders::_2));
    myClient.setPacketRoutine(Packet::NOP, std::bind(&ShowClient::clientNop,this,std::placeholders::_1,std::placeholders::_2));
    myClient.setPacketRoutine(Packet::PLAY, std::bind(&ShowClient::clientPlay,this,std::placeholders::_1,std::placeholders::_2));
    myClient.setPacketRoutine(Packet::SHOW, std::bind(&ShowClient::clientShow,this,std::placeholders::_1,std::placeholders::_2));
    myClient.setPacketRoutine(Packet::SYNC, std::bind(&ShowClient::clientSync,this,std::placeholders::_1,std::placeholders::_2));
    
    clock.setStopCallback(std::bind(&ShowClient::clockStop,this,std::placeholders::_1)) ;
    clock.setUpdateCallback(std::bind(&ShowClient::clockUpdate,this,std::placeholders::_1)) ;
}

//======================================================================
auto ShowClient::initialize(const ClientConfig &config) -> void {
    initRoutines() ;
    
    mediaController.setConfiguration(config.musicPath, config.musicExtension, true) ;
    mediaController.setUseSync(true) ;
    
    lightController.setLocation(config.lightPath, config.lightExtension) ;
    lightController.configurePRU(config.pruSetting.at(0), config.pruSetting.at(1)) ;
}

//======================================================================
ShowClient::ShowClient():useAudio(false),useLight(false),showAudio(false),showLight(false),isPlaying(false),inShow(false),threadRunning(false),handle("Beagle"),server_key(0xDEADBEEF),connect_failures(0),last_resolved(util::ourclock::now()),run_forever(true),endFrame(0) {
    runThread = std::thread(&ShowClient::runLoop,this) ;
    ledController.clearAll() ;
}
//======================================================================
ShowClient::~ShowClient() {
    myClient.close() ;
    clock.close() ;
    mediaController.play(false) ;
    mediaController.setShow(false);
    io_context.stop() ;
    if (runThread.joinable()){
        runThread.join() ;
    }
}

//======================================================================
auto ShowClient::connect(const std::string &ip, int port, int clientPort) -> bool{
    // First, bind and setup our client
    if (myClient.is_open()){
        myClient.close() ;
    }
    if (clientPort != 0) {
        // we are suppose to bind our client
        if (!myClient.bind(configuration.clientPort)) {
            DBGMSG(std::cerr,"Connection could not bind client port"s) ;
            return false ;
        }
    }
    // Now, should we resolve our
    if (server_endpoint == asio::ip::tcp::endpoint() || this->minutesSinceLastResolved() > 60 || ( (connect_failures % 10 == 0) && connect_failures !=0) ){
        DBGMSG(std::cout, "Resolving endpoint") ;
        server_endpoint  = Client::resolve(ip, port);
        last_resolved = util::ourclock::now() ;
    }
    if (server_endpoint == asio::ip::tcp::endpoint()){
        // This is a pretty serious error, should we just exit?
        DBGMSG(std::cerr, "Unable to open an endpoint for: "s + ip + ":"s+std::to_string(port));
        run_forever = false ; // I wonder about our led status
        return false ;
    }
    // we are finally ready to try to connect
    if (!myClient.connect(server_endpoint)){
        // We did not connect
        connect_failures += 1 ;
        DBGMSG(std::cout, "Did not connect, failures: "s + std::to_string(connect_failures));
        // SHould we delay or something
        auto mag = connect_failures/10 ;
        // we increment our delay time
        
        switch (mag) {
            case 0:
                DBGMSG(std::cout, "Sleeping for 250 milliseconds");
                std::this_thread::sleep_for(std::chrono::milliseconds(250)) ;
                break;
            case 1:
                DBGMSG(std::cout, "Sleeping for 10 seconds");
                std::this_thread::sleep_for(std::chrono::seconds(10)) ;
                break;
            case 2:
                DBGMSG(std::cout, "Sleeping for 1 minute");
                std::this_thread::sleep_for(std::chrono::minutes(1)) ;
                break;
            default:
                DBGMSG(std::cout, "Sleeping for 5 minutes");
                std::this_thread::sleep_for(std::chrono::minutes(5)) ;
                break;
        }
        return false ;
        
    }
    DBGMSG(std::cout, "Connected, resetting failures");
    connect_failures = 0 ;
    return true ;
}

//======================================================================
auto ShowClient::setConfiguration(const std::filesystem::path &path) -> bool {
    // It takes a while for the SD card to mount, so we have a check here
    auto retry = 10 ;
    auto good = true ;
    while(!std::filesystem::exists(path)){
        ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::FLASH) ;
        ledController.setLed(StatusLed::CONNECTSTATUS, StatusLed::FLASH) ;
        std::this_thread::sleep_for(std::chrono::milliseconds(500)) ;
        retry -= 1 ;
        if (retry< 0) {
            good = false ;
            break;
        }
    }
    // are we good or not?
    if (good) {
        ledController.clearAll() ;
        return configuration.load(path) ;
    }
    return good ;
}
//======================================================================
auto ShowClient::run() -> void {
    ledController.clearAll() ;
    try {
        isPlaying = false ;
        inShow = false ;
        ledController.setLed(StatusLed::CONNECTSTATUS, StatusLed::OFF);
        ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::OFF);
        ledController.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF);
        ledController.setLed(StatusLed::RUNSTATUS, StatusLed::FLASH);
        while (configuration.runSpan.inRange()) {
            // We are in an acceptable time span
            isPlaying = false ;
            inShow = false ;
            ledController.setLed(StatusLed::RUNSTATUS, StatusLed::FLASH);
            ledController.setLed(StatusLed::CONNECTSTATUS, StatusLed::OFF);
            ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::OFF);
            ledController.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF);
            while (configuration.connectTime.inRange() && configuration.runSpan.inRange() ) {
                isPlaying = false ;
                inShow = false ;
                ledController.setLed(StatusLed::RUNSTATUS, StatusLed::ON);
                ledController.setLed(StatusLed::CONNECTSTATUS, StatusLed::FLASH);
                ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::OFF);
                ledController.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF);
                if (connect(configuration.serverIP,configuration.serverPort,configuration.clientPort)) {
                    // We connected!
                    ledController.setLed(StatusLed::CONNECTSTATUS, StatusLed::ON);
                    
                    // We need to identify ourself
                    auto packet = IdentPacket() ;
                    packet.setKey(configuration.serverKey);
                    packet.setHandle(configuration.name);
                    packet.setClientType(IdentPacket::CLIENT);
                    myClient.sendPacket(packet);
                    ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::FLASH);
                    
                    // Now we idle, and do keep alives. Callbacks do the rest of the work.
                    util::ourclock::time_point lastKeepAlive = util::ourclock::now() ;
                    while (myClient.is_open() || isPlaying ) {
                        // Now, while we are open, we just periodic check our idleness
                        if (myClient.is_open()){
                            auto minutesIn = myClient.minutesSinceReceive() ;
                            
                            if ((minutesIn > 5) && (std::chrono::duration_cast<std::chrono::seconds>(util::ourclock::now() - lastKeepAlive ).count() > 60) ) {
                                // We should send a keep alive
                                DBGMSG(std::cout, "Sending keep alive request");
                                auto packet = NopPacket() ;
                                packet.setRespond(true) ;
                                myClient.sendPacket(packet) ;
                            }
                        }
                    }
                    // we closed
                    myClient.clearSendBuffer() ;
                }
            }
        }
        ledController.clearAll() ;
    }
    catch (const std::exception &e) {
        DBGMSG(std::cerr, "Exception: "s + e.what());
        ledController.flashAll() ;
    }
    catch(...) {
        DBGMSG(std::cerr, "Unknown exception"s);
        ledController.flashAll() ;
    }
}


// ==================================================================================
auto ShowClient::clockUpdate(std::uint32_t frame) -> void {
    if (useAudio) {
        mediaController.sync(frame);
    }
    if (useLight) {
        lightController.sync(frame);
    }
}

// ==================================================================================
auto ShowClient::clockStop(std::uint32_t frame ) -> void {
    if (useAudio) {
        mediaController.play(false) ;
    }
    if (useLight) {
        lightController.play(false);
    }
    useAudio = showAudio ;
    useLight = showLight ;
    ledController.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF);
    isPlaying = false ;
    endFrame = 0 ;
}

// Clent callbacks ;
// ==================================================================================
auto ShowClient::clientLoad( const Packet &packet,Client *) -> bool{
    auto ptr = static_cast<const LoadPacket*>(&packet) ;
    DBGMSG(std::cout, "Received Load Packet");
    auto ledstate = StatusLed::OFF ;
    auto music = ptr->music() ;
    auto lights = ptr->light() ;
    endFrame = 0 ;
    if (showAudio) {
        endFrame = mediaController.load(music);
        useAudio = endFrame != 0 ;
        
        if (useAudio != showAudio){
            ledstate = StatusLed::FLASH ;
        }
    }
    if(showLight) {
        auto temp  = lightController.load(lights);
        useLight = temp != 0 ;
        endFrame = std::max(temp,endFrame) ;
        if (useLight != showLight){
            ledstate = StatusLed::FLASH ;
        }
    }
    ledController.setLed(StatusLed::PLAYSTATUS, ledstate);
    return true ;
    
}

// ==================================================================================
auto ShowClient::clientNop( const Packet &packet, Client *) -> bool{
    auto ptr = static_cast<const NopPacket*>(&packet) ;
    DBGMSG(std::cout, "Received Nop Packet");
    auto respond = ptr->respond() ;
    if (respond){
        myClient.sendPacket(nack);
    }
    return true ;
}
// ==================================================================================
auto ShowClient::clientPlay( const Packet &packet, Client *) -> bool{
    auto ptr = static_cast<const PlayPacket*>(&packet) ;
    DBGMSG(std::cout, "Received Play Packet");
    auto state = ptr->state() ;
    auto frame = ptr->frame() ;
    if (state) {
        if (!inShow){
            // do something and return
            ledController.setLed(StatusLed::PLAYSTATUS, StatusLed::FLASH);
            return true ;
        }
        auto ledstate = StatusLed::ON ;
        clock.run(true,frame,endFrame);
        if (useAudio) {
            useAudio = mediaController.play(true,frame);
            if (!useAudio) {
                ledstate = StatusLed::FLASH;
            }
        }
        if (useLight) {
            useLight = mediaController.play(true,frame) ;
            if (!useLight) {
                ledstate = StatusLed::FLASH;
            }
        }
        ledController.setLed(StatusLed::PLAYSTATUS, ledstate);
        isPlaying = true ;
    }
    else {
        clock.run(false,0,0) ;
    }
    return true ;
}
// ==================================================================================
auto ShowClient::clientShow( const Packet &packet, Client *) -> bool{
    auto ptr = static_cast<const ShowPacket*>(&packet) ;
    DBGMSG(std::cout, "Received Show Packet");
    auto state = ptr->state() ;
    if (state) {
        inShow = true ;
        showAudio = useAudio = configuration.useAudio ;
        showLight = useLight = configuration.useLights ;
        if (showAudio){
            showAudio = mediaController.setShow(state) ;
        }
        if (showLight){
            showLight = lightController.setShow(state) ;
        }
        ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::ON) ;
    }
    else {
        inShow = false ;
        if (showAudio) {
            mediaController.setShow(false);
        }
        if (showLight) {
            lightController.setShow(false);
        }
        showAudio = useAudio = configuration.useAudio ;
        showLight = useLight = configuration.useLights ;
        ledController.setLed(StatusLed::SHOWSTATUS, StatusLed::FLASH) ;
        ledController.setLed(StatusLed::PLAYSTATUS, StatusLed::OFF) ;
    }
    
    return true ;
}
// ==================================================================================
auto ShowClient::clientSync( const Packet &packet, Client *) -> bool{
    auto ptr = static_cast<const SyncPacket*>(&packet) ;
    DBGMSG(std::cout, "Received Sync Packet");
    
    auto frame = ptr->frame() ;
    if (inShow && isPlaying) {
        clock.sync(frame);
    }
    return true ;
}
