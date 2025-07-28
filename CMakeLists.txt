################################################################################
#
#   CMakeLists.txt for kulgadd.
#
#   Authors: Hoyong Jeong (hoyong5419@korea.ac.kr)
#            Kyungmin Lee (  railroad@korea.ac.kr)
#            Changi Jeong (  jchg3876@korea.ac.kr)
#
################################################################################



#-------------------------------------------------------------------------------
# Setup the project
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)
project(kulgadd VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



#-------------------------------------------------------------------------------
# Required libs (libwebsockets, pthread)
#-------------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(LWS REQUIRED libwebsockets)



#-------------------------------------------------------------------------------
# Prefix
#-------------------------------------------------------------------------------
get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
set(CMAKE_INSTALL_PREFIX ${PARENT_DIR})



#-------------------------------------------------------------------------------
# Setup include directory for this project
#-------------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)



#-------------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#-------------------------------------------------------------------------------
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)



#-------------------------------------------------------------------------------
# Add the executable
#-------------------------------------------------------------------------------
add_executable(kulgadd main.cc ${sources} ${headers})
target_include_directories(kulgadd PRIVATE ${LWS_INCLUDE_DIRS})
target_link_libraries(kulgadd PRIVATE ${LWS_LIBRARIES} pthread)



#-------------------------------------------------------------------------------
# Configure installation
#-------------------------------------------------------------------------------
install(TARGETS kulgadd DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)



#-------------------------------------------------------------------------------
# Configure systemd service unit file
#-------------------------------------------------------------------------------
set(EXEC_START_PATH "${CMAKE_INSTALL_PREFIX}/bin/kulgadd")
configure_file(${CMAKE_SOURCE_DIR}/kulgadd.service.in
               ${CMAKE_BINARY_DIR}/kulgadd.service @ONLY)



#-------------------------------------------------------------------------------
# Copy the service unit file to /etc/systemd/system
#-------------------------------------------------------------------------------
install(FILES ${CMAKE_BINARY_DIR}/kulgadd.service DESTINATION /etc/systemd/system)
