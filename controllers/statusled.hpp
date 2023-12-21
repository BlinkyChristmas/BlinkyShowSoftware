//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef statusled_hpp
#define statusled_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

//======================================================================
class StatusLed {
public:
    static constexpr auto  SHOWSTATUS = 0 ;
    static constexpr auto  CONNECTSTATUS = 1 ;
    static constexpr auto  PLAYSTATUS = 2 ;
    static constexpr auto  RUNSTATUS = 3 ;
    static constexpr auto  LEDCOUNT = 4 ;

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
