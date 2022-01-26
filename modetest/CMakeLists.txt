#link_directories(
#${CMAKE_SYSROOT}/usr/lib64
#${PROJECT_BINARY_DIR}/lib
#)

set(modetest "modetest")

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

add_executable(${modetest} modetest.c cursor.c buffers.c)
target_link_libraries(${modetest} libdrm.so.2)
