cmake_minimum_required(VERSION 3.16)
project(EPPGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(epp-gui src/main.cpp)
install(TARGETS epp-gui RUNTIME DESTINATION bin)

enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_gui.cpp")
	add_executable(epp_gui_tests tests/test_gui.cpp)
	target_compile_definitions(epp_gui_tests PRIVATE EPP_GUI_NO_MAIN)
	add_test(NAME epp_gui_tests COMMAND epp_gui_tests)
endif()
