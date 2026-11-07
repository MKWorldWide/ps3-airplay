#!/usr/bin/env bash
set -euo pipefail

: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source tree}"
PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"
DEPS_ROOT="${DEPS_ROOT:-$HOME/ps3-airplay-host/crypto-deps}"
VERSION=3.6.7
ARCHIVE="$DEPS_ROOT/downloads/mbedtls-$VERSION.tar.bz2"
SOURCE="$DEPS_ROOT/src/mbedtls-$VERSION"
HOST_BUILD="$DEPS_ROOT/build-host-$VERSION"
PPU_BUILD="$DEPS_ROOT/build-ppu-$VERSION"
SHA256=a7e8bcbec0e6f761b4af24f25677626b35f762f68eef79c08677a363212d11f6
JOBS="${JOBS:-2}"

mkdir -p "$DEPS_ROOT/downloads" "$DEPS_ROOT/src"
if [[ ! -f "$ARCHIVE" ]]; then
	curl --fail --location --output "$ARCHIVE" \
		"https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-$VERSION/mbedtls-$VERSION.tar.bz2"
fi
printf '%s  %s\n' "$SHA256" "$ARCHIVE" | sha256sum --check --status || {
	echo "Mbed TLS $VERSION archive checksum mismatch" >&2
	exit 1
}
if [[ ! -d "$SOURCE" || "${FORCE_SOURCE_REEXTRACT:-0}" == 1 ]]; then
	rm -rf "$SOURCE"
	tar -xjf "$ARCHIVE" -C "$DEPS_ROOT/src"
fi

cmake -S "$SOURCE" -B "$HOST_BUILD" \
	-DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF \
	-DCMAKE_C_FLAGS=-DMBEDTLS_NO_PLATFORM_ENTROPY
cmake --build "$HOST_BUILD" --target mbedcrypto --parallel "$JOBS"

PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"
PPU_CC="$PS3DEV/ppu/bin/powerpc64-ps3-elf-gcc"
PPU_CFLAGS='-DMBEDTLS_NO_PLATFORM_ENTROPY -Dunix -D_POSIX_VERSION=200809L'
cmake -S "$SOURCE" -B "$PPU_BUILD" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake" \
	-DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF \
	-DCMAKE_C_FLAGS="$PPU_CFLAGS"
cmake --build "$PPU_BUILD" --target mbedcrypto --parallel "$JOBS"

MBEDTLS_INCLUDE="$SOURCE/include"
HOST_LIBRARY="$HOST_BUILD/library/libmbedcrypto.a"
PPU_LIBRARY="$PPU_BUILD/library/libmbedcrypto.a"
if [[ ! -f "$HOST_LIBRARY" || ! -f "$PPU_LIBRARY" ]]; then
	echo "Expected host and PPU libmbedcrypto.a outputs were not produced" >&2
	exit 1
fi

HOST_TEST="$PROJECT_ROOT/build/lima-crypto/raop_crypto_test"
HOST_SESSION_TEST="$PROJECT_ROOT/build/lima-crypto/raop_session_crypto_test"
HOST_SDP_TEST="$PROJECT_ROOT/build/lima-crypto/raop_sdp_test"
PPU_OBJECT="$PROJECT_ROOT/build/lima-crypto/raop_crypto.ppu.o"
PPU_SESSION_OBJECT="$PROJECT_ROOT/build/lima-crypto/raop_session.ppu.o"
PPU_RECEIVER_OBJECT="$PROJECT_ROOT/build/lima-crypto/raop_receiver.ppu.o"
PPU_ALAC_OBJECT="$PROJECT_ROOT/build/lima-crypto/raop_alac.ppu.o"
mkdir -p "$(dirname "$HOST_TEST")"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/crypto" -I"$MBEDTLS_INCLUDE" \
	-DMBEDTLS_NO_PLATFORM_ENTROPY \
	"$PROJECT_ROOT/src/crypto/raop_crypto.c" \
	"$PROJECT_ROOT/tests/protocol/raop_crypto_test.c" "$HOST_LIBRARY" \
	-o "$HOST_TEST"
"$HOST_TEST"

cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/crypto" -I"$PROJECT_ROOT/src/protocol" \
	-I"$MBEDTLS_INCLUDE" -DMBEDTLS_NO_PLATFORM_ENTROPY \
	"$PROJECT_ROOT/src/crypto/raop_crypto.c" \
	"$PROJECT_ROOT/src/protocol/raop_sdp.c" \
	"$PROJECT_ROOT/src/protocol/raop_session.c" \
	"$PROJECT_ROOT/tests/protocol/raop_session_crypto_test.c" "$HOST_LIBRARY" -lm \
	-o "$HOST_SESSION_TEST"
"$HOST_SESSION_TEST"

HOST_STATE_TEST="$PROJECT_ROOT/build/lima-crypto/raop_session_test"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/crypto" -I"$PROJECT_ROOT/src/protocol" \
	-I"$MBEDTLS_INCLUDE" -DMBEDTLS_NO_PLATFORM_ENTROPY \
	"$PROJECT_ROOT/src/crypto/raop_crypto.c" \
	"$PROJECT_ROOT/src/protocol/raop_sdp.c" \
	"$PROJECT_ROOT/src/protocol/raop_session.c" \
	"$PROJECT_ROOT/tests/protocol/raop_session_test.c" "$HOST_LIBRARY" -lm \
	-o "$HOST_STATE_TEST"
"$HOST_STATE_TEST"

cc -std=c11 -Wall -Wextra -Werror \
	-I"$PROJECT_ROOT/src/crypto" -I"$PROJECT_ROOT/src/protocol" \
	-I"$MBEDTLS_INCLUDE" -DMBEDTLS_NO_PLATFORM_ENTROPY \
	"$PROJECT_ROOT/src/crypto/raop_crypto.c" \
	"$PROJECT_ROOT/src/protocol/raop_sdp.c" \
	"$PROJECT_ROOT/src/protocol/raop_session.c" \
	"$PROJECT_ROOT/tests/protocol/raop_sdp_test.c" "$HOST_LIBRARY" -lm \
	-o "$HOST_SDP_TEST"
"$HOST_SDP_TEST"

MDNS_TEST="$PROJECT_ROOT/build/lima-crypto/raop_mdns_test"
cc -D_DEFAULT_SOURCE -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/discovery" \
	"$PROJECT_ROOT/src/discovery/raop_mdns.c" \
	"$PROJECT_ROOT/src/discovery/raop_mdns_service.c" \
	"$PROJECT_ROOT/tests/discovery/raop_mdns_test.c" -o "$MDNS_TEST"
"$MDNS_TEST"

cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined -pthread \
	-I"$PROJECT_ROOT/src/crypto" -I"$PROJECT_ROOT/src/protocol" \
	-I"$PROJECT_ROOT/src/observability" -I"$MBEDTLS_INCLUDE" \
	-DMBEDTLS_NO_PLATFORM_ENTROPY \
	"$PROJECT_ROOT/src/crypto/raop_crypto.c" \
	"$PROJECT_ROOT/src/protocol/raop_sdp.c" \
	"$PROJECT_ROOT/src/protocol/raop_session.c" \
	"$PROJECT_ROOT/src/protocol/raop_rtp.c" \
	"$PROJECT_ROOT/src/protocol/raop_receiver.c" \
	"$PROJECT_ROOT/src/observability/receiver_log.c" \
	"$PROJECT_ROOT/tests/protocol/raop_receiver_test.c" "$HOST_LIBRARY" -lm \
	-o "$PROJECT_ROOT/build/lima-crypto/raop_receiver_test"
"$PROJECT_ROOT/build/lima-crypto/raop_receiver_test"

RTSP_LOG_TEST="$PROJECT_ROOT/build/lima-crypto/rtsp_listener_test"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined -pthread \
	-I"$PROJECT_ROOT/src/protocol" -I"$PROJECT_ROOT/src/observability" \
	"$PROJECT_ROOT/src/protocol/rtsp_diagnostic.c" \
	"$PROJECT_ROOT/src/protocol/rtsp_listener.c" \
	"$PROJECT_ROOT/src/observability/receiver_log.c" \
	"$PROJECT_ROOT/tests/protocol/rtsp_listener_test.c" \
	-o "$RTSP_LOG_TEST"
"$RTSP_LOG_TEST"

ALAC_TEST="$PROJECT_ROOT/build/lima-crypto/raop_alac_test"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/codec" -I"$PROJECT_ROOT/src/protocol" \
	"$PROJECT_ROOT/src/codec/alac.c" \
	"$PROJECT_ROOT/src/codec/raop_alac.c" \
	"$PROJECT_ROOT/tests/protocol/raop_alac_test.c" -o "$ALAC_TEST"
