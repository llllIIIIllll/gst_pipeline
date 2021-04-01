cmake_minimum_required(VERSION 3.5)
project(gst_pipeline)

# Default to C99
if(NOT CMAKE_C_STANDARD)
	set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Wall -Wextra -O2) #-Wpedantic
endif()

# add host cpu option
configure_file(
	"${PROJECT_SOURCE_DIR}/host_cpu.h.in"
	"${PROJECT_BINARY_DIR}/host_cpu.h"
)

#####################
# find dependencies #
#####################
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

#################
# include files #
#################
include_directories(
	include
	${GSTREAMER_INCLUDE_DIRS}
	${PROJECT_BINARY_DIR}
	/usr/include/gstreamer-1.0/
)
# set(DS_LIB /opt/nvidia/deepstream/deepstream-4.0/lib)
# link_libraries(
# 		/opt/nvidia/deepstream/deepstream-4.0/lib/libnvdsgst_meta.so
# 		/opt/nvidia/deepstream/deepstream-4.0/lib/libnvds_meta.so
# )

add_executable(main
    test/main.cpp
	src/receiver.cpp
	src/general_receiver.cpp
)

# target_link_libraries(socket_repeater
#     Threads::Threads
#     ${Boost_LIBRARIES}
# )

target_link_libraries(main
	${GSTREAMER_LIBRARIES} 
	${OpenCV_LIBS}
	pthread
	gstapp-1.0
)


