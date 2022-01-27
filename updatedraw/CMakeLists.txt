#link_directories(
#${CMAKE_SYSROOT}/usr/lib64
#${PROJECT_BINARY_DIR}/lib
#)

set(updatedraw "updatedraw")

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

add_executable(${updatedraw} updatedraw.c drmtest.c)
target_link_libraries(${updatedraw} libdrm.so.2)
