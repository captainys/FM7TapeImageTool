set(TARGET_NAME soundcrest_edit)
add_library(${TARGET_NAME} 
	yswaveedit.cpp
	yswaveedit_analyze.cpp
	yswaveedit_edit.cpp
	yswaveedit_filter.cpp
	yswaveedit_fm7.cpp
	yswaveedit.h
)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} soundcrest_kernel soundcrest_kernelutil)
