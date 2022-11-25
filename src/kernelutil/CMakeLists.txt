set(TARGET_NAME soundcrest_kernelutil)
add_library(${TARGET_NAME} 
	yswave_peakutil.cpp
	yswave_envelopeutil.cpp
	yswave_medianfilter.cpp
	yswave_waveutil.cpp
	yswave_fm7util.cpp
	yswave_peakutil.h
	yswave_envelopeutil.h
	yswave_medianfilter.h
	yswave_waveutil.h
	yswave_fm7util.h)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} yssimplesound ysclass ysport soundcrest_kernel)
