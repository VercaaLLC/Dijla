# Dijla — دجلة

Dijla is an Arabic Windows desktop inventory application. It manages items,
groups, recipients, incoming and outgoing invoices, stock balances, and reports.

**Development preview. It is not yet a signed or validated production release.**
Windows 7 through Windows 11 are compatibility targets; passing a build on a
recent Windows runner does not establish compatibility with each target system.

## Two portable files

The working folder contains:

- `Dijla.exe` — the application, with no installer or extracted runtime.
- `Dijla.data` — all records and settings in one uncompressed file.

Close Dijla and copy both files together to transfer a complete working copy.
There is no account, login, cloud service, or online activation in the application.
The executable never changes when records are saved. Keep dated copies of both
files as backups before destructive changes.

The public distribution starts with an empty data file. Customer databases,
original Access files, migration exports, and signing credentials are excluded
from this repository and from automated builds.

## Features

- Arabic item, group, recipient and invoice screens.
- Item deletion removes its invoice lines, units, notifications, and related
  item cache records. Shared invoices retain other items. Empty affected
  invoices are removed; shared groups and recipients remain.
- A dedicated low-stock window for meter-based items at **30 meters or less**.
  Replenishing above 30 rearms the threshold notification. Piece-based items
  are excluded from the meter threshold.
- Inventory as of a date, incoming/outgoing transactions, and item movement
  reports, with printing and optional CSV export.
- Internal checksummed snapshots for interrupted-write recovery, without
  external journal or lock files. Only one writer may open a data file.

## Build and test

Use Visual Studio 2022 with the **v142 x86 C++ toolset** and Windows SDK
**10.0.19041.0**, plus CMake 3.24 or later. These are developer tools; they are
not required on computers running Dijla.

```text
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 -T v142 -DCMAKE_SYSTEM_VERSION=10.0.19041.0
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tools/verify_pe.py build/Release/Dijla.exe
```

The release configuration statically links Microsoft's C/C++ runtime with `/MT`.
The dependency check rejects external CRT DLLs and unsupported PE targets.
Tests generate only synthetic records in a disposable build directory.
Create an empty distribution data file with the developer test utility:

```text
build\Release\DijlaTests.exe --create-empty build\Release\Dijla.data
```

Do not replace an existing user's `Dijla.data` with that blank file when updating
the application. The data reader retains compatible imported tables, including
legacy tables that the current interface does not display.

## Code signing policy

Trusted code signing is **not configured or approved yet**. The project intends
to apply to [SignPath Foundation](https://signpath.org/) for free signing after
it meets the foundation's eligibility and reputation requirements.
There is no claim of affiliation, approval, or a warning-free release.

Only binaries built from this public source may be submitted for signing.
Business data must never be submitted. Releases require maintainer review and
manual signing approval. See [SIGNING.md](SIGNING.md) for the proposed process.

## Privacy and security

The application does not upload records, check for updates, collect analytics,
or modify Windows security settings. Printing and CSV export are initiated by
the user. Data stays in the selected local working folder.

Checksums detect damage; they do not encrypt the database or authenticate its
author. Anyone with access to a data file can read its contents. Internal
recovery snapshots and external backups can retain older records after deletion.
See [SECURITY.md](SECURITY.md) for reporting and release checks.

## Layout and validation status

The main menu and item window preserve the inspected legacy layout and button
positions with updated colors and standard Windows controls. Some original
transaction and report screens could not be inspected because of legacy
OLE/ActiveX errors. Exact visual parity for those screens is still unverified.

Native runtime tests, visual checks, trusted launch validation, and the complete
Windows compatibility matrix must pass before a production release.

## License

New application source: [MIT](LICENSE). Windows and compiler runtime components
remain subject to their respective licenses; this repository does not bundle
their source or binaries.
