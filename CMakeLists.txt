cmake_minimum_required(VERSION 3.10)
project(kumtdd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(INCLUDE_DIR ${SOURCE_DIR}/include)
set(SRC_DIR ${SOURCE_DIR}/src)

# Headers
include_directories(${INCLUDE_DIR})

# Source
set(SOURCES
    ${SOURCE_DIR}/main.cc
    ${SRC_DIR}/CellGrid.cc
    ${SRC_DIR}/SerialManager.cc
)

# Executable
add_executable(kumtdd ${SOURCES})

# Required libs (libwebsockets, pthread)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LWS REQUIRED libwebsockets)

target_include_directories(kumtdd PRIVATE ${LWS_INCLUDE_DIRS})
target_link_libraries(kumtdd PRIVATE ${LWS_LIBRARIES} pthread)
