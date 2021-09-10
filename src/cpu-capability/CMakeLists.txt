add_library(cpu-capability STATIC cpu-capability.c)
target_include_directories(cpu-capability PUBLIC .)
set_target_properties(cpu-capability PROPERTIES PUBLIC_HEADER cpu-capability.h)
