"""Fail before publication if tracked files fall outside the reviewed allowlist."""
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
ALLOWED={
    '.gitignore','.gitattributes','CMakeLists.txt','LICENSE','README.md','SECURITY.md','SIGNING.md',
    '.github/workflows/build.yml',
    'src/data.hpp','src/data.cpp','src/ui.cpp','src/app.rc','src/app.manifest',
    'tests/core_tests.cpp','tools/verify_pe.py','tools/check_public_tree.py',
}
def check():
    run=subprocess.run(['git','ls-files','-z'],cwd=ROOT,capture_output=True,check=True)
    paths={p for p in run.stdout.decode('utf-8').split('\0') if p}
    assert paths, 'No tracked files; stage the explicit source allowlist first.'
    assert not paths-ALLOWED, 'Unreviewed tracked paths: '+str(sorted(paths-ALLOWED))
    assert not ALLOWED-paths, 'Expected source missing: '+str(sorted(ALLOWED-paths))
    forbidden=[b'DIJLADB1',b'DIJLA2DB\x00',b'BEGIN PRIVATE KEY',b'BEGIN RSA PRIVATE KEY',b'ghp_',b'github_pat_']
    for name in paths:
        path=ROOT/name
        assert not path.is_symlink(), 'Do not publish linked files: '+name
        raw=path.read_bytes()
        assert len(raw)<200000, 'Unexpected large source: '+name
        raw.decode('utf-8')
        if name!='tools/check_public_tree.py':
            for marker in forbidden:assert marker not in raw, 'Potential secret or private payload marker: '+name
    print('PASS: '+str(len(paths))+' source/documentation files; no databases, exports, binaries or keys are tracked.')

if __name__=='__main__':check()
