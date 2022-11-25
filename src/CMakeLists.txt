cmake_minimum_required(VERSION 3.2)
project(FM7TapeImageTool)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../public)
	set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../public/src)
else()
	set(PUBLIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../public/src)
endif()

add_subdirectory(${PUBLIC_SOURCE_DIR} ${CMAKE_BINARY_DIR}/public)
add_subdirectory(kernel)
add_subdirectory(kernelutil)
add_subdirectory(waveedit)
add_subdirectory(gui)
add_subdirectory(externals/t77)
add_subdirectory(externals/lib)
