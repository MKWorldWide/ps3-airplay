# Third-party notices

## AirPort Express RAOP compatibility key

`src/crypto/airport_private_key.h` contains the well-known RSA private key
extracted from Apple's AirPort Express and published for legacy RAOP receiver
interoperability. It is not a project, maintainer, console, or account
credential; the same protocol key is present in established open-source RAOP
receivers such as [Shairport Sync](https://github.com/mikebrady/shairport-sync)
and is discussed in the [OpenAirPlay 1 design notes](https://github.com/st3fan/openairplay1/blob/main/notes/design.md).
It is third-party material and is not covered by this repository's MIT license.

The repository's initial public commit already included this compatibility
key. Removing it from the current tree would not erase that historical copy.

## `azarucha/sys-airplay` mDNS reference

The RAOP DNS-SD record layout in `src/discovery/raop_mdns.c` was informed by
the MIT-licensed [sys-airplay project](https://github.com/azarucha/sys-airplay),
particularly its `sysmodule/source/mdns.cpp` implementation. No platform-specific
Switch networking or audio code is used here.

```text
MIT License

Copyright (c) 2026 Alexis Zarucha

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## AirPort Express receiver RSA key data

`src/crypto/airport_private_key.h` embeds the well-known AirPort Express RAOP
receiver key as data. The key copy and provenance note are from the MIT-licensed
[OpenAirPlay 1 receiver](https://github.com/st3fan/openairplay1), whose source
identifies the key as originating in `shairport-sync`'s `common.c`. No receiver
implementation code was copied.

```text
MIT License

Copyright (c) 2026 Stefan Arentz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Mbed TLS build dependency

The Lima crypto build uses the external Mbed TLS 3.6.7 source release for the
`mbedcrypto` host and PS3 PPU libraries. The source is downloaded and SHA-256
verified by `scripts/build-lima-crypto.sh`; it is not copied into this
repository. Mbed TLS is dual-licensed under Apache-2.0 or GPL-2.0-or-later;
see the [upstream license](https://github.com/Mbed-TLS/mbedtls/blob/mbedtls-3.6.7/LICENSE)
and [3.6.7 release](https://github.com/Mbed-TLS/mbedtls/releases/tag/mbedtls-3.6.7).

## Hammerton ALAC decoder

`src/codec/alac.c` and `src/codec/alac.h` contain David Hammerton's 2005 C
ALAC decoder as retained in Shairport Sync at commit
`0a34fbda83724cc06266aadcf7c88acc8fef4e7d`. The project wrapper and a
`sign_extend32` correction are maintained in `src/codec/raop_alac.c` and
`src/codec/alac.c`. Upstream file headers carry this notice:

```text
Copyright (c) 2005 David Hammerton
All rights reserved.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
```
```
