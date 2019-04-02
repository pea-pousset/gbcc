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
)
set(
    inc
    commons.h
    version.h
    ${CMAKE_SOURCE_DIR}/common/errors.h
    ${CMAKE_SOURCE_DIR}/common/utils.h
    ${CMAKE_SOURCE_DIR}/common/files.h
    ${CMAKE_SOURCE_DIR}/common/options.h
)
add_executable(gbas ${src} ${inc})
install(TARGETS gbas DESTINATION bin)
