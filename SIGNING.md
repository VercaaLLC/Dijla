# Proposed free code-signing process

Status: **application submitted on 2 November 2026; approval pending. Not signed.**

The application identifies Dijla as a new public project, discloses its limited
reputation and preview-only download, and requests an eligibility assessment.
Submission is not acceptance and creates no claim of provider affiliation.

The intended provider is [SignPath Foundation](https://signpath.org/), subject to
its [conditions](https://signpath.org/terms.html). Approval is discretionary and
requires verifiable project reputation; publishing a new repository does not
automatically qualify it.

## Maintainer responsibilities

- Maintain an OSI-approved open-source license and a public source repository.
- Enable multi-factor authentication on the repository and signing accounts.
- Define contributors, reviewers and signing approvers before application.
- Document releases, privacy behavior and user-visible functionality accurately.
- Build releases from reviewed commits through a verifiable public CI pipeline.
- Approve signing requests manually. Never expose signing secrets to pull-request
  jobs or submit binaries produced by an untrusted or unreviewed branch.

The repository owner, [VercaaLLC](https://github.com/VercaaLLC), is responsible
for authorship and review and is the proposed signing approver. Signing roles
have not been provisioned by SignPath. Additional contributors need a documented
review path.

## Submission boundary

Only `Dijla.exe` is submitted for signing. A signature is embedded in that file.
`Dijla.data` is never sent to SignPath, stored in a public build artifact from a
customer installation, or included in repository history.

The build creates a fresh empty data file for public testing. Imported business
data is migrated and verified privately on the owner's computer.

## Application preparation

The application needs a real public repository and download/release page,
accurate project description, maintainer contact, documented license and build
instructions, and evidence of maintenance and reputation. Do not invent users,
downloads, security reviews or test results to satisfy eligibility.

Describe Dijla as an Arabic native desktop inventory application with a stable
executable and one local data file. It has no telemetry, online login, updater,
runtime extraction or system-security modification. Its native validation status
must be stated as it actually stands when the application is sent.

After approval, add the provider's required attribution, assign the actual team
roles, configure the verified build origin and artifact rules, and pin the
official signing action to a reviewed commit. Do not claim free signing is
provided before the project has actually been accepted.

The public source remains useful if approval is declined. Keep the release
unsigned and clearly marked as such; do not weaken Windows security to hide it.
