cmake_minimum_required(VERSION 3.10)
project(CrypMath)

set(CMAKE_CXX_STANDARD 17)

# Find libcurl
find_package(CURL REQUIRED)

# Add executable
add_executable(crypmath main.cpp)

# Link libraries
target_link_libraries(crypmath CURL::libcurl)
