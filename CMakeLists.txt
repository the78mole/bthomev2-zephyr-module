cmake_minimum_required(VERSION 3.20.0)

zephyr_library_amend()
zephyr_library_include_directories(include)
zephyr_library_sources_ifdef(CONFIG_BTHOME src/bthome.c)
