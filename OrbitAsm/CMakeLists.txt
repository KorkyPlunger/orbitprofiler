enable_language(ASM_MASM)

set(HEADERS OrbitAsm.h)
set(SOURCES Orbit.asm OrbitAsm.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitAsm STATIC ${SOURCES} ${HEADERS})
#set_source_files_properties(Orbit.asm PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
