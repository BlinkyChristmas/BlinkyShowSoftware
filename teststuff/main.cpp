
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
    
    auto now = util::ourclock::now() ;
    std::cout << util::sysTimeToString(now,"%b %d %H:%M") << std::endl;
     return 0;
}
