cmake_minimum_required(VERSION 3.12)
project(TEST_HYDRA_RTSP VERSION 1.0)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)

find_package(Boost 1.84.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_libraries(${Boost_LIBRARIES})
endif()

# Gstreamer
set(GSTREAMER_LIB "/usr/lib/x86_64-linux-gnu")
set(GSTREAMER_INC "/usr/include/gstreamer-1.0")

link_directories(${GSTREAMER_LIB})
include_directories(${GSTREAMER_INC})

# GLIB
set(GLIB_INC_1 "/usr/include/glib-2.0")
set(GLIB_INC_2 "/usr/lib/x86_64-linux-gnu/glib-2.0/include/")

include_directories(${GLIB_INC_1})
include_directories(${GLIB_INC_2})

include_directories("/usr/include")

include_directories("${CMAKE_SOURCE_DIR}/include")

link_directories("/usr/lib/x86_64-linux-gnu/")


add_subdirectory(src)