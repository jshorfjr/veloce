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
)

# Platform-specific settings
if(WIN32)
    set_target_properties(nes_plugin PROPERTIES SUFFIX ".dll")
elseif(APPLE)
    set_target_properties(nes_plugin PROPERTIES SUFFIX ".dylib")
else()
    set_target_properties(nes_plugin PROPERTIES SUFFIX ".so")
endif()

# Copy plugin to plugins directory in build output
# Handle both single-config (Makefiles) and multi-config (Visual Studio) generators
add_custom_command(TARGET nes_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/plugins
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nes_plugin> ${CMAKE_BINARY_DIR}/bin/plugins/
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:nes_plugin>/../plugins
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nes_plugin> $<TARGET_FILE_DIR:nes_plugin>/../plugins/
)
