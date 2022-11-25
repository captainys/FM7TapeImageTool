set(TARGET_NAME soundcrest)
add_executable(${TARGET_NAME}
	fsguiapp.cpp
	fsguiapp_draw.cpp
	fsguimain.cpp
	singleton.cpp
	filemenu.cpp
	viewmenu.cpp
	automenu.cpp
	analyzemenu.cpp
	editmenu.cpp
	filtermenu.cpp
	selectmenu.cpp
	fsguiapp.h
)
target_link_libraries(${TARGET_NAME} fslazywindow soundcrest_kernel soundcrest_kernelutil soundcrest_edit ysport ysgl ysglcpp ysglcpp_gl2 fsguilib fsguilib_gl2 fsguifiledialog t77lib)
