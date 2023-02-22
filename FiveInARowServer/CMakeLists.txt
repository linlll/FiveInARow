cmake_minimum_required(VERSION 3.10.2)

project(SERVER)

aux_source_directory(. DIR_SRCS)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

add_executable(server server.cpp)
# add_executable(client client.cpp)

