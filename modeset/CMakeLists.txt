#link_directories(
#${CMAKE_SYSROOT}/usr/lib64
#${PROJECT_BINARY_DIR}/lib
#)

set(modeset "modeset")
set(modeset-double-buffered "modeset-double-buffered")
set(modeset-double-buffered-vsync "modeset-double-buffered-vsync")

if($ENV{SDKTARGETSYSROOT} MATCHES "aarch64-agl-linux")
        SET(CMAKE_SYSROOT "$ENV{SDKTARGETSYSROOT}")
#else()
#        SET(CMAKE_SYSROOT ${CMAKE_CURRENT_SOURCE_DIR}/recipe-sysroot/)
endif()


include_directories(
${CMAKE_SYSROOT}/usr/include/drm
)

link_directories(
${CMAKE_SYSROOT}/usr/lib64
)

MESSAGE(STATUS "cmake sysroot :${CMAKE_SYSROOT}")

aux_source_directory(. SOURCE_DIR)

add_executable(${modeset} modeset.c)
add_executable(${modeset-double-buffered} modeset-double-buffered.c)
add_executable(${modeset-double-buffered-vsync} modeset-double-buffered-vsync.c)
target_link_libraries(${modeset} libdrm.so.2)
target_link_libraries(${modeset-double-buffered} libdrm.so.2)
target_link_libraries(${modeset-double-buffered-vsync} libdrm.so.2)
