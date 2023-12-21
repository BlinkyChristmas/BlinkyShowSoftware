//Copyright © 2023 Charles Kerr. All rights reserved.

#if defined(_WIN32)
#include <Windows.h>
#endif
#include <iostream>

#include <string>
#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"


using namespace std::string_literals ;

int main(int argc, const char * argv[]) {
    
    DBGMSG(std::cout, "This is a test "s + "howdy"s) ;
    return 0;
}
