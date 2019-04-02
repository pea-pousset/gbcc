cmake_minimum_required(VERSION 2.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(src
	main.c
	map.c
	rom.c
	../common/options.c
	../common/utils.c
	../common/errors.c
	../common/files.c
)
set(inc
    version.h
    ../common/errors.h
    ../common/files.h
    ../common/options.h
    ../common/utils.h  
)
add_executable(gbld ${src} ${inc})
install(TARGETS gbld DESTINATION bin)
