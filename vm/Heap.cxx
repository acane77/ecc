#include "vm/Heap.h"

namespace Miyuki::VM {
    Heap::~Heap() {
        for (MemoryAddressList::iterator it = memAddr.begin(); it != memAddr.end(); ++it) {
            delete[] it->first;
            delete it->second;
            //printf("Heap::~Heap(): free address %d\n", it->first);
        }
    }

    Address Heap::malloc(size_t size) {
        Address addr = new Byte[size];
        if (!addr)
            return nullptr;
        HeapAddress* HA = new HeapAddress(addr);
        HA->size = size;
        memAddr[addr] = HA;
        //printf("Heap::malloc(): alloc address %d\n", addr);
        return addr;
    }

    bool Miyuki::VM::Heap::free(Address& addr) {
        MemoryAddressList::iterator it = memAddr.find(addr);
        if (it == memAddr.end())
            return false;
        delete[] addr;
        delete it->second;
        memAddr.erase(it);
        //printf("Heap::free(): free address %d\n", addr);
        addr = nullptr;
        return true;
    }
    
    void Heap::ref(Address addr) {
        MemoryAddressList::iterator it = memAddr.find(addr);
        assert(it != memAddr.end() && "no such address");
        it->second->refCount++;
    }

    void Heap::deref(Address addr) {
        MemoryAddressList::iterator it = memAddr.find(addr);
        assert(it != memAddr.end() && "no such address");
        it->second->refCount--;
        assert(it->second->refCount > 0 && "invalid refCount");
    }

}