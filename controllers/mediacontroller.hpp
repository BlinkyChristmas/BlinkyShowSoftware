//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef mediacontroller_hpp
#define mediacontroller_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <atomic>

#include "support/rtaudio/RtAudio.h"
#include "containers/wavfile/mwavfile.hpp"
//======================================================================
class MediaController {
    static auto rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,double StreamTime, RtAudioStreamStatus status , void *ptr) -> int ;
protected:
    static constexpr auto SSDRATE = 0.037 ;
    std::filesystem::path mediaLocation ;
    std::string extension ;
    
    MWAVFile mediaFile ;
    
    RtAudio soundDac ;
    RtAudio::StreamParameters rtParameters ;
    std::uint32_t bufferFrames ; // This is how many buffers the RtAudio will use, it gets passed back by RtAudio
    
    std::atomic<std::uint32_t> syncFrame ;
    std::atomic<std::uint32_t> currentFrame ;
    
    bool useSync ;
    
    auto getSoundDevice() -> int  ;
public:
    MediaController() ;
    auto requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int ;
    auto setConfiguration(const std::filesystem::path &musicdir, const std::string &extension, bool usesync) -> void ;
    auto setShow(bool state) -> bool ;

    auto isPlaying() const -> bool ;
    auto load(const std::string &media) ->std::uint32_t ;
    auto play(bool state, std::uint32_t frame = 0) -> bool ;
    auto sync( std::uint32_t frame) -> void ;
    auto setUseSync(bool state) -> void ;
};

#endif /* mediacontroller_hpp */
