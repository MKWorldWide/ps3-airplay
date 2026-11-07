# Development setup

## Requirements

- Git, Make, Python 3, and a PS3 PPU cross-compiler/SDK.
- For the PS3DK PRX path, use the Linux x86_64 host tools and SDK version documented by the relevant build script or upstream release.
- For hardware validation, use a PS3 development environment with a tested recovery method.

Toolchain installation details change over time. Follow the upstream [ps3dev](https://github.com/ps3dev/ps3dev), [ps3toolchain](https://github.com/ps3dev/ps3toolchain), [PSL1GHT](https://github.com/ps3dev/PSL1GHT), and [PS3DK](https://github.com/PS3-Homebrew/PS3DK) documentation. Verify published checksums before installing release archives.

## Build environment variables

Adapt these paths to the SDK installation on your own machine:

```sh
export PS3DEV=/opt/ps3dev
export PSL1GHT="$PS3DEV"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"
command -v powerpc64-ps3-elf-gcc
```

The compiler name depends on the toolchain. Confirm the selected compiler and SDK version before building. Keep SDK sources, generated files, credentials, and device logs outside tracked project files.

## Build entry points

The `scripts/build-lima-*.sh` scripts are tailored to a configured Linux x86_64 PS3DK environment. Review each script and set its expected SDK variables before use. The scripts build host/PPU artifacts; successful compilation or PRX structural validation does not establish that an artifact can load or run on a PS3.

For a controlled comparison against the PSL1GHT runtime from 2020, `scripts/build-lima-legacy-runtime.sh` requires an isolated PS3DEV bundle, the PSL1GHT source at commit `6e565a70e927f55813babe86dc6f64149535f908`, the PS3DK source tree for its CMake toolchain, and Mbed TLS 3.6.7 source/build paths. It verifies the PSL1GHT commit before rebuilding Mbed TLS and the standalone receiver. Package it separately by setting `APP_VERSION`, `ELF`, and `OUT_DIR` for `scripts/package-lima-standalone-receiver.sh`; do not overwrite a known package or deploy without the project's hardware-test approval gate.

The standalone receiver sources are under `prototype/standalone-receiver`. The experimental resident candidate is under `experimental/resident`. See each directory's README for scope and known limits.

## Validation boundaries

Keep these claims separate: host test, cross-compilation, package/container inspection, PS3 load/start/stop/unload, iPhone discovery/session, and audible output. Record only the evidence actually observed in the [test matrix](TEST_MATRIX.md).

Before any hardware test, review [risks and safety gates](RISKS.md). Do not modify boot plugin settings without a tested recovery procedure and an explicit, reviewed test plan.
