cmake_minimum_required (VERSION 2.8.0 FATAL_ERROR)
project (pld9bc_hw2)

set(CMAKE_COLOR_MAKEFILE ON)

set(SPEEDY_FLAGS "-march=native -O3 -m64 -DNDEBUG -mveclibabi=svml -mfpmath=sse -msse4.2 -flto -ftracer -ffast-math -fmerge-all-constants -fno-signaling-nans -fno-unsafe-math-optimizations -fivopts -fipa-matrix-reorg -fvect-cost-model -floop-flatten -funroll-loops -floop-strip-mine -floop-parallelize-all -floop-block -ftree-vectorize -ftree-loop-optimize -ftree-loop-if-convert -ftree-loop-if-convert-stores -ftree-loop-distribution -ftree-loop-distribute-patterns -faggressive-loop-optimizations -funsafe-loop-optimizations
-fvariable-expansion-in-unroller -fbranch-target-load-optimize --param omega-eliminate-redundant-constraints=1 ")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror -g")

add_library(${PROJECT_NAME} SHARED src/util.cpp)

add_executable(${PROJECT_NAME}_main src/hw2.cpp)

add_executable(test test/test.cpp)
