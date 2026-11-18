# Mennell's instances based generator
add_executable(mennell mennell.c)
target_link_libraries(mennell libcetsp libcetspwdc)

# Behdani's instances based generator
add_executable(behdani behdani.c)
target_link_libraries(behdani libcetsp libcetspwdc)

