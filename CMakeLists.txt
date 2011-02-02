cmake_minimum_required(VERSION 2.8)
project(m4api C)
set(m4api_VERSION_MAJOR 0)
set(m4api_VERSION_MINOR 1)
set(m4api_VERSION 0.1)
option(enable_maintainer "Enables the maintainer CFLAGS (-Wall -Werror)" OFF)
if(enable_maintainer)
	add_definitions(-Wall -Werror)
endif(enable_maintainer)
include(FindPkgConfig)
pkg_check_modules(lusb REQUIRED libusb)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/m4api.pc.in
	${CMAKE_BINARY_DIR}/m4api.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/m4api.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/m4api.h @ONLY)
include_directories(${lusb_INCLUDE_DIRS})
link_directories(${lusb_LIBRARY_DIRS})
set(library_sources m4api.c)
set(library_headers m4api.h)
add_executable(m4ctl m4ctl.c)
add_library(m4api SHARED ${library_sources})
add_library(m4api-static STATIC ${library_sources})
add_dependencies(m4ctl m4api)
set_target_properties(m4api-static PROPERTIES OUTPUT_NAME m4api
	CLEAN_DIRECT_OUTPUT 1)
set_target_properties(m4api PROPERTIES CLEAN_DIRECT_OUTPUT 1 SOVERSION ${m4api_VERSION})
set_property(TARGET m4api PROPERTY PUBLIC_HEADER ${library_headers})
set_property(TARGET m4ctl PROPERTY COMPILE_FLAGS ${lusb_CFLAGS_OTHER})
set_property(TARGET m4ctl PROPERTY LINK_FLAGS ${lusb_LDFLAGS_OTHER})
target_link_libraries(m4ctl m4api)
target_link_libraries(m4api ${lusb_LDFLAGS})
target_link_libraries(m4api-static ${lusb_LDFLAGS})
install(TARGETS m4ctl m4api m4api-static
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include/m4api)
install(FILES ${CMAKE_BINARY_DIR}/m4api.pc DESTINATION lib/pkgconfig)
