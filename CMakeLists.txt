cmake_minimum_required(VERSION 3.25)
project(CodeAnalysis)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
add_definitions(${LLVM_DEFINITIONS})
# Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(llvm_libs Support Core IRReader)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${LLVM_INCLUDE_DIRS})
add_subdirectory(src)
# Link against LLVM libraries
target_link_libraries(main PRIVATE ${llvm_libs} libclang utils)
