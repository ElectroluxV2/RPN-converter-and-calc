cmake_minimum_required(VERSION 3.17)
project(ONP)

set(CMAKE_CXX_STANDARD 20)

add_executable(ONP POP_2021-01-11_projekt_2_Budzisz_Mateusz_EiT_2_184325.cpp)

configure_file(input.txt input.txt COPYONLY)