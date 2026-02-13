# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(SHADES)

# Add the source file and define it as an executable
add_executable(SHADES Injector.cpp)

# Set as Windows GUI application (not console)
set_target_properties(SHADES PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "SHADES"
)

# Link against the necessary Windows libraries
target_link_libraries(SHADES PRIVATE user32 shell32)
