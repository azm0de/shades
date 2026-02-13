# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Injector)

# Add the source file and define it as an executable
add_executable(Injector Injector.cpp)

# Set as Windows GUI application (not console)
set_target_properties(Injector PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Link against the necessary Windows libraries
target_link_libraries(Injector PRIVATE user32 shell32)
