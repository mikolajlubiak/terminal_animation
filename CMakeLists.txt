cmake_minimum_required(VERSION 3.14)
project(terminal_animation VERSION 5.0.2 LANGUAGES C CXX)

include(CheckCXXCompilerFlag)
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

function(enable_cxx_compiler_flag_if_supported flag)
  string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
  if(flag_already_set EQUAL -1)
    check_cxx_compiler_flag("${flag}" flag_supported)
    if(flag_supported)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    endif()
    unset(flag_supported CACHE)
  endif()
endfunction()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files (explicit list for reliable rebuilds)
set(SOURCES
  src/main.cpp
  src/animation_ui.cpp
  src/common.cpp
  src/media_to_ascii.cpp
)

set(HEADERS
  src/animation_ui.hpp
  src/common.hpp
  src/media_to_ascii.hpp
  src/slider_with_callback.hpp
)

find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
  PRIVATE src
  PRIVATE ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE ${OpenCV_LIBS}
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE spdlog::spdlog
)

# --- Tests ---
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
  enable_testing()

  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(googletest)

  add_executable(common_test
    tests/common_test.cpp
    src/common.cpp
  )

  target_include_directories(common_test
    PRIVATE src
  )

  target_link_libraries(common_test
    PRIVATE GTest::gtest_main
  )

  include(GoogleTest)
  gtest_discover_tests(common_test)
endif()
