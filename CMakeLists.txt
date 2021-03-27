cmake_minimum_required(VERSION 2.8)
project(zcserver)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O1 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(D:/boost_1_75_0)
include_directories(D:/YAML_CPP/include)

link_directories(D:/YAML_CPP/lib)

# find_library(YAMLCPP libyaml-cpp.a)

set(LIB_SRC
    src/log.cpp
    src/util.cpp
    src/config.cpp
)

add_library(zcserver ${LIB_SRC})
# add_library(zcserver_static STATIC ${LIB_SRC})
# SET_TARGET_PROPERTIES (zcserver_static PROPERTIES OUTPUT_NAME "zcserver")

add_executable(test tests/test.cpp)
add_dependencies(test zcserver)
target_link_libraries(test zcserver)

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config zcserver)
target_link_libraries(test_config zcserver -LD:/YAML_CPP/lib -lyaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
