cmake_minimum_required(VERSION 3.15)

project(GcsCore VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(GCS_BUILD_TESTS "Build unit tests" ON)

# Find Packages (If using CMake native finders)
# find_package(spdlog REQUIRED)

# Include Directories
include_directories(GcsCore/include)

# Source Files
file(GLOB_RECURSE GCS_SOURCES 
    "GcsCore/src/*.cpp"
    "GcsCore/include/*.h"
)

# Static Library
add_library(GcsCore STATIC ${GCS_SOURCES})

# Target Specific Includes
target_include_directories(GcsCore PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/GcsCore/include>
    $<INSTALL_INTERFACE:include>
)

# WinRT & Windows Dependencies (Windows Specific)
if(WIN32)
    target_compile_definitions(GcsCore PRIVATE 
        _UNICODE 
        UNICODE
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
        SPDLOG_HEADER_ONLY
    )
    target_link_libraries(GcsCore PRIVATE 
        runtimeobject.lib
    )
endif()

# Tests
if(GCS_BUILD_TESTS)
    enable_testing()
    # Note: In a real scenario, you'd use FetchContent or find_package(GTest)
    # add_executable(GcsCoreTests tests/test_signal.cpp)
    # target_link_libraries(GcsCoreTests PRIVATE GcsCore GTest::gtest_main)
    # add_test(NAME SignalTest COMMAND GcsCoreTests)
endif()
