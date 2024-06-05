#ifndef STRIPEDHASHSET_H
#define STRIPEDHASHSET_H

#include <vector>
#include <list>
#include <mutex>
#include <string>
#include <functional>
#include <iostream>

std::hash<std::string> hasher;

struct Website {
    std::string url;

    Website(const std::string& u) : url(u) {}

    bool operator==(const Website& other) const {
        return url == other.url;
    }
};

class BaseHashSet {
private:
    int setSize;
    std::vector<std::list<Website>> table;
    std::vector<std::mutex> locks;

    void acquire(const Website& x) {
        size_t hash = hasher(x.url);
        locks[hash % locks.size()].lock();
    }

    void release(const Website& x) {
        locks[hasher(x.url) % locks.size()].unlock();
    }

    bool policy() {
        return setSize / table.size() > 4;
    }

    void resize() {
        int oldCapacity = table.size();
        for (auto& lock : locks) {
            lock.lock();
        }
        if (oldCapacity != int(table.size())) {
            return;
        }
        int newCapacity = 2 * oldCapacity;
        std::vector<std::list<Website>> oldTable = table;
        table.resize(newCapacity);
        for (int i = 0; i < newCapacity; i++) {
            table[i] = std::list<Website>();
        }
        for (auto& bucket : oldTable) {
            for (const Website& x : bucket) {
                table[hasher(x.url) % table.size()].push_back(x);
            }
        }
        for (auto& lock : locks) {
            lock.unlock();
        }
    }

public:
    BaseHashSet(int capacity) : setSize(0), table(capacity), locks(capacity) {
        for (int i = 0; i < capacity; i++) {
            table[i] = std::list<Website>();
        }
    }

    bool contains(const Website& x) {
        acquire(x);
        int myBucket = hasher(x.url) % table.size();
        bool result = (find(x) != table[myBucket].end());
        release(x);
        return result;
    }

    typename std::list<Website>::iterator find(const Website x){
        int myBucket = hasher(x.url) % table.size();
        for(auto i = table[myBucket].begin(); i != table[myBucket].end(); ++i){
            if (*i == x){
                return i;
            }
        }
        return table[myBucket].end();
    }

    bool add(const Website& x) {
        bool result = false;
        acquire(x);
        int myBucket = hasher(x.url) % table.size();
        if (find(x) == table[myBucket].end()) {
            table[myBucket].push_back(x);
            result = true;
            setSize++;
        }
        release(x);
        if (policy()) {
            resize();
        }
        return result;
    }

    void printAllWebsites() {
        bool isEmpty = true;
        for (const auto& bucket : table) {
            if (!bucket.empty()) {
                if (isEmpty) {
                    isEmpty = false;
                    std::cout << "Begin:"; 
                }
                for (const auto& website : bucket) {
                    std::cout << website.url << " ";
                }
            }
        }
        std::cout << std::endl;
        if (isEmpty) {
            std::cout << "The hash set is empty." << std::endl;
        }
    }
};

#endif // STRIPEDHASHSET_H