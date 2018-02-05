#ifndef _MIYUKI_VM_REGISTER_H
#define _MIYUKI_VM_REGISTER_H

#include <cstdint>
#include <conio.h>
#include <cstdio>

namespace Miyuki::VM {

    template <class Ty>
    class RegisterBase {
    protected:
        Ty       data;
    public:
        explicit RegisterBase(Ty d = 0) { data = d; }

        virtual Ty get() { return data; }
        virtual void set(Ty val) { data = val; }

        virtual RegisterBase& operator=(Ty val) { data = val; return *this; }
        virtual operator uint64_t () { return data; }

        //RegisterBase& operator+(Ty I) { data += I; return *this; }
        //RegisterBase& operator-(Ty I) { data -= I; return *this; }
        RegisterBase& operator++() { ++data; return *this; }
    };

    typedef RegisterBase<uint64_t> Register;
    typedef Register Register64;
    typedef RegisterBase<uint32_t> Register32;
    typedef RegisterBase<uint8_t> Register8;

    class KeyboardDataRegister : public Register {
    public:
        virtual Register& operator=(uint64_t val) { data = val; return *this; }
        virtual uint64_t get() { return _getch(); }

        virtual operator uint64_t () { return get(); }
    };

    class DisplayDataRegister : public Register {
    public:
        virtual void set(uint64_t val) { putchar(val); data = val; }

        virtual Register& operator=(uint64_t val) { set(val); return *this; }
    };

    class ConditionFlagRegister : public Register8 {
    public:
        void setFlag(uint8_t C) { data = C; }
        bool hasFlag(uint8_t C) { return data & C; }
    };

    namespace Conditions {
        enum {
            CC_GT = 1,
            CC_LT = 1 << 1,
            CC_EQ = 1 << 2
            
        };
    }

    namespace Registers {
        extern Register R[16];
        extern Register &PC, &SP, &SS;
        extern ConditionFlagRegister CF;
        extern KeyboardDataRegister KBDR;
        extern DisplayDataRegister DDR;
        extern Register8 IR;

        extern void printRegisters();
    };

}

#endif