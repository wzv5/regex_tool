cmake_minimum_required(VERSION 3.16)

project(regex-tool LANGUAGES CXX)

if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  set(CMAKE_WIN32_EXECUTABLE true)
endif()

add_subdirectory(regex_engine)
add_subdirectory(regex_tool)
