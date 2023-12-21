//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef pruconfig_hpp
#define pruconfig_hpp

#include <cstdint>
#include <iostream>
#include <string>
//======================================================================
struct PRUConfig {
    int pru ;
    int mode ;
    int offset ;
    int length ;
    PRUConfig() ;
    PRUConfig(const std::string &line) ;
};

#endif /* pruconfig_hpp */
