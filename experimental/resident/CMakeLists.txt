cmake_minimum_required(VERSION 3.20)
project(ps3_airplay_resident C)

include(ps3-self)

if(NOT DEFINED ENV{PS3DK_SOURCE})
	message(FATAL_ERROR "Set PS3DK_SOURCE to the PS3DK v0.27.0 source tree")
endif()
set(PS3DK_SOURCE "$ENV{PS3DK_SOURCE}")
set(MBEDTLS_INCLUDE_DIR "$ENV{MBEDTLS_INCLUDE_DIR}")
set(MBEDTLS_CRYPTO_LIBRARY "$ENV{MBEDTLS_CRYPTO_LIBRARY}")
if(NOT EXISTS "${MBEDTLS_INCLUDE_DIR}/mbedtls/aes.h" OR
	NOT EXISTS "${MBEDTLS_CRYPTO_LIBRARY}")
	message(FATAL_ERROR "Set MBEDTLS_INCLUDE_DIR and MBEDTLS_CRYPTO_LIBRARY to the verified Mbed TLS 3.6.7 build")
endif()

add_executable(ps3_airplay_resident
	module.c
	"${PS3DK_SOURCE}/runtime/lv2/crt/crt1.c"
	../../src/discovery/raop_mdns.c
	../../src/discovery/raop_mdns_service.c
	../../src/protocol/raop_session.c
	../../src/protocol/raop_sdp.c
	../../src/protocol/raop_rtp.c
	../../src/protocol/raop_receiver.c
	../../src/codec/alac.c
	../../src/codec/raop_alac.c
	../../src/crypto/raop_crypto.c
	../../src/observability/receiver_log.c)

target_compile_features(ps3_airplay_resident PRIVATE c_std_11)
target_compile_options(ps3_airplay_resident PRIVATE
	-Wall -Wextra -Werror -ffunction-sections -fdata-sections
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-exceptions)
target_compile_definitions(ps3_airplay_resident PRIVATE MBEDTLS_NO_PLATFORM_ENTROPY)
target_link_options(ps3_airplay_resident PRIVATE -Wl,--gc-sections)
target_include_directories(ps3_airplay_resident PRIVATE
	../../src/discovery
	../../src/protocol
	../../src/codec
	../../src/crypto
	../../src/observability)
target_include_directories(ps3_airplay_resident PRIVATE
	"${MBEDTLS_INCLUDE_DIR}"
	"${PS3DK_SOURCE}/sdk/include")
set(PS3_LV2_CRT_DIR "${PS3DEV}/ppu/powerpc64-ps3-elf/lib")
target_link_libraries(ps3_airplay_resident PRIVATE
	net sysmodule netctl_stub lv2
	"${MBEDTLS_CRYPTO_LIBRARY}"
	"${PS3_LV2_CRT_DIR}/lv2-crti.o"
	"${PS3_LV2_CRT_DIR}/lv2-crtn.o")

ps3_add_prx(ps3_airplay_resident
	NAME ps3_airplay_receiver
	VERSION 0.1
	SIGN
	OUTPUT_NAME ps3-airplay-receiver.prx
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ps3-airplay-receiver.prx")
