#include "vm/MemoryMap.h"
#include "vm/Register.h"
#include <conio.h>

namespace Miyuki::VM {
    void Miyuki::VM::MemoryMap::set(uint32_t addr, Byte val) {
        if (addr >= VirtualAddress::SysCodeStart) {
            sysCode.set(addr - VirtualAddress::SysCodeStart, val);
        }
        else if (addr >= VirtualAddress::CodeStart) {
            code.set(addr - VirtualAddress::CodeStart, val);
        }
        else if (addr >= VirtualAddress::StackStart) {
            stack.set(addr - VirtualAddress::StackStart, val);
        }
        else if (addr >= VirtualAddress::HeapStart) {
            *((Address)(uint64_t)addr) = val;
        }
        else if (addr >= VirtualAddress::TrapStart) {
            trapVector.set(addr - VirtualAddress::TrapStart, val);
        }
        else if (addr >= VirtualAddress::IOAddrStart) {
            requestIO(addr, val);
        }
        else {
            throw "inaccessible address";
        }
    }

    Byte Miyuki::VM::MemoryMap::get(uint32_t addr) {
        if (addr >= VirtualAddress::SysCodeStart) {
            return sysCode.get(addr - VirtualAddress::SysCodeStart);
        }
        else if (addr >= VirtualAddress::CodeStart) {
            return code.get(addr - VirtualAddress::CodeStart);
        }
        else if (addr >= VirtualAddress::StackStart) {
            return stack.get(addr - VirtualAddress::StackStart);
        }
        else if (addr >= VirtualAddress::HeapStart) {
            return *((Address)(uint64_t)addr);
        }
        else if (addr >= VirtualAddress::TrapStart) {
            return trapVector.get(addr - VirtualAddress::TrapStart);
        }
        else if (addr >= VirtualAddress::IOAddrStart) {
            return requestIO(addr);
        }
        else {
            throw "inaccessible address";
        }
    }

    Byte Miyuki::VM::MemoryMap::requestIO(uint32_t addr) {
        assert(addr >= VirtualAddress::IOAddrStart && addr < VirtualAddress::TrapStart && "invalid use of this function");
        if (addr == VirtualAddress::IO_KBR) {
            return (Byte)Registers::KBDR;
        }
        else if (addr == VirtualAddress::IO_DR) {
            return (Byte)Registers::DDR;
        }
        return 0;
    }

    Byte Miyuki::VM::MemoryMap::requestIO(uint32_t addr, Byte value) {
        assert(addr >= VirtualAddress::IOAddrStart && addr < VirtualAddress::TrapStart && "invalid use of this function");
        if (addr == VirtualAddress::IO_KBR) {
            Registers::KBDR = value;
            return (Byte)Registers::KBDR;
        }
        else if (addr == VirtualAddress::IO_DR) {
            Registers::DDR = value;
            return Registers::DDR;
        }
        return value;
    }
    Byte Miyuki::VM::MemoryMap::operator[](uint32_t addr) {
        return get(addr);
    }
}