//Copyright © 2023 Charles Kerr. All rights reserved.

#include "frameclock.hpp"

#include <algorithm>
#include <stdexcept>

//======================================================================
auto FrameClock::tick(const asio::error_code &ec) -> void {
    if (!ec) {
        {
            // We didn't get an error
            auto lock = std::lock_guard(frameAccess) ;
            currentFrame += 1 ;
        }
        if (threadFlag) {
            timer.expires_at( timer.expiry() + asio::chrono::milliseconds( FrameClock::FRAMETIME ) ) ;
            timer.async_wait(std::bind(&FrameClock::tick,this,std::placeholders::_1) ) ;
            frameUpdated(currentFrame) ;
        }
    }
    // Should there be an else todo something with an error? Right now, the clock stops (tick not resubmitted)
}

//======================================================================
auto FrameClock::close() -> void {
    threadFlag = false ;
    io_context.stop() ;
    if (runThread.joinable()) {
        runThread.join() ;
        runThread = std::thread() ;
    }
    io_context.restart() ;
}

//======================================================================
auto FrameClock::frameUpdated(std::uint32_t currentFrame) -> void {
    if (updateCallback != nullptr){
        updateCallback(currentFrame) ;
    }
}
//======================================================================
FrameClock::FrameClock():timer(io_context),threadFlag(false),currentFrame(0),updateCallback(nullptr){
}

//======================================================================
auto FrameClock::runLoop(std::uint32_t startframe) -> void {
    currentFrame = startframe ;
    threadFlag = true ;
    timer.expires_after(asio::chrono::milliseconds( FrameClock::FRAMETIME )) ;
    timer.async_wait(std::bind(&FrameClock::tick,this,std::placeholders::_1) ) ;
    io_context.run() ;
}
//======================================================================
FrameClock::~FrameClock() {
    threadFlag = false ;
    io_context.stop() ;
    if (runThread.joinable()){
        runThread.join() ;
        runThread = std::thread() ;
    }
}

//======================================================================
auto FrameClock::frame() const -> std::uint32_t {
    auto lock = std::lock_guard(frameAccess) ;
    return currentFrame ;
}

//======================================================================
auto FrameClock::setUpdateCallback( const std::function<void(std::uint32_t)> &function ) -> void {
    updateCallback = function ;
}

//======================================================================
auto FrameClock::play(bool state,std::uint32_t frame) -> bool{
    this->close() ;
    if (state) {
        runThread = std::thread(&FrameClock::runLoop,this,frame) ;
    }
    return true ;
}
//======================================================================
auto FrameClock::sync(std::uint32_t frame) -> bool{
    auto lock = std::lock_guard(frameAccess) ;
    std::uint32_t  current = currentFrame ; // Get what we are actually on, and make it local in case other things are updating as we work
    auto delta = std::abs(std::int64_t(current) - std::int64_t(frame)); // What is the differenc between where we should be, and where we are?
    if ( delta > TOLERANCE) { // If that is within our tolerance, do nothing
        if (delta <= BAND) {  // Ok, it is outside our tolerance, is it out side our "band".  Band is the delta that we would only nudge the frame in the correct direction, so to avoid constant big jumps
            if (current > frame){
                currentFrame -= 1 ;  // Nudge it forward
            }
            else {
                currentFrame += 1 ; // Nudge it back
            }
        }
        else { // If we are here, we beyond our band of acceptable, so we "jump" to the the frame asked for.
            currentFrame = frame ;
        }
    }
    return true ;
}
