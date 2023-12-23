//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef frameclock_hpp
#define frameclock_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

#include "asio/asio.hpp"

//======================================================================
class FrameClock {
public:
    using FrameCallback = std::function<void(std::uint32_t)> ;
    static constexpr auto FRAMETIME = 37 ;
    static constexpr auto TOLERANCE = 3 ;
    static constexpr auto BAND = 9 ;
    
protected:
    asio::io_context io_context ;
    asio::steady_timer timer;
    std::thread runThread ;
    std::atomic<bool> threadFlag ;
    
    mutable std::mutex frameAccess ;
    std::atomic<std::uint32_t> currentFrame ;
    
    
    auto tick(const asio::error_code &ec) -> void ;
    auto close() -> void ;
    
    auto runLoop(std::uint32_t startframe = 0) -> void ;
    
    virtual auto frameUpdated(std::uint32_t currentFrame) -> void ;
    
    FrameCallback updateCallback ;
public:
    FrameClock() ;
    virtual ~FrameClock() ;
    
    auto frame() const -> std::uint32_t ;
    
    auto setUpdateCallback( const FrameCallback &function ) -> void ;
    
    [[maybe_unused]] auto play([[maybe_unused]] bool state, [[maybe_unused]] std::uint32_t frame = 0) -> bool ;
    [[maybe_unused]] auto sync([[maybe_unused]] std::uint32_t frame) -> bool  ;
    
    
};


#endif /* frameclock_hpp */
