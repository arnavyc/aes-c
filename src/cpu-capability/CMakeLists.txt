include(GNUInstallDirs)

add_library(cpu-capability STATIC src/cpu-capability.c)
target_include_directories(
  cpu-capability PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(
  cpu-capability
  PROPERTIES PUBLIC_HEADER
             ${CMAKE_CURRENT_SOURCE_DIR}/include/ay/cpu-capability.h
)
