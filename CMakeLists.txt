cmake_minimum_required(VERSION 3.28)
project(DB2_project1)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
include_directories(dataset)

add_executable(DB2_project1
        dataset/csv.hpp
        dataset/data.h
        main.cpp
        Sequential_file.h
        indexes/ISAM.h)
