get_filename_component(EXTERN_ROOT ${CMAKE_SOURCE_DIR}/external REALPATH)

add_library(OrbitBase STATIC
    CallstackPOD.cpp
    CrashHandler.cpp
    Log.cpp
    Message.cpp
    OrbitAsio.cpp
    Path.cpp
    ScopeTimer.cpp
    TcpEntity.cpp
    TimerManager.cpp
    Utils.cpp

	${EXTERN_ROOT}/xxHash-r42/xxhash.c
    ${EXTERN_ROOT}/xxHash-r42/xxhsum.c
)

target_include_directories(OrbitBase
PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXTERN_ROOT}/xxHash-r42
    ${EXTERN_ROOT}/stlsoft-1.9.118/include
    ${EXTERN_ROOT}/concurrentqueue
    ${EXTERN_ROOT}/multicore/common
    ${EXTERN_ROOT}/oqpi/include
    ${DIASDKDir}/include
PRIVATE
    #${EXTERN_ROOT}/mintomic/include
    ${EXTERN_ROOT}/breakpad/src
)
