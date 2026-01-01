cmake_minimum_required(VERSION 3.16)
project(nes_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# NES Plugin as shared library
add_library(nes_plugin SHARED
    src/nes_plugin.cpp
    src/cpu.cpp
    src/ppu.cpp
    src/apu.cpp
    src/bus.cpp
    src/cartridge.cpp
    src/mappers/mapper.cpp
    src/mappers/mapper_000.cpp
    src/mappers/mapper_001.cpp
    src/mappers/mapper_002.cpp
    src/mappers/mapper_003.cpp
    src/mappers/mapper_004.cpp
    src/mappers/mapper_007.cpp
    src/mappers/mapper_009.cpp
    src/mappers/mapper_010.cpp
    src/mappers/mapper_011.cpp
    src/mappers/mapper_034.cpp
    src/mappers/mapper_066.cpp
    src/mappers/mapper_071.cpp
    src/mappers/mapper_079.cpp
    src/mappers/mapper_206.cpp
)

target_include_directories(nes_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Set output name without 'lib' prefix on all platforms
set_target_properties(nes_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "nes"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
)

# Handle multi-config generators (Visual Studio, Xcode)
foreach(CONFIG_TYPE IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set_target_properties(nes_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/bin/plugins
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/bin/plugins
    )
endforeach()

# Platform-specific settings
if(WIN32)
    set_target_properties(nes_plugin PROPERTIES SUFFIX ".dll")
elseif(APPLE)
    set_target_properties(nes_plugin PROPERTIES SUFFIX ".dylib")
else()
    set_target_properties(nes_plugin PROPERTIES SUFFIX ".so")
endif()
