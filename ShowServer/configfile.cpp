//Copyright © 2023 Charles Kerr. All rights reserved.

#include "configfile.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <vector>

#include "utility/strutil.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
auto ConfigFile::reset() -> void {
    showTime = util::HourRange() ;
    listenTime  =  util::HourRange();
    runTime = util::MonthRange();
    delay = 0 ;
    port = 50000 ;
    key = 0xDEADBEEF;
    lastRead =  std::chrono::file_clock::now() ;
}
// ==========================================================================================
ConfigFile::ConfigFile() : port(50000), delay(0), key(0xDEADBEEF), lastRead(std::chrono::file_clock::now()) {
}
// ==========================================================================================
auto ConfigFile::load(const std::filesystem::path &path) -> bool {
    this->reset();
    auto input = std::ifstream(path.string()) ;
    if (!input.is_open()){
        return false ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    while (input.good() && !input.eof()){
        input.getline(buffer.data(),buffer.size()-1) ;
        if (input.gcount()>0) {
            buffer[input.gcount()] = 0 ;
            std::string line = buffer.data();
            line = util::trim(util::strip(line,"#")) ;
            if (!line.empty()){
                auto [skey,value] = util::split(line,"=") ;
                auto key = util::upper(skey) ;
                try {
                    if (key == "PORT"){
                        port = std::stoi(value,nullptr,0);
                    }
                    else if (key == "KEY") {
                        key = static_cast<std::uint32_t>(std::stoul(value,nullptr,0)) ;
                    }
                    else if (key == "RUNSPAN") {
                        runTime = util::MonthRange(value) ;
                    }
                    else if (key == "LISTEN") {
                        listenTime = util::HourRange(value) ;
                    }
                    else if (key == "SHOW") {
                        showTime = util::HourRange(value) ;
                    }
                    else if (key == "DELAY") {
                        delay = std::stoi(value,nullptr,0) ;
                    }
                    else if (key == "PLAYLIST") {
                        playlist = std::filesystem::path(value) ;
                    }
                }
                catch(...) {
                    DBGMSG(std::cerr, "Error parsing line: "s + line);
                }
           }
        }
    }
    lastRead = std::filesystem::last_write_time(path) ;
    return true ;
}
// ==========================================================================================
auto ConfigFile::hasBeenUpdated(const std::filesystem::path &path) const -> bool {
    auto lastwrite = std::filesystem::last_write_time(path) ;
    
    if ( std::chrono::duration_cast<std::chrono::seconds>(lastwrite - lastRead).count() > 0 ){
        return true ;
    }
    return false ;
}

