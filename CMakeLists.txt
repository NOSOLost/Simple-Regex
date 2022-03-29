cmake_minimum_required(VERSION 3.4.1)
project(pcc VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(BOOST_INCLUDE ) #boost library
set(PCC_INCLUDE ./input ./lexer ./pcc_header ./demo)
set(INCLUDE_LISTS ${PCC_INCLUDE} ${BOOST_INCLUDE})
include_directories(${INCLUDE_LISTS})

set(SRC_LIST ./demo/demo.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})