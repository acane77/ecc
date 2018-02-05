#include "vm/VMInstance.h"
#include "vm/Register.h"
#include "vm/InstructionBuilder.h"

namespace Miyuki::VM {
    
    void VMInstance::initialize() {
        using namespace Constants;

        // initialize system call and trap vectors

        /// GETC - Get char from keyboard without echo it
        IS.setMultibyte<uint32_t>(MemoryMap::SYSCALL_GETC, MemoryMap::SysCodeStart + 0x100);
        B.setInsertPos(MemoryMap::SysCodeStart + 0x100);
        B.buildMOV(R11, InstructionBuilder::doubleWord, MemoryMap::IO_KBR);
        B.buildLDR(InstructionBuilder::byte, R12, R11, 0);
        B.buildRET();

        /// PUTC - echo a char
        IS.setMultibyte<uint32_t>(MemoryMap::SYSCALL_PUTC, MemoryMap::SysCodeStart + 0x200);
        B.setInsertPos(MemoryMap::SysCodeStart + 0x200);
        B.buildMOV(R11, InstructionBuilder::doubleWord, MemoryMap::IO_DR);
        B.buildSTR(InstructionBuilder::byte, R12, R11, 0);
        B.buildRET();

        // GETS
        IS.setMultibyte<uint32_t>(MemoryMap::SYSCALL_GETS, MemoryMap::SysCodeStart + 0x300);
        B.setInsertPos(MemoryMap::SysCodeStart + 0x300);
        B.buildMOV(R10, R12);
        B.buildLabel(0xc8208d60);
        B.buildPUSH(InstructionBuilder::byte, '\r');
        B.buildTRAP(MemoryMap::SYSCALL_GETCHAR - MemoryMap::TrapStart);
        B.buildPUSHR(InstructionBuilder::byte, R12);
        B.buildCMP(InstructionBuilder::byte);
        B.buildSTR(InstructionBuilder::byte, R12, R10, 0);
        B.buildRADD(InstructionBuilder::byte, R10, 1);
        B.buildPOP(InstructionBuilder::byte);
        B.buildJNE_Label(0xc8208d60);
        B.buildMOV(R12, InstructionBuilder::byte, '\n');
        B.buildTRAP(MemoryMap::SYSCALL_PUTC - MemoryMap::TrapStart);
        B.buildMOV(R12, InstructionBuilder::byte, 0);
        B.buildSTR(InstructionBuilder::byte, R12, R10, -1);
        B.buildRET();

        // PUTS - put a string
        IS.setMultibyte<uint32_t>(MemoryMap::SYSCALL_PUTS, MemoryMap::SysCodeStart + 0x400);
        B.setInsertPos(MemoryMap::SysCodeStart + 0x400);
        B.buildMOV(R10, R12);
        B.buildLabel(0xc8208d30);
        B.buildPUSH(InstructionBuilder::byte, 0);
        B.buildLDR(InstructionBuilder::byte, R12, R10, 0);
        B.buildTRAP(MemoryMap::SYSCALL_PUTC - MemoryMap::TrapStart);
        B.buildPUSHR(InstructionBuilder::byte, R12);
        B.buildCMP(InstructionBuilder::byte);
        B.buildPOP(InstructionBuilder::byte);
        B.buildRADD(InstructionBuilder::byte, R10, 1);
        B.buildJNE_Label(0xc8208d30);
        B.buildRET();

        /// GETCHAR - Get char from keyboard and echo it
        IS.setMultibyte<uint32_t>(MemoryMap::SYSCALL_GETCHAR, MemoryMap::SysCodeStart + 0x600);
        B.setInsertPos(MemoryMap::SysCodeStart + 0x600);
        B.buildMOV(R11, InstructionBuilder::doubleWord, MemoryMap::IO_KBR);
        B.buildLDR(InstructionBuilder::byte, R12, R11, 0);
        B.buildMOV(R11, InstructionBuilder::doubleWord, MemoryMap::IO_DR);
        B.buildSTR(InstructionBuilder::byte, R12, R11, 0);
        B.buildRET();

        /// HALT - Halt the VM
        IS.setMultibyte<uint32_t>(MemoryMap::SYSCALL_HALT, MemoryMap::SysCodeStart + 0x500);
        B.setInsertPos(MemoryMap::SysCodeStart + 0x500);
        B.buildHALT();

        B.insertUserCode();
    }

    void Miyuki::VM::VMInstance::printMemory(uint32_t from, uint32_t to) {
        from = from - from % 16;
        to = to + (16 - to % 16 - 1);
        char str[17] = {0};
        puts("\nMemory dump\n");
        for (; from <= to; from++) {
            if (from % 16 == 0) {
                printf("[%x]", from);
            }
            Byte B = M.get(from);
            if (B < 16)
                printf("0");
            printf("%x ", B);
            str[from % 16] = B >= 32 && B < 127 ? B : '.';
            if (from % 16 == 15) {
                str[(from + 1) % 16 == 0?16: (from + 1) % 16] = 0;
                printf("   %s\n", str);
            }
        }
            
    }
}