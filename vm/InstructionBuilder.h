#ifndef _MIYUKI_VM_INSTRUCTION_BUILDER_H
#define _MIYUKI_VM_INSTRUCTION_BUILDER_H

#include "vm/Instruction.h"

namespace Miyuki::VM {

    namespace Constants {
        enum {
            R0 = 0, R1, R2, R3, R4, R5, R6, R7,
            R8, R9, R10, R11, R12, R13, R14, R15,

            SS = 13, SP, PC,

            byte = 0, word, doubleWord, quadraWord,
            floatType = 0, doubleType = 1
        };

    }

    class InstructionBuilder {
        MemoryMap * MM;
        uint32_t insertPos;
        std::unordered_map<int, uint32_t> labels;
        std::unordered_map<uint32_t, int> labelFilledPos;

        void emit(Byte b);

    public:
        InstructionBuilder(MemoryMap* _MM);
        
        enum VarType {
            byte = 0,
            word,
            doubleWord,
            quadraWord,
            floatType = 0, 
            doubleType
        };

        void insertSystemCode() { insertPos = MemoryMap::SysCodeStart; }
        void insertUserCode() { insertPos = MemoryMap::CodeStart; }
        void setInsertPos(uint32_t pos);

        void buildADD(VarType type);//
        void buildSUB(VarType type);//
        void buildMUL(VarType type);//
        void buildDIV(VarType type);//
        void buildMOD(VarType type);//
        void buildINC(VarType type);//
        void buildDEC(VarType type);//
        void buildRADD(VarType type, uint8_t regID, uint64_t imm);//
        void buildRSUB(VarType type, uint8_t regID, uint64_t imm);//
        void buildFADD(VarType type);//
        void buildFSUB(VarType type);//
        void buildFMUL(VarType type);//
        void buildFDIV(VarType type);//
        void buildCMP(VarType type);//
        void buildFCMP(VarType type);//
        void buildAND(VarType type);//
        void buildOR(VarType type);//
        void buildNOT(VarType type);//
        void buildXOR(VarType type);//
        void buildLSF(VarType type);//
        void buildRSF(VarType type);//

        void buildJE(uint32_t addr); //
        void buildJNE(uint32_t addr);//
        void buildJGT(uint32_t addr);//
        void buildJGE(uint32_t addr);//
        void buildJLT(uint32_t addr);//
        void buildJLE(uint32_t addr);//
        void buildJMP(uint32_t addr);//

        void buildJE_Label(int label); //
        void buildJNE_Label(int label);//
        void buildJGT_Label(int label);//
        void buildJGE_Label(int label);//
        void buildJLT_Label(int label);//
        void buildJLE_Label(int label);//
        void buildJMP_Label(int label);//

        void buildJE_P(uint16_t PCOffset);//
        void buildJNE_P(uint16_t PCOffset);//
        void buildJGT_P(uint16_t PCOffset);//
        void buildJGE_P(uint16_t PCOffset);//
        void buildJLT_P(uint16_t PCOffset);//
        void buildJLE_P(uint16_t PCOffset);//
        void buildJMP_P(uint16_t PCOffset);//

        void buildJE(uint8_t baseR, uint16_t offset);//
        void buildJNE(uint8_t baseR, uint16_t offset);//
        void buildJGT(uint8_t baseR, uint16_t offset);//
        void buildJGE(uint8_t baseR, uint16_t offset);//
        void buildJLT(uint8_t baseR, uint16_t offset);//
        void buildJLE(uint8_t baseR, uint16_t offset);//
        void buildJMP(uint8_t baseR, uint16_t offset);//

        void buildCall(uint16_t PCOffset); //
        void buildCall(uint8_t baseR, uint16_t offset);//
        void buildRET();//
        void buildTRAP(uint8_t trapVector);//

        void buildLEA(uint8_t dstR, uint16_t PCOffset);
        void buildLD(VarType type, uint8_t dstR, uint16_t PCOffset);//
        void buildLDI(VarType type, uint8_t dstR, uint16_t PCOffset);//
        void buildLDR(VarType type, uint8_t dstR, uint8_t baseR, uint16_t offset);//
        void buildST(VarType type, uint8_t srcR, uint16_t PCOffset);//
        void buildSTI(VarType type, uint8_t srcR, uint16_t PCOffset);//
        void buildSTR(VarType type, uint8_t srcR, uint8_t baseR, uint16_t offset);//

        void buildMOV(uint8_t dstR, uint8_t srcR);//
        void buildMOV(uint8_t dstR, VarType type, uint64_t imm);//
        void buildMOVF(uint8_t dstR, VarType type, double imm);//

        void buildPUSH(VarType type, uint64_t imm);
        void buildPUSHF(VarType type, double imm);
        void buildPUSHR(VarType type, uint8_t srcR);
        void buildPOP(VarType type);

        void buildHALT();
        void buildHEAP_AllOC(uint32_t size);
        void buildHEAP_AllOC_R(uint8_t reg);
        void buildFREE();

        void buildString(const char * str);
        void buildLabel(int label);
        void buildZeros(size_t count);
        
        template <class T>
        void fill(T val);

        uint32_t getLabel(int label);
        uint32_t getInsertPos();

        uint32_t holdLabel(int label, int offset);

        void endInsert();
    };

}

#endif