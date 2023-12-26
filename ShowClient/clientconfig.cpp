//Copyright © 2023 Charles Kerr. All rights reserved.

#include "clientconfig.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "utility/strutil.hpp"

using namespace std::string_literals ;

//======================================================================
ClientConfig::ClientConfig():clientPort(0),serverPort(0),useAudio(false),useLights(false),musicExtension(".wav"),serverKey(0xDEADBEEF),name("Forgot"),lightExtension(".light"){
    
}

//======================================================================
ClientConfig::ClientConfig(const std::filesystem::path &configpath){
    if (!load(configpath)){
        throw std::runtime_error("Unable to process: "s + configpath.string());
    }
}

//======================================================================
auto ClientConfig::load(const std::filesystem::path &configpath) -> bool {
    auto buffer = std::vector<char>(1024,0) ;
    auto input = std::ifstream(configpath.string()) ;
    if (!input.is_open()){
        return false ;
    }
    while (input.good() && !input.eof()) {
        input.getline(buffer.data(),buffer.size()-1) ;
        if(input.gcount()>0){
            buffer[input.gcount()] = 0 ;
            std::string line = buffer.data() ;
            line = util::trim(util::strip(line,"#")) ;
            if (!line.empty()){
                auto [key,value] = util::split(line,"=");
                if (!value.empty()){
                    auto ukey = util::upper(key) ;
                    if (!processKeyValue(ukey, value)){
                        std::cerr <<"Error processing line: " << line << std::endl;
                    }
                }
            }
        }
    }
    return true ;
}
//======================================================================
auto ClientConfig::processKeyValue(const std::string &ukey, const std::string &value) -> bool {
    try {
        if (ukey == "CLIENTPORT") {
            
            clientPort = std::stoi(value,nullptr,0) ;
        }
        else if (ukey == "SERVER") {
            auto [ip,port] = util::split(value,",") ;
            serverIP = ip ;
            serverPort = std::stoi(port,nullptr,0) ;
        }
        else if (ukey == "NAME") {
            name = value ;
        }
        else if (ukey == "KEY") {
            serverKey = static_cast<std::uint32_t>(std::stoul(value,nullptr,0)) ;
        }
        else if (ukey == "MUSICPATH") {
            musicPath = std::filesystem::path(value) ;
        }
        else if (ukey == "LIGHTPATH") {
            lightPath = std::filesystem::path(value) ;
        }
        else if (ukey == "MUSICEXTENSION") {
            musicExtension = value ;
        }
        else if (ukey == "LIGHTEXTENSION") {
            lightExtension = value ;
        }
        else if (ukey == "PRU") {
            auto pru = PRUConfig(value)  ;
            if (pru.pru >=0 && pru.pru <= 1) {
                pruSetting.at(pru.pru) = pru ;
            }
        }
        else if (ukey == "CONNECTHOURS") {
            connectTime = util::HourRange(value) ;
        }
        else if (ukey == "RUNSPAN") {
            runSpan = util::MonthRange(value) ;
        }
        else if (ukey == "AUDIO") {
            useAudio = std::stoi(value,nullptr,0) != 0 ;
        }
        else if (ukey == "LIGHTS") {
            useLights = std::stoi(value,nullptr,0) != 0 ;
        }
        else {
            return false ;
        }
        return true ;
    }
    catch (...) {
        return false ;
    }
}
