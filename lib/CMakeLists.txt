add_library(libutils utils.c)
target_link_libraries(libutils m)

add_library(libcetsp cetsp.c)
target_link_libraries(libcetsp libutils)

add_library(libcetspwdc cetspwdc.c)
target_link_libraries(libcetspwdc libutils)

