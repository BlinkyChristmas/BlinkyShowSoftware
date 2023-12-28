cmake_minimum_required(VERSION 3.24)
project(ShowClient VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(ShowClient
    main.cpp
    clientconfig.cpp
    clientconfig.hpp
    showclient.cpp
    showclient.hpp
    
    ../utility/buffer.cpp
    ../utility/buffer.hpp
    ../utility/timeutil.cpp
    ../utility/timeutil.hpp
    ../utility/strutil.hpp
    ../utility/dbgutil.hpp
    ../utility/mapfile.cpp
    ../utility/mapfile.hpp
    
    ../containers/lightfile/lightfile.cpp
    ../containers/lightfile/lightfile.hpp
    ../containers/lightfile/lightheader.cpp
    ../containers/lightfile/lightheader.hpp
    ../containers/wavfile/mwavfile.cpp
    ../containers/wavfile/mwavfile.hpp
    ../containers/wavfile/chunkheader.cpp
    ../containers/wavfile/chunkheader.hpp
    ../containers/wavfile/fileheader.cpp
    ../containers/wavfile/fileheader.hpp
    ../containers/wavfile/wavfmtchunk.cpp
    ../containers/wavfile/wavfmtchunk.hpp

    ../controllers/frameclock.cpp
    ../controllers/frameclock.hpp
    ../controllers/lightcontroller.cpp
    ../controllers/lightcontroller.hpp
    ../controllers/mediacontroller.cpp
    ../controllers/mediacontroller.hpp
    ../controllers/pruconfig.cpp
    ../controllers/pruconfig.hpp
    ../controllers/statusled.cpp
    ../controllers/statusled.hpp

    ../network/client.cpp
    ../network/client.hpp
    ../network/identpacket.cpp
    ../network/identpacket.hpp
    ../network/loadpacket.cpp
    ../network/loadpacket.hpp
    ../network/noppacket.cpp
    ../network/noppacket.hpp
    ../network/packet.cpp
    ../network/packet.hpp
    ../network/playpacket.cpp
    ../network/playpacket.hpp
    ../network/showpacket.cpp
    ../network/showpacket.hpp
    ../network/syncpacket.cpp
    ../network/syncpacket.hpp
    ../network/allpackets.hpp

    ../support/rtaudio/RtAudio.cpp
    ../support/rtaudio/RtAudio.h

)
target_compile_definitions(ShowClient PRIVATE
        ASIO_STANDALONE
)

if (WIN32)
    target_compile_definitions(ShowClient PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
        __WINDOWS_DS__
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	_WIN32_WINNT=0x0A00
    )

    target_compile_options(ShowClient PRIVATE
       $<$<CONFIG::Release>:/02>
    )

    target_link_libraries(ShowClient
        PUBLIC
            ole32
            winnm
            dsound
    )
else()
    target_compile_definitions(ShowClient PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )

    target_compile_options(ShowClient PRIVATE
        $<$<CONFIG:Release>:-O2>
        -Wno-deprecated-declarations
    )
    if (APPLE)
        target_compile_definitions(ShowClient PRIVATE
            __MACOSX_CORE__
        )

        FIND_LIBRARY(COREAUDIO CoreAudio)
        FIND_LIBRARY(COREFOUNDATION CoreFoundation )
        MARK_AS_ADVANCED ( COREAUDIO COREFOUNDATION )

        target_link_libraries(ShowClient
            PUBLIC
                pthread
                ${COREAUDIO}
                ${COREFOUNDATION}
        )

    else ()
        target_compile_definitions(ShowClient PRIVATE
            __LINUX_ALSA__
        )

        if (BEAGLE)
            target_compile_definitions(ShowClient PRIVATE
                BEAGLE
            )
        endif (BEAGLE)
    
        target_link_libraries(ShowClient
            PUBLIC
                asound
                pthread
        )

    endif (APPLE)
    
endif (WIN32)

target_include_directories(ShowClient
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../
        ${PROJECT_SOURCE_DIR}/../support/
        ${PROJECT_SOURCE_DIR}/../support/asio/
)

