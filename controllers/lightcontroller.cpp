//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightcontroller.hpp"

#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utility>

#if defined(BEAGLE)
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
using namespace std::string_literals ;

//======================================================================
LightController::LightController() {
    for (size_t index = 0 ; index < pruBuffers.size();index++){
        pruBuffers[index] = nullptr ;
    }
}

//======================================================================
LightController::~LightController() {
    freePRU(0);
    freePRU(1) ;
    lightFile.clear();
}
//======================================================================
auto LightController::mapPRU(int number) -> void {
    auto pru = pruBuffers[number] ;
    DBGMSG(std::cout, "mapping pru: " + std::to_string(number)) ;
    if (pru != nullptr){
        this->freePRU(number) ;
    }
    pru = nullptr ;
#if defined(BEAGLE)
    off_t target = (number == 0 ? 0x4a300000 : 0x4a302000) ;
    DBGMSG(std::cout,"Offset is: "s +util::ntos(target,16,true,8) );
    auto fd = ::open("/dev/mem", O_RDWR | O_SYNC) ;
    if (fd <0) {
        // we couldn't open it
        std::cerr << "unable to open PRU " << std::to_string(number)<<"\n"s;
        return  ;
    }
    auto temp = mmap(0,PRUMAPSIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd, target) ;
    ::close(fd) ;
    if (temp == MAP_FAILED ) {
        DBGMSG(std::cerr,"Mapped failed!" );

        return  ;
    }
    DBGMSG(std::cout,"Mapped success!" );
    pru = reinterpret_cast<std::uint8_t*>(temp) ;
    
#endif
    pruBuffers[number] = pru ;
}
//======================================================================
auto LightController::freePRU(int prunumber) -> void {
    [[maybe_unused]] auto pru = pruBuffers[prunumber] ;
#if defined(BEAGLE)
    if (pru != nullptr) {
        auto ptr = reinterpret_cast<void*>(const_cast<unsigned char*>(pru)) ; // the const cast gets rids of volatile
        munmap(ptr, PRUMAPSIZE) ;
    }
#endif
    pruBuffers[prunumber] = nullptr ;
    
}
//======================================================================
auto LightController::initPRU(int number) -> void {
    DBGMSG(std::cout,"Initialize pru: "s+std::to_string(number) );
    mapPRU(number) ;
    auto pru = pruBuffers[number] ;
    auto bit = (number == 0 ? 1 : 14) ; // PRU 0 is on bit 0, pru 1 is on bit 14 ;
    DBGMSG(std::cout,"Bit is: "s+std::to_string(bit) );

    auto mode = pruConfiguration[number].mode ;
    auto zero = 0 ;
    auto size = (mode == PRUConfig::MODE_SSD ? 3072 : 512) ;
    if (pru != nullptr) {
        auto buffer = std::vector<char>(3072, 0 ) ;
        DBGMSG(std::cout,"Setting up pru memory header" );

        std::copy(reinterpret_cast<char*>(&mode),reinterpret_cast<char*>(&mode)+4,pru + INDEX_TYPE) ;
        std::copy(reinterpret_cast<char*>(&bit),reinterpret_cast<char*>(&bit)+4,pru + INDEX_BITREG) ;
        std::copy(reinterpret_cast<char*>(&size),reinterpret_cast<char*>(&size)+4,pru + INDEX_OUTPUTCOUNT) ;
        std::copy(reinterpret_cast<char*>(&zero),reinterpret_cast<char*>(&zero)+4,pru + INDEX_DATAREADY) ;
        std::copy(buffer.begin(),buffer.end(),pru + INDEX_PRUOUTPUT) ;
    }
    
}

//======================================================================
auto LightController::setLocation(const std::filesystem::path &path,const std::string &extension) -> void {
    this->lightLocation = path ;
    this->lightExtension = extension ;
}
//======================================================================
auto LightController::configurePRU(PRUConfig pru0,PRUConfig pru1) -> void {
    pruConfiguration[0] = pru0 ;
    DBGMSG(std::cout, "Initializing pru 0");
    initPRU(0);
    pruConfiguration[1] = pru1 ;
    DBGMSG(std::cout, "Initializing pru 1");
    initPRU(1) ;
    
}
//======================================================================
auto LightController::setShow(bool state) -> bool {
    if (state) {
        return std::filesystem::exists(lightLocation) ;
    }
    else {
        //
        return true ;
    }
}
//======================================================================
auto LightController::load(const std::string &lightname) -> std::uint32_t {
    try {
        if (lightname.empty()){
            return 0 ;
        }
        if (!lightFile.loadFile( lightLocation / std::filesystem::path (lightname + lightExtension)) ){
            return 0 ;
        }
        return lightFile.frameCount() ;
    }
    catch(...) {
        return 0 ;
    }
}
//======================================================================
auto LightController::play(bool state, std::uint32_t frame) -> bool {
    if (state) {
        if (!lightFile.isLoaded()) {
            return false ;
        }
        // Do something here?
        for ( auto pru = 0 ; pru < 2; pru++ ){
            lightFile.copy(frame, reinterpret_cast<char*>(const_cast<std::uint8_t*>(pruBuffers.at(pru))),pruConfiguration.at(pru).offset,pruConfiguration.at(pru).length) ;
        }
    }
    else {
        lightFile.clear() ;
    }
    return true ;
}

//======================================================================
auto LightController::sync(std::uint32_t frame) -> void {
    // We should update the lights here!
    if (frame < lightFile.frameCount()){
        for (auto p=0 ; p < 2; p++) {
            auto index = pruConfiguration.at(p).offset ;
            auto length = static_cast<unsigned int>(pruConfiguration.at(p).length) ;
            auto maxlength = (lightFile.frameLength() - index) ;
            if (length > maxlength) {
                length  = maxlength ;
            }
            lightFile.copy(frame, reinterpret_cast<char*>(const_cast<std::uint8_t*>(pruBuffers.at(p))),index,length) ;
        }
    }
}
