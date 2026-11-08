#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${CC:=cc}"
mkdir -p build/forensic
"$CC" -O2 -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 -Isrc/protocol -Isrc/audio tests/protocol/forensic_rtp_test.c \
 src/protocol/raop_rtp.c src/audio/pcm_ring.c -o build/forensic/rtp
build/forensic/rtp
python3 tests/observability/hardware_log_analysis_test.py
python3 tests/observability/display_resume_test.py
"$CC" -O2 -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 -Isrc/protocol -Isrc/codec tests/protocol/forensic_payload_test.c \
 src/protocol/raop_rtp.c src/protocol/raop_sdp.c src/codec/raop_alac.c src/codec/alac.c \
 -o build/forensic/payload
build/forensic/payload
# Linux GNU ld can discard the unused receiver functions and their dependencies.
# MBEDTLS_INCLUDE must name an existing Mbed TLS 3.x include directory.
if [[ "$(uname -s)" == Linux && -n "${MBEDTLS_INCLUDE:-}" ]]; then
 "$CC" -O2 -std=c11 -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
  -I"$MBEDTLS_INCLUDE" -Isrc/protocol -Isrc/crypto -Isrc/observability \
  tests/protocol/forensic_resend_test.c src/observability/receiver_log.c -Wl,--gc-sections -o build/forensic/resend
 build/forensic/resend
else
 echo 'Private resend-policy reproduction requires Linux and MBEDTLS_INCLUDE; skipped.'
fi

"$CC" -O2 -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 -DPROBE_TEST -Isrc/audio tests/audio/resampler_probe.c src/audio/pcm_ring.c -o build/forensic/timing
build/forensic/timing
"$CC" -O2 -std=c11 -Wall -Wextra -Werror -Isrc/observability \
 tests/observability/log_cost_test.c src/observability/receiver_log.c -o build/forensic/log-cost
build/forensic/log-cost
