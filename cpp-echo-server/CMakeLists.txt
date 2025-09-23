cmake_minimum_required(VERSION 3.10)

project(cpp-echo-server)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#add_executable(echo_server src/server.cpp)
#add_executable(echo_server_fork src/server_fork.cpp)
#add_executable(echo_server_epoll src/server_epoll.cpp)
#add_executable(echo_client src/client.cpp)
add_executable(server_private_epoll src/server_private_epoll.cpp)
add_executable(client_threaded src/client_threaded.cpp)
