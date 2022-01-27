set(viasplash "viasplash")

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

add_executable(${viasplash} viasplash.c)
target_link_libraries(${viasplash} libdrm.so.2)
