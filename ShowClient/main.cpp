//Copyright © 2023 Charles Kerr. All rights reserved.

#if defined(_WIN32)
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <iostream>
#include <filesystem>

#include <string>
#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"

#include "network/blinkclient.hpp"
#include "network/allpackets.hpp"

#include "containers/wavfile/mwavfile.hpp"

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

// Foward declares, where the real action occurs
auto clientLoad(const Packet& packet, BlinkClient* ptr) -> bool;
auto clientNop(const Packet& packet, BlinkClient* ptr) -> bool;
auto clientPlay(const Packet& packet, BlinkClient* ptr) -> bool;
auto clientShow(const Packet& packet, BlinkClient* ptr) -> bool;
auto clientSync(const Packet& packet, BlinkClient* ptr) -> bool;

//======================================================================
// Main
//======================================================================
int main(int argc, const char * argv[]) {

    try {

        MWAVFile wavFile;

     if ( wavFile.load( std::filesystem::path("C:\\Users\\charles\\Documents\\projects\\BlinkyChristmas\\music\\Frosty_the_Snowman.wav") )) {
         DBGMSG(std::cerr, "Error opening wav file") ;
          return 1;
     }

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unkown exception!" << std::endl;
        return 1;
    }
    return 0;
}


//======================================================================
// Client routines
//======================================================================

//======================================================================
auto clientLoad(const Packet& packet, BlinkClient* ptr) -> bool {

    return true;
}

//======================================================================
auto clientNop(const Packet& packet, BlinkClient* ptr) -> bool {

    return true;
}

//======================================================================
auto clientPlay(const Packet& packet, BlinkClient* ptr) -> bool {

    return true;
}

//======================================================================
auto clientShow(const Packet& packet, BlinkClient* ptr) -> bool {

    return true;
}

//======================================================================
auto clientSync(const Packet& packet, BlinkClient* ptr) -> bool {

    return true;
}
