cmake_minimum_required(VERSION 2.8.11)

OPTION (BUILD_SHARED_LIBS "Build shared libraries." ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)
# Instruct CMake to run rcc automatically when needed.
set(CMAKE_AUTORCC ON)

project(qcost)

add_subdirectory( libmathparser )
add_subdirectory( libodtcreator )
add_subdirectory( libqcost )
add_subdirectory( qcostgui )
