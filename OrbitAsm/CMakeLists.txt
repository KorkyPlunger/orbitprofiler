enable_language(ASM_MASM)

add_library(OrbitAsm STATIC Orbit.asm OrbitAsm.cpp)
#set_source_files_properties(Orbit.asm PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
