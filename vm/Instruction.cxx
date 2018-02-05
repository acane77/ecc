#include "vm/Instruction.h"

namespace Miyuki::VM {
    using namespace Registers;

    InstructionSet::InstructionSet(MemoryMap * _MM) {
        MM = _MM;
        PC = MemoryMap::VirtualAddress::CodeStart;
        SP = MemoryMap::VirtualAddress::StackStart;
    }

    uint8_t Miyuki::VM::InstructionSet::getBits(uint8_t s, uint8_t e) {
        return uint8_t(IR << 7 - e) >> (7 - e) >> s;
    }

    template<class T>
    uint8_t Miyuki::VM::InstructionSet::getBits(T V, uint8_t s, uint8_t e)
    {
        return V << (15 - e) >> (s + 15 - e);
    }

    uint8_t Miyuki::VM::InstructionSet::fetch() {
        Byte B = MM->get(PC);
        PC = PC + 1;
        Registers::IR = B;
        return B;
    }

    template<class T>
    void Miyuki::VM::InstructionSet::setMultibyte(uint32_t addr, T _val) {
        const uint32_t size = sizeof(T);
        union {
            Byte raw[sizeof(T)];
            T val;
        } U;
        U.val = _val;
        for (int i = 0; i < size; i++) {
            MM->set(addr + i, U.raw[i]);
        }
    }

    template<class T>
    T Miyuki::VM::InstructionSet::getMultibyte(uint32_t addr) {
        const uint32_t size = sizeof(T);
        union {
            Byte raw[sizeof(T)];
            T val;
        } U;
        for (int i = 0; i < size; i++)
            U.raw[i] = MM->get(addr + i);
        return U.val;
    }

    bool Miyuki::VM::InstructionSet::executeInstruction() {
        uint8_t opcode = fetch() >> 2;
        return handles[opcode]();
    }

    /// Format of A & L Instructions
    ///  ADD, SUB, MUL, DIV, MOD, INC, DEC, FADD, FSUB, FMUL, FDIV,
    ///  CMP, FCMP, AND, OR, NOT, XOR, LSF, RSF
    ///  |        Opcode      | Type |
    ///             6             2
#define ADD_BINARY_ARITH_FUNCTION_DEF(name, op)\
    bool Miyuki::VM::InstructionSet::Instr##name() {\
        uint32_t S = 1 << getBits(0, 1);\
        if (S == 8)      setMultibyte(SP - 2 * S + 1, getMultibyte<uint64_t>(SP - 2 * S + 1) op getMultibyte<uint64_t>(SP - S + 1));\
        else if (S == 4) setMultibyte(SP - 2 * S + 1, getMultibyte<uint32_t>(SP - 2 * S + 1) op getMultibyte<uint32_t>(SP - S + 1));\
        else if (S == 2) setMultibyte(SP - 2 * S + 1, getMultibyte<uint16_t>(SP - 2 * S + 1) op getMultibyte<uint16_t>(SP - S + 1));\
        else if (S == 1) setMultibyte(SP - 2 * S + 1, getMultibyte<uint8_t>(SP - 2 * S + 1) op getMultibyte<uint8_t>(SP - S + 1));\
        SP = SP - S;\
        return true;\
    }

    ADD_BINARY_ARITH_FUNCTION_DEF(ADD, +)
    ADD_BINARY_ARITH_FUNCTION_DEF(SUB, -)
    ADD_BINARY_ARITH_FUNCTION_DEF(MUL, *)
    ADD_BINARY_ARITH_FUNCTION_DEF(DIV, *)
    ADD_BINARY_ARITH_FUNCTION_DEF(MOD, %)
    ADD_BINARY_ARITH_FUNCTION_DEF(AND, &)
    ADD_BINARY_ARITH_FUNCTION_DEF(OR, |)
    ADD_BINARY_ARITH_FUNCTION_DEF(XOR, ^)
    ADD_BINARY_ARITH_FUNCTION_DEF(LSF, <<)
    ADD_BINARY_ARITH_FUNCTION_DEF(RSF, >>)

