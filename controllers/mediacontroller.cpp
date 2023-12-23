//Copyright © 2023 Charles Kerr. All rights reserved.

#include "mediacontroller.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;



/* ************************************************************************************************************************************
 This is our global callback, that then relays it to the instance version
 ************************************************************************************************************************************ */
//======================================================================
auto MediaController::rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,double StreamTime, RtAudioStreamStatus status , void *ptr) -> int {
    // We pass the address to our instance in ptr!
    return reinterpret_cast<MediaController*>(ptr)->requestData(reinterpret_cast<std::uint8_t*>(outputBuffer),nFrames,StreamTime,status) ;
}


//======================================================================
auto MediaController::getSoundDevice() -> int  {
    auto devices = soundDac.getDeviceIds() ;
    auto mydevice = soundDac.getDefaultOutputDevice() ;
    if (mydevice != 0) {
        auto iter = std::find_if(devices.begin(),devices.end(),[mydevice](int id) {
            return mydevice == id ;
        });
        if (iter == devices.end()) {
            mydevice = 0 ;
        }
    }
    return mydevice ;
}

//======================================================================
MediaController::MediaController():syncFrame(0),currentFrame(0),useSync(true),bufferFrames(1632) {
    
}

//======================================================================
auto MediaController::requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int {
    if (useSync) {
        auto delta = 0 ;
        if (currentFrame > syncFrame) {
            delta = currentFrame - syncFrame ;
        }
        else {
            delta = syncFrame - currentFrame ;
        }
        if (delta >= 2) {
            mediaFile.setFrame(syncFrame) ;
            currentFrame = static_cast<std::uint32_t>(syncFrame) ;
        }
    }
    auto amount = mediaFile.loadBuffer(data, frameCount) ;
    currentFrame += 1 ;
    if (amount < frameCount) {
        return 1 ;
    }
    else if (amount == 0 ){
        return 2 ;
    }
    return 0 ;
    
}
//======================================================================
auto MediaController::setConfiguration(const std::filesystem::path &musicdir, const std::string &extension,bool usesync) -> void {
    this->mediaLocation = musicdir ;
    this->extension = extension ;
    this->useSync = usesync ;
}

//======================================================================
auto MediaController::setShow(bool state) -> bool {
    try {
        // verifiy the music directory exists
        if (state) {
            if (!std::filesystem::exists(mediaLocation)){
                return false ;
            }
            auto  device = getSoundDevice() ;
            if (device == 0) {
                // We have no sound device
                return false ;
            }
            // Ok, we have a sound device
            if (soundDac.isStreamOpen()) {
                if (soundDac.isStreamRunning()){
                    soundDac.abortStream() ;
                }
                soundDac.closeStream();
            }
            
            rtParameters.deviceId = device ;
            rtParameters.nChannels = 2 ;
            soundDac.openStream(&rtParameters, NULL, RTAUDIO_SINT16, 44100, &bufferFrames, &MediaController::rtCallback, this);
            return soundDac.isStreamOpen() ;
        }
        else {
            if (soundDac.isStreamOpen()) {
                if (soundDac.isStreamRunning()){
                    soundDac.abortStream() ;
                }
                soundDac.closeStream();
            }
            if (mediaFile.isLoaded()) {
                mediaFile.clear() ;
            }
            return true ;
        }
    }
    catch(...) {
        DBGMSG(std::cerr,"Error opening/closing stream");
        return false ;
    }
    
}

//======================================================================
auto MediaController::isPlaying() const -> bool {
    return soundDac.isStreamRunning() ;
}


//======================================================================
auto MediaController::load(const std::string &media) -> bool {
    try {
        if (soundDac.isStreamRunning()) {
            soundDac.abortStream() ;
        }
        if (mediaFile.isLoaded()){
            mediaFile.clear() ;
        }
        syncFrame = 0 ;
        currentFrame = 0 ;
        return mediaFile.load( mediaLocation / std::filesystem::path(media + extension) ) ;
    }
    catch(...) {
        DBGMSG(std::cerr, "Error loading media file");
        return false ;
    }
}


//======================================================================
auto MediaController::play(bool state, std::uint32_t frame ) -> bool{
    if (state) {
        if (!soundDac.isStreamOpen()) {
            return false ;
        }
        currentFrame = frame ;
        if (useSync) {
            syncFrame = frame ;
        }
        soundDac.startStream() ;
        return soundDac.isStreamRunning() ;
    }
    else  if (soundDac.isStreamRunning()) {
        soundDac.stopStream() ;
        if (mediaFile.isLoaded()) {
            mediaFile.clear() ;
        }
        return !soundDac.isStreamRunning() ;
    }
    return true ;
}

//======================================================================
auto MediaController::sync( std::uint32_t frame) -> void {
    this->syncFrame = frame ;
}
