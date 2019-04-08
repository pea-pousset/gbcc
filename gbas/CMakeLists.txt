cmake_minimum_required(VERSION 2.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(../common)
set(src
	main.c
	opcodes.c
	outfile.c
	sections.c
	syms.c
	../common/utils.c
	../common/options.c
	../common/errors.c
	../common/files.c
	../common/gbmmap.c
)
set(inc
	commons.h
	version.h
	opcodes.h
	outfile.h
	sections.h
	syms.h
	../common/errors.h
	../common/utils.h
	../common/files.h
	../common/options.h
	../common/gbmmap.h
)
add_executable(gbas ${src} ${inc})
install(TARGETS gbas DESTINATION bin)
