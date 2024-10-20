cmake_minimum_required(VERSION 3.5.0)
project(terminal_animations VERSION 0.1.0 LANGUAGES C CXX)

# Sources
file(GLOB_RECURSE SOURCES
	src/*.cpp
)

file(GLOB_RECURSE HEADERS
	src/*.h
)

# Add binary
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE src)
