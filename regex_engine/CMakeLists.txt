find_program(CARGO cargo REQUIRED PATHS $ENV{HOME}/.cargo/bin)

add_custom_target(regex_engine_target ALL
  COMMAND ${CARGO} build --release
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "runing cargo build"
)