#define ADD_UNARY_ARITH_FUNCTION_DEF(name, op)\
    bool Miyuki::VM::InstructionSet::Instr##name() {\
        uint32_t S = 1 << getBits(0, 1);\
        if (S == 8)      setMultibyte(SP - 2 * S + 1, op getMultibyte<uint64_t>(SP - S + 1));\
        else if (S == 4) setMultibyte(SP - 2 * S + 1, op getMultibyte<uint32_t>(SP - S + 1));\
        else if (S == 2) setMultibyte(SP - 2 * S + 1, op getMultibyte<uint16_t>(SP - S + 1));\
        else if (S == 1) setMultibyte(SP - 2 * S + 1, op getMultibyte<uint8_t>(SP - S + 1));\
        SP = SP - S;\
        return true;\
    }

    ADD_UNARY_ARITH_FUNCTION_DEF(INC, 1+)
    ADD_UNARY_ARITH_FUNCTION_DEF(DEC, 1-)
    ADD_UNARY_ARITH_FUNCTION_DEF(NOT, ~)

    template<class T>
    void Miyuki::VM::InstructionSet::setCondition(T a, T b) {
        if (a > b)  CF.set(Conditions::CC_GT);
        else if (a == b) CF.set(Conditions::CC_EQ);
        else if (a < b) CF.set(Conditions::CC_LT);
    }

