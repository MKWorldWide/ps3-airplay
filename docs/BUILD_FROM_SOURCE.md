# Build AIR PS3 from source

This guide is for developers who want to compile AIR PS3, inspect its PS3 package, or independently validate the release. **No Sony proprietary SDK is distributed with this project.** The tested build uses the open-source PS3DK/PSL1GHT toolchain on **Ubuntu 24.04 x86_64 inside Lima**. Native x86_64 Linux should also be suitable once the same dependencies are installed; that configuration has not been independently validated here.

The scripts run real host tests and cross-compile PS3 code. They do **not** provision a fresh PS3 SDK automatically. Prepare the pinned environment first, following the upstream toolchain instructions.

## 1. Obtain the source

```sh
git clone https://github.com/MKWorldWide/ps3-airplay.git
cd ps3-airplay
git checkout v1.0.0
git status --short
git rev-parse HEAD
```

For an audit of the published v1.0.0 package, use the **v1.0.0 tag**, not the latest `main` branch. The release evidence identifies source commit `e1fdb44b03e2beb048954aded0eda5ae11248215`. Git tags may be annotated: use `git rev-parse 'v1.0.0^{commit}'` to read the underlying commit.

## 2. Prepare a Linux x86_64 environment

On Ubuntu 24.04, install standard host utilities:

```sh
sudo apt update
sudo apt install -y build-essential cmake git make curl ca-certificates \
  bzip2 coreutils python3 python3-pil
python3 -c 'from PIL import Image; print("Pillow ready")'
```

Our macOS development setup uses a Lima Linux guest. Lima itself isn't a build dependency: if you're on a Mac, install [Lima](https://lima-vm.io/) and configure an **x86_64 Ubuntu 24.04** guest, then run the remaining commands **inside that guest**, with a writable Git checkout accessible to Linux. On ARM Macs, x86_64 emulation may require additional configuration and is not claimed as tested.

The required cross-toolchain and dependencies are:

| Dependency | Tested build |
|---|---|
| PS3DK | v0.27.0 PPU SDK and tools |
| PPU compiler | GCC 16.1.0 |
| Linker | GNU binutils 2.42 |
| PSL1GHT | commit `dd4e8c9be744e3273d665c393fa6d97a87402e2e` |
| Mbed TLS | 3.6.7, built for host and PPU by project scripts |
| Python imaging | Pillow |

Obtain and install [PS3DK](https://github.com/FirebirdTA01/PS3DK), [PSL1GHT](https://github.com/ps3dev/PSL1GHT), and the PS3DK host packaging utilities following their upstream build instructions. Use versions compatible with the pinned environment. The complete binary toolchain is **not checked into this repository**. Installing a random `ps3dev` package with a different compiler is not equivalent to reproducing this build.

## 3. Point the scripts at your SDK

Set these paths to match **your own Linux installation** (the examples reflect the tested directory layout, not required absolute locations):

```sh
export PS3DK_SOURCE="$HOME/ps3-airplay-host/ps3-sdk-src-v0.27.0"
export PS3DEV="$PS3DK_SOURCE/stage/ps3dev"
export PS3DK_TOOLS="$HOME/ps3-airplay-host/ps3-sdk-tools-v0.27.0-linux-x86_64/bin"
export PSL1GHT="$HOME/ps3-airplay-host/psl1ght/source"
export DEPS_ROOT="$HOME/ps3-airplay-host/crypto-deps"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"
export SOURCE_REVISION="$(git rev-parse HEAD)"
```

Check the required tools **before** building:

```sh
test -f "$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake"
test -x "$PS3DEV/ppu/bin/powerpc64-ps3-elf-gcc"
test -x "$PS3DEV/bin/sfo"
test -x "$PS3DK_TOOLS/pkg"
test -x "$PS3DK_TOOLS/make_self_npdrm"
test -x "$PS3DK_TOOLS/package_finalize"
git -C "$PSL1GHT" rev-parse HEAD
"$PS3DEV/ppu/bin/powerpc64-ps3-elf-gcc" -dumpfullversion
```

The compiler and PSL1GHT values should match the versions listed above for the closest reproduction. The `PS3DEV` tree must include the compatible SFO template (`bin/sfo.xml`). The packaging scripts use your existing SDK; they will fail rather than silently install a different one.

## 4. Run the exact-source validation and build

From a **clean checkout**, on Linux:

```sh
python3 scripts/test-release-config.py
python3 scripts/test-release-evidence.py
python3 scripts/validate-air-ps3-artwork.py assets/air-ps3
bash scripts/release-lima-standalone-receiver.sh
```

The release script verifies that the tracked source is clean and that `SOURCE_REVISION` matches it. It runs host tests (including AddressSanitizer/UndefinedBehaviorSanitizer), builds PS3 crypto and the standalone receiver, generates the screen background from approved artwork, validates the CellOS ELF, creates the debug PKG, re-extracts package contents, and writes local evidence.

The Mbed TLS helper downloads the pinned **3.6.7** source archive, verifies its hard-coded SHA-256, and compiles host and PPU libraries. You need network access on the first run.

The PS3 package output is:

```text
build/lima-standalone-package/ps3-airplay-receiver-1.0.0.pkg
build/lima-standalone-package/release-evidence.json
build/lima-standalone-package/SHA256SUMS.txt
```

Build output is Git-ignored. The packager refuses to overwrite an existing versioned PKG, so use a clean output directory for each new build. Do not edit `release.json` or SFO version numbers if you're trying to reproduce v1.0.0.

## 5. Verify what you built

```sh
cd build/lima-standalone-package
sha256sum -c SHA256SUMS.txt
cd ../..
```

Review `release-evidence.json` for the source SHA, app version, SFO identity, PKG/ELF hashes, extracted artwork, and completed validation gates. You can also run the evidence validator explicitly:

```sh
python3 scripts/verify-release-evidence.py \
  --package build/lima-standalone-package/ps3-airplay-receiver-1.0.0.pkg \
  --evidence build/lima-standalone-package/release-evidence.json \
  --expected-source "$SOURCE_REVISION" \
  --expected-sha256 "$(sha256sum build/lima-standalone-package/ps3-airplay-receiver-1.0.0.pkg | cut -d' ' -f1)" \
  --release-config release.json
```

For comparison with the [published v1.0.0 release](https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0), download the published PKG and `SHA256SUMS.txt` and check its published hash. The official PKG's SHA-256 is `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`. A locally built package might not be byte-identical if toolchain inputs or packaging timestamps differ; investigate differences instead of assuming identical output without verifying.

## 6. Know what the result proves

- **Host tests passing:** protocol, codec, packet handling, memory-safety instrumentation, and packaging checks passed under that host configuration.
- **ELF/PKG validation passing:** the package has the expected structure, version, and assets.
- **Neither result guarantees:** that a PS3 will boot the package, that audio will be audible, or that your console is compatible.

The public release was tested on **Evilnat 4.93 CFW** with Cobra 8.5 observed. If you choose to test your own build on hardware, use a console you control, keep a known-good rollback package, and review [installation and recovery](INSTALLATION.md), the [test matrix](TEST_MATRIX.md), and [safety notes](RISKS.md).

For engineering internals, see [Architecture](ARCHITECTURE.md), [the detailed pipeline and build history](BUILD_PIPELINE.md), and [Contributing](../CONTRIBUTING.md). The legacy GCC 7 comparison path and experimental resident module are not part of the public build.
