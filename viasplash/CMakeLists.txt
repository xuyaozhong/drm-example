set(viasplash "viasplash")
set(example_decode "example_decode")

if($ENV{SDKTARGETSYSROOT} MATCHES "aarch64-agl-linux")
        SET(CMAKE_SYSROOT "$ENV{SDKTARGETSYSROOT}")
endif()

include_directories(
${CMAKE_SYSROOT}/usr/include/drm
)

link_directories(
${CMAKE_SYSROOT}/usr/lib64
)


MESSAGE(STATUS "cmake sysroot :${CMAKE_SYSROOT}")

aux_source_directory(. SOURCE_DIR)

add_definitions(-DLODEPNG_COMPILE_DISK)

add_executable(${viasplash} viasplash.c lodepng.c)
add_executable(${example_decode} example_decode.c lodepng.c)
target_link_libraries(${viasplash} libdrm.so.2)
