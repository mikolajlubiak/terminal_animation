cmake_minimum_required(VERSION 3.5.0)
project(terminal_animations VERSION 0.2.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Sources
file(GLOB_RECURSE SOURCES
	src/*.cpp
)

# Headers
file(GLOB_RECURSE HEADERS
	src/*.hpp
)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add binary
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE src PUBLIC ${OpenCV_INCLUDE_DIRS})

# Add libraries to link
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})
