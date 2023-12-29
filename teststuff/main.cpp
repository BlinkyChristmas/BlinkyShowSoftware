
#include <iostream>
#include <functional>
#include <cstdlib>
#include <string>
#include <thread>
#include <stdexcept>
#include <filesystem>


#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/dbgutil.hpp"


#include "controllers/lightcontroller.hpp"
#include "controllers/pruconfig.hpp"

using namespace std::string_literals ;

int main(int argc, const char * argv[]) {
    try {
        LightController lights ;
        PRUConfig pru0("0,0,0,3072") ;
        PRUConfig pru1("1,0,0,3072") ;
        std::cout << "pru0 is: " << pru0.describe() << std::endl;
        std::cout << "pru1 is: " << pru1.describe() << std::endl;
        lights.configurePRU(pru0, pru1) ;

    }
    
    catch( const std::exception &e ) {
        DBGMSG( std::cerr, "Error: "s + e.what() );

    }
     return 0;
}
