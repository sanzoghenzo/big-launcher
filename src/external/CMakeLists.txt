add_library(inih ini.c)
target_compile_definitions(inih PRIVATE INI_MAX_LINE=1027)
