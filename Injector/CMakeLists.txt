# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Injector)

# Add the source file and define it as an executable
add_executable(Injector Injector.cpp)

# Link against the necessary Windows libraries
target_link_libraries(Injector PRIVATE user32)
