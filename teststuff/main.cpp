
#include <iostream>
#include <functional>
#include <cstdlib>
#include <string>
#include <thread>
#include <stdexcept>

#include "asio/asio.hpp"

#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/dbgutil.hpp"

#include "network/client.hpp"
#include "network/allpackets.hpp"


using namespace std::string_literals ;


int main(int argc, const char * argv[]) {
    util::HourRange one("10:10,12:12") ;
    util::HourMinute two("10:40") ;
    auto value  = one.inRange(two) ;
    
     return 0;
}
