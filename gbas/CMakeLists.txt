cmake_minimum_required(VERSION 2.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(../common)
set(src
	main.c
	opcodes.c
	sections.c
	syms.c
    relocs.c
	../common/utils.c
	../common/options.c
	../common/errors.c
	../common/files.c
	../common/gbmmap.c
    ../common/objfile.c
)
set(inc
	commons.h
	version.h
	opcodes.h
	sections.h
	syms.h
    relocs.h
	../common/errors.h
	../common/utils.h
	../common/files.h
	../common/options.h
	../common/gbmmap.h
    ../common/objfile.h
    ../common/defs.h
)
add_executable(gbas ${src} ${inc})
install(TARGETS gbas DESTINATION bin)
