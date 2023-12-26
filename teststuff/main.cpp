
#include <iostream>
#include <functional>
#include <cstdlib>
#include <string>
#include <thread>
#include <stdexcept>
#include <filesystem>


#include "asio/asio.hpp"
#include "rtaudio/RtAudio.h"

#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/dbgutil.hpp"

#include "network/client.hpp"
#include "network/allpackets.hpp"

#include "containers/wavfile/mwavfile.hpp"
#include "containers/lightfile/lightfile.hpp"

#include "controllers/mediacontroller.hpp"
#include "controllers/lightcontroller.hpp"
#include "controllers/frameclock.hpp"
#include "controllers/statusled.hpp"


using namespace std::string_literals ;
asio::io_context io_context ;

asio::ip::tcp::socket netSocket{io_context} ;

Packet incomingPacket ;
int incomingBytes = 0 ;
std::thread runThread ;
auto printHello() -> void {
    std::cout << "hello" << std::endl;
}
auto packetRead(const asio::error_code& ec, size_t bytes_transferred) -> void {
    if (ec) {
        
    }
}

int main(int argc, const char * argv[]) {
    try {
        runThread = std::thread(&printHello) ;
        runThread.join() ;
        runThread = std::thread() ;
        if (runThread.joinable()) {
            std::cout <<"Joinable" << std::endl;
        }
    }
    
    catch( const std::exception &e ) {
        DBGMSG( std::cerr, "Error: "s + e.what() );

    }
     return 0;
}
