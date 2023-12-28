//Copyright © 2023 Charles Kerr. All rights reserved.

#include "showserver.hpp"

#include <algorithm>
#include <stdexcept>
#include <functional>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;


//======================================================================
auto ShowServer::runConnect() ->void {
    connectIsRunning = true ;
    connectContext.run() ;
    connectIsRunning = false ;
}

//======================================================================
auto ShowServer::runClient() ->void {
    clientIsRunning = true ;
    clientContext.run() ;
    clientIsRunning = false ;
}
//======================================================================
auto ShowServer::clearConnections() -> void {
    auto lock = std::lock_guard(clientAccess) ;
    for (auto iter = connections.begin(); iter != connections.end();){
        (*iter)->close() ;
        iter = connections.erase(iter) ;
    }
}

//======================================================================
auto ShowServer::runServer() -> void {
    serverShouldRun = true ;
    auto now = util::ourclock::now() ;
    while (serverShouldRun){
        // what do we do, mainly just do houskeeping
        if (std::chrono::duration_cast<std::chrono::seconds>(util::ourclock::now() - now).count() > 75) {
            checkBroken() ;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)) ;
    }
}
//======================================================================
auto ShowServer::checkBroken() -> std::vector<std::string> {
    auto info = std::vector<std::string>() ;
    auto lock = std::lock_guard(clientAccess) ;
    
    for (auto iter = connections.begin(); iter != connections.end();) {
        if ( (*iter)->is_open()) {
            info.push_back((*iter)->information());
            iter++;
        }
        else {
            iter = connections.erase(iter) ;
        }
    }
    return info ;
}

//======================================================================
auto ShowServer::sendAll(const Packet &packet) -> void {
    auto lock = std::lock_guard(clientAccess) ;
    /*
#if defined(DEBUG) || defined(_DEBUG)
    std::cout << "Packet to be sent: "s << Packet::nameForPacket(packet.packetID()) << std::endl;
    packet.dump(std::cout);
    std::cout << std::endl;
#endif
     */
    for (auto iter = connections.begin();iter!= connections.end();) {
        if ((*iter)->is_open()) {
            if ((*iter)->type() == IdentPacket::CLIENT) {
                DBGMSG(std::cout, "Sending packet to: "s + (*iter)->information());
                (*iter)->sendPacket(packet) ;
            }
            iter++;
        }
        else {
            iter = connections.erase(iter) ;
        }
    }
}

//======================================================================
auto ShowServer::createClient() -> ServerClient::Pointer {
    auto client = std::make_shared<ServerClient>(clientContext) ;
    client->identify = std::bind(&ShowServer::identify,this,std::placeholders::_1) ;
    return client ;
}
//======================================================================
auto ShowServer::handleConnect( ServerClient::Pointer client ,const asio::error_code& ec) -> void {
    if (ec) {
        DBGMSG(std::cerr,"Error on accepting: "s + ec.message()) ;
        if ((ec.value() == asio::error::connection_aborted) || (ec.value() == asio::error::operation_aborted)) {
            // Should we do something special?
        }
        if (acceptor.is_open()){
            acceptor.close() ;
        }
        // and we should close all of our connections
        auto lock = std::lock_guard(clientAccess) ;
        for (auto iter = connections.begin();iter != connections.end();){
            (*iter)->close() ;
            iter = connections.erase(iter) ;
        }

        return ;
    }
    DBGMSG(std::cout,"Connection: "s ) ;
    client->setPeerInformation() ;
    DBGMSG(std::cout,"Connection: "s + client->address() ) ;
    {
        auto lock = std::lock_guard(clientAccess) ;
        connections.push_back(client) ;
    }
    client->initialRead() ;
    DBGMSG(std::cout, "queuing another accept");
    client = createClient() ;
    // Now we need to queue another accept
    acceptor.async_accept(client->netSocket, std::bind(&ShowServer::handleConnect, this,client, std::placeholders::_1));
}
//======================================================================
ShowServer::ShowServer():stopPlayingCallback(nullptr),clientIsRunning(false),connectIsRunning(false),inShow(false),isPlaying(false),server_key(0xdeadbeef) {
    threadConnect = std::thread(&ShowServer::runConnect,this);
    threadClient = std::thread(&ShowServer::runClient,this);
    
    clock.setUpdateCallback(std::bind(&ShowServer::clockUpdate,this,std::placeholders::_1));
    clock.setStopCallback(std::bind(&ShowServer::clockStop,this,std::placeholders::_1));
}

//======================================================================
ShowServer::~ShowServer() {
    acceptor.cancel() ;
    acceptor.close() ;
    connectContext.stop();
    clientContext.stop() ;
    serverShouldRun = false ;
    if (threadConnect.joinable()){
        threadConnect.join();
    }
    if (threadClient.joinable()){
        threadClient.join();
    }
    if (serverThread.joinable()){
        serverThread.join();
    }
}

