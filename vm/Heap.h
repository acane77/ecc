#ifndef _MIYUKI_VM_HEAP_H
#define _MIYUKI_VM_HEAP_H

#include "vm/ISA.h"
#include <unordered_map>
#include <cassert>

namespace Miyuki::VM {
    typedef Byte* Address;

    class HeapAddress {
        Address addr;
        bool isFreed = false;
        int16_t refCount = 0;
        size_t size;

        friend class Heap;
        HeapAddress(Address _addr) { addr = _addr; }
    };

    class Heap {
        typedef std::unordered_map<Address, HeapAddress*> MemoryAddressList;
        MemoryAddressList memAddr;
    public:
        ~Heap();

        Address malloc(size_t size);
        bool free(Address& addr);
        void ref(Address addr);
        void deref(Address addr);
        
    };

}

#endif // ! _MIYUKI_VM_HEAP_H