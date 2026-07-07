# DS Native Binary Size Report Design

## Goal

Add a reusable native binary size report seam that the Nintendo DS builder can run after native compilation so we can see what the final linked ROM is made of without relying on raw object file guesses.

## Why This Exists

The current DS optimization work has one hard limitation: we can measure the final `.nds` size, but we do not have one builder-owned report that explains which linked native objects and sections consume that size. Raw `.o` file sizes overstate many contributors because the linker strips dead sections. We need one report that reflects what the linker actually kept.

## Requirements

- The first pass must be binary-only.
- The report must describe native code/data, not packaged assets.
- The report must come from linked build artifacts, not only raw `.o` file sizes.
- The DS builder must emit the report automatically as part of the native build output.
- The reporting code should use generic naming and boundaries so other platforms can adopt the same seam later.
- The first pass must work with the existing GNU linker map output already produced by the DS Makefile.

## Non-Goals

- Reporting NitroFS or cooked asset sizes.
- Building a cross-platform report UI in the editor.
- Changing DS optimization behavior in the same slice.
- Replacing the existing linker map or native build logs.

## Existing Constraints

- The DS Makefile already emits `helengine_ds.map`, `helengine_ds.elf`, and `helengine_ds.nds`.
- The builder already stages native build logs and copies the final `.nds` package to the output root.
- The report must fit the repository conventions: one class per file, XML comments on members, and small focused tests.

## Proposed Approach

Add one builder-side native binary size report service that reads the linked `.map`, `.elf`, and `.nds` outputs after the DS native build completes.

The service will:

1. Read the final `.nds` size from disk.
2. Read the linked `.elf` size from disk.
3. Parse the GNU linker map and aggregate linked section contribution by object file.
4. Normalize detailed linker section names such as `.text.foo` into stable root buckets such as `.text`, `.rodata`, `.data`, `.bss`, `.ARM.exidx`, and `.ARM.extab`.
5. Emit one human-readable text report into the DS build output.

## Output Shape

The first report should contain:

- package size in bytes
- linked ELF size in bytes
- total linked bytes accounted for by parsed map sections
- section totals grouped by normalized section root
- top object contributors sorted by linked bytes
- per-object breakdown for the main normalized sections

This gives an actionable answer to “what are the 2 MB?” while staying tied to the linked binary.

## Architecture

### Workspace paths

Extend the DS build workspace to expose the repository-native `.map` and `.elf` artifact paths plus the exported text report path.

### Parsing and aggregation

Add one generic-named parser/report writer in the DS builder assembly:

- parse GNU linker map lines that include section, address, size, and source object
- aggregate per-section totals
- aggregate per-object totals
- format a deterministic text report for logs and diffs

### Native build integration

After the DS native build succeeds and the final `.nds` is copied to the output root, run the report writer and store the text file beside the exported package.

## Testing Strategy

- Add workspace tests that lock the new artifact/report paths.
- Add report parser tests using a small synthetic linker map fixture so the aggregation logic stays deterministic.
- Run focused DS builder tests.
- Run one real DS build and inspect the generated report from the `city` output root.

## Recommendation

Implement the DS report first, but keep the parser/report types free of DS-specific naming so other native platform builders can reuse the same seam later.
