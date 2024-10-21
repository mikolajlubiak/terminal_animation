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

# Get FTXUI
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
	GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
	GIT_TAG        v5.0.0
	GIT_PROGRESS   TRUE
	GIT_SHALLOW    TRUE
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

# Add binary
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE src PUBLIC ${OpenCV_INCLUDE_DIRS})

# Add libraries to link
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS}
	PRIVATE ftxui::screen
	PRIVATE ftxui::dom
	PRIVATE ftxui::component
)
