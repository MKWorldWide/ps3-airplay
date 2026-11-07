# RAOP DNS-SD discovery groundwork

`src/discovery/raop_mdns.c` serializes a minimal authoritative mDNS response
containing the DNS-SD PTR, SRV, TXT, A, and service-enumeration PTR records for
one `_raop._tcp.local` service. The host-side test checks the record layout,
target names, advertised port, address, and buffer/input bounds.

This is packet-format groundwork only. It does not open a UDP socket, join
224.0.0.251, answer queries, advertise `_airplay._tcp`, select truthful RAOP
capabilities, or accept RTSP/RTP sessions. Do not enable LAN advertisements
until a working RTSP listener and supported TXT capabilities exist; otherwise
the iPhone could show a receiver that cannot play audio.

The current upstream `azarucha/sys-airplay` mDNS implementation was reviewed as
a protocol reference. Its device-specific TXT values are not copied into this
packet builder because the PS3 receiver capabilities have not yet been proven.

Run the current host validation from the repository root:

```sh
clang -std=c99 -Wall -Wextra -Werror -pedantic \
  tests/discovery/raop_mdns_test.c src/discovery/raop_mdns.c \
  -o /tmp/raop-mdns-test
/tmp/raop-mdns-test
```
