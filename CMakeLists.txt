cmake_minimum_required(VERSION 3.28)
project(DB2_project1)

set(CMAKE_CXX_STANDARD 17)

include_directories(dataset)
include_directories(indexes)

add_executable(DB2_project1
        main.cpp
        dataset/csv.hpp
        dataset/data.h
        indexes/Sequential_file.h
        indexes/ISAM.h)
