cmake_minimum_required(VERSION 3.20)
project(ps3_airplay_resident_harness C)

include(ps3-self)

set(RESIDENT_PRX "" CACHE FILEPATH "Built diagnostic resident PRX to embed")
set(HARNESS_PKGFILES "${CMAKE_CURRENT_BINARY_DIR}/pkg_files" CACHE PATH
	"Package overlay containing the diagnostic PRX")
file(MAKE_DIRECTORY "${HARNESS_PKGFILES}")
if(NOT EXISTS "${RESIDENT_PRX}")
	message(FATAL_ERROR "Set RESIDENT_PRX to the isolated resident .prx artifact")
endif()

set(OVERLAY_PRX "${HARNESS_PKGFILES}/USRDIR/ps3-airplay-receiver.prx")
add_custom_command(OUTPUT "${OVERLAY_PRX}"
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${HARNESS_PKGFILES}/USRDIR"
	COMMAND "${CMAKE_COMMAND}" -E copy "${RESIDENT_PRX}" "${OVERLAY_PRX}"
	DEPENDS "${RESIDENT_PRX}"
	VERBATIM)
add_custom_target(resident_prx_overlay DEPENDS "${OVERLAY_PRX}")

add_executable(ps3-airplay-resident-harness source/main.c)
target_compile_features(ps3-airplay-resident-harness PRIVATE c_std_11)
target_compile_options(ps3-airplay-resident-harness PRIVATE
	-Wall -Wextra -Werror)
target_link_libraries(ps3-airplay-resident-harness PRIVATE lv2)

ps3_add_self(ps3-airplay-resident-harness)
ps3_add_pkg(ps3-airplay-resident-harness
	TITLE "AIR PS3 Resident Test"
	APPID "RPRX90001"
	CONTENTID "UP0001-RPRX90001_00-0000000000000000"
	SFOXML "${CMAKE_CURRENT_SOURCE_DIR}/sfo.xml"
	PKGFILES "${HARNESS_PKGFILES}")
add_dependencies(ps3-airplay-resident-harness_pkg resident_prx_overlay)
