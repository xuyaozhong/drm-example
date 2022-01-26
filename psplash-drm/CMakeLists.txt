set(psplash "psplash-drm")

if($ENV{SDKTARGETSYSROOT} MATCHES "aarch64-agl-linux")
        SET(CMAKE_SYSROOT "$ENV{SDKTARGETSYSROOT}")
endif()


include_directories(
${CMAKE_SYSROOT}/usr/include/libdrm
)

link_directories(
${CMAKE_SYSROOT}/usr/lib64
)

MESSAGE(STATUS "cmake sysroot :${CMAKE_SYSROOT}")

aux_source_directory(. SOURCE_DIR)

add_executable(${psplash} psplash.c psplash-drm.c psplash-kms.c psplash-main.c psplash-draw.c psplash-scanout.c)
target_link_libraries(${psplash} libdrm.so.2)
