cmake_minimum_required(VERSION 3.10)
project(mapped_queue)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(example main.cpp)
if(WIN32)
else()
  target_link_libraries(example pthread)
endif()

enable_testing()
add_subdirectory(tests)