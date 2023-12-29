cmake_minimum_required(VERSION 3.24)
project(teststuff VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(teststuff
    main.cpp
    
    
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


    ../controllers/lightcontroller.cpp
    ../controllers/lightcontroller.hpp
    ../controllers/frameclock.cpp
    ../controllers/frameclock.hpp
    ../controllers/pruconfig.cpp
    ../controllers/pruconfig.hpp
    ../controllers/statusled.cpp
    ../controllers/statusled.hpp
)
target_compile_definitions(teststuff PRIVATE
        ASIO_STANDALONE
)

if (WIN32)
    target_compile_definitions(teststuff PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
        __WINDOWS_DS__
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        _WIN32_WINNT=0x0A00
    )

    target_compile_options(teststuff PRIVATE
       $<$<CONFIG::Release>:/02>
    )

    target_link_libraries(teststuff
        PUBLIC
            ole32
            winnm
            dsound
    )
else()
    target_compile_definitions(teststuff PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )

    target_compile_options(teststuff PRIVATE
        $<$<CONFIG:Release>:-O2>
        -Wno-deprecated-declarations
    )
    if (APPLE)
        target_compile_definitions(teststuff PRIVATE
            __MACOSX_CORE__
        )

        FIND_LIBRARY(COREAUDIO CoreAudio)
        FIND_LIBRARY(COREFOUNDATION CoreFoundation )
        MARK_AS_ADVANCED ( COREAUDIO COREFOUNDATION )

        target_link_libraries(teststuff
            PUBLIC
                pthread
                ${COREAUDIO}
                ${COREFOUNDATION}
        )

    else ()
        target_compile_definitions(teststuff PRIVATE
            __LINUX_ALSA__
        )

        if (BEAGLE)
            target_compile_definitions(teststuff PRIVATE
                BEAGLE
            )
        endif (BEAGLE)
    
        target_link_libraries(teststuff
            PUBLIC
                asound
                pthread
        )

    endif (APPLE)
    
endif (WIN32)

target_include_directories(teststuff
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../
        ${PROJECT_SOURCE_DIR}/../support/
        ${PROJECT_SOURCE_DIR}/../support/asio/
)