(cd "$PROJECT_ROOT" && "$ALAC_TEST")

PCM_RING_TEST="$PROJECT_ROOT/build/lima-crypto/pcm_ring_test"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/audio" "$PROJECT_ROOT/src/audio/pcm_ring.c" \
	"$PROJECT_ROOT/tests/audio/pcm_ring_test.c" -o "$PCM_RING_TEST"
"$PCM_RING_TEST"

PCM_GAIN_TEST="$PROJECT_ROOT/build/lima-crypto/pcm_gain_test"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/audio" "$PROJECT_ROOT/src/audio/pcm_gain.c" \
	"$PROJECT_ROOT/tests/audio/pcm_gain_test.c" -o "$PCM_GAIN_TEST"
"$PCM_GAIN_TEST"

TONE_TEST="$PROJECT_ROOT/build/lima-crypto/diagnostic_tone_test"
cc -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
	-I"$PROJECT_ROOT/src/audio" "$PROJECT_ROOT/src/audio/diagnostic_tone.c" \
	"$PROJECT_ROOT/tests/audio/diagnostic_tone_test.c" -lm -o "$TONE_TEST"
"$TONE_TEST"

"$PPU_CC" -std=c11 -Wall -Wextra -Werror -mcpu=cell -mhard-float \
	-fmodulo-sched $PPU_CFLAGS -I"$PROJECT_ROOT/src/crypto" \
	-I"$MBEDTLS_INCLUDE" -I"$PS3DK_SOURCE/sdk/include" \
	-c "$PROJECT_ROOT/src/crypto/raop_crypto.c" \
	-o "$PPU_OBJECT"
for source in raop_session raop_receiver; do
	"$PPU_CC" -std=c11 -Wall -Wextra -Werror -mcpu=cell -mhard-float \
		-fmodulo-sched $PPU_CFLAGS -I"$PROJECT_ROOT/src/crypto" \
		-I"$PROJECT_ROOT/src/protocol" \
		-I"$PROJECT_ROOT/src/observability" -I"$MBEDTLS_INCLUDE" \
		-I"$PS3DK_SOURCE/sdk/include" \
		-c "$PROJECT_ROOT/src/protocol/$source.c" \
		-o "$PROJECT_ROOT/build/lima-crypto/$source.ppu.o"
done
for source in alac raop_alac; do
	"$PPU_CC" -std=c11 -Wall -Wextra -Werror -mcpu=cell -mhard-float \
		-fmodulo-sched $PPU_CFLAGS -I"$PROJECT_ROOT/src/codec" \
		-I"$PROJECT_ROOT/src/protocol" -I"$PS3DK_SOURCE/sdk/include" \
		-c "$PROJECT_ROOT/src/codec/$source.c" \
		-o "$PROJECT_ROOT/build/lima-crypto/$source.ppu.o"
done
"$PPU_CC" -std=c11 -Wall -Wextra -Werror -mcpu=cell -mhard-float \
	-fmodulo-sched $PPU_CFLAGS -I"$PROJECT_ROOT/src/audio" \
	-I"$PS3DK_SOURCE/sdk/include" -c "$PROJECT_ROOT/src/audio/pcm_ring.c" \
	-o "$PROJECT_ROOT/build/lima-crypto/pcm_ring.ppu.o"
"$PPU_CC" -std=c11 -Wall -Wextra -Werror -mcpu=cell -mhard-float \
	-fmodulo-sched $PPU_CFLAGS -I"$PROJECT_ROOT/src/audio" \
	-I"$PS3DK_SOURCE/sdk/include" -c "$PROJECT_ROOT/src/audio/pcm_gain.c" \
	-o "$PROJECT_ROOT/build/lima-crypto/pcm_gain.ppu.o"
file "$PPU_OBJECT" "$PPU_SESSION_OBJECT" "$PPU_RECEIVER_OBJECT" \
	"$PROJECT_ROOT/build/lima-crypto/alac.ppu.o" "$PPU_ALAC_OBJECT" \
	"$PROJECT_ROOT/build/lima-crypto/pcm_ring.ppu.o" \
	"$PROJECT_ROOT/build/lima-crypto/pcm_gain.ppu.o" \
	"$PPU_LIBRARY"
