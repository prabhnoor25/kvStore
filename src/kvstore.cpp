#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>
#include <sstream>
#include <vector>
#include <list>
#include <fstream>

using namespace std;
using Clock = chrono::steady_clock;

struct Entry {
    string type; // string or list
    string value;
    list<string> list_value;
    long long expiry;
};

class KVStore {

private:

    unordered_map<string, Entry> store;
    int expired_cleaned = 0;
    int max_keys = 1000;

    long long now() {
        return chrono::duration_cast<chrono::seconds>(
            Clock::now().time_since_epoch()).count();
    }

public:

    void cleanup() {

        long long current = now();

        for(auto it = store.begin(); it != store.end();) {

            if(it->second.expiry != -1 && current > it->second.expiry) {

                it = store.erase(it);
                expired_cleaned++;

            } else {
                ++it;
            }
        }
    }

    void evict_if_needed() {

        if(store.size() <= max_keys)
            return;

        auto it = store.begin();
        store.erase(it);
    }

    void set(string key, string value, int ttl) {

        cleanup();

        Entry e;
        e.type = "string";
        e.value = value;

        if(ttl > 0)
            e.expiry = now() + ttl;
        else
            e.expiry = -1;

        store[key] = e;

        evict_if_needed();

        cout << "OK" << endl;
    }

    void get(string key) {

        cleanup();

        if(store.find(key) == store.end()) {
            cout << "(nil)" << endl;
            return;
        }

        Entry &e = store[key];

        if(e.type != "string") {
            cout << "(wrong type)" << endl;
            return;
        }

        cout << e.value << endl;
    }

    void del(string key) {

        cleanup();

        if(store.erase(key))
            cout << "OK" << endl;
        else
            cout << "(nil)" << endl;
    }

    void ttl(string key) {

        cleanup();

        if(store.find(key) == store.end()) {
            cout << -2 << endl;
            return;
        }

        Entry &e = store[key];

        if(e.expiry == -1) {
            cout << -1 << endl;
            return;
        }

        long long remaining = e.expiry - now();

        if(remaining < 0)
            remaining = 0;

        cout << remaining << endl;
    }

    void incr(string key) {

        cleanup();

        if(store.find(key) == store.end()) {

            set(key,"1",-1);
            return;
        }

        Entry &e = store[key];

        int val = stoi(e.value);
        val++;

        e.value = to_string(val);

        cout << val << endl;
    }

    void decr(string key) {

        cleanup();

        if(store.find(key) == store.end()) {

            set(key,"-1",-1);
            return;
        }

        Entry &e = store[key];

        int val = stoi(e.value);
        val--;

        e.value = to_string(val);

        cout << val << endl;
    }

    void lpush(string key,string value) {

        cleanup();

        Entry &e = store[key];

        if(e.type == "") {
            e.type = "list";
            e.expiry = -1;
        }

        if(e.type != "list") {
            cout << "(wrong type)" << endl;
            return;
        }

        e.list_value.push_front(value);

        cout << e.list_value.size() << endl;
    }

    void rpush(string key,string value) {

        cleanup();

        Entry &e = store[key];

        if(e.type == "") {
            e.type = "list";
            e.expiry = -1;
        }

        if(e.type != "list") {
            cout << "(wrong type)" << endl;
            return;
        }

        e.list_value.push_back(value);

        cout << e.list_value.size() << endl;
    }

    void lpop(string key) {

        cleanup();

        if(store.find(key) == store.end()) {
            cout << "(nil)" << endl;
            return;
        }

        Entry &e = store[key];

        if(e.list_value.empty()) {
            cout << "(nil)" << endl;
            return;
        }

        cout << e.list_value.front() << endl;

        e.list_value.pop_front();
    }

    void rpop(string key) {

        cleanup();

        if(store.find(key) == store.end()) {
            cout << "(nil)" << endl;
            return;
        }

        Entry &e = store[key];

        if(e.list_value.empty()) {
            cout << "(nil)" << endl;
            return;
        }

        cout << e.list_value.back() << endl;

        e.list_value.pop_back();
    }

    void save(string filename) {

        ofstream file(filename);

        for(auto &p : store) {

            file << p.first << " " << p.second.type << " ";

            if(p.second.type == "string")
                file << p.second.value;

            file << endl;
        }

        file.close();

        cout << "OK" << endl;
    }

    void load(string filename) {

        ifstream file(filename);

        string key,type,value;

        while(file >> key >> type >> value) {

            Entry e;
            e.type = type;
            e.value = value;
            e.expiry = -1;

            store[key] = e;
        }

        file.close();

        cout << "OK" << endl;
    }

    void stats() {

        cleanup();

        cout << "total_keys=" << store.size() << endl;
        cout << "expired_cleaned=" << expired_cleaned << endl;
    }

};

int main() {

    KVStore kv;

    string line;

    while(true) {

        getline(cin,line);

        if(line.empty())
            continue;

        stringstream ss(line);

        string cmd;
        ss >> cmd;

        if(cmd=="SET") {

            string key,value,ex;
            int ttl=-1;

            ss>>key>>value;

            if(ss>>ex)
                if(ex=="EX")
                    ss>>ttl;

            kv.set(key,value,ttl);
        }

        else if(cmd=="GET") {

            string key;
            ss>>key;

            kv.get(key);
        }

        else if(cmd=="DEL") {

            string key;
            ss>>key;

            kv.del(key);
        }

        else if(cmd=="TTL") {

            string key;
            ss>>key;

            kv.ttl(key);
        }

        else if(cmd=="INCR") {

            string key;
            ss>>key;

            kv.incr(key);
        }

        else if(cmd=="DECR") {

            string key;
            ss>>key;

            kv.decr(key);
        }

        else if(cmd=="LPUSH") {

            string key,val;
            ss>>key>>val;

            kv.lpush(key,val);
        }

        else if(cmd=="RPUSH") {

            string key,val;
            ss>>key>>val;

            kv.rpush(key,val);
        }

        else if(cmd=="LPOP") {

            string key;
            ss>>key;

            kv.lpop(key);
        }

        else if(cmd=="RPOP") {

            string key;
            ss>>key;

            kv.rpop(key);
        }

        else if(cmd=="SAVE") {

            string file;
            ss>>file;

            kv.save(file);
        }

        else if(cmd=="LOAD") {

            string file;
            ss>>file;

            kv.load(file);
        }

        else if(cmd=="STATS") {

            kv.stats();
        }

    }

}