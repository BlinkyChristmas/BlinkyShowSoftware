//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef statusled_hpp
#define statusled_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

constexpr auto  SHOWSTATUS = 0 ;
constexpr auto  CONNECTSTATUS = 1 ;
constexpr auto  PLAYSTATUS = 2 ;
constexpr auto  RUNSTATUS = 3 ;
constexpr auto  LEDCOUNT = 4 ;
//======================================================================
class StatusLed {
public:
    enum LEDMode {
        OFF,ON,FLASH
    };
private:
    static const std::vector<std::string> LEDLOCATION ;
    static const std::vector<std::string> LEDNAMES ;
    std::vector<int> ledstates ;

    auto ledNameFor(int lednumber) -> const std::string& ;
public:
    StatusLed() ;
    auto setLed(int lednumber, LEDMode mode,bool force = false) -> void ;
    auto clearAll(bool force = false) -> void ;
    auto setAll(bool force = false ) -> void ;
    auto flashAll(bool force = false) -> void ;
};

#endif /* statusled_hpp */
