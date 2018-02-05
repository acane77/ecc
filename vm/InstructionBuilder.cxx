#include "vm/InstructionBuilder.h"

namespace Miyuki::VM {
    using namespace Registers;

    typedef union { Byte raw[8]; uint64_t val; } ByteSet64;
    typedef union { Byte raw[4]; uint32_t val; } ByteSet32;
    typedef union { Byte raw[2]; uint16_t val; } ByteSet16;
    typedef union { Byte raw[4]; float    val; } ByteSetF32;
    typedef union { Byte raw[8]; double   val; } ByteSetF64;

    InstructionBuilder::InstructionBuilder(MemoryMap* _MM) {
        MM = _MM;
        insertPos = MemoryMap::CodeStart;
    }

    void Miyuki::VM::InstructionBuilder::emit(Byte b) {
        MM->set(insertPos++, b);
        //printf("[%x]  %s%x %c\n", insertPos - 1, b&0xF0?"":"0", b, b >= 32 && b < 127 ? b : ' ');
    }

#define ADD_EMIT_FUNCTION_ARITH(opcode)\
    void Miyuki::VM::InstructionBuilder::build##opcode(VarType type) {\
        emit(InstructionSet::opcode << 2 | type);\
    }

    ADD_EMIT_FUNCTION_ARITH(ADD)
    ADD_EMIT_FUNCTION_ARITH(SUB)
    ADD_EMIT_FUNCTION_ARITH(MUL)
    ADD_EMIT_FUNCTION_ARITH(DIV)
    ADD_EMIT_FUNCTION_ARITH(MOD)
    ADD_EMIT_FUNCTION_ARITH(INC)
    ADD_EMIT_FUNCTION_ARITH(DEC)
    ADD_EMIT_FUNCTION_ARITH(FADD)
    ADD_EMIT_FUNCTION_ARITH(FSUB)
    ADD_EMIT_FUNCTION_ARITH(FMUL)
    ADD_EMIT_FUNCTION_ARITH(FDIV)
    ADD_EMIT_FUNCTION_ARITH(CMP)
    ADD_EMIT_FUNCTION_ARITH(FCMP)
    ADD_EMIT_FUNCTION_ARITH(AND)
    ADD_EMIT_FUNCTION_ARITH(OR)
    ADD_EMIT_FUNCTION_ARITH(NOT)
    ADD_EMIT_FUNCTION_ARITH(XOR)
    ADD_EMIT_FUNCTION_ARITH(LSF)
    ADD_EMIT_FUNCTION_ARITH(RSF)

    void Miyuki::VM::InstructionBuilder::buildRADD(VarType type, uint8_t regID, uint64_t imm) {
        emit(InstructionSet::RADD << 2 | type);
        emit(regID);
        ByteSet64 U; U.val = imm;
        uint8_t size = 1 << type;
        for (int i = 0; i < size; i++)
            emit(U.raw[i]);
    }

    void Miyuki::VM::InstructionBuilder::buildRSUB(VarType type, uint8_t regID, uint64_t imm) {
        emit(InstructionSet::RSUB << 2 | type);
        emit(regID);
        ByteSet64 U; U.val = imm;
        uint8_t size = 1 << type;
        for (int i = 0; i < size; i++)
            emit(U.raw[i]);
    }

#define ADD_EMIT_FUNCTION_JUMP_IMM(opcode)\
    void Miyuki::VM::InstructionBuilder::build##opcode(uint32_t addr)  {\
        emit(InstructionSet::opcode << 2);\
        ByteSet32 U; U.val = addr;\
        for (int i = 0; i < 4; i++)\
            emit(U.raw[i]);\
    }

    ADD_EMIT_FUNCTION_JUMP_IMM(JE)
        ADD_EMIT_FUNCTION_JUMP_IMM(JNE)
        ADD_EMIT_FUNCTION_JUMP_IMM(JGT)
        ADD_EMIT_FUNCTION_JUMP_IMM(JGE)
        ADD_EMIT_FUNCTION_JUMP_IMM(JLT)
        ADD_EMIT_FUNCTION_JUMP_IMM(JLE)
        ADD_EMIT_FUNCTION_JUMP_IMM(JMP)

#define ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(opcode)\
    void Miyuki::VM::InstructionBuilder::build##opcode##_P(uint16_t PCOffset) {\
        emit(InstructionSet::opcode << 2 | 1);\
        ByteSet16 U; U.val = PCOffset;\
        emit(U.raw[0]);\
        emit(U.raw[1]);\
    }

        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JE)
        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JNE)
        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JGT)
        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JGE)
        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JLT)
        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JLE)
        ADD_EMIT_FUNCTION_JUMP_PC_OFFSET(JMP)

