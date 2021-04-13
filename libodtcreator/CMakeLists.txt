cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the required modules
find_package(Qt5 COMPONENTS Core Gui REQUIRED)

find_package( ZLIB REQUIRED )
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif(ZLIB_FOUND)

set( SRC
    zip.cpp
    odtwriter.cpp
)

set( HDR
    zipreader.h
    zipwriter.h
    odtwriter.h
)

# Tell CMake to create the library
add_library( odtcreator
    ${SRC} ${HDR}
)

# Use required modules from Qt 5
target_link_libraries(odtcreator Qt5::Core Qt5::Gui ${ZLIB_LIBRARIES})
