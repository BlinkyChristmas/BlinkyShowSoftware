//Copyright © 2023 Charles Kerr. All rights reserved.

#if defined(_WIN32)
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <iostream>
#include <filesystem>
#include <chrono>
#include <string>
#include <cstdlib>

#include "showclient.hpp"
#include "controllers/statusled.hpp"

using namespace std::string_literals ;


//======================================================================
auto usage() -> void {
    std::cout << "Usage:\n";
    std::cout << "   ShowClient configfile\n";
    std::cout <<" Where:\n";
    std::cout <<"     configfile is a file path to the client configuration file" << std::endl;
}


//======================================================================
// Main
//======================================================================
int main(int argc, const char * argv[]) {
    ShowClient showClient ;
    StatusLed statusLed ;
    auto status = EXIT_SUCCESS ;
    try {
        statusLed.clearAll() ;
        if (argc != 2) {
            usage() ;
            throw std::runtime_error("Insufficient parameters.");
        }
        auto path = std::filesystem::path(argv[1]) ;
        if (!showClient.setConfiguration(path)) {
            
        }
        showClient.run() ;
    }
    catch(const std::exception &e){
        std::cerr << "Exception: "<<e.what() << std::endl;
        statusLed.flashAll();
        status = EXIT_FAILURE ;
    }
    catch(...){
        statusLed.flashAll();
        std::cerr << "Unknown exception" << std::endl;
        status = EXIT_FAILURE ;
    }
     return status;
}