#define ADD_BINARY_FARITH_FUNCTION_DEF(name, op)\
    bool Miyuki::VM::InstructionSet::Instr##name() {\
        uint32_t S = 4 << getBits(0, 1);\
        if (S == 4) setMultibyte(SP - 2 * S + 1, getMultibyte<float> (SP - 2 * S + 1) op getMultibyte<float>(SP - S + 1)); \
        else if (S == 8) setMultibyte(SP - 2 * S + 1, getMultibyte<double>(SP - 2 * S + 1) op getMultibyte<double>(SP - S + 1)); \
        SP = SP - S;\
        return true;\
    }

    ADD_BINARY_FARITH_FUNCTION_DEF(FADD, +)
    ADD_BINARY_FARITH_FUNCTION_DEF(FSUB, -)
    ADD_BINARY_FARITH_FUNCTION_DEF(FMUL, *)
    ADD_BINARY_FARITH_FUNCTION_DEF(FDIV, /)

    /// Format of Compare Instruction
    ///  CMP  FCMP
    ///  |        Opcode      | Type |
    ///             6             2
    bool Miyuki::VM::InstructionSet::InstrCMP() {
        uint32_t S = 1 << getBits(0, 1);
        if (S == 8) setCondition<uint64_t>(getMultibyte<uint64_t>(SP - 2 * S + 1), getMultibyte<uint64_t>(SP - S + 1));
        else if (S == 4) setCondition<uint32_t>(getMultibyte<uint32_t>(SP - 2 * S + 1), getMultibyte<uint32_t>(SP - S + 1));
        else if (S == 2) setCondition<uint16_t>(getMultibyte<uint16_t>(SP - 2 * S + 1), getMultibyte<uint16_t>(SP - S + 1));
        else if (S == 1) setCondition<uint8_t>(getMultibyte<uint8_t>(SP - 2 * S + 1), getMultibyte<uint8_t>(SP - S + 1));
        SP = SP - S;
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrFCMP() {
        uint32_t S = 1 << getBits(0, 1);
        if (S == 8) setCondition<float>(getMultibyte<float>(SP - 2 * S + 1), getMultibyte<float>(SP - S + 1));
        else if (S == 4) setCondition<double>(getMultibyte<double>(SP - 2 * S + 1), getMultibyte<double>(SP - S + 1));
        return true;
    }

    /// Format of Conditional Jump Instructions
    ///  JE  JNE  JGT  JGE  JLT  JLE  JMP
    ///  Immediate
    ///  |        Opcode      |  00  |
    ///             6             2
    ///  |          addr             |
    ///                8
    ///  |          addr             |
    ///                8
    ///  |          addr             |
    ///                8              
    ///  |          addr             |
    ///                8
    ///  PC-Relative
    ///  |        Opcode      |  01  |
    ///             6             2
    ///  |          offset           |
    ///                8
    ///  |          offset           |
    ///                8
    ///
    ///  Register-Relative
    ///  |        Opcode      |  10  |
    ///             6             2
    ///  |              |   reg id   |
    ///         4             4
    ///  |          offset           |
    ///                8
    ///  |          offset           |
    ///                8
#define ADD_CONDITIONAL_JUMP_FUNCTION(name, flag) \
    bool Miyuki::VM::InstructionSet::Instr##name() {\
        uint8_t type = getBits(0, 1);\
        if (!CF.hasFlag(flag)) {\
            if (type == AddressingMode::Immediate) {\
                PC = PC + 4;\
            }\
            else if (type == AddressingMode::PCRelative) {\
                PC = PC + 2;\
            }\
            else if (type == AddressingMode::RegisterRelative) {\
                PC = PC + 3;\
            }\
            return true;\
        }\
        int32_t addr;\
        if (type == AddressingMode::Immediate) {\
            Byte b1 = fetch(); Byte b2 = fetch();\
            Byte b3 = fetch(); Byte b4 = fetch();\
            addr = b4;\
            addr = addr << 8 | b3;\
            addr = addr << 8 | b2;\
            addr = addr << 8 | b1;\
        }\
        else if (type == AddressingMode::PCRelative) {\
            Byte b1 = fetch(); Byte b2 = fetch();\
            addr = PC + (b2 << 8) | b1;\
        }\
        else if (type == AddressingMode::RegisterRelative) {\
            Byte B = fetch(); Byte b1 = fetch(); Byte b2 = fetch();\
            addr = R[getBits(B, 0, 3)] + (b2 << 8) | b1;\
        }\
        else assert(false && "invalid addression mode");\
        PC = addr; PC = PC & 0xFFFFFFFFL;\
        return true;\
    }

    ADD_CONDITIONAL_JUMP_FUNCTION(JE, Conditions::CC_EQ)
    ADD_CONDITIONAL_JUMP_FUNCTION(JNE, Conditions::CC_GT | Conditions::CC_LT)
    ADD_CONDITIONAL_JUMP_FUNCTION(JGT, Conditions::CC_GT)
    ADD_CONDITIONAL_JUMP_FUNCTION(JGE, Conditions::CC_GT | Conditions::CC_EQ)
    ADD_CONDITIONAL_JUMP_FUNCTION(JLT, Conditions::CC_LT)
    ADD_CONDITIONAL_JUMP_FUNCTION(JLE, Conditions::CC_LT | Conditions::CC_EQ)
    ADD_CONDITIONAL_JUMP_FUNCTION(JMP, Conditions::CC_LT | Conditions::CC_EQ | Conditions::CC_GT)
    
    /// Format of call
    ///  CALL
    ///  PC-Relative
    ///  |        Opcode      |  00  |
    ///             6             2
    ///  |          offset           |
    ///                8
    ///  |          offset           |
    ///                8
    ///
    ///  Register-Relative
    ///  |        Opcode      |  01  |
    ///             6             2
    ///  |              |   reg id   |
    ///         4             4
    ///  |          offset           |
    ///                8
    ///  |          offset           |
    ///                8
    bool Miyuki::VM::InstructionSet::InstrCALL() {
        bool isPCRelative = !getBits(0, 1);
        if (isPCRelative) {
            Byte b1 = fetch();  Byte b2 = fetch();
            int16_t offset = (b2 << 8) | b1;
            callStack.push(PC);
            PC = PC + offset;
        }
        else {
            fetch(); uint8_t baseRegID = getBits(0, 3);
            Byte b1 = fetch();  Byte b2 = fetch();
            int16_t offset = (b2 << 8) | b1;
            callStack.push(PC);
            PC = R[baseRegID] + offset;
        }
        return true;
    }

    /// Format of RET
    ///  |        Opcode      |  00  |
    ///             6             2
    bool Miyuki::VM::InstructionSet::InstrRET() {
        PC = callStack.top();
        callStack.pop();    
        return true;
    }

    /// Format of TRAP
    ///  |        Opcode      |  00  |
    ///             6             2
    ///  |        trap-vector        |
    ///             8
    bool Miyuki::VM::InstructionSet::InstrTRAP() {
        uint8_t uOffset = fetch();
        callStack.push(PC);
        PC = getMultibyte<uint32_t>(MM->TrapStart + uOffset);
        return true;
    }

    /// Format of PUSH, PUSHF(same opcode as PUSH), PUSHR
    ///  Immediate number (PUSH)
    ///  |       Opcode      |  Type |
    ///            6             2
    ///  |            imm            |
    ///                8
    ///  |            imm            | *
    ///                8
    ///  |            imm            | **
    ///                8
    ///  |            imm            | **
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  This instruct is length-varible
    ///  TODO: Test operator++
    ///  Register (PUSHR)
    ///  |       Opcode      |  Type |
    ///            6             2
    ///  |              |   reg id   |
    ///         4             4
    ///  *    only in 16, 32, 64-bit immediate number
    ///  **   only in 32, 64-bit number
    ///  ***  only in 64-bit number  
    bool Miyuki::VM::InstructionSet::InstrPUSH() {
        switch (getBits(0, 1)) {
        case 3:
            MM->set(++SP, fetch());
            MM->set(++SP, fetch());
            MM->set(++SP, fetch());
            MM->set(++SP, fetch());
        case 2:
            MM->set(++SP, fetch());
            MM->set(++SP, fetch());
        case 1:
            MM->set(++SP, fetch());
        case 0:
            MM->set(++SP, fetch());
            break;
        }
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrPUSHR() {
        uint8_t X = getBits(0, 1);
        fetch();
        uint8_t Rid = getBits(0, 3);
        uint64_t Rval = R[Rid];
        switch (X) {
        case 3:
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
        case 2:
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
        case 1:
            MM->set(++SP, Rval & 0xFF); Rval >>= 8;
        case 0:
            MM->set(++SP, Rval & 0xFF);
            break;
        }
        return true;
    }

    /// Format of POP
    ///  |       Opcode      |  Type |
    ///            6             2
    bool Miyuki::VM::InstructionSet::InstrPOP() {
        uint8_t S = 1 >> getBits(0, 1);
        SP = SP - S;
        return true;
    }

    /// Format of MOV
    /// Move register to register
    ///  |       Opcode      |   00  |
    ///            6             2
    ///  |    dst reg   |   src reg  |
    ///         4             4
    /// Move immediate to register
    ///  |       Opcode      |   01  |
    ///            6             2
    ///  |    dst reg   | 00 | Type  |
    ///         4          2     2
    ///  |            imm            |
    ///                8
    ///  |            imm            | *
    ///                8
    ///  |            imm            | **
    ///                8
    ///  |            imm            | **
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  *    only in 16, 32, 64-bit immediate number
    ///  **   only in 32, 64-bit number
    ///  ***  only in 64-bit number  
    bool Miyuki::VM::InstructionSet::InstrMOV() {
        uint8_t X = getBits(0, 1);  fetch();
        if (X == 0) {
            R[getBits(4, 7)] = R[getBits(0, 3)];
        }
        else {
            uint8_t dstRegID = getBits(4, 7);
            union {
                uint8_t raw[8];
                uint64_t val = 0;
            } V;
            uint8_t Vi = 0;
            switch (getBits(0, 1)) {
            case 3:
                V.raw[Vi++] = fetch();
                V.raw[Vi++] = fetch();
                V.raw[Vi++] = fetch();
                V.raw[Vi++] = fetch();
            case 2:
                V.raw[Vi++] = fetch();
                V.raw[Vi++] = fetch();
            case 1:
                V.raw[Vi++] = fetch();
            case 0:
                V.raw[Vi++] = fetch();
                
                R[dstRegID] = V.val;
                break;
            }
        }
        
        return true;
    }

    /// Format oF ST  STI  STR
    ///  ST (PC-Relative),  STI (Indirect)
    ///  |       Opcode      |  Type |
    ///            6             2
    ///  |              |   reg id   |
    ///         4             4
    ///  |          PC-offset        |
    ///                8
    ///  |          PC-offset        |
    ///                8
    ///  STR (Register-Relative)
    ///  |       Opcode      |  Type |
    ///            6             2
    ///  |   base reg   |   src reg  |
    ///         4             4
    ///  |           offset          |
    ///                8
    ///  |           offset          |
    ///                8
    bool Miyuki::VM::InstructionSet::InstrST() {
        uint8_t S = 1 << getBits(0, 1); fetch();
        uint8_t srcRegID = getBits(0, 3);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        if (S == 8)
            setMultibyte<uint64_t>(PC + offset, R[srcRegID]);
        else if (S == 4)
            setMultibyte<uint32_t>(PC + offset, R[srcRegID]);
        else if (S == 2)
            setMultibyte<uint16_t>(PC + offset, R[srcRegID]);
        else if (S == 1)
            setMultibyte<uint8_t>(PC + offset, R[srcRegID]);
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrSTI() {
        uint8_t S = 1 << getBits(0, 1); fetch();
        uint8_t srcRegID = getBits(0, 3);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        if (S == 8) {
            uint32_t addr = getMultibyte<uint64_t>(PC + offset);
            setMultibyte<uint64_t>(addr, R[srcRegID]);
        }
        else if (S == 4) {
            // QUESTION:  Address is fixed 4 bits?
            uint32_t addr = getMultibyte<uint32_t>(PC + offset);
            setMultibyte<uint32_t>(addr, R[srcRegID]);
        }
        else if (S == 2) {
            uint32_t addr = getMultibyte<uint16_t>(PC + offset);
            setMultibyte<uint16_t>(addr, R[srcRegID]);
        }
        else if (S == 1) {
            uint32_t addr = getMultibyte<uint8_t>(PC + offset);
            setMultibyte<uint8_t>(addr, R[srcRegID]);
        }
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrSTR() {
        uint8_t S = 1 << getBits(0, 1); fetch();
        uint8_t srcRegID = getBits(0, 3), baseRegID = getBits(4, 7);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        // M[BaseR+offset] = SR
        if (S == 8)
            setMultibyte<uint64_t>(R[baseRegID] + offset, R[srcRegID]);
        else if (S == 4)
            setMultibyte<uint32_t>(R[baseRegID] + offset, R[srcRegID]);
        else if (S == 2)
            setMultibyte<uint16_t>(R[baseRegID] + offset, R[srcRegID]);
        else if (S == 1)
            setMultibyte<uint8_t>(R[baseRegID] + offset, R[srcRegID]);
        return true;
    }

    /// Format oF LD  LDI  LDR
    ///  LD (PC-Relative),  LDI (Indirect)
    ///  |       Opcode      |  Type |
    ///            6             2
    ///  |              |   dst reg  |
    ///         4             4
    ///  |          PC-offset        |
    ///                8
    ///  |          PC-offset        |
    ///                8
    ///  LDR (Register-Relative)
    ///  |       Opcode      |  Type |
    ///            6             2
    ///  |   base reg   |   dst reg  |
    ///         4             4
    ///  |           offset          |
    ///                8
    ///  |           offset          |
    ///                8
    bool Miyuki::VM::InstructionSet::InstrLD() {
        uint8_t S = 1 << getBits(0, 1); fetch();
        uint8_t dstRegID = getBits(0, 3);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        if (S == 8) {
            R[dstRegID] = getMultibyte<uint64_t>(PC + offset);
        }
        else if (S == 4) {
            R[dstRegID] = getMultibyte<uint32_t>(PC + offset);
        }
        else if (S == 2) {
            R[dstRegID] = getMultibyte<uint16_t>(PC + offset);
        }
        else if (S == 1) {
            R[dstRegID] = getMultibyte<uint8_t>(PC + offset);
        }
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrLDI() {
        uint8_t S = 1 << getBits(0, 1); fetch();
        uint8_t dstRegID = getBits(0, 3);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        if (S == 8) {
            uint32_t addr = getMultibyte<uint64_t>(PC + offset);
            R[dstRegID] = getMultibyte<uint64_t>(addr);
        }
        else if (S == 4) {
            // QUESTION:  Address is fixed 4 bits?
            uint32_t addr = getMultibyte<uint32_t>(PC + offset);
            R[dstRegID] = getMultibyte<uint32_t>(addr);
        }
        else if (S == 2) {
            uint32_t addr = getMultibyte<uint16_t>(PC + offset);
            R[dstRegID] = getMultibyte<uint16_t>(addr);
        }
        else if (S == 1) {
            uint32_t addr = getMultibyte<uint8_t>(PC + offset);
            R[dstRegID] = getMultibyte<uint8_t>(addr);
        }
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrLDR() {
        uint8_t S = 1 << getBits(0, 1); fetch();
        uint8_t dstRegID = getBits(0, 3), baseRegID = getBits(4, 7);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        // M[BaseR+offset] = SR
        if (S == 8)
            R[dstRegID] = getMultibyte<uint64_t>(R[baseRegID] + offset);
        else if (S == 4)
            R[dstRegID] = getMultibyte<uint32_t>(R[baseRegID] + offset);
        else if (S == 2)
            R[dstRegID] = getMultibyte<uint16_t>(R[baseRegID] + offset);
        else if (S == 1)
            R[dstRegID] = getMultibyte<uint8_t>(R[baseRegID] + offset);
        return true;
    }

    /// Format of HALT
    ///  |        Opcode      |  00  |
    ///             6             2
    bool Miyuki::VM::InstructionSet::InstrHalt() {
        return false;
    }

    /// Format of LEA
    ///  |        Opcode      |      |
    ///             6             2
    ///  |              |   dst reg  |
    ///         4             4
    ///  |          PC-offset        |
    ///                8
    ///  |          PC-offset        |
    ///                8
    bool Miyuki::VM::InstructionSet::InstrLEA() {
        fetch();
        uint8_t dstRegID = getBits(0, 3);
        Byte b1 = fetch();  Byte b2 = fetch();
        int16_t offset = (b2 << 8) | b1;
        R[dstRegID] = PC + offset;
        return true;
    }

    // Signatific Extension
    uint32_t Miyuki::VM::InstructionSet::SEXT32(uint16_t u16) {
        return (u16 & 0x80) ? ~0L : 0L & (uint32_t)u16;
    }

    /// Format of RADD, RSUB
    ///  |        Opcode      | Type |
    ///             6             2
    ///  |              |   dst reg  |
    ///         4             4     
    ///  |            imm            |
    ///                8
    ///  |            imm            | *
    ///                8
    ///  |            imm            | **
    ///                8
    ///  |            imm            | **
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  |            imm            | ***
    ///                8
    ///  *    only in 16, 32, 64-bit immediate number
    ///  **   only in 32, 64-bit number
    ///  ***  only in 64-bit number  
    bool Miyuki::VM::InstructionSet::InstrRADD() {
        uint8_t S = getBits(0, 1); fetch();
        uint8_t dstRegID = getBits(0, 3);
        union {
            uint8_t raw[8];
            uint64_t val = 0;
        } V;
        uint8_t Vi = 0;
        switch (S) {
        case 3:
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
        case 2:
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
        case 1:
            V.raw[Vi++] = fetch();
        case 0:
            V.raw[Vi++] = fetch();

            R[dstRegID] = R[dstRegID] + V.val;
            break;
        }
        return true;
    }

    bool Miyuki::VM::InstructionSet::InstrRSUB() {
        uint8_t S = getBits(0, 1); fetch();
        uint8_t dstRegID = getBits(0, 3);
        union {
            uint8_t raw[8];
            uint64_t val = 0;
        } V;
        uint8_t Vi = 0;
        switch (S) {
        case 3:
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
        case 2:
            V.raw[Vi++] = fetch();
            V.raw[Vi++] = fetch();
        case 1:
            V.raw[Vi++] = fetch();
        case 0:
            V.raw[Vi++] = fetch();

            R[dstRegID] = R[dstRegID] - V.val;
            break;
        }
        return true;
    }

    /// Format of HEAP_ALLOC
    ///  |        Opcode      |  00  |
    ///             6             2
    ///  |            size           |
    ///                8
    ///  |            size           |
    ///                8
    ///  |            size           |
    ///                8
    ///  |            size           |
    ///                8

    ///  |        Opcode      |  01  |
    ///             6             2
    ///  |              |     reg    |
    ///         4             4  
    bool Miyuki::VM::InstructionSet::InstrALLOC_HEAP() {
        uint32_t size = 0;
        if (getBits(0, 1) == 0) {
            Byte b1 = fetch();
            Byte b2 = fetch();
            Byte b3 = fetch();
            Byte b4 = fetch();
            size = (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
        }
        else {
            fetch(); 
            uint8_t regID = getBits(0, 3);
            size = R[regID];
        }
        R[12] = (uint64_t)MM->getHeap()->malloc(size);
        return true;
    }

    /// Format of FREE
    ///  |        Opcode      |  00  |
    ///             6             2
    bool Miyuki::VM::InstructionSet::InstrFREE() {
        uint64_t addr = R[12];
        Address _addr = (Address)addr;
        
        if (!MM->getHeap()->free(_addr))
            throw "invalid memory address";
        return true;
    }
}