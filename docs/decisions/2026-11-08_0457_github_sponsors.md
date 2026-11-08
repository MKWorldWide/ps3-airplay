# Add native GitHub Sponsors support

**Timestamp:** 2026-11-08 04:57 EDT

## Scope of the pass

Updated the active personal GitHub Sponsors profile with the supplied Short bio and Introduction, confirmed AIR PS3 was already featured as work, and prepared native GitHub funding metadata plus a restrained README support section. Application code and release artifacts are untouched.

## Why this pass was needed

The maintainer requested a direct, optional way for readers to support ongoing AIR PS3 and MKWorldWide work while keeping the software free.

## Files changed

- `.github/FUNDING.yml`
- `README.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0457_github_sponsors.md`

## Architecture / logic changes

`.github/FUNDING.yml` uses the supported native format with only `github: EsKaye`. The README support section links to the same active personal Sponsors listing. The organization is not used as the sponsorable account because its profile is not enabled.

## Runtime behavior changes

The GitHub Sponsors profile now displays the supplied text, and `MKWorldWide/ps3-airplay` remains in its Featured work list. After the funding file reaches `main`, GitHub should render its native Sponsor button on the repository page.

## Validation performed

- Authenticated GitHub GraphQL: viewer `EsKaye`, `hasSponsorsListing=true`, public URL `https://github.com/sponsors/EsKaye`; organization `MKWorldWide`, `hasSponsorsListing=false`.
- Profile GraphQL confirms supplied Short bio, exact Introduction text, and featured repository `MKWorldWide/ps3-airplay`.
- Sponsors dashboard showed “Your profile has been updated”.
- Python YAML parse confirms the funding file is exactly `{github: EsKaye}`.
- README check confirms support section near bottom, direct profile link, optional/free wording, and unchanged MIT license link.
- GitHub Sponsor-button visibility, staged privacy review, commit, and push remain pending.

## Immediate results

The active profile is updated. Local repository changes are uncommitted. No release, application, or console state changed.

## Known limitations / follow-ups

Verify the Sponsor button only after the default-branch funding file is pushed. Confirm the profile displays publicly and the README's direct link resolves. The software remains free; sponsorship does not buy access or features.

## Risks introduced

The supplied bio and introduction are public account content, and the README adds public funding messaging. The user provided the exact profile copy and requested the public repository change. No financial terms or sponsorship tiers were altered.

## Decision record

### Decision: Use only the verified EsKaye Sponsors listing

- **Context:** The authenticated developer account has an active public Sponsors profile; the organization does not.
- **Options considered:** Configure the developer, organization, or external funding providers.
- **Choice:** Use only `EsKaye`, matching the active listing URL; feature AIR PS3 on that profile.
- **Tradeoffs accepted:** Sponsorship benefits accrue through the personal listing rather than an organization listing. The requested branding identifies MKWorldWide in the profile copy.

## Next recommended step

Run final staged-diff/privacy checks, commit and push the two public-facing changes plus required project notes to `main`, and verify the Sponsor button on the live repository.
