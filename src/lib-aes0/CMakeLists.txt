zero_get_files(SRCS .)
add_library(aes0 STATIC)
target_sources(aes0 PRIVATE ${SRCS})
target_include_directories(aes0 PUBLIC .)

zero_target_preset_definitions(aes0)
zero_target_info(aes0)
