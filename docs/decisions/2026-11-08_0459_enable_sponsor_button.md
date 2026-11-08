# Enable and verify the AIR PS3 Sponsor button

**Timestamp:** 2026-11-08 04:59 EDT

## Scope of the pass

Enabled GitHub's repository-level Sponsorships feature and confirmed the Sponsor button on the public `MKWorldWide/ps3-airplay` page. Updated the project memory, lessons learned, and active scratchpad with the result.

## Why this pass was needed

The active EsKaye Sponsors profile and `.github/FUNDING.yml` were already correct, but the repository page lacked the requested Sponsor button.

## Files changed

- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0459_enable_sponsor_button.md`

## Architecture / logic changes

No code or repository funding metadata changed in this pass. The GitHub repository's Sponsorships feature toggle was enabled; with the default-branch funding file, GitHub now renders the repository Sponsor button to EsKaye.

## Runtime behavior changes

Visitors to the repository can now see a Sponsor button and the EsKaye sponsor card. Sponsorship remains optional; the app remains free.

## Validation performed

- Inspected GitHub repository Settings → General → Features and confirmed Sponsorships was initially unchecked.
- Enabled the toggle and confirmed it became checked.
- Opened the public repository page; the header displayed Sponsor, and the page showed the EsKaye sponsor card.
- Confirmed the rendered README support section links to the EsKaye Sponsors profile.
- Captured a screenshot of the public repository page showing the button and card.

## Immediate results

The native Sponsor button is enabled and visible on the default-branch repository page.

## Known limitations / follow-ups

The organization `MKWorldWide` has no Sponsors listing; donations therefore go to the verified personal EsKaye profile. No other funding services are configured.

## Risks introduced

The repository now advertises optional sponsorship. No financial terms, access controls, application behavior, release assets, licensing, or console state were changed.

## Decision record

### Decision: Enable GitHub's native repository Sponsorships feature

- **Context:** `.github/FUNDING.yml` was present on `main`, but the repository UI did not render the Sponsor button.
- **Options considered:** Leave only the README link, troubleshoot the repository feature toggle, or add an external provider.
- **Choice:** Enable the GitHub feature toggle and retain only the EsKaye native Sponsors link.
- **Tradeoffs accepted:** Sponsorship uses the personal developer listing rather than an organization listing; the repository's funding UI is now more prominent, as requested.

## Next recommended step

No further Sponsors work is required. Keep future changes focused on the AIR PS3 engineering roadmap.
