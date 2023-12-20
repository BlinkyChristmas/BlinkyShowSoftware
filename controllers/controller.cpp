//Copyright © 2023 Charles Kerr. All rights reserved.

#include "controller.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
Controller::Controller() : type(ControllerType::BASE){
    
}

//======================================================================
auto Controller::play(bool state, std::uint32_t frame) -> bool {
    return true ;
}

//======================================================================
auto Controller::show(bool state) -> bool {
    return true ;
}

//======================================================================
auto Controller::sync(std::uint32_t frame) -> bool {
    return true ;
}
