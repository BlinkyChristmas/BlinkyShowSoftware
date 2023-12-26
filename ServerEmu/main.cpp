//Copyright © 2023 Charles Kerr. All rights reserved.

#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <string>

#include "ShowServer/showserver.hpp"
#include "containers/wavfile/mwavfile.hpp"
#include "utility/strutil.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

ShowServer server ;
MWAVFile mediaFile ;
auto runForever = true ;

auto mediaLocation = std::filesystem::path("/Users/charles/Documents/BlinkyChristmas/music");

auto  process(const std::string &cmd, const std::string &value) -> bool;
// ===============================================================================
// Main
// ===============================================================================
int main(int argc, const char * argv[]) {
    
     
    auto ipaddress = "192.168.1.231"s ;
    
    std::filesystem::path media_path ;
    
    auto statuscode = EXIT_SUCCESS ;
    auto buffer = std::vector<char>(2048,0) ;
    try {
        if (!server.run(ipaddress, 50)) {
            std::cout << "Error trying to start server listening" << std::endl;
        }
        
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
    return statuscode ;
}
// ===============================================================================
// Command routines
// ===============================================================================
// ===============================================================================
auto commandListen(const std::string &value) -> bool {
    try {
        auto [st,pt] = util::split(value," ") ;
        auto state = std::stoi(st,nullptr,0) != 0 ;
        if (state) {
            try {
                auto [ip,sp] = util::split(pt," ") ;
                auto port = std::stoi(sp,nullptr,0) ;
                server.run(ip,port) ;
            }
            catch(...) {
                std::cerr << "Bad parameter" << std::endl;
            }
        }
        else {
            server.stop() ;
        }
    }
    catch(...) {
        std::cout << "Bad parameter" << std::endl;
    }
    return true ;
}
// ===============================================================================
auto commandLoad(const std::string &value) -> bool {
    auto [music,light] = util::split(value," ") ;
    mediaFile.load(mediaLocation / std::filesystem::path(music + ".wav"s));
    auto frame = mediaFile.frameCount() ;
    DBGMSG(std::cout,value+" has "s+std::to_string(frame)+" frames"s);
    server.load(frame, value, value);
    return true ;
}

// ==============================================================================
auto commandPlay(const std::string &value) -> bool {
    try {
        auto state = std::stoi(value,nullptr,0) != 0 ;
        server.play(state);
    }
    catch(...) {
        std::cout <<"Bad parameter" << std::endl;
    }
    return true ;
}

// ==============================================================================
auto commandShow(const std::string &value) -> bool {
    if (!value.empty()) {
        try {
            auto state = std::stoi(value,nullptr,0) != 0;
            server.show(state) ;
        }
        catch(...) {
            std::cout <<"Bad parameter" << std::endl;
        }
    }
    return true ;
}


// ===============================================================================
auto commandConnections() -> bool {
    auto con = server.informationOnConnections() ;
    std::cout << std::endl;
    for (const auto &entry:con){
        std::cout << "\t"<< entry << "\n";
    }
    std::cout<<"Total: "<< con.size() << std::endl;
   return true ;
}
// ==============================================================================
auto commandExit() -> bool {
    try {
        server.stop();
        runForever = false ;
    }
    catch(...) {
     }
    return false ;
}

// ==============================================================================
auto commandKey(const std::string &value) -> bool {
    try {
        server.setServerKey(static_cast<unsigned int>(std::stoul(value,nullptr,0)));
    }
    catch(...) {
        std::cout <<"Bad parameter" << std::endl;
    }
    return true ;
}



// ==============================================================================
auto commandNow() -> bool {
    const auto format = "%b %d %H:%M:%S"s ;
    std::cout << util::sysTimeToString(util::ourclock::now(),format) << std::endl;
    return true ;
}

// ==============================================================================
auto commandHelp(const std::string &value) -> bool {
    std::cout << std::endl;
    std::cout <<"Commands:"<<std::endl;
    std::cout <<"      help                     - prints this screen"<<std::endl;
    std::cout <<"      key serverkey            - sets the server key to look for" << std::endl;
    std::cout <<"      connections              - lists all connections" << std::endl;
    std::cout <<"      exit                     - closes all sockets and exits"<<std::endl;
    std::cout <<"      listen 0/1 portnumber    - starts listening (1) on port, or stops if 0"<<std::endl;
    std::cout <<"      load musicname lightname - sends a load packet to connected clients" << std::endl;
    std::cout <<"      play 0/1 framenumber     - sends a play packet to connected clients" << std::endl;
    std::cout <<"      show 0/1                 - sends a show packet to connected clients" << std::endl;
    std::cout << std::endl;
    return true ;
}

// ===============================================================================
auto  process(const std::string &cmd, const std::string &value) -> bool {
    if (cmd == "LISTEN") {
        return commandListen(value) ;
    }
    else if (cmd == "LOAD") {
        return commandLoad(value) ;
    }
    else if (cmd == "PLAY") {
        return commandPlay(value) ;
    }
    else if (cmd == "SHOW") {
        return commandShow(value) ;
    }
    else if (cmd == "CONNECTIONS") {
        return  commandConnections();
    }
    else if (cmd == "EXIT") {
        return  commandExit() ;
    }
    else if (cmd == "KEY") {
        return commandKey(value) ;
    }
   else if (cmd == "NOW") {
        return commandNow() ;
    }
    else if (cmd == "HELP") {
        return commandHelp(value) ;
    }
    return true ;
}


