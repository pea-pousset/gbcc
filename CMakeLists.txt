cmake_minimum_required(VERSION 2.8)
project(gbcc)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/gb)

file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

include_directories(${CMAKE_SOURCE_DIR}/common)
add_subdirectory(gbcc)
add_subdirectory(gbas)
add_subdirectory(gbld)

#set(CRT0 "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/crt0.o")
#add_custom_command(
#     OUTPUT ${CRT0}
#     $(CMAKE_COMMAND) -E ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gbas ${CMAKE_SOURCE_DIR}/lib/crt0.s -c -o ${CRT0}
#     #DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gbas  
#)

#add_custom_target(libs ALL DEPENDS ${CRT0})
#install(TARGETS libs DESTINATION bin) 
