zero_get_files(SRCS .)
add_executable(aeszero ${SRCS})
target_link_libraries(aeszero PRIVATE aes0)
target_compile_definitions(aeszero PRIVATE -DVERSION_INFO="${PROJECT_VERSION}")

zero_target_preset_definitions(aeszero)
zero_target_info(aeszero)
