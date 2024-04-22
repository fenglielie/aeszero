add_subdirectory(lib-aes0)

zero_get_files(SRCS .)
add_executable(aeszero ${SRCS})
target_link_libraries(aeszero PRIVATE aes0)
zero_target_preset_definitions(aeszero)
zero_target_info(aeszero)