#define ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(opcode)\
    void Miyuki::VM::InstructionBuilder::build##opcode(uint8_t baseR, uint16_t offset) {\
        emit(InstructionSet::opcode << 2 | 2);\
        emit(baseR);\
        emit(offset & 0xFF);\
        emit((offset & 0xFF00) >> 8);\
    }

        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JE)
        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JNE)
        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JGT)
        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JGE)
        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JLT)
        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JLE)
        ADD_EMIT_FUNCTION_JUMP_REG_OFFSET(JMP)

#define ADD_EMIT_FUNCTION_JUMP_LABEL(opcode)\
    void Miyuki::VM::InstructionBuilder::build##opcode##_Label(int label) {\
        emit(InstructionSet::opcode << 2);\
        labelFilledPos[insertPos] = label;\
        /*printf("%x -> label %d\n", insertPos, label);*/\
        emit(0); emit(0); emit(0); emit(0);\
        /* fill back address after insert */\
    }

    ADD_EMIT_FUNCTION_JUMP_LABEL(JE)
    ADD_EMIT_FUNCTION_JUMP_LABEL(JNE)
    ADD_EMIT_FUNCTION_JUMP_LABEL(JGT)
    ADD_EMIT_FUNCTION_JUMP_LABEL(JGE)
    ADD_EMIT_FUNCTION_JUMP_LABEL(JLT)
    ADD_EMIT_FUNCTION_JUMP_LABEL(JLE)
    ADD_EMIT_FUNCTION_JUMP_LABEL(JMP)

    void Miyuki::VM::InstructionBuilder::buildCall(uint16_t PCOffset) {
        emit(InstructionSet::CALL << 2);
        emit(PCOffset & 0xFF);
        emit((PCOffset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildCall(uint8_t baseR, uint16_t offset) {
        emit(InstructionSet::CALL << 2 | 1);
        emit(baseR);
        emit(offset & 0xFF);
        emit((offset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildRET() {
        emit(InstructionSet::RET << 2);
    }

    void Miyuki::VM::InstructionBuilder::buildTRAP(uint8_t trapVector) {
        emit(InstructionSet::TRAP << 2);
        emit(trapVector);
    }

    void Miyuki::VM::InstructionBuilder::buildLD(VarType type, uint8_t dstR, uint16_t PCOffset) {
        emit(InstructionSet::LD << 2 | type);
        emit(dstR);
        emit(PCOffset & 0xFF);
        emit((PCOffset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildLDI(VarType type, uint8_t dstR, uint16_t PCOffset) {
        emit(InstructionSet::LDI << 2 | type);
        emit(dstR);
        emit(PCOffset & 0xFF);
        emit((PCOffset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildLDR(VarType type, uint8_t dstR, uint8_t baseR, uint16_t offset) {
        emit(InstructionSet::LDR << 2 | type);
        emit(baseR << 4 | dstR);
        emit(offset & 0xFF);
        emit((offset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildST(VarType type, uint8_t srcR, uint16_t PCOffset) {
        emit(InstructionSet::ST << 2 | type);
        emit(srcR);
        emit(PCOffset & 0xFF);
        emit((PCOffset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildSTI(VarType type, uint8_t srcR, uint16_t PCOffset) {
        emit(InstructionSet::STI << 2 | type);
        emit(srcR);
        emit(PCOffset & 0xFF);
        emit((PCOffset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildSTR(VarType type, uint8_t srcR, uint8_t baseR, uint16_t offset) {
        emit(InstructionSet::STR << 2 | type);
        emit(baseR << 4 | srcR);
        emit(offset & 0xFF);
        emit((offset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildMOV(uint8_t dstR, uint8_t srcR) {
        emit(InstructionSet::MOV << 2);
        emit(dstR << 4 | srcR);
    }

    void Miyuki::VM::InstructionBuilder::buildMOV(uint8_t dstR, VarType type, uint64_t imm) {
        emit(InstructionSet::MOV << 2 | 1);
        emit(dstR << 4 | type);
        ByteSet64 U; U.val = imm;
        uint8_t size = 1 << type;
        for (int i = 0; i < size; i++)
            emit(U.raw[i]);
    }

    void Miyuki::VM::InstructionBuilder::buildPUSH(VarType type, uint64_t imm) {
        emit(InstructionSet::PUSH << 2 | type);
        ByteSet64 U; U.val = imm;
        uint8_t size = 1 << type;
        for (int i = 0; i < size; i++)
            emit(U.raw[i]);
    }

    void Miyuki::VM::InstructionBuilder::buildPUSHR(VarType type, uint8_t srcR) {
        emit(InstructionSet::PUSHR << 2 | type);
        emit(srcR);
    }

    void Miyuki::VM::InstructionBuilder::buildPOP(VarType type) {
        emit(InstructionSet::POP << 2 | type);
    }

    void Miyuki::VM::InstructionBuilder::buildHALT() {
        emit(InstructionSet::HALT << 2);
    }

    void Miyuki::VM::InstructionBuilder::buildHEAP_AllOC(uint32_t size) {
        emit(InstructionSet::HEAP_ALLOC << 2);
        ByteSet32 U; U.val = size;
        for (int i = 0; i < 4; i++)
            emit(U.raw[i]);
    }

    void Miyuki::VM::InstructionBuilder::buildHEAP_AllOC_R(uint8_t reg) {
        emit(InstructionSet::HEAP_ALLOC << 2 | 1);
        emit(reg);
    }

    void Miyuki::VM::InstructionBuilder::buildFREE() {
        emit(InstructionSet::FREE << 2);
    }

    void Miyuki::VM::InstructionBuilder::buildLEA(uint8_t dstR, uint16_t PCOffset) {
        emit(InstructionSet::LEA << 2);
        emit(dstR);
        emit(PCOffset & 0xFF);
        emit((PCOffset & 0xFF00) >> 8);
    }

    void Miyuki::VM::InstructionBuilder::buildMOVF(uint8_t dstR, VarType type, double imm)  {
        emit(InstructionSet::MOV << 2);
        emit(dstR << 4 | (type + 2));
        if (type == VarType::floatType) {
            ByteSetF32 U; U.val = imm;
            for (int i = 0; i < 4; i++)
                emit(U.raw[i]);
        }
        else if (type == VarType::doubleType) {
            ByteSetF64 U; U.val = imm;
            for (int i = 0; i < 8; i++)
                emit(U.raw[i]);
        }
        else assert(false && "invalid VarType");
    }

    void Miyuki::VM::InstructionBuilder::buildPUSHF(VarType type, double imm) {
        emit(InstructionSet::PUSH << 2 | (type + 2));
        if (type == VarType::floatType) {
            ByteSetF32 U; U.val = imm;
            for (int i = 0; i < 4; i++)
                emit(U.raw[i]);
        }
        else if (type == VarType::doubleType) {
            ByteSetF64 U; U.val = imm;
            for (int i = 0; i < 8; i++)
                emit(U.raw[i]);
        }
        else assert(false && "invalid VarType");
    }

    void Miyuki::VM::InstructionBuilder::setInsertPos(uint32_t pos) {
        if (pos < MemoryMap::CodeStart) {
            throw "invalid insert position";
        }
        insertPos = pos;
    }

    void Miyuki::VM::InstructionBuilder::buildLabel(int label) {
        //printf("label %d:\n", label);
        labels[label] = insertPos;
    }

    uint32_t Miyuki::VM::InstructionBuilder::getLabel(int label) {
        std::unordered_map<int, uint32_t>::iterator it = labels.find(label);
        if (it != labels.end()) return it->second;
        return 0;
    }

    uint32_t Miyuki::VM::InstructionBuilder::getInsertPos() {
        return insertPos;
    }

    void Miyuki::VM::InstructionBuilder::endInsert() {
        // Fillback label
        for (std::unordered_map<uint32_t, int>::iterator it = labelFilledPos.begin();
            it != labelFilledPos.end(); ++it) {
            ByteSet32 U; U.val = getLabel(it->second);
            if (U.val == 0) {
                printf("No such label\n");
                continue;
            }
            //printf("%x -> label %d (%x)\n", it->first, it->second, U.val);
            for (int i = 0; i < 4; i++) {
                MM->set(it->first + i, U.raw[i]);
            }
        }

        //printf("Insert complete\n");
    }

    void Miyuki::VM::InstructionBuilder::buildString(const char * str) {
        for (; *str; str++)
            emit(*str);
        emit(0);
    }

    uint32_t Miyuki::VM::InstructionBuilder::holdLabel(int label, int offset) {
        labelFilledPos[insertPos + offset] = label; 
        //printf("%x -> label %d\n", insertPos + offset, label);
        return 0;
    }

    void Miyuki::VM::InstructionBuilder::buildZeros(size_t count) {
        for (int i = 0; i < count; i++)
            emit(0);
    }

    template<class T>
    void Miyuki::VM::InstructionBuilder::fill(T val) {
        union {
            Byte raw[sizeof(T)];
            T val;
        } U; U.val = val;
        for (int i = 0; i < sizeof(T); i++)
            emit(U.raw[i]);
    }


}