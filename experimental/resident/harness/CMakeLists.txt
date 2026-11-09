cmake_minimum_required(VERSION 3.20)
project(ps3_airplay_resident_harness C ASM)

include(ps3-self)

set(RESIDENT_SPRX "" CACHE FILEPATH "Built RPCS3-loadable diagnostic SPRX to embed")
set(HARNESS_TEST_CYCLES "50" CACHE STRING "Audio-enabled resident lifecycle cycles")
set(HARNESS_PROBE_CYCLES "3" CACHE STRING "Cycles for each loader diagnostic module")
set(HARNESS_TITLE_ID "RPRX90001" CACHE STRING "Installed test application's title ID")
set(HARNESS_RESIDENT_SPRX_NAME "ps3-airplay-receiver.fake.sprx" CACHE STRING
	"Resident module filename embedded in the test package")
set(HARNESS_RESIDENT_MODULE_PATH "" CACHE STRING
	"Absolute GameOS path used by sys_prx_load_module")
set(HARNESS_PKGFILES "${CMAKE_CURRENT_BINARY_DIR}/pkg_files" CACHE PATH
	"Package overlay containing the diagnostic PRX")
set(PS3DK_SOURCE "$ENV{PS3DK_SOURCE}")
set(HARNESS_PROBE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/probe_bins")
set(HARNESS_PROBE_MINIMAL_NAME "resident-loader-minimal.sprx")
set(HARNESS_PROBE_REFERENCE_NAME "resident-loader-reference.sprx")
set(HARNESS_PROBE_MINIMAL_PATH
	"/dev_hdd0/game/${HARNESS_TITLE_ID}/USRDIR/${HARNESS_PROBE_MINIMAL_NAME}")
set(HARNESS_PROBE_REFERENCE_PATH
	"/dev_hdd0/game/${HARNESS_TITLE_ID}/USRDIR/${HARNESS_PROBE_REFERENCE_NAME}")
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

add_executable(resident-loader-minimal probes/minimal.c)
target_compile_features(resident-loader-minimal PRIVATE c_std_11)
target_compile_options(resident-loader-minimal PRIVATE -Wall -Wextra -Werror)
ps3_add_prx(resident-loader-minimal
	NAME resident_loader_minimal
	VERSION 1.0
	OUTPUT_NAME resident-loader-minimal.prx
	OUTPUT "${HARNESS_PROBE_OUTPUT_DIR}/resident-loader-minimal.prx"
	SIGN)

add_executable(resident-loader-reference
	probes/reference.c
	probes/reference-exports.S)
target_compile_features(resident-loader-reference PRIVATE c_std_11)
target_compile_options(resident-loader-reference PRIVATE -Wall -Wextra -Werror)
target_include_directories(resident-loader-reference PRIVATE
	"${PS3DK_SOURCE}/sdk/include")
ps3_add_prx(resident-loader-reference
	NAME resident_loader_reference
	VERSION 1.0
	OUTPUT_NAME resident-loader-reference.prx
	OUTPUT "${HARNESS_PROBE_OUTPUT_DIR}/resident-loader-reference.prx"
	SIGN)

set(PROBE_OVERLAYS)
foreach(PROBE_NAME IN ITEMS ${HARNESS_PROBE_MINIMAL_NAME} ${HARNESS_PROBE_REFERENCE_NAME})
	set(PROBE_SPRX "${HARNESS_PROBE_OUTPUT_DIR}/${PROBE_NAME}")
	set(PROBE_OVERLAY "${HARNESS_PKGFILES}/USRDIR/${PROBE_NAME}")
	add_custom_command(OUTPUT "${PROBE_OVERLAY}"
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${HARNESS_PKGFILES}/USRDIR"
		COMMAND "${CMAKE_COMMAND}" -E copy "${PROBE_SPRX}" "${PROBE_OVERLAY}"
		DEPENDS "${PROBE_SPRX}"
		VERBATIM)
	list(APPEND PROBE_OVERLAYS "${PROBE_OVERLAY}")
endforeach()
add_custom_target(resident_probe_overlays DEPENDS ${PROBE_OVERLAYS})
add_dependencies(resident_probe_overlays resident-loader-minimal resident-loader-reference)

add_executable(ps3-airplay-resident-harness source/main.c)
target_compile_features(ps3-airplay-resident-harness PRIVATE c_std_11)
target_include_directories(ps3-airplay-resident-harness PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_options(ps3-airplay-resident-harness PRIVATE
	-Wall -Wextra -Werror)
target_compile_definitions(ps3-airplay-resident-harness PRIVATE
	TEST_CYCLES=${HARNESS_TEST_CYCLES}
	PROBE_CYCLES=${HARNESS_PROBE_CYCLES}
	RESIDENT_MODULE_PATH=\"${HARNESS_RESIDENT_MODULE_PATH}\"
	MINIMAL_PROBE_PATH=\"${HARNESS_PROBE_MINIMAL_PATH}\"
	REFERENCE_PROBE_PATH=\"${HARNESS_PROBE_REFERENCE_PATH}\")
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
add_dependencies(ps3-airplay-resident-harness_pkg resident_prx_overlay resident_probe_overlays)
