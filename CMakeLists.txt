cmake_minimum_required(VERSION 3.25)
project(CodeAnalysis)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(src)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)

add_executable(main main.cpp)

target_link_libraries(main
        PRIVATE
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
        utils)
