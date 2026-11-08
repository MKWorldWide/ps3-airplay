cmake_minimum_required(VERSION 3.20)
project(ps3_airplay_resident_harness C)

include(ps3-self)

set(RESIDENT_SPRX "" CACHE FILEPATH "Built RPCS3-loadable diagnostic SPRX to embed")
set(HARNESS_PKGFILES "${CMAKE_CURRENT_BINARY_DIR}/pkg_files" CACHE PATH
	"Package overlay containing the diagnostic PRX")
file(MAKE_DIRECTORY "${HARNESS_PKGFILES}")
if(NOT EXISTS "${RESIDENT_SPRX}")
	message(FATAL_ERROR "Set RESIDENT_SPRX to the isolated resident .fake.sprx artifact")
endif()

set(OVERLAY_SPRX "${HARNESS_PKGFILES}/USRDIR/ps3-airplay-receiver.fake.sprx")
add_custom_command(OUTPUT "${OVERLAY_SPRX}"
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${HARNESS_PKGFILES}/USRDIR"
	COMMAND "${CMAKE_COMMAND}" -E copy "${RESIDENT_SPRX}" "${OVERLAY_SPRX}"
	DEPENDS "${RESIDENT_SPRX}"
	VERBATIM)
add_custom_target(resident_prx_overlay DEPENDS "${OVERLAY_SPRX}")

add_executable(ps3-airplay-resident-harness source/main.c)
target_compile_features(ps3-airplay-resident-harness PRIVATE c_std_11)
target_include_directories(ps3-airplay-resident-harness PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/..")
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
