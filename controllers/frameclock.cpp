//Copyright © 2023 Charles Kerr. All rights reserved.

#include "frameclock.hpp"

#include <algorithm>
#include <stdexcept>

//======================================================================
auto FrameClock::tick(const asio::error_code &ec) -> void {
    if (ec) {
        timer.cancel() ;
        // Should we send a stop?
        
        return ;
    }
    {
        // We didn't get an error
        auto lock = std::lock_guard(frameAccess) ;
        currentFrame += 1 ;
    }
    if (currentFrame >= stopFrame) {
        
        if (stopCallback != nullptr) {
            stopCallback(currentFrame) ;
        }
    }
    else {
        timer.expires_at( timer.expiry() + asio::chrono::milliseconds( FrameClock::FRAMETIME ) ) ;
        timer.async_wait(std::bind(&FrameClock::tick,this,std::placeholders::_1) ) ;
        if (updateCallback != nullptr){
            updateCallback(currentFrame) ;
        }
    }
}


//======================================================================
FrameClock::FrameClock():timer(io_context),threadRunning(false),currentFrame(0),updateCallback(nullptr),stopCallback(nullptr),contextguard{asio::make_work_guard(io_context)},internal_state(false){
    runThread = std::thread(&FrameClock::runLoop,this) ;
}

//======================================================================
auto FrameClock::runLoop() -> void {
    threadRunning = true ;
    io_context.run() ;
    threadRunning = false ;
}
//======================================================================
FrameClock::~FrameClock() {
    io_context.stop() ;
    if (runThread.joinable()){
        runThread.join() ;
        runThread = std::thread() ;
    }
}
//======================================================================
auto FrameClock::close() -> void {
    timer.cancel() ;
}
//======================================================================
auto FrameClock::frame() const -> std::uint32_t {
    auto lock = std::lock_guard(frameAccess) ;
    return currentFrame ;
}

//======================================================================
auto FrameClock::setUpdateCallback( const FrameUpdateCallback &function ) -> void {
    updateCallback = function ;
}
//======================================================================
auto FrameClock::setStopCallback( const FrameUpdateCallback &function ) -> void {
    stopCallback = function ;
}

//======================================================================
auto FrameClock::run(bool state,std::uint32_t frame,std::uint32_t endFrame) -> bool{
    if (state) {
        if (!internal_state) {
            internal_state = true ;
            currentFrame = frame ;
            stopFrame = endFrame ;
            timer.expires_after(asio::chrono::milliseconds( FrameClock::FRAMETIME )) ;
            timer.async_wait(std::bind(&FrameClock::tick,this,std::placeholders::_1) ) ;
        }
   }
    else {
        if (internal_state){
            internal_state = false ;
            timer.cancel() ;
            if (stopCallback != nullptr) {
                stopCallback(currentFrame) ;
            }
         }
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
