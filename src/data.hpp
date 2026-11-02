// SPDX-License-Identifier: MIT
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <functional>
#include <stdexcept>
namespace dijla {
using Bytes=std::vector<unsigned char>;
using Id=int64_t;
std::wstring wide(const std::string& s);
std::string utf8(const std::wstring& s);
std::string number(double x);
std::string today();
std::string now();
double parseNumber(const std::wstring& s);
void require(bool ok,const std::string& text);
struct Value {
    bool present=false; Id i=0; double d=0; std::string s;
    static Value integer(Id v){Value x;x.present=true;x.i=v;return x;}
    static Value real(double v){Value x;x.present=true;x.d=v;return x;}
    static Value text(std::string v){Value x;x.present=true;x.s=std::move(v);return x;}
};
using Row=std::vector<Value>;
struct Column{std::string name;uint8_t kind;};
struct Table {
    std::string name;std::vector<Column> cols;std::vector<Row> rows;
    size_t col(const std::string& n)const;
    bool has(const std::string& n)const;
    Value& at(Row& r,const std::string& n){return r.at(col(n));}
    const Value& at(const Row& r,const std::string& n)const{return r.at(col(n));}
    Row* find(const std::string& key,Id id);
    const Row* find(const std::string& key,Id id)const;
    Id next(const std::string& key)const;
    void remove(const std::string& key,Id id);
};
struct Database {
    std::vector<Table> tables;
    Table& t(const std::string& name);
    const Table& t(const std::string& name)const;
    Bytes encode()const;
    static Database decode(const Bytes& raw);
    static Database empty();
    void validate()const;
};
std::array<unsigned char,32> hash(const Bytes& b);
struct Slot{uint64_t generation=0,offset=0,length=0,capacity=0;std::array<unsigned char,32> digest{};};
class DataFile {
    HANDLE file=INVALID_HANDLE_VALUE;Slot slots[2];int active=-1;bool fresh=false;
    Bytes read(uint64_t pos,size_t n);
    void write(uint64_t pos,const Bytes& b);
    void flush();
public:
    std::wstring path;bool recovered=false,uncertain=false;
    int failBeforeCommit=0; // Used only by the test harness; never changed by the application.
    explicit DataFile(const std::wstring& name,bool create=false);
    ~DataFile();
    DataFile(const DataFile&)=delete;DataFile& operator=(const DataFile&)=delete;
    Database load();
    void save(const Database& db);
    uint64_t generation()const{return active<0?0:slots[active].generation;}
};
struct Line{Id item;double qty;Id identity=0;};
struct Result{std::vector<std::wstring> columns;std::vector<std::vector<std::wstring>> rows;std::vector<Id> ids;};
class Store {
    DataFile& file;
    void audit(std::string action,std::string object,Id id,std::string details="");
    void notify(Row& item,bool wasLow);
public:
    Database db;
    explicit Store(DataFile& f):file(f),db(f.load()){db.validate();}
    Table& t(const std::string& name){return db.t(name);}
    const Table& t(const std::string& name)const{return db.t(name);}
    Row* find(const std::string& table,const std::string& key,Id id){return t(table).find(key,id);}
    const Row* find(const std::string& table,const std::string& key,Id id)const{return t(table).find(key,id);}
    std::string name(const std::string& table,const std::string& key,Id id,const std::string& col)const;
    std::string unit(Id id)const;
    bool low(const Row& r)const;
    int lowCount()const;
    int unread()const;
    void acknowledge();
    void transaction(const std::function<void()>& fn);
    void recalculate(Id id);
    Id saveItem(Id id,std::string code,std::string name,Id group,double opening,std::string unit);
    Id saveReference(std::string kind,Id id,std::string name);
    void deleteReference(std::string kind,Id id);
    int relatedLines(Id id)const;
    void deleteItem(Id id);
    std::vector<Line> lines(std::string kind,Id id)const;
    Id saveDocument(std::string kind,Id id,std::string num,std::string date,Id person,std::string note,const std::vector<Line>& lines);
    void deleteDocument(std::string kind,Id id);
    Result inventory(std::string until,Id group=0)const;
    Result transactions(std::string kind,std::string from,std::string until,Id person=0,Id item=0,Id group=0)const;
    Result movement(Id item,std::string from,std::string until)const;
};
}
