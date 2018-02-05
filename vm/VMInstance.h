#ifndef _MIYUKI_VM_VMINSTANCE_H
#define _MIYUKI_VM_VMINSTANCE_H

#include "vm/InstructionBuilder.h"

namespace Miyuki::VM {

    class VMInstance {
    public:
        MemoryMap M;
        InstructionBuilder B;
        InstructionSet IS;

        VMInstance(): B(&M), IS(&M) {
            initialize();
        }

        void printMemory(uint32_t from, uint32_t to);

    private:
        void initialize();
        
    };

    

}

#endif