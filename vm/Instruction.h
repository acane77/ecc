#ifndef _MIYUKI_VM_INSTRUCTION_H

#include "vm/MemoryMap.h"
#include "vm/Register.h"
#include <stack>
#include <functional>

namespace Miyuki::VM {

    class InstructionSet {
        MemoryMap * MM;
        typedef std::stack<uint64_t> CallStack;
        CallStack callStack;

        uint8_t fetch();
    public:
        enum Opcode {
            // Arithmetic & Logical
            ADD = 0, SUB, MUL, DIV, MOD, INC, DEC, 
            RADD, RSUB,
            FADD, FSUB, FMUL, FDIV,
            CMP, FCMP, AND, OR, NOT, XOR, LSF, RSF,
            // Branches
            JE, JNE, JGT, JLT, JGE, JLE, JMP, 
            CALL, RET, TRAP,
            // Storage
            LEA, LD, LDI, LDR, ST, STI, STR, MOV,
            PUSH, PUSHR, POP,
            // Others
            HALT, HEAP_ALLOC, FREE
        };
    private:
#define AF(name) [this]()->bool { return this->Instr##name(); } 

        typedef std::function<bool ()> InstructionHandle;
        InstructionHandle handles[FREE - ADD + 1] = {
            AF(ADD), AF(SUB), AF(MUL), AF(DIV), AF(MOD), AF(INC), AF(DEC),
            AF(RADD), AF(RSUB),
            AF(FADD), AF(FSUB), AF(FMUL), AF(FDIV),
            AF(CMP), AF(FCMP), AF(AND), AF(OR), AF(NOT), AF(XOR), AF(LSF), AF(RSF),
            AF(JE), AF(JNE), AF(JGT), AF(JLT), AF(JGE), AF(JLE), AF(JMP),
            AF(CALL), AF(RET), AF(TRAP),
            AF(LEA), AF(LD), AF(LDI), AF(LDR), AF(ST), AF(STI), AF(STR), AF(MOV),
            AF(PUSH), AF(PUSHR), AF(POP),
            AF(Halt), AF(ALLOC_HEAP), AF(FREE)
        };
#undef AF

        // handlers ///////
        bool InstrADD(); //  tested
        bool InstrSUB(); //  tested !
        bool InstrMUL(); //  tested !
        bool InstrDIV(); //  tested !
        bool InstrMOD(); //  tested !
        bool InstrINC(); //  non
        bool InstrDEC(); //  non
        bool InstrCMP(); //  tested
        bool InstrAND(); //  tested !
        bool InstrOR();  //  tested !
        bool InstrNOT();  // tested !
        bool InstrXOR(); //  tested !
        bool InstrLSF(); //  tested !
        bool InstrRSF(); //  tested !
        bool InstrFADD();//  tested !
        bool InstrFSUB();//  tested !
        bool InstrFMUL();//  tested !
        bool InstrFDIV();//  tested !
        bool InstrFCMP();//  tested !!
        bool InstrRADD();//  tested
        bool InstrRSUB();//  tested !
        bool InstrJE(); //   tested
        bool InstrJNE();//   tested !
        bool InstrJGT();//   tested !
        bool InstrJGE();//   tested !
        bool InstrJLT();//   tested !
        bool InstrJLE();//   tested !
        bool InstrLEA();//   non
        bool InstrJMP();//   tested !
        bool InstrCALL();//   non
        bool InstrRET(); //   tested
        bool InstrTRAP();//   tested
        bool InstrPUSH(); //  tested
        bool InstrPUSHR();//  tested
        bool InstrPOP();  //  tested
        bool InstrMOV();  //  tested
        bool InstrST();   //  non
        bool InstrSTI();  //  non
        bool InstrSTR();  //  tested
        bool InstrLD();   //  non
        bool InstrLDI();  //  non
        bool InstrLDR();  //  tested
        bool InstrHalt(); //  tested
        bool InstrALLOC_HEAP();//  tested
        bool InstrFREE(); //  tested

        template <class T> void setCondition(T a, T b);
        
        uint8_t getBits(uint8_t s, uint8_t e);
        template <class T> uint8_t getBits(T V, uint8_t s, uint8_t e);
    
    public:
        template <class T> void setMultibyte(uint32_t addr, T val);
        template <class T> T getMultibyte(uint32_t addr);

        uint32_t SEXT32(uint16_t u16);

    public:
        InstructionSet(MemoryMap * _MM);
        bool executeInstruction();

        enum AddressingMode {
            Immediate = 0,
            PCRelative,
            RegisterRelative,
            MemoryRelative
        };
    };

}

#endif
