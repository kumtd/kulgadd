################################################################################
#
#   CMakeLists.txt for ktmtdd.
#
#   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
#            Kyungmin Lee (  railroad@korea.ac.kr)
#            Changi Jeong (  jchg3876@korea.ac.kr)
#
################################################################################



#-------------------------------------------------------------------------------
#   Setup the project
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)
project(kumtdd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



#-------------------------------------------------------------------------------
# Required libs (libwebsockets, pthread)
#-------------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(LWS REQUIRED libwebsockets)



#-------------------------------------------------------------------------------
#   Setup include directory for this project
#-------------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)



#-------------------------------------------------------------------------------
#   Locate sources and headers for this project
#   NB: headers are included so they will show up in IDEs
#-------------------------------------------------------------------------------
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)



#-------------------------------------------------------------------------------
#   Add the executable
#-------------------------------------------------------------------------------
add_executable(kumtdd main.cc ${sources} ${headers})
target_include_directories(kumtdd PRIVATE ${LWS_INCLUDE_DIRS})
target_link_libraries(kumtdd PRIVATE ${LWS_LIBRARIES} pthread)
