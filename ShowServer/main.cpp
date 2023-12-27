//Copyright © 2023 Charles Kerr. All rights reserved.

#include <iostream>
#include <filesystem>

#include "showmanager.hpp"
int main(int argc, const char * argv[]) {
    if (argc !=2) {
        std::cerr <<"Configuration file is required" << std::endl;
        return 1 ;
    }
    ShowManager show ;
    if (!show.run(std::filesystem::path(argv[1]))) {
        std::cerr <<"Error running show" << std::endl;
        return 1 ;
    }
    return 0 ;
}
