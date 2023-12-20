//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef controller_hpp
#define controller_hpp

#include <cstdint>
#include <iostream>
#include <string>

//======================================================================
class Controller {
public:
    enum ControllerType {
        BASE,FRAME,MEDIA,LIGHT,PROJECTOR
    };
protected:
    ControllerType type ;
public:
    Controller() ;
    virtual ~Controller() = default ;
    
    [[maybe_unused]] virtual auto play([[maybe_unused]] bool state,[[maybe_unused]] std::uint32_t frame) -> bool ;
    [[maybe_unused]] virtual auto show([[maybe_unused]] bool state) -> bool ;
    [[maybe_unused]] virtual auto sync([[maybe_unused]] std::uint32_t frame) -> bool ;
};

#endif /* controller_hpp */
