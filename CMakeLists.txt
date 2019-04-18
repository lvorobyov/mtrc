cmake_minimum_required(VERSION 3.13)

project(mtrc)



set(CMAKE_CXX_STANDARD 17)

find_package(BISON)
find_package(FLEX)

if (BISON_FOUND)
	BISON_TARGET(grammar grammar.y ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cpp)
	FLEX_TARGET(scan scan.l        ${CMAKE_CURRENT_BINARY_DIR}/scan.flex.cpp)
	ADD_FLEX_BISON_DEPENDENCY(scan grammar)
endif ()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SRCS mtrc-*.cpp)
aux_source_directory(nodes NODES)
add_executable(mtrc main.cpp 
	${SRCS} ${NODES} 
	${BISON_grammar_OUTPUTS} 
	${FLEX_scan_OUTPUTS})