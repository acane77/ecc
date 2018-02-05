#include "vm/Register.h"
#include <iostream>
#include <iomanip>

namespace Miyuki::VM {
    namespace Registers {
        Register R[16]; // Register file
        Register &PC = R[15],    // Program counter 
                 &SP = R[14],    // Stack pointer 
                 &SS = R[13];    // Stack pointer (value = last allocated var)
        KeyboardDataRegister  KBDR; // Keyboard Data Register
        DisplayDataRegister   DDR;  // Display Data Register
        Register8             IR;   // Instruction register
        ConditionFlagRegister CF;   // Conditional flag

        void printRegisters(){
            std::cout << "\n\n  Register values";
            for (int i = 0; i < 13; i++) {
                std::cout << "\nR" << i << ":  " << std::hex << R[i] << std::dec;
            }
            std::cout << std::hex << "\nR13 [SS]:  " << R[13];
            std::cout << std::hex << "\nR14 [SP]:  " << R[14];
            std::cout << std::hex << "\nR15 [PC]:  " << R[15];
            std::cout << std::hex << "\nIR:  " << IR;
            std::cout << std::hex << "\nCF:  " << CF;
        }
    }
}