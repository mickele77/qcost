cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the required modules
find_package(Qt5 COMPONENTS Core REQUIRED)

set( SRC
    mathparser.cpp
)

set( HDR
    mathparser.h
)

# Tell CMake to create the library
add_library( libmathparser
    ${SRC} ${HDR}
)

# Use required modules from Qt 5
target_link_libraries(libmathparser Qt5::Core)
