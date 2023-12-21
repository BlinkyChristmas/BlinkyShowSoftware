cmake_minimum_required(VERSION 3.24)
project(showclient VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(showclient
    main.cpp
    
    ../utility/buffer.cpp
    ../utility/buffer.hpp
    ../utility/timeutil.cpp
    ../utility/timeutil.hpp
    ../utility/strutil.hpp
    
    ../support/rtaudio/RtAudio.cpp
    

)
target_compile_definitions(showclient PRIVATE
        ASIO_STANDALONE
)

if (WIN32)
    target_compile_definitions(showclient PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
        __WINDOWS_DS__
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    )

    target_compile_options(showclient PRIVATE
       $<$<CONFIG::Release>:/02>
    )

    target_link_libraries(showclient
        PUBLIC
            ole32
            winnm
            dsound
    )
else()
    target_compile_definitions(showclient PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )

    target_compile_options(showclient PRIVATE
        $<$<CONFIG:Release>:-O2>
    )
    if (APPLE)
        target_compile_definitions(showclient PRIVATE
            __MACOSX_CORE__
        )

        FIND_LIBRARY(COREAUDIO CoreAudio)
        FIND_LIBRARY(COREFOUNDATION CoreFoundation )
        MARK_AS_ADVANCED ( COREAUDIO COREFOUNDATION )

        target_link_libraries(showclient
            PUBLIC
                pthread
                ${COREAUDIO}
                ${COREFOUNDATION}
        )

    else ()
        target_compile_definitions(showclient PRIVATE
            __LINUX_ALSA__
        )

        if (BEAGLE)
            target_compile_definitions(showclient PRIVATE
                BEAGLE
            )
        endif (BEAGLE)
    
        target_link_libraries(showclient
            PUBLIC
                asound
                pthread
        )

    endif (APPLE)
    
endif (WIN32)

target_include_directories(showclient
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../
        ${PROJECT_SOURCE_DIR}/../support/
        ${PROJECT_SOURCE_DIR}/../support/asio/
)

