#ifndef _MIYUKI_MEMORY_MAP_H
#define _MIYUKI_MEMORY_MAP_H

#include "vm/Stack.h"
#include "vm/Heap.h"

namespace Miyuki::VM {  

    class MemoryMap {
        Stack stack;
        Heap  heap;
        Stack code, sysCode;
        Stack trapVector;

        // request I/O
        Byte requestIO(uint32_t addr);
        Byte requestIO(uint32_t addr, Byte value);
        
    public:
        enum VirtualAddress : uint32_t {
            IOAddrStart = 0x00000400UL,
            TrapStart = 0x00000800UL,
            HeapStart = 0x00001000UL,
            StackStart = 0x80000000UL,
            CodeStart = 0x82000000UL,
            SysCodeStart = 0x8a000000UL,

            // I/O Addresses
            IO_KBR = 0x404, //KEYBOARD
            IO_DR = 0x408,  //DISPLAY
            
            // System Call Addresses
            SYSCALL_GETC = 0x800,
            SYSCALL_PUTC = 0x804,
            SYSCALL_GETS = 0x808,
            SYSCALL_PUTS = 0x80c,
            SYSCALL_HALT = 0x810,
            SYSCALL_GETCHAR = 0x814
        };

        // general
        void set(uint32_t addr, Byte val);
        Byte get(uint32_t addr);
        Byte operator[](uint32_t addr);

        Stack* getStack() { return &stack; }
        Heap*  getHeap() { return &heap; }
        Stack* getCode() { return &code; }
    };

}

#endif