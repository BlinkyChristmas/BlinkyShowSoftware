//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef lightcontroller_hpp
#define lightcontroller_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <array>

#include "containers/lightfile/lightfile.hpp"
#include "pruconfig.hpp"
//======================================================================
class LightController {
    static constexpr size_t PRUMAPSIZE = 8192 ;
    static constexpr auto INDEX_TYPE = 0 ;
    static constexpr auto INDEX_BITREG = 4 ;
    static constexpr auto INDEX_DATAREADY = 8 ;
    static constexpr auto INDEX_OUTPUTCOUNT = 12 ;
    static constexpr auto INDEX_PRUOUTPUT = 16 ;

    std::filesystem::path lightLocation ;
    std::string lightExtension ;
    LightFile lightFile ;
    std::array<PRUConfig,2> pruConfiguration ;
    std::array<volatile std::uint8_t *,2> pruBuffers ;
    
    auto mapPRU(int number) -> void ;
    auto freePRU(int prunumber) -> void ;
    auto initPRU(int number) -> void ;

    
public:
    LightController() ;
    ~LightController() ;
    
    auto setLocation(const std::filesystem::path &path,const std::string &extension) -> void ;
    auto configurePRU(PRUConfig pru0,PRUConfig pru1) -> void ;
    auto setShow(bool state) -> bool ;
    auto load(const std::string &lightname) -> std::uint32_t ;
    auto play(bool state, std::uint32_t frame = 0) -> bool ;
    auto sync(std::uint32_t frame) -> void ;
    
};

#endif /* lightcontroller_hpp */
