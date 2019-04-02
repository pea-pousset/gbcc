cmake_minimum_required(VERSION 2.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(../common)
set(src 
	ast.c
	codegen.c
	lexer.c
	main.c
	parser.c
	pp.c
	syms.c
	../common/options.c
	../common/utils.c
	../common/errors.c
	../common/files.c
)
set(inc
    astcommons.h
    version.h
    ../common/errors.h
    ../common/files.h
    ../common/options.h
    ../common/utils.h    
)
add_executable(gbcc ${src} ${inc})
install(TARGETS gbcc DESTINATION bin)
