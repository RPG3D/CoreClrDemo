cmake_minimum_required(VERSION 3.10)

project(clrdemo)

enable_language(C ASM)

if(ANDROID_NDK_MAJOR VERSION_LESS "23")
    message(FATAL_ERROR "Need at least Android NDK 23, got ${ANDROID_NDK_REVISION}!")
endif()

add_compile_options(-Wall -Wno-missing-prototypes -Wno-missing-declarations -std=c99)

add_library(
    clrdemo
    SHARED
    ${CMAKE_SOURCE_DIR}/clrdemo.c
)

# coreclrhost.h from the .NET runtime source tree
include_directories(
    "${DOTNET_RUNTIME_SRC}/src/coreclr/hosts/inc"
)

target_link_libraries(
    clrdemo
    PRIVATE
    log
    z
    ${DOTNET_PUBLISH_DIR}/libcoreclr.so
)
