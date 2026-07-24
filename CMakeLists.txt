# ==============================================================================
# windows-media-session-0.8.0 - Production-Grade Windows Media Integration
# ==============================================================================
# Description: Framework-agnostic C++ library for Windows Media Session
# License:     MIT
# Author:      AI-Projects Team
# Created:     2026-06-22
# ==============================================================================

cmake_minimum_required(VERSION 3.20)
project(windows-media-session VERSION 0.8.0 LANGUAGES CXX)

# ==============================================================================
# C++20 Standard (MANDATORY)
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Build Configuration
# ==============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "[windows-media-session] Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "[windows-media-session] C++ Standard: C++${CMAKE_CXX_STANDARD}")

# ==============================================================================
# Library Target
# ==============================================================================
add_library(windows-media-session STATIC
    # Public API Implementation
    src/media_session.cpp
    
    # PIMPL Implementation (WinRT)
    src/media_session_impl.cpp
    
    # Thumbnail Conversion
    src/thumbnail_converter.cpp
)

# ==============================================================================
# Include Directories
# ==============================================================================
target_include_directories(windows-media-session
    PUBLIC
        # For external projects using this library
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        # For internal implementation files
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ==============================================================================
# Compiler Options
# ==============================================================================
if(MSVC)
    target_compile_options(windows-media-session PRIVATE
        /W4                 # Warning Level 4
        /WX                 # Warnings as Errors
        /await              # Enable C++20 Coroutines (required for WinRT)
        /EHsc               # Exception Handling Model
        /utf-8              # UTF-8 Source and Execution Character Sets
    )
else()
    # Clang/GCC
    target_compile_options(windows-media-session PRIVATE
        -Wall
        -Wextra
        -Werror
        -pedantic
    )
endif()

# ==============================================================================
# Windows SDK & WinRT Dependencies
# ==============================================================================
if(WIN32)
    target_link_libraries(windows-media-session
        PRIVATE
            WindowsApp  # UWP/WinRT APIs (SystemMediaTransportControls)
    )
else()
    message(FATAL_ERROR "[windows-media-session] This library requires Windows 10+")
endif()

# ==============================================================================
# Library Properties
# ==============================================================================
set_target_properties(windows-media-session PROPERTIES
    OUTPUT_NAME "windows-media-session"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ==============================================================================
# Examples (Optional)
# ==============================================================================
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==============================================================================
# Tests (Optional)
# ==============================================================================
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    # Create tests directory structure
    add_executable(unit_tests tests/test_unit.cpp)
    
    target_link_libraries(unit_tests
        PRIVATE
            windows-media-session
    )
    
    target_include_directories(unit_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    # Copy to tests output directory
    set_target_properties(unit_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/${CMAKE_CFG_INTDIR}"
    )
    
    enable_testing()
    add_test(NAME UnitTests COMMAND unit_tests)
endif()

# ==============================================================================
# Installation
# ==============================================================================
# Install library
install(TARGETS windows-media-session
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install public headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install documentation
install(FILES
    README.md
    LICENSE
    DESTINATION share/doc/windows-media-session
)

# ==============================================================================
# Summary
# ==============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "windows-media-session Configuration")
message(STATUS "========================================")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:   C++${CMAKE_CXX_STANDARD}")
if(MSVC)
    message(STATUS "Compiler:       MSVC ${MSVC_VERSION}")
else()
    message(STATUS "Compiler:       ${CMAKE_CXX_COMPILER_ID}")
endif()
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Tests:    ${BUILD_TESTING}")
message(STATUS "========================================")
message(STATUS "")
