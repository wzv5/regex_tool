find_program(CARGO cargo REQUIRED PATHS $ENV{HOME}/.cargo/bin)
message(STATUS "Using cargo: ${CARGO}")

find_program(CXXBRIDGE cxxbridge REQUIRED PATHS $ENV{HOME}/.cargo/bin)
message(STATUS "Using cxxbridge: ${CXXBRIDGE}")

add_custom_target(regex_engine_target ALL
  COMMAND ${CARGO} build --release --target-dir ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Runing cargo build"
)

set(cxxbridge_in ${CMAKE_CURRENT_SOURCE_DIR}/src/cppbridge.rs)
set(cxxbridge_source_out ${CMAKE_CURRENT_BINARY_DIR}/bridge.cpp)
set(cxxbridge_header_out ${CMAKE_CURRENT_BINARY_DIR}/bridge.h)

add_custom_command(
  OUTPUT ${cxxbridge_source_out} ${cxxbridge_header_out}
  COMMAND ${CXXBRIDGE} ${cxxbridge_in} ${cxxbridge_args} --header --output ${cxxbridge_header_out}
  COMMAND ${CXXBRIDGE} ${cxxbridge_in} ${cxxbridge_args} --output ${cxxbridge_source_out}
  COMMENT "Generating CXX Bridge: ${cxxbridge_header_out} ${cxxbridge_source_out}"
  DEPENDS ${cxxbridge_in}
)

if (WIN32)
  set(RUST_DEPEND ws2_32 userenv advapi32 bcrypt)
else()
  set(RUST_DEPEND pthread m dl)
endif()

add_library(bridge STATIC ${cxxbridge_source_out})
add_dependencies(bridge regex_engine_target)
target_link_directories(bridge PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/release)
target_include_directories(bridge PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(bridge PUBLIC regex_engine ${RUST_DEPEND})
