add_library(OrbitDll SHARED
    Hijacking.cpp
    OrbitDll.cpp
    OrbitLib.cpp
    TcpClient.cpp
)

set(MINHOOK_ROOT ${EXTERN_ROOT}/minhook)
target_include_directories(OrbitDll
PRIVATE
    ${MINHOOK_ROOT}/include
)

#message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
#message(STATUS "CMAKE_ARCH=${CMAKE_ARCH}")

#message(STATUS "CMAKE_VS_PLATFORM_NAME=${CMAKE_VS_PLATFORM_NAME}")
#message(STATUS "CMAKE_VS_PLATFORM_TOOLSET=${CMAKE_VS_PLATFORM_TOOLSET}")
#message(STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

#link_directories(${MINHOOK_ROOT}/lib/debug)
message(STATUS "BREAKPAD_LIBS_DEBUG: ${BREAKPAD_LIBS_DEBUG}")
message(STATUS "BREAKPAD_LIBS_RELEASE: ${BREAKPAD_LIBS_RELEASE}")

target_link_libraries(OrbitDll
    OrbitBase
	OrbitAsm
	debug "${MINHOOK_ROOT}/lib/Debug/libMinHook.${CMAKE_VS_PLATFORM_NAME}.lib"
	#${BREAKPAD_LIBS_DEBUG}
	optimized "${MINHOOK_ROOT}/lib/Release/libMinHook.${CMAKE_VS_PLATFORM_NAME}.lib"
	#${BREAKPAD_LIBS_RELEASE}
)

FOREACH(f ${BREAKPAD_LIBS_DEBUG})
	target_link_libraries(OrbitDll debug ${f})
ENDFOREACH()

FOREACH(f ${BREAKPAD_LIBS_RELEASE})
	target_link_libraries(OrbitDll optimized ${f})
ENDFOREACH()
