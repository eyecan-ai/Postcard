cmake_minimum_required(VERSION 2.8)
project( PostcardCppClient )

add_definitions(-std=c++11)

find_package( OpenCV REQUIRED )

add_executable( client_example client_example.cpp )
target_link_libraries( client_example ${OpenCV_LIBS} )

add_executable( client_gevis_example client_gevis_example.cpp )
target_link_libraries( client_gevis_example ${OpenCV_LIBS} )

add_executable( client_gevis_fullresolution client_gevis_fullresolution.cpp )
target_link_libraries( client_gevis_fullresolution ${OpenCV_LIBS} )