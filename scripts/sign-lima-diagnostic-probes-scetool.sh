#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
	echo "usage: $0 SCETOOL_EXECUTABLE PROBE_DIRECTORY" >&2
	exit 2
fi

SCETOOL="$1"
PROBE_DIR="$2"
: "${PS3:?Set PS3 to a local scetool data directory containing keys}"

for name in resident-loader-minimal resident-loader-reference; do
	input="$PROBE_DIR/$name.prx"
	output="$PROBE_DIR/$name.sprx"
	[[ -f "$input" ]] || { echo "missing diagnostic PRX: $name" >&2; exit 1; }
	"$SCETOOL" \
		-0 SELF -1 TRUE -s FALSE -2 0A \
		-3 1070000052000001 -4 01000002 -5 APP \
		-6 0003004000000000 -A 0001000000000000 \
		--self-ctrl-flags \
		4000000000000000000000000000000000000000000000000000000000000002 \
		--self-cap-flags \
		00000000000000000000000000000000000000000000007B0000000100020000 \
		-e "$input" "$output"
	python3 - "$output" <<'PY'
import pathlib
import struct
import sys

path = pathlib.Path(sys.argv[1])
data = path.read_bytes()
if len(data) < 12 or data[:4] != b"SCE\0":
    raise SystemExit(f"{path.name}: scetool did not produce a SELF container")
flags, file_type = struct.unpack_from(">HH", data, 8)
if (flags, file_type) != (0x000A, 1):
    raise SystemExit(
        f"{path.name}: expected CEX SPRX flags/type 0x000a/0x0001, "
        f"got 0x{flags:04x}/0x{file_type:04x}"
    )
print(f"{path.name}: independent CEX SPRX signing metadata verified")
PY
done
