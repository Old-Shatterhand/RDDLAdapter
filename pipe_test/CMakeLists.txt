cmake_minimum_required(VERSION 2.8)
project(rddlparser)

set(CMAKE_CXX_STANDARD 14)

add_executable(test_parser
	main.cpp
	pstream.cpp
	pstream.h
)

