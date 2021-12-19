find_package(Qt6 COMPONENTS Widgets QUIET)
if (NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(regex-tool
  main.cpp
  mainwindow.cpp
  mainwindow.h
  csv.hpp
)

target_precompile_headers(regex-tool PRIVATE pch.h)

target_link_libraries(regex-tool PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  bridge
)
