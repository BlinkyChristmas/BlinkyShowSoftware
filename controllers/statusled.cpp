//Copyright © 2023 Charles Kerr. All rights reserved.

#include "statusled.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
const std::vector<std::string> StatusLed::LEDLOCATION = {
    "/sys/class/leds/beaglebone:green:usr0"s ,
    "/sys/class/leds/beaglebone:green:usr1"s ,
    "/sys/class/leds/beaglebone:green:usr2"s ,
    "/sys/class/leds/beaglebone:green:usr3"s
};

const std::vector<std::string> StatusLed::LEDNAMES{
    "SHOWSTATUS"s,"CONNECTSTATUS"s,"PLAYSTATUS"s,"RUNSTATUS"s
};

//======================================================================
auto StatusLed::ledNameFor(int lednumber) -> const std::string& {
    static const auto BADLED = "BADLED"s ;
    if (lednumber <0 || lednumber >= LEDCOUNT) {
        return BADLED ;
    }
    return LEDNAMES.at(lednumber) ;
}

//======================================================================
StatusLed::StatusLed(){
    ledstates = std::vector<int>(LEDCOUNT,static_cast<int>(LEDMode::OFF)) ;
}
//======================================================================
auto StatusLed::setLed(int lednumber, LEDMode mode, bool force) -> void {
    if (ledstates.at(lednumber) != static_cast<int>(mode) || force){
#if defined(BEAGLE)
        switch (mode) {
            case LEDMode::ON: {
                
                DBGMSG(std::cerr, "Opening: "s + LEDLOCATION.at(lednumber)+"/trigger"s) ;
                auto output = std::ofstream(LEDLOCATION.at(lednumber)+"/trigger"s) ;
                if (!output.is_open()) {
                    std::cerr << "Unable to open led: " << LEDLOCATION.at(lednumber) + "/trigger"s << std::endl;
                    return ;
                }
                output << "none" ;
                output.close();
                
                output = std::ofstream(LEDLOCATION.at(lednumber)+"/brightness"s) ;
                if (!output.is_open()) {
                    std::cerr << "Unable to open led: " << LEDLOCATION.at(lednumber) + "/brightness"s << std::endl;
                    return ;
                }
                output << "1" ;
                output.close();
                break;
            }
            case LEDMode::OFF: {
                auto output = std::ofstream(LEDLOCATION.at(lednumber)+"/trigger"s) ;
                if (!output.is_open()) {
                    std::cerr << "Unable to open led: " << LEDLOCATION.at(lednumber) + "/trigger"s << std::endl;
                    return ;
                }
                output << "none" ;
                output.close();
                output = std::ofstream(LEDLOCATION.at(lednumber)+"/brightness"s) ;
                if (!output.is_open()) {
                    std::cerr << "Unable to open led: " << LEDLOCATION.at(lednumber) + "/brightness"s << std::endl;
                }
                output << "0" ;
                output.close();
                
                break;
            }
            case LEDMode::FLASH: {
                auto output = std::ofstream(LEDLOCATION.at(lednumber)+"/trigger"s) ;
                if (!output.is_open()) {
                    std::cerr << "Unable to open led: " << LEDLOCATION.at(lednumber) + "/trigger"s << std::endl;
                    return ;
                }
                output << "timer" ;
                output.close();
                
                
                break;
            }
            default:
                break;
                
        }
#else
#if defined(_DEBUG) || defined(DEBUG)
        auto state = " UNKNOWN"s ;
        switch (mode) {
            case LEDMode::ON: {
                state =  " ON"s ;
                break;
            }
            case LEDMode::OFF: {
                state =  " OFF"s ;
                break;
            }
            case LEDMode::FLASH: {
                state =  " FLASH"s ;
                break;
            }
            default:
                break;
                
        }
        DBGMSG(std::cout , "Turning LED "s + ledNameFor(lednumber) + state) ;
#endif
#endif
        ledstates.at(lednumber) = static_cast<int>(mode) ;
    }
}

//======================================================================
auto StatusLed::clearAll(bool force) -> void {
    for (auto led = 0 ; led < LEDCOUNT ; led += 1){
        this->setLed(led, LEDMode::OFF,force);
    }
}

//======================================================================
auto StatusLed::setAll(bool force ) -> void {
    for (auto led = 0 ; led < LEDCOUNT ; led += 1){
        this->setLed(led, LEDMode::ON,force);
    }
    
}

//======================================================================
auto StatusLed::flashAll(bool force) -> void {
    for (auto led = 0 ; led < LEDCOUNT ; led += 1){
        this->setLed(led, LEDMode::FLASH,force);
    }
}

