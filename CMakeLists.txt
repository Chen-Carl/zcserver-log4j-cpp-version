# g++ src/config.cpp src/log.cpp src/util.cpp tests/test.cpp -std=c++11 -g -I ../zoe/boost_1_76_0 -I ../zoe/yaml-cpp/include -lyaml-cpp -o bin/test
# g++ src/config.cpp src/log.cpp src/util.cpp tests/test_config.cpp -g -I ../zoe/boost_1_76_0 -I ../zoe/yaml-cpp/include -L ../zoe/yaml-cpp/lib -lyaml-cpp -o bin/test_config
# g++ src/config.cpp src/log.cpp src/util.cpp src/thread.cpp tests/test_thread.cpp -g -I ../zoe/boost_1_76_0 -I ../zoe/yaml-cpp/include -L ../zoe/yaml-cpp/lib -lyaml-cpp -lpthread -o bin/test_thread

# g++ -shared -fpic -o lib/libzcserver.so src/config.cpp src/log.cpp src/util.cpp -I../zoe/boost_1_76_0 -I../zoe/yaml-cpp/include -std=c++11

# ar rcs lib/libzcserver.a src/config.cpp src/log.cpp src/util.cpp
# g++ -static -o bin/test tests/test.cpp ./lib/libzcserver.a -lyaml-cpp

cmake_minimum_required(VERSION 3.0)
project(zcserver)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(../zoe/boost_1_76_0)
include_directories(../zoe/yaml-cpp/include)

set(LIB_SRC
    src/log.cpp
    src/util.cpp
    src/config.cpp
    src/thread.cpp
)


add_library(zcserver SHARED ${LIB_SRC})
# add_library(zcserver_static STATIC ${LIB_SRC})
# SET_TARGET_PROPERTIES (zcserver_static PROPERTIES OUTPUT_NAME "zcserver")

set(LIBS
    zcserver
    pthread
    yaml-cpp
)

add_executable(test tests/test.cpp)
add_dependencies(test zcserver)
target_link_libraries(test ${LIBS})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config zcserver)
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread zcserver)
target_link_libraries(test_thread ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
