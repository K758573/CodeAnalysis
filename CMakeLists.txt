cmake_minimum_required(VERSION 3.25)
project(CodeAnalysis)
set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.3/msvc2019_64/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Sql
        )

add_executable(main WIN32 main.cpp ReportGenerator.cpp mainwindow.cpp RiskFunctionDb.cpp SyntaxHighLighter.cpp finderwindow.cpp AstParser.cpp)
add_executable(EncryptDatabase encrypt.cpp)
target_link_libraries(EncryptDatabase PUBLIC Qt::Core utils)
target_link_libraries(main
        PUBLIC
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
        )
target_link_libraries(main
        PUBLIC
        Qt::Gui Qt::Core Qt::Widgets Qt::Sql
        )
add_subdirectory(src)
target_link_libraries(main
        PUBLIC
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

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif ()