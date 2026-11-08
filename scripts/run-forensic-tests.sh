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
"$CC" -O2 -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 -Isrc/protocol -Isrc/codec tests/protocol/forensic_payload_test.c \
 src/protocol/raop_rtp.c src/protocol/raop_sdp.c src/codec/raop_alac.c src/codec/alac.c \
 -o build/forensic/payload
build/forensic/payload
# Isolated boundary experiment; production headers and artifacts stay intact.
mkdir -p build/forensic/expanded
cp -R src build/forensic/expanded/
python3 - <<'PYINNER'
from pathlib import Path
for name in ['protocol/raop_rtp.h', 'codec/raop_alac.h', 'crypto/raop_crypto.h']:
    p = Path('build/forensic/expanded/src') / name
    p.write_text(p.read_text().replace('MAX 1400', 'MAX 1536'))
PYINNER
"$CC" -O2 -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 -Ibuild/forensic/expanded/src/protocol -Ibuild/forensic/expanded/src/codec \
 tests/protocol/forensic_payload_test.c build/forensic/expanded/src/protocol/raop_rtp.c \
 build/forensic/expanded/src/protocol/raop_sdp.c build/forensic/expanded/src/codec/raop_alac.c \
 build/forensic/expanded/src/codec/alac.c -o build/forensic/payload-expanded
build/forensic/payload-expanded

# Linux GNU ld can discard the unused receiver functions and their dependencies.
# MBEDTLS_INCLUDE must name an existing Mbed TLS 3.x include directory.
if [[ "$(uname -s)" == Linux && -n "${MBEDTLS_INCLUDE:-}" ]]; then
 "$CC" -O2 -std=c11 -Wall -Wextra -Werror -ffunction-sections -fdata-sections \
  -I"$MBEDTLS_INCLUDE" -Isrc/protocol -Isrc/crypto -Isrc/observability \
  tests/protocol/forensic_resend_test.c -Wl,--gc-sections -o build/forensic/resend
 build/forensic/resend
else
 echo 'Private resend-policy reproduction requires Linux and MBEDTLS_INCLUDE; skipped.'
fi
