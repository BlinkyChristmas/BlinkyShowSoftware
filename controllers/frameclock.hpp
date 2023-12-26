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
    using FrameUpdateCallback = std::function<void(std::uint32_t)> ;
    using FrameStopCallback = std::function<void(std::uint32_t)> ;
    
    static constexpr auto FRAMETIME = 37 ;
    static constexpr auto TOLERANCE = 3 ;
    static constexpr auto BAND = 9 ;
    
protected:
    asio::io_context io_context ;
    asio::steady_timer timer;
    std::thread runThread ;
    std::atomic<bool> threadRunning ;
    
    mutable std::mutex frameAccess ;
    std::atomic<std::uint32_t> currentFrame ;
    std::uint32_t stopFrame ;
    
    bool internal_state ;
    
    // We dont want our context to stop unless we say so
    asio::executor_work_guard<asio::io_context::executor_type> contextguard ;
    
    auto tick(const asio::error_code &ec) -> void ;
    
    auto runLoop() -> void ;
    
    FrameUpdateCallback updateCallback ;
    FrameStopCallback  stopCallback ;
public:
    FrameClock() ;
    ~FrameClock() ;
    auto close() -> void ;
    auto frame() const -> std::uint32_t ;
    
    auto setUpdateCallback( const FrameUpdateCallback &function ) -> void ;
    auto setStopCallback( const FrameStopCallback &function ) -> void ;
    
    auto run(bool state, [[maybe_unused]] std::uint32_t initialFrame, [[maybe_unused]] std::uint32_t stopFrame) -> bool ;
    [[maybe_unused]] auto sync([[maybe_unused]] std::uint32_t frame) -> bool  ;
    
    
};


#endif /* frameclock_hpp */
