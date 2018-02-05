#ifndef _MIYUKI_VM_STACK_H
#define _MIYUKI_VM_STACK_H

#include <cstring>
#include <cassert>
#include <cstdint>
#include "vm/ISA.h"

// Stack memory allocation

namespace Miyuki::VM {

#define STACK_PAGE_SIZE 1024

    class StackPagedItem {
        const uint32_t _size = 1024;
        int16_t sp;
        ByteArray st;
    public:
        Byte& get(uint32_t P) { return st[P]; }
        void set(uint32_t P, Byte b) { st[P] = b; }
        void push(Byte b) { st[++sp] = b; }
        void pop() { --sp; }
        Byte top() { return st[sp]; }
        bool isFull() { return sp == STACK_PAGE_SIZE - 1; }
        bool isEmpty() { return sp == -1; }

        StackPagedItem();
        ~StackPagedItem();
    };

    class Stack {
        StackPagedItem* pages[128];
        StackPagedItem** currentPage = pages;

        void allocPage(uint32_t pageID) { assert(pageID < 127 && "pageID is bigger than 127"); pages[pageID] = new StackPagedItem(); }
    public:
        Stack();
        ~Stack();

        StackPagedItem* getOrAlloc(uint32_t pageID);
        Byte& get(uint32_t addr);
        void set(uint32_t addr, Byte b) { get(addr) = b; }
        Byte& operator[](uint32_t addr) { return get(addr); }
        void push(Byte B);
        Byte top();
        void pop();
    };

}

#endif