cmake_minimum_required(VERSION 3.31)
project(HexagonOneSolver)

set(CMAKE_CXX_STANDARD 20)

add_executable(HexagonOneSolver main.cpp
        Puzzle.h
        Puzzle.cpp)
