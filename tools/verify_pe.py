"""Inspect the release without executing it. Reject external runtime dependencies."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

def inspect(path):
    data=Path(path).read_bytes()
    pe=struct.unpack_from('<I',data,0x3c)[0]
    assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0', 'Not a PE image'
    machine,count,_,_,_,optional_size,_=struct.unpack_from('<HHIIIHH',data,pe+4)
    opt=pe+24
    assert machine==0x14c and struct.unpack_from('<H',data,opt)[0]==0x10b, 'Expected x86 PE32'
    sections=[]
    for index in range(count):
        pos=opt+optional_size+index*40
        virtual_size,address,raw_size,raw=struct.unpack_from('<IIII',data,pos+8)
        sections.append((address,max(virtual_size,raw_size),raw))
    def offset(rva):
        for address,size,raw in sections:
            if address<=rva<address+size:return raw+rva-address
        raise ValueError('Invalid RVA '+hex(rva))
    def string(pos):return data[pos:data.index(b'\0',pos)].decode('ascii')
    def imports(rva,delay=False):
        found={}
        if not rva:return found
        pos=offset(rva)
        while True:
            if delay:
                record=struct.unpack_from('<8I',data,pos)
                if not any(record):break
                attributes,name,_,first,original,*_=record
                assert attributes&1, 'Unexpected legacy delay-import address format'
                step=32
            else:
                record=struct.unpack_from('<5I',data,pos)
                if not any(record):break
                original,_,_,name,first=record
                step=20
            dll=string(offset(name));symbols=[];thunk=offset(original or first)
            while True:
                symbol=struct.unpack_from('<I',data,thunk)[0]
                if not symbol:break
                symbols.append('ordinal:'+str(symbol&0xffff) if symbol&0x80000000 else string(offset(symbol)+2))
                thunk+=4
            found[dll]=symbols;pos+=step
        return found
    normal=imports(struct.unpack_from('<I',data,opt+104)[0])
    delayed=imports(struct.unpack_from('<I',data,opt+96+13*8)[0],True)
    allowed={'kernel32.dll','user32.dll','gdi32.dll','comctl32.dll','comdlg32.dll','bcrypt.dll','shell32.dll','advapi32.dll','ntdll.dll','ole32.dll','oleaut32.dll','version.dll','winmm.dll'}
    unexpected=sorted(d for d in set(normal)|set(delayed) if d.lower() not in allowed)
    assert not unexpected, 'External runtime or unreviewed DLLs: '+str(unexpected)
    post_win7={'GetSystemTimePreciseAsFileTime','SetThreadDescription','GetDpiForWindow','GetDpiForSystem','WaitOnAddress','WakeByAddressSingle','WakeByAddressAll','GetSystemCpuSetInformation','GetTempPath2W','GetTempPath2A'}
    newer=sorted({symbol for table in [normal,delayed] for symbols in table.values() for symbol in symbols if symbol in post_win7})
    assert not newer, 'Direct imports newer than Windows 7: '+str(newer)
    subsystem=struct.unpack_from('<H',data,opt+68)[0]
    version=struct.unpack_from('<HH',data,opt+48)
    flags=struct.unpack_from('<H',data,opt+70)[0]
    assert subsystem==2 and version<=(6,1), 'Unexpected subsystem target'
    assert flags&0x40 and flags&0x100, 'DEP/ASLR missing'
    assert struct.unpack_from('<I',data,opt+112)[0], 'Missing manifest/version resources'
    return {'sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data),'machine':'x86','subsystem_version':list(version),'ASLR':True,'DEP':True,'imports':normal,'delay_imports':delayed,'static_checks':'PASS','limitations':'Static inspection does not establish actual Windows compatibility or trusted launch.'}

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('exe');parser.add_argument('--out');args=parser.parse_args()
    result=inspect(args.exe);report=json.dumps(result,indent=2)
    if args.out:Path(args.out).write_text(report,encoding='utf-8')
    print(report)
