cmake_minimum_required(VERSION 3.16)

project(regex-tool LANGUAGES CXX)

if (MSVC)
  # 必须，rust 始终链接到 release dll
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  set(CMAKE_WIN32_EXECUTABLE true)
endif()

add_subdirectory(regex_engine)
add_subdirectory(regex_tool)
