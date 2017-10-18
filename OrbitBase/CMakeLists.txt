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
    ${DIASDKDir}/include
PRIVATE
    #${LLVM_INCLUDE_DIRS}    
    #${EXTERN_ROOT}/peparse
    ${EXTERN_ROOT}/mintomic/include
    ${EXTERN_ROOT}/oqpi/include
    
    #${EXTERN_ROOT}/sqlite
    #${EXTERN_ROOT}/asio/include
    #${EXTERN_ROOT}/minhook/include
    #${EXTERN_ROOT}/cereal-1.1.2/include
    #${EXTERN_ROOT}/curl-7.52.1/include
    #$(VCInstallDir)/${CMAKE_SOURCE_DIR}/DIA SDK/include 
)
