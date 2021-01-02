set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  # 必须，rust 始终链接到 release dll
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

if (WIN32)
  set(CMAKE_WIN32_EXECUTABLE true)
endif()

find_package(Qt6 COMPONENTS Widgets)
if (NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

find_program(CXXBRIDGE cxxbridge REQUIRED PATHS $ENV{HOME}/.cargo/bin)
message(STATUS "Using cxxbridge: ${CXXBRIDGE}")
set(cxxbridge_in ${CMAKE_SOURCE_DIR}/regex_engine/src/cppbridge.rs)
set(cxxbridge_source_out ${CMAKE_SOURCE_DIR}/qt1/bridge.cpp)
set(cxxbridge_header_out ${CMAKE_SOURCE_DIR}/qt1/bridge.h)

add_custom_command(
  OUTPUT ${cxxbridge_source_out} ${cxxbridge_header_out}
  COMMAND ${CXXBRIDGE} ${cxxbridge_in} ${cxxbridge_args} --header --output ${cxxbridge_header_out}
  COMMAND ${CXXBRIDGE} ${cxxbridge_in} ${cxxbridge_args} --output ${cxxbridge_source_out}
  COMMENT "Generating CXX Bridge: ${cxxbridge_header_out} ${cxxbridge_source_out}"
  DEPENDS ${cxxbridge_in}
)

add_executable(qt1
  main.cpp
  mainwindow.cpp
  mainwindow.h
  bridge.cpp
  bridge.h
  csv.hpp
)

target_precompile_headers(qt1 PRIVATE stdafx.h)

set_source_files_properties(bridge.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS true)

if (MSVC)
  set(REGEX_ENGINE_LIB "${CMAKE_SOURCE_DIR}/regex_engine/target/release/regex_engine.lib")
else()
  set(REGEX_ENGINE_LIB "${CMAKE_SOURCE_DIR}/regex_engine/target/release/libregex_engine.a")
endif()

if (WIN32)
  set(RUST_DEPEND ws2_32 userenv advapi32)
else()
  set(RUST_DEPEND pthread m dl)
endif()

target_link_libraries(qt1 PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  ${REGEX_ENGINE_LIB}
  ${RUST_DEPEND}
)
add_dependencies(qt1 regex_engine_target)
