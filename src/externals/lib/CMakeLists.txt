set(TARGET_NAME fm7lib)

set(SRCS
fm7lib.cpp
cpplib.cpp
)
set(HEADERS
fm7lib.h
cpplib.h
)

if(NOT YS_PUBLIC_LIBRARIES)
add_subdirectory(yscomport)
endif()

add_library(${TARGET_NAME} ${SRCS} ${HEADERS})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
