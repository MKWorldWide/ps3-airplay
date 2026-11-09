cmake_minimum_required(VERSION 3.20)
project(ps3_airplay_resident_harness C)

include(ps3-self)

set(RESIDENT_SPRX "" CACHE FILEPATH "Built RPCS3-loadable diagnostic SPRX to embed")
set(HARNESS_TEST_CYCLES "50" CACHE STRING "Audio-enabled resident lifecycle cycles")
set(HARNESS_TITLE_ID "RPRX90001" CACHE STRING "Installed test application's title ID")
set(HARNESS_RESIDENT_SPRX_NAME "ps3-airplay-receiver.fake.sprx" CACHE STRING
	"Resident module filename embedded in the test package")
set(HARNESS_RESIDENT_MODULE_PATH "" CACHE STRING
	"Absolute GameOS path used by sys_prx_load_module")
set(HARNESS_PKGFILES "${CMAKE_CURRENT_BINARY_DIR}/pkg_files" CACHE PATH
	"Package overlay containing the diagnostic PRX")
file(MAKE_DIRECTORY "${HARNESS_PKGFILES}")
string(LENGTH "${HARNESS_TITLE_ID}" HARNESS_TITLE_ID_LENGTH)
if(NOT HARNESS_TITLE_ID MATCHES "^[A-Z0-9]+$" OR
	NOT HARNESS_TITLE_ID_LENGTH EQUAL 9)
	message(FATAL_ERROR "HARNESS_TITLE_ID must be exactly nine uppercase letters/digits")
endif()
if(NOT EXISTS "${RESIDENT_SPRX}")
	message(FATAL_ERROR "Set RESIDENT_SPRX to the resident SPRX artifact to embed")
endif()
if(HARNESS_RESIDENT_MODULE_PATH STREQUAL "")
	set(HARNESS_RESIDENT_MODULE_PATH
		"/dev_hdd0/game/${HARNESS_TITLE_ID}/USRDIR/${HARNESS_RESIDENT_SPRX_NAME}")
endif()

set(OVERLAY_SPRX "${HARNESS_PKGFILES}/USRDIR/${HARNESS_RESIDENT_SPRX_NAME}")
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
target_compile_definitions(ps3-airplay-resident-harness PRIVATE
	TEST_CYCLES=${HARNESS_TEST_CYCLES}
	RESIDENT_MODULE_PATH=\"${HARNESS_RESIDENT_MODULE_PATH}\")
target_link_libraries(ps3-airplay-resident-harness PRIVATE lv2)
target_link_libraries(ps3-airplay-resident-harness PRIVATE audio sysmodule m)

ps3_add_self(ps3-airplay-resident-harness)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/sfo.xml" HARNESS_SFO_XML)
string(REPLACE "RPRX90001" "${HARNESS_TITLE_ID}" HARNESS_SFO_XML
	"${HARNESS_SFO_XML}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sfo.xml" "${HARNESS_SFO_XML}")
ps3_add_pkg(ps3-airplay-resident-harness
	TITLE "AIR PS3 Resident Test"
	APPID "${HARNESS_TITLE_ID}"
	CONTENTID "UP0001-${HARNESS_TITLE_ID}_00-0000000000000000"
	SFOXML "${CMAKE_CURRENT_BINARY_DIR}/sfo.xml"
	PKGFILES "${HARNESS_PKGFILES}")
add_dependencies(ps3-airplay-resident-harness_pkg resident_prx_overlay)