//======================================================================
auto ShowServer::setServerKey(std::uint32_t key) -> void {
    this->server_key = key ;
}

//======================================================================
auto ShowServer::setPlayStopCallback( const StopCallback &callback) -> void {
    stopPlayingCallback = callback ;
}

//======================================================================
auto ShowServer::informationOnConnections()  -> std::vector<std::string> {
    return checkBroken() ;
}

// ===================================================================
auto ShowServer::is_open() const -> bool {
    return acceptor.is_open() ;
}
//======================================================================

// ===========================================================================
// Clent callbacks ;
// ============================================================================
// ============================================================================
auto ShowServer::identify(ServerClient *client) -> void{
    DBGMSG(std::cout, "A client identified itself: "s + client->handle);
    // we should do something here, like if in a show, load, play, etc;
    if (inShow){
        DBGMSG(std::cout, "Sending show setting ");
        auto packet = ShowPacket();
        packet.setState(true);
        client->sendPacket(packet);
    }
    if (framecount != 0) {
        DBGMSG(std::cout, "Sending load ");
        auto packet = LoadPacket();
        packet.setLight(lights);
        packet.setMusic(music) ;
        client->sendPacket(packet) ;
    }
    if (isPlaying) {
        DBGMSG(std::cout, "Sending play ");
       auto packet = PlayPacket();
        packet.setState(true);
        packet.setFrame(clock.frame());
        client->sendPacket(packet);
    }
}

// our frame clock callback
auto syncPacket = SyncPacket() ;
// ============================================================================
auto ShowServer::clockUpdate(std::uint32_t frame) -> void {
    if (frame%300 == 0 || frame == 3) {
        syncPacket.setFrame(frame) ;
        sendAll(syncPacket);
    }
}

auto stopPacket = PlayPacket() ;
// ============================================================================
auto ShowServer::clockStop(std::uint32_t frame ) -> void {
    stopPacket.setState(false);
    sendAll(stopPacket);
    isPlaying = false ;
    framecount = 0 ;
    // now do we have a callback?
    if (stopPlayingCallback != nullptr){
        stopPlayingCallback() ;
    }
}

// ============================================================================
auto ShowServer::run( int port) -> bool {
    asio::error_code ec;
    if (acceptor.is_open()) {
        // We all ready have an acceptor
        DBGMSG(std::cerr, "All ready have an acceptor");
        return false ;
    }
    
    acceptor.open(asio::ip::tcp::v4(),ec);
    if (ec) {
        std::cerr << "Error opening acceptor"<< ec.message() << std::endl;
        return false ;
    }
    asio::socket_base::reuse_address option(true) ;
    acceptor.set_option(option) ;
    DBGMSG(std::cout, "bind listen socket") ;
    acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port ),ec) ;
    if (ec) {
        std::cerr << "Error binding acceptor: " << ec.message() << std::endl;
        acceptor.close() ;
        return false ;
    }
    DBGMSG(std::cout, "listen socket") ;
    acceptor.listen(50, ec) ;
    if (ec) {
        std::cerr << "Error listening: " << ec.message() << std::endl;
        acceptor.close() ;
        return false ;
        
    }

    auto client = createClient() ;
    // Now we need to queue another accept
    acceptor.async_accept(client->netSocket, std::bind(&ShowServer::handleConnect, this,client, std::placeholders::_1));
    return true ;
}

// ============================================================================
auto ShowServer::stop() -> void {
    if (acceptor.is_open()){
        acceptor.cancel() ;
        acceptor.close() ;
    }
    inShow = false ;
    isPlaying = false ;
    clearConnections() ;
}


// ============================================================================
auto ShowServer::show(bool state) -> void {
    DBGMSG(std::cout, "Show called with: "s + std::to_string(state));
    auto packet = ShowPacket() ;
    packet.setState(state) ;
    sendAll(packet);
    inShow = state ;
}

// ============================================================================
auto ShowServer::load(std::uint32_t framecount, const std::string &music, const std::string &light) -> void{
    this->framecount = framecount ;
    this->music = music ;
    this->lights = light ;
    auto packet = LoadPacket() ;
    packet.setLight(lights);
    packet.setMusic(music) ;
    sendAll(packet);
}

// ============================================================================
auto ShowServer::play(bool state) -> bool {
    if (state) {
        if (isPlaying) {
            return false ;
        }
        isPlaying = true ;
        auto packet = PlayPacket() ;
        packet.setState(true) ;
        clock.run(true, 0, framecount);
        sendAll(packet) ;
    }
    else {
        if (!isPlaying){
            return false ;
        }
        clock.run(false,0,0);
        
    }
    return true ;
}
