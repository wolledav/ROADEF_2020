cmake_minimum_required(VERSION 3.10)
project(ROADEF)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -fopenmp")

add_executable(challengeRTE src/main.cpp src/util.cpp src/instance/hashing.cpp src/parser/parser.cpp src/instance/instance.cpp src/solution/solution.cpp src/constructions/constructions.cpp src/solution/objective.cpp src/params.cpp src/insertions/insertions.cpp src/removals/removals.cpp src/alns/alns.cpp src/local_search/local_search.cpp)

