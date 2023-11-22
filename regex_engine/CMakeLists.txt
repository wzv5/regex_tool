find_program(CARGO cargo REQUIRED PATHS $ENV{HOME}/.cargo/bin)
message(STATUS "Using cargo: ${CARGO}")

set(CARGO_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/target)
set(CARGO_PROFILE $<IF:$<CONFIG:Debug>,debug,release>)
set(CXXBRIDGE_DIR ${CARGO_TARGET_DIR}/cxxbridge/regex_engine/src)
set(CXXBRIDGE_CC ${CXXBRIDGE_DIR}/cppbridge.rs.cc)

if (MSVC)
  set(CARGO_BUILD_ENV ${CMAKE_COMMAND} -E env CXXFLAGS=/MD$<$<CONFIG:Debug>:d> --)
endif()

add_custom_target(cargo_build_target
  COMMAND ${CARGO_BUILD_ENV} ${CARGO} build $<$<NOT:$<CONFIG:Debug>>:--release> --target-dir ${CARGO_TARGET_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  BYPRODUCTS ${CXXBRIDGE_CC}
)

if (WIN32)
  set(RUST_DEPEND ntdll ws2_32 userenv advapi32 bcrypt)
else()
  set(RUST_DEPEND pthread m dl)
endif()

add_library(bridge STATIC ${CXXBRIDGE_CC})
target_link_directories(bridge PUBLIC ${CARGO_TARGET_DIR}/${CARGO_PROFILE})
target_include_directories(bridge PUBLIC ${CXXBRIDGE_DIR})
target_link_libraries(bridge PUBLIC regex_engine ${RUST_DEPEND})
set_target_properties(bridge PROPERTIES ADDITIONAL_CLEAN_FILES ${CARGO_TARGET_DIR})
