set(TARGET_NAME soundcrest_kernel)
add_library(${TARGET_NAME} yswavekernel.cpp yswavekernel.h)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} yssimplesound ysclass ysport)
