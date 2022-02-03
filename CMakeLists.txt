cmake_minimum_required(VERSION 3.4)
project(cppcoin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -pedantic -Werror)

include(${CMAKE_SOURCE_DIR}/cmake/StaticAnalysis.cmake)
enable_clang_tidy()
enable_cppcheck()

find_package(OpenSSL REQUIRED)

add_executable(cppcoin src/blockchain.cpp)
target_link_libraries(cppcoin PRIVATE ${OPENSSL_LIBRARIES})
