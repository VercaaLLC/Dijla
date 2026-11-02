# Security and data handling

This project is a development preview. Do not use an unsigned, unvalidated build
as the only working copy of business records.

The native application uses a bounded primitive database decoder. It does not
deserialize executable objects, load database macros, download code, create
subprocesses, extract runtime components, or modify Windows security settings.
It opens one local data file with an exclusive writer handle.

Data is not encrypted. Store it on a device and volume with appropriate access
controls. Never attach real inventory databases, invoices, Access files, private
exports, certificates, passwords or tokens to public issues or pull requests.
Use synthetic reproductions. A checksum is not an authenticity signature.

Two internal commit records occupy separate 4 KiB regions. A save writes a new
snapshot away from the current committed snapshot, flushes it, writes the next
checksummed commit record, and flushes again. On reopen, the reader chooses the
newest valid snapshot. Recovery from a damaged snapshot is shown to the user.
Storage hardware, operating system errors and simultaneous copying during writes
can still cause loss. Close the application before transferring or backing it up.

Deletion removes current application records and references. It is not a secure
erasure operation: older internal snapshots and external backups may retain data.

Before a production release:

1. Build the exact reviewed commit with the static-runtime configuration.
2. Pass synthetic storage and business-logic tests, including interrupted writes,
   corruption, item deletion, shared invoices, notification rearming and copying.
3. Verify PE imports, architecture, manifest, DEP/ASLR and file version metadata.
4. Test UI layout, editing, printing and notification behavior on supported OSes.
5. Obtain a publicly trusted signature and validate it and its timestamp.
6. Test the signed artifact on a clean Smart App Control-enabled Windows device.
7. Publish the executable hash and test evidence for the exact signed artifact.

Never ask users to disable protection or trust an unrelated self-signed root.
Code signing confirms publisher and file integrity; it cannot promise that a
program contains no bugs or that every local policy permits it to run.
