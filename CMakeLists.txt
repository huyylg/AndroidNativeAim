cmake_minimum_required(VERSION 3.18)
project(AndroidNativeAim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -mfpu=neon -march=armv8-a")

# Android NDK detection (for Termux)
if(NOT DEFINED ENV{ANDROID_NDK_HOME})
    set(ANDROID_NDK_HOME $ENV{HOME}/android-ndk-r25c)  # Adjust path
endif()
set(CMAKE_TOOLCHAIN_FILE ${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
set(TARGET_ARCH aarch64)
set(TARGET_PLATFORM android-26)

# Find packages
find_package(OpenCV REQUIRED PATHS $ENV{HOME}/opencv-mobile-4.8.0-android/sdk/native/jni)
find_package(ncnn REQUIRED PATHS $ENV{HOME}/ncnn/build-android-vulkan/install/lib/cmake/ncnn)

# Vulkan conditional
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    add_definitions(-DNCNN_VULKAN=1)
    message(STATUS "Vulkan enabled")
else()
    message(STATUS "Vulkan not found, using CPU")
endif()

# Include dirs
include_directories(${OpenCV_INCLUDE_DIRS} src/ $ENV{HOME}/ncnn/src)

# Sources
file(GLOB SOURCES "src/*.cpp" "src/capture/*.cpp" "src/ai/*.cpp" "src/aim/*.cpp" "src/input/*.cpp" "src/utils/*.cpp" "src/gui/*.cpp")

add_executable(aimbot ${SOURCES})
target_link_libraries(aimbot ${OpenCV_LIBS} ncnn)

# Install
install(TARGETS aimbot DESTINATION bin)

# Tests
enable_testing()
add_executable(test_capture tests/test_capture.cpp ${SOURCES})
target_link_libraries(test_capture ${OpenCV_LIBS} ncnn)
add_test(NAME CaptureTest COMMAND test_capture)
