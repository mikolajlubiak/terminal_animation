cmake_minimum_required(VERSION 3.5.0)
project(terminal_animation VERSION 3.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Gather source and header files
file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file (GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.hpp")

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

# Check if OpenCV is installed
find_package(OpenCV REQUIRED)

# Get FTXUI
FetchContent_Declare(ftxui
	GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
	GIT_TAG        v5.0.0
	GIT_PROGRESS   TRUE
	GIT_SHALLOW    TRUE
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

# Add binary
add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE src PRIVATE ${OpenCV_INCLUDE_DIRS})

# Add libraries to link
target_link_libraries(${PROJECT_NAME}
	PRIVATE ${OpenCV_LIBS}
	PRIVATE ftxui::screen
	PRIVATE ftxui::dom
	PRIVATE ftxui::component
)
