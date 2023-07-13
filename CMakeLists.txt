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

add_executable(main main.cpp mainwindow.cpp SyntaxHighLighter.cpp finderwindow.cpp AstParser.cpp)

target_link_libraries(main
        PRIVATE
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
        )
target_link_libraries(main
        PRIVATE
        Qt::Gui Qt::Core Qt::Widgets
        )
add_subdirectory(src)
target_link_libraries(main
        PRIVATE
        utils
        )
function(define_file_basename target)
    get_target_property(source_files ${target} SOURCES)
    foreach (sourcefile ${source_files})
        get_property(defs SOURCE ${sourcefile} PROPERTY COMPILE_DEFINITIONS)
        get_filename_component(basename ${sourcefile} NAME)
        list(APPEND defs "__FILENAME__=\"${basename}\"")
        set_property(SOURCE ${sourcefile} PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach ()
endfunction()
define_file_basename(main)