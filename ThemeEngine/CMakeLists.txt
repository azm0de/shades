# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(ThemeEngine)

# --- Detours Configuration ---
set(DETOURS_PATH "${CMAKE_SOURCE_DIR}/../libs/detours")
include_directories(${DETOURS_PATH}/include)

# --- JSON Configuration ---
include_directories(${CMAKE_SOURCE_DIR}/../libs)

# Use static runtime library
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# --- Project Build ---
add_library(ThemeEngine SHARED ThemeEngine.cpp)

# Link against the necessary Windows libraries
target_link_libraries(ThemeEngine PRIVATE
    user32 gdi32 uxtheme Comctl32
    "${DETOURS_PATH}/lib.x64/detours.lib"
)
