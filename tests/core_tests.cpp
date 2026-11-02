// SPDX-License-Identifier: MIT
#include "data.hpp"
#include <cstdio>
#include <cmath>
#include <algorithm>

using namespace dijla;
namespace {
int checks=0;
void check(bool result,const char* label){require(result,label);printf("PASS %s\n",label);++checks;}
template<class F> void rejects(F fn,const char* label){bool failed=false;try{fn();}catch(const std::exception&){failed=true;}check(failed,label);}
Bytes read(const std::wstring& path){
    HANDLE h=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    require(h!=INVALID_HANDLE_VALUE,"Test read failed.");
    LARGE_INTEGER size;GetFileSizeEx(h,&size);Bytes b((size_t)size.QuadPart);DWORD n=0;
    bool ok=ReadFile(h,b.data(),(DWORD)b.size(),&n,nullptr)&&n==b.size();CloseHandle(h);require(ok,"Test read was short.");return b;
}
void write(const std::wstring& path,const Bytes& data){
    HANDLE h=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);
    require(h!=INVALID_HANDLE_VALUE,"Test write failed.");DWORD n=0;
    bool ok=WriteFile(h,data.data(),(DWORD)data.size(),&n,nullptr)&&n==data.size()&&FlushFileBuffers(h);
    CloseHandle(h);require(ok,"Test write was short.");
}
uint64_t u64(const Bytes& b,size_t p){uint64_t v=0;for(int i=0;i<8;i++)v|=uint64_t(b.at(p+i))<<(8*i);return v;}
void make(const std::wstring& path){DataFile f(path,true);f.save(Database::empty());}
double balance(Store& s,Id id){return s.t("item").at(*s.find("item","item_no",id),"item_bal").d;}
size_t files(const std::wstring& dir){WIN32_FIND_DATAW info;HANDLE h=FindFirstFileW((dir+L"\\*").c_str(),&info);size_t n=0;if(h!=INVALID_HANDLE_VALUE){do{if(!(info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))n++;}while(FindNextFileW(h,&info));FindClose(h);}return n;}
}
int wmain(int argc,wchar_t** argv){try{
    if(argc==3&&std::wstring(argv[1])==L"--create-empty"){
        make(argv[2]);puts("Created an empty data file; no customer data included.");return 0;
    }
    require(argc==3,"Usage: DijlaTests <test-directory> <Dijla.exe>");
    std::wstring parent=argv[1],exe=argv[2];CreateDirectoryW(parent.c_str(),nullptr);
    std::wstring root=parent+L"\\case-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount());
    require(CreateDirectoryW(root.c_str(),nullptr),"Could not create disposable test directory.");
    std::wstring path=root+L"\\Dijla.data";make(path);
    Bytes exeBefore=read(exe),saved;
    Id a=0,b=0,g=0,p=0,shared=0,empty=0;
    {
        DataFile f(path);Store s(f);
        check(s.t("item").rows.empty(),"blank distribution has no customer records");
        g=s.saveReference("group",0,"مجموعة تجريبية");
        p=s.saveReference("person",0,"مستلم تجريبي");
        a=s.saveItem(0,"A-1","مادة اختبار أ",g,31,"m");
        b=s.saveItem(0,"B-1","مادة اختبار ب",g,100,"m");
        auto codec=s.db.encode();check(Database::decode(codec).encode()==codec,"primitive codec preserves exact cells and Arabic text");
        check(s.lowCount()==0,"stock above threshold is excluded");
        check(parseNumber(L"١٢٫٥")==12.5,"Arabic digits and decimal separator");
        rejects([&]{parseNumber(L"NaN");},"non-finite quantity rejected");
        rejects([&]{parseNumber(L"12abc");},"invalid numeric suffix rejected");
        rejects([&]{s.saveDocument("sale",0,"BAD","2026-02-30",p,"",{{a,1}});},"impossible calendar date rejected");
        rejects([&]{s.saveDocument("sale",0,"BAD","2026-aa-01",p,"",{{a,1}});},"nonnumeric date rejected");
        shared=s.saveDocument("sale",0,"S-1","2026-01-02",p,"اختبار",{{a,1},{b,2}});
        check(balance(s,a)==30&&balance(s,b)==98,"stock balances follow invoice quantities");
        check(s.lowCount()==1&&s.unread()==1,"31 to 30 creates one low-stock notification");
        empty=s.saveDocument("sale",0,"S-2","2026-01-03",p,"",{{a,1}});
        check(s.unread()==1,"remaining low does not repeatedly notify");
        Id receipt=s.saveDocument("shop",0,"P-1","2026-01-04",p,"",{{a,9}});
        check(balance(s,a)==38,"replenishment raises stock above threshold");
        Id again=s.saveDocument("sale",0,"S-3","2026-01-05",p,"",{{a,8}});
        check(balance(s,a)==30&&s.unread()==2,"replenishment rearms notification");
        s.saveDocument("sale",again,"S-3","2026-01-05",p,"edited",{{a,5}});
        check(balance(s,a)==33,"invoice edit restores replaced quantities");
        s.deleteDocument("sale",again);
        check(balance(s,a)==38,"invoice deletion updates stock");
        auto inventory=s.inventory("2099-12-31");auto where=std::find(inventory.ids.begin(),inventory.ids.end(),a);
        check(where!=inventory.ids.end()&&inventory.rows[where-inventory.ids.begin()][3]==L"38","inventory report agrees with balance");
        auto movement=s.movement(a,"2026-01-01","2026-12-31");
        check(movement.rows.back().back()==L"38","movement report running balance agrees");
        auto beforePeriod=s.movement(a,"2026-01-04","2026-12-31");
        check(beforePeriod.rows.front().back()==L"29","movement report opening includes prior movements");
        check(s.transactions("sale","2026-01-01","2026-01-02",p,b,g).rows.size()==1,"transaction report filters date, person, item and group");
        s.acknowledge();check(s.unread()==0&&s.lowCount()==0,"acknowledgement changes read state only");
        s.saveItem(b,"B-1","مادة اختبار ب",g,20,"piece");
        check(balance(s,b)==18&&!s.low(*s.find("item","item_no",b)),"piece quantities excluded from meter alerts");
        auto before=s.db.encode();f.failBeforeCommit=1;
        rejects([&]{s.saveReference("group",0,"Uncommitted");},"injected interruption before commit reports failure");
        f.failBeforeCommit=0;check(s.db.encode()==before,"failed commit rolls memory back");
        check(f.load().encode()==before,"failed commit preserves last committed disk state");
        rejects([&]{DataFile second(path);},"second writer rejected without sidecar lock file");
        rejects([&]{s.deleteReference("group",g);},"in-use group deletion rejected");
        rejects([&]{s.deleteReference("person",p);},"in-use recipient deletion rejected");
        check(s.db.encode()==before,"rejected reference deletion leaves all data unchanged");
        s.transaction([&]{
            s.db.tables.push_back({"legacy_app_item_jrd",{{"item_no",1},{"SumOfbal",2}},{{Value::integer(a),Value::real(38)},{Value::integer(b),Value::real(18)}}});
            s.db.tables.push_back({"legacy_app_excel1",{{"اسم المادة",0}},{{Value::text("مادة اختبار أ")},{Value::text("مادة اختبار ب")}}});
        });
        double bBefore=balance(s,b);s.deleteItem(a);
        check(!s.find("item","item_no",a)&&s.relatedLines(a)==0,"item and every live invoice line removed");
        auto remaining=s.lines("sale",shared);
        check(s.find("sale","shop_no",shared)&&remaining.size()==1&&remaining[0].item==b&&remaining[0].qty==2,"shared invoice retains B and its quantity");
        check(!s.find("sale","shop_no",empty)&&!s.find("shop","shop_no",receipt),"invoices emptied by item deletion removed");
        check(s.find("person","p_no",p)&&s.find("group","group_no",g),"shared recipients and groups retained");
        check(balance(s,b)==bBefore,"unrelated item balance unchanged");
        bool noRefs=true;for(auto& tab:s.db.tables)if(tab.has("item_no"))for(auto& r:tab.rows)if(tab.at(r,"item_no").i==a)noRefs=false;
        check(noRefs,"item references cleared from legacy caches, units and notifications");
        check(s.t("legacy_app_excel1").rows.size()==1,"legacy display cache removes deleted item only");
        saved=s.db.encode();
    }
    {DataFile f(path);Store s(f);check(s.db.encode()==saved,"all committed changes survive reopen");}
    check(files(root)==1,"saving creates only one data file");
    check(read(exe)==exeBefore,"executable bytes unchanged by all data saves");
    {
        auto missing=root+L"\\missing.data";
        rejects([&]{DataFile f(missing);},"missing data fails explicitly");
        check(GetFileAttributesW(missing.c_str())==INVALID_FILE_ATTRIBUTES,"missing data is never silently replaced with empty data");
    }
    std::wstring copyDir=root+L"\\نقل نسخة";require(CreateDirectoryW(copyDir.c_str(),nullptr),"Could not create copy directory.");
    require(CopyFileW(exe.c_str(),(copyDir+L"\\Dijla.exe").c_str(),TRUE)&&CopyFileW(path.c_str(),(copyDir+L"\\Dijla.data").c_str(),TRUE),"Copy failed.");
    {DataFile f(copyDir+L"\\Dijla.data");Store s(f);check(s.db.encode()==saved,"copy to Arabic path preserves complete dataset");s.saveReference("group",0,"Copied independently");}
    check(files(copyDir)==2,"transferred folder stays exactly EXE plus DATA");
    {DataFile f(path);Store s(f);check(s.db.encode()==saved,"saving copied data leaves original unchanged");}
    auto corrupt=root+L"\\corrupt.data";auto bytes=read(path);auto gen0=u64(bytes,8),gen1=u64(bytes,4104);size_t newest=gen0>gen1?0:4096;size_t payload=(size_t)u64(bytes,newest+16);bytes[payload]^=0x40;write(corrupt,bytes);
    {DataFile f(corrupt);auto recovered=f.load();recovered.validate();check(f.recovered,"corrupted newest payload recovers prior complete commit with warning");}
    bytes[0]^=0x40;bytes[4096]^=0x40;write(corrupt,bytes);
    rejects([&]{DataFile f(corrupt);f.load();},"two damaged commits rejected");
    auto truncated=Database::empty().encode();truncated.pop_back();
    rejects([&]{Database::decode(truncated);},"truncated primitive payload rejected");
    auto malformed=Database::empty().encode();malformed.push_back(0);
    rejects([&]{Database::decode(malformed);},"trailing payload rejected");
    {DataFile f(path);rejects([&]{f.save(Database::empty());},"saving unopened existing data rejected");}
    printf("TOTAL %d checks\n",checks);return 0;
}catch(const std::exception& e){fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
