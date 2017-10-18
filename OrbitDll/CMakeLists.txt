get_filename_component(EXTERN_ROOT ${CMAKE_SOURCE_DIR}/external REALPATH)

add_library(OrbitDll SHARED
    Hijacking.cpp
    OrbitDll.cpp
    OrbitLib.cpp
    TcpClient.cpp
)

set(MINHOOK_ROOT ${EXTERN_ROOT}/minhook)
set(BREAKPAD_ROOT ${EXTERN_ROOT}/breakpad)
set(BREAKPAD_LIBROOT_RELEASE ${BREAKPAD_ROOT}/src/client/windows/${CMAKE_VS_PLATFORM_NAME}/Release/lib)
set(BREAKPAD_LIBROOT_DEBUG ${BREAKPAD_ROOT}/src/client/windows/${CMAKE_VS_PLATFORM_NAME}/Debug/lib)

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(BREAKPAD_LIBS_RELEASE ${BREAKPAD_LIBROOT} "crash_generation_client.lib;common.lib;exception_handler.lib;processor_bits.lib")
PREPEND(BREAKPAD_LIBS_DEBUG ${BREAKPAD_LIBROOT_DEBUG} "crash_generation_client.lib;common.lib;exception_handler.lib;processor_bits.lib")

target_include_directories(OrbitDll
PRIVATE
    ${MINHOOK_ROOT}/include
)

#message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
#message(STATUS "CMAKE_ARCH=${CMAKE_ARCH}")

message(STATUS "CMAKE_VS_PLATFORM_NAME=${CMAKE_VS_PLATFORM_NAME}")
message(STATUS "CMAKE_VS_PLATFORM_TOOLSET=${CMAKE_VS_PLATFORM_TOOLSET}")
message(STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

message(STATUS "MinHook lib: ${MINHOOK_ROOT}/lib/Debug/libMinHook.${CMAKE_VS_PLATFORM_TOOLSET}.lib")

#link_directories(${MINHOOK_ROOT}/lib/debug)
target_link_libraries(OrbitDll
    OrbitBase
	OrbitAsm
	debug
		"${MINHOOK_ROOT}/lib/Debug/libMinHook.${CMAKE_VS_PLATFORM_NAME}.lib"
		${BREAKPAD_LIBS_DEBUG}
	optimized
		"${MINHOOK_ROOT}/lib/Release/libMinHook.${CMAKE_VS_PLATFORM_NAME}.lib"
		${BREAKPAD_LIBS_RELEASE}
)
