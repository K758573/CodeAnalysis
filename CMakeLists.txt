cmake_minimum_required(VERSION 3.25)
project(CodeAnalysis)
set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.3/msvc2019_64/bin")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        )

add_executable(main main.cpp)

target_link_libraries(main
        PRIVATE
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
        )
#target_link_libraries(main
#        PRIVATE
#        Qt::Gui Qt::Core Qt::Widgets
#        )
add_subdirectory(src)

target_link_libraries(main
        PRIVATE
        utils
        )