cmake_minimum_required(VERSION 2.8)

file (GLOB_RECURSE HEADERS "*.[h,hpp]")

add_definitions(-std=c++11)
add_definitions(-pthread)

 ${HEADERS})
add_executable(Main main.cpp Epoll.cpp GameServer.cpp MessageCenter.cpp Player.cpp Server.cpp SoloBattle.cpp SoloMessage.cpp)






g++ -g -pthread -lprotobuf  -Wall -std=c++11  main.cpp Epoll.cpp Player.cpp Server.cpp ServerMessage.cpp ServerMessageProto.pb.cc SoloBattle.cpp SoloMessage.cpp -o RunServer





