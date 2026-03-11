#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>
#include <sstream>
#include <vector>

using namespace std;

using Clock = chrono::steady_clock;

struct Entry {
    string value;
    long long expiry;
};

class KVStore {

private:

    unordered_map<string, Entry> store;
    int expired_cleaned = 0;

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

    void set(string key, string value, int ttl) {

        cleanup();

        Entry e;
        e.value = value;

        if(ttl > 0)
            e.expiry = now() + ttl;
        else
            e.expiry = -1;

        store[key] = e;

        cout << "OK" << endl;
    }

    void get(string key) {

        cleanup();

        if(store.find(key) == store.end()) {

            cout << "(nil)" << endl;
            return;
        }

        cout << store[key].value << endl;
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

    void keys(string pattern) {

        cleanup();

        if(pattern == "*") {

            for(auto &p : store)
                cout << p.first << endl;
        }
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

        getline(cin, line);

        if(line.empty())
            continue;

        stringstream ss(line);

        string cmd;
        ss >> cmd;

        if(cmd == "SET") {

            string key,value,ex;
            int ttl = -1;

            ss >> key >> value;

            if(ss >> ex) {

                if(ex == "EX")
                    ss >> ttl;
            }

            kv.set(key,value,ttl);
        }

        else if(cmd == "GET") {

            string key;
            ss >> key;

            kv.get(key);
        }

        else if(cmd == "DEL") {

            string key;
            ss >> key;

            kv.del(key);
        }

        else if(cmd == "TTL") {

            string key;
            ss >> key;

            kv.ttl(key);
        }

        else if(cmd == "KEYS") {

            string pattern;
            ss >> pattern;

            kv.keys(pattern);
        }

        else if(cmd == "STATS") {

            kv.stats();
        }

    }

}