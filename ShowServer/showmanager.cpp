//Copyright © 2023 Charles Kerr. All rights reserved.

#include "showmanager.hpp"

#include <algorithm>
#include <stdexcept>
#include <functional>
#include <thread>

#include "utility/dbgutil.hpp"

#include "playlist.hpp"
using namespace std::string_literals ;

//======================================================================
auto ShowManager::waitToStop() -> void {
    constexpr auto runaway =  (7 * 60) * 1000 ;  // 7 minutes * 60 seconds to a minute * 1000 milliseconds to a second
    auto milli = 0 ;
    while (isPlaying) {
        if (milli < runaway) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250)) ;
            milli += 250 ;
        }
        else {
            isPlaying = false;
            server.play(false) ;
        }
    }
}

//======================================================================
auto ShowManager::playPlaylist( const std::vector<ListEntry> &entries) -> void {
    auto iter =entries.begin() ;
    while (iter != entries.end() && server.is_open() && config.showTime.inRange()) {
        DBGMSG(std::cout ,"Playing "s + iter->musicName );

        server.load(iter->framecount,iter->musicName,iter->lightName) ;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        isPlaying = true ;
        server.play(1) ;
        waitToStop() ;
        iter++ ;
    }

}

//======================================================================
auto ShowManager::eventStop() -> void {
    isPlaying = false ;
}

//======================================================================
ShowManager::ShowManager():inShow(false),isListening(false),isPlaying(false) {
    server.setPlayStopCallback(std::bind(&ShowManager::eventStop,this));
}

// =====================================================================
auto ShowManager::run(const std::filesystem::path &path) -> bool {
    PlayList event ;
    
    if (!config.load(path) ) {
        return false ;
    }
    // we have at least an intial load
    while (config.runTime.inRange()){
        // We should be running
        while(config.listenTime.inRange()){
            if (!isListening && !server.is_open()){
                server.setServerKey(config.key) ;
                DBGMSG(std::cout, "Starting server on port: "s + std::to_string(config.port));
                if (!server.run(config.port)) {
                    DBGMSG(std::cout, "Error listening on port: "s + std::to_string(config.port));
                    return false ;
                }
                if(config.showTime.inRange() && server.is_open()){
                    if (!inShow) {
                        DBGMSG(std::cout, "Loading playlists");
                        if (!event.load(config.playlist)) {
                            std::cerr<<"Error loading playlist: "s << config.playlist.string()<< std::endl;
                            server.stop() ;
                            return false ;
                        }
                        inShow = true ;
                        server.show(true) ;
                        if (config.delay > 0){
                            DBGMSG(std::cout, "Show delay: "s + std::to_string(config.delay) + " minutes"s);
                            std::this_thread::sleep_for(std::chrono::minutes(config.delay));
                        }
                    }
                    if (server.is_open() && inShow){
                        DBGMSG(std::cout ,"Playing prologue" );
                        playPlaylist(event.prologue);
                        // Now we need to play our body
                        while (server.is_open() && config.showTime.inRange()) {
                            DBGMSG(std::cout ,"Playing body" );
                           playPlaylist(event.body);
                        }
                    }
                    if (inShow && server.is_open() ) {
                        DBGMSG(std::cout ,"Playing epilogue" );

                        playPlaylist(event.epilogue);
                        server.show(false) ;
                    }
                    inShow = false ;
                }
                

            }
        }
        server.stop() ;
        isListening = false ;
        if (config.hasBeenUpdated(path)){
            DBGMSG(std::cout ,"Reloading "s + path.string() );

            config.load(path) ;
        }
    }
    return true ;
}
