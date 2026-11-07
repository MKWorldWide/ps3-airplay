# Structure-aware ALAC sanitizer coverage

## Timestamp in EDT

2026-11-07 10:36 EDT

## Scope of the pass

Expanded the deterministic ALAC sanitizer smoke test with mutations of the valid reference frame's compressed payload and corrected the RTSP session API comment that contradicted the implemented Apple-Challenge and key-negotiation behavior.

## Why this pass was needed

Unconstrained random packet bytes often fail at the initial frame header, so they provide little exercise of entropy decoding and predictor reconstruction.

## Files changed

- `tests/protocol/raop_audio_pipeline_test.c`
- `src/protocol/raop_session.h`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1036_alac_mutation_corpus.md`

## Architecture / logic changes

The host integration test now keeps the 54-bit fixed ALAC frame header intact and flips 1–8 deterministic bits in the compressed region for each of 4,096 cases. One in four cases is truncated to a bounded random length. The previous 256 unconstrained random packets remain. No production decoding algorithm changed in this pass.

## Runtime behavior changes

No application runtime behavior change. The header comment now accurately describes the existing limited classic RAOP behavior: challenge replies and ANNOUNCE key negotiation are supported when crypto is configured; ALAC decoding is separate.

## Validation performed

- `scripts/build-lima-crypto.sh` in the existing Lima VM.
- Full host crypto, session/SDP, mDNS, persistent RTSP/RTP loopback, capped logging, ALAC, PCM ring/gain, tone and PPU object checks: PASS.
- Structure-aware sanitizer smoke: 4,096 seeded mutations plus 256 random packets, ASan/UBSan halt-on-error: PASS.
- Encrypted RTP-to-PCM reference equality and 353-vs-352 / `UINT32_MAX` regressions: PASS.
- Existing RPCS3 app log inspection: last saved log identifies v0.1.15 and stops at `NET_IP_TIMEOUT`; it is not fresh v0.1.20 evidence. Computer Use reports the desktop locked.

## Immediate results

The deeper mutation corpus completed without sanitizer findings, and the full Lima test/build checks passed. The current package remains v0.1.20; no new PKG was generated because the application decoder code did not change.

## Known limitations / follow-ups

Host-generated mutations do not prove compatibility with an iPhone sender. v0.1.20 still needs an RPCS3 run with a fresh version-matched log and then a controlled iPhone session. No PS3 or audible playback claim.

## Risks introduced

The test runtime is higher than the old 256-input smoke (roughly tens of seconds in the emulated Lima guest). It remains bounded and deterministic. Mutation corpus coverage is not a replacement for a persistent coverage-guided fuzz campaign.

## Decision record (embedded)

### Decision: Mutate a valid fixture while preserving its frame header

- **Context:** Random bytes mostly fail before exercising decoder internals.
- **Options considered:** Increase random-only count; add a third-party fuzzing dependency; mutate a checked-in valid fixture deterministically.
- **Why chosen:** The deterministic fixture-based approach stays dependency-free and is reproducible in the existing sanitizer pipeline.
- **Tradeoffs accepted:** It samples a bounded set of malformed cases and does not replace long-running fuzzing.

### Decision: Do not issue a new package version for test-only expansion

- **Context:** Only test code and an inaccurate API comment changed; production code is unchanged.
- **Options considered:** Rebuild v0.1.21 anyway or reserve the next version for a runtime-relevant application change.
- **Why chosen:** Keep the current validated runtime candidate at v0.1.20 and avoid producing an identical application package.
- **Tradeoffs accepted:** The source branch now contains additional tests beyond the exact committed revision embedded in the candidate's provenance. The candidate remains a valid artifact of its recorded source commit.

## Next recommended step

With the desktop unlocked, install and launch v0.1.20 in RPCS3 and inspect a fresh log. Use it to guide the next application-level fix and assign that package v0.1.21.
