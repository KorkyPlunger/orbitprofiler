get_filename_component(EXTERN_ROOT ${CMAKE_SOURCE_DIR}/external REALPATH)

add_library(OrbitCore STATIC
    Callstack.cpp
    Capture.cpp
    ContextSwitch.cpp
    CoreApp.cpp
    DiaManager.cpp
    DiaParser.cpp
    Diff.cpp
    EventBuffer.cpp
    EventCallbacks.cpp
    EventTracer.cpp
    EventUtils.cpp
    FunctionStats.cpp
    Injection.cpp
    LogInterface.cpp
    MemoryTracker.cpp
    MiniDump.cpp
    ModuleManager.cpp
    OrbitDia.cpp
    OrbitFunction.cpp
    OrbitModule.cpp
    OrbitProcess.cpp
    OrbitRule.cpp
    OrbitSession.cpp
    OrbitThread.cpp
    OrbitType.cpp
    OrbitUnreal.cpp
    Params.cpp
    Pdb.cpp
    ProcessUtils.cpp
    SamplingProfiler.cpp
    Serialization.cpp
    SymbolUtils.cpp
    Tcp.cpp
    TcpServer.cpp
    #TypeInfo.cpp
    Variable.cpp
    VariableTracing.cpp
    Version.cpp
)

target_include_directories(OrbitCore
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
    ${EXTERN_ROOT}/breakpad/src
)

target_link_libraries(OrbitCore
    OrbitBase
)
