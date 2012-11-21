/**
 * @author Your Name Here
 * @brief This file contains functions for the soc to run
 */

#include <stdio.h>
#include "socbasic.h"

// All known CPU OPCodes
enum {
    OPCODE_LOADLI = 0x01, // Fmt1
    OPCODE_LOADHI = 0x02, // Fmt1
    OPCODE_ADD    = 0x03, // Fmt2
    OPCODE_SUB    = 0x04, // Fmt2
    OPCODE_DIV    = 0x05, // Fmt2
    OPCODE_STORE  = 0x10, // Fmt3
    OPCODE_LOAD   = 0x11, // Fmt3
    OPCODE_PUSH   = 0x20, // Fmt2
    OPCODE_POP    = 0x21  // Fmt2
};

#define LOWVALUE(_value) (_value&0x0000FFFF)
#define HIGHVALUE(_value) ((_value&0xFFFF0000) >> 16)
#define NOT_USED    0xFF

/**
 * @brief resets everything
 * @param ctx CPU Context
 * @param mem Memory
 * @return true if success, otherwise failure
 */
bool resetSoC(CPUContext &ctx, Memory &mem)
{
    // Initialize CPU
    for (int i=0; i<MAX_CPU_REGISTERS; ++i) {
        ctx.reg[i] = CPU_REGISTER_RESET_VALUE;
    }

    // Set PC to Reset Vector
    ctx.reg[REG_PC] = CPU_PC_RESET_VECTOR;

    // Set SP to top of memory
    ctx.reg[REG_SP] = MEMORY_SIZE;

    // Initialize Memory
    for (int j=0; j<MEMORY_SIZE; ++j) {
        mem.data[j] = MEMORY_RESET_VALUE;
    }

    return true;
}


/**
 * @brief load the program into memory
 * @param mem where to store program
 * @return true if success, otherwise failure
 */
bool loadProgram(Memory &mem)
{
    TestInstruction instruction;
    U32 instrAddr = 0x0000;

    // Load Instructions

    {
        // reg0 = Return Address
        instruction = buildCPUInstructionFmt1(0x01, 0x00, LOWVALUE(0x28));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x00, HIGHVALUE(0x28));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // push reg0 on to the stack
        instruction = buildCPUInstructionFmt2(0x20, 0x00, NOT_USED, NOT_USED);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // A = 4... reg[0] = 4
        instruction = buildCPUInstructionFmt1(0x01, 0x00, LOWVALUE(4));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x00, HIGHVALUE(4));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // B = 3... reg[1] = 3
        instruction = buildCPUInstructionFmt1(0x01, 0x01, LOWVALUE(3));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x01, HIGHVALUE(3));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // push reg0 on to the stack
        instruction = buildCPUInstructionFmt2(0x20, 0x00, NOT_USED, NOT_USED);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // push reg1 on to the stack
        instruction = buildCPUInstructionFmt2(0x20, 0x01, NOT_USED, NOT_USED);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

		// Jump to funcAdd, 0x30
        instruction = buildCPUInstructionFmt1(0x01, REG_PC, LOWVALUE(0x30));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

		// answer
		instrAddr += 4;

		// end of program
		instrAddr += 4;

		// int funcAdd(int A, int B)

        // pop reg1 (B) off of the stack
        instruction = buildCPUInstructionFmt2(0x21, 0x01, NOT_USED, NOT_USED);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // pop reg0 (A) off of the stack
        instruction = buildCPUInstructionFmt2(0x21, 0x00, NOT_USED, NOT_USED);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

		// C = A + B
        instruction = buildCPUInstructionFmt2(0x3, 0x00, 0x01, 0x01);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr +=4;

		// pop PC, answer returned in reg1
        instruction = buildCPUInstructionFmt2(0x21, REG_PC, NOT_USED, NOT_USED);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
    }

    // Load Static Data

    return true;
}


/**
 * @brief executes 1 CPU instruction
 * @param ctx CPU Context
 * @param mem Memory
 * @return true if everything ok, otherwise stop program
 */
bool executeCPUInstruction(CPUContext &ctx, Memory &mem)
{
    bool retval;
    TestInstruction data;
    U32 oldPC = ctx.reg[REG_PC];

    if (!read32Memory(mem, ctx.reg[REG_PC], data.value32)) {
        printf("ERROR: Invalid address: 0x%08x\n", ctx.reg[REG_PC]);
        return false;
    }

    // Increment Program Count
    ctx.reg[REG_PC] += CPU_INSTRUCTION_SIZE;

    // Process the instruction
    switch (data.format1.opcode) {
    case OPCODE_LOADLI: // LOAD LOW Immediate data into register
        printf("0x%08x: OpCode: %8s(0x%02x) Reg[0x%02x].low = 0x%04x\n",
                   oldPC,
                   "LOADLI",
                   data.format1.opcode,
                   data.format1.regIndex,
                   data.format1.data);

        if (data.format1.regIndex >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            U32 value = ctx.reg[data.format1.regIndex];

            value = (value&0xFFFF0000) | (data.format1.data&0xFFFF);

            ctx.reg[data.format1.regIndex] = value;
        }
        break;
    case OPCODE_LOADHI: // LOAD HIGH Immediate data into register
        printf("0x%08x: OpCode: %8s(0x%02x) Reg[0x%02x].high = 0x%04x\n",
                   oldPC,
                   "LOADHI",
                   data.format1.opcode,
                   data.format1.regIndex,
                   data.format1.data);

        if (data.format1.regIndex >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            U32 value = ctx.reg[data.format1.regIndex];

            value = (value&0x00000FFFF) | ((data.format1.data&0xFFFF) << 16);

            ctx.reg[data.format1.regIndex] = value;
        }
        break;
    case OPCODE_ADD: // ADD reg1 + reg2 -> reg3
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x RegIndex: 0x%02x RegIndex: 0x%02x\n",
                   oldPC,
                   "ADD",
                   data.format2.opcode,
                   data.format2.regIndex1,
                   data.format2.regIndex2,
                   data.format2.regIndex3);

        if (data.format2.regIndex1 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else if (data.format2.regIndex2 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else if (data.format2.regIndex3 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            // reg1 + reg2 -> reg3
            U32 reg1 = ctx.reg[data.format2.regIndex1];
            U32 reg2 = ctx.reg[data.format2.regIndex2];
            U32 reg3;

            reg3 = reg1 + reg2;

            ctx.reg[data.format2.regIndex3] = reg3;
        }
        break;
    case OPCODE_SUB: // SUB reg1 - reg2 -> reg3
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x RegIndex: 0x%02x RegIndex: 0x%02x\n",
                   oldPC,
                   "SUB",
                   data.format2.opcode,
                   data.format2.regIndex1,
                   data.format2.regIndex2,
                   data.format2.regIndex3);

        if (data.format2.regIndex1 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else if (data.format2.regIndex2 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else if (data.format2.regIndex3 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            // reg1 - reg2 -> reg3
            U32 reg1 = ctx.reg[data.format2.regIndex1];
            U32 reg2 = ctx.reg[data.format2.regIndex2];
            U32 reg3;

            reg3 = reg1 - reg2;

            ctx.reg[data.format2.regIndex3] = reg3;
        }
        break;
    case OPCODE_STORE: // mem[reg1] <- reg2
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x RegIndex: 0x%02x Offset: 0x%02x\n",
               oldPC,
               "STORE",
               data.format3.opcode,
               data.format3.regIndex1,
               data.format3.regIndex2,
               data.format3.data);

        if (data.format3.regIndex1 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else if (data.format3.regIndex2 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            // reg1 has address
            // reg2 has value

            write32Memory(mem,
                          ctx.reg[data.format3.regIndex1] + (S8)data.format3.data,  // address
                          ctx.reg[data.format3.regIndex2]); // value
        }
        break;
    case OPCODE_LOAD: // reg2 <- mem[reg1]
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x RegIndex: 0x%02x Offset: 0x%02x\n",
               oldPC,
               "LOAD",
               data.format3.opcode,
               data.format3.regIndex1,
               data.format3.regIndex2,
               data.format3.data);

        if (data.format3.regIndex1 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else if (data.format3.regIndex2 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            // reg1 has address
            // reg2 has value

            read32Memory(mem,
                         ctx.reg[data.format3.regIndex1] + (S8)data.format3.data,  // address
                         ctx.reg[data.format3.regIndex2]); // value
        }
        break;
    case OPCODE_PUSH: // SP = SP - 4, mem[SP] = reg1
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x\n",
               oldPC,
               "PUSH",
               data.format2.opcode,
               data.format2.regIndex1);

        if (data.format2.regIndex1 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            // reg1 has value

            // Update Stack Pointer
            ctx.reg[REG_SP] = ctx.reg[REG_SP] - 4;

            // Store value on the stack
            write32Memory(mem,
                          ctx.reg[REG_SP],  // address
                          ctx.reg[data.format2.regIndex1]); // value
        }
        break;
    case OPCODE_POP: // reg1 = mem[SP], SP = SP + 4
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x\n",
               oldPC,
               "POP",
               data.format2.opcode,
               data.format2.regIndex1);

        if (data.format2.regIndex1 >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            // reg1 is where the value is stored

            // Store value on the stack
            read32Memory(mem,
                         ctx.reg[REG_SP],  // address
                         ctx.reg[data.format2.regIndex1]); // value

            // Update Stack Pointer
            ctx.reg[REG_SP] = ctx.reg[REG_SP] + 4;
        }
        break;
    default:
        // Invalid opcode
        printf("0x%08x: OpCode: %8s(0x%02x) Byte1: 0x%02x Byte2: 0x%02x Byte3: 0x%02x\n",
               oldPC,
               "INVALID",
               data.format1.opcode,
               data.value8[1],
               data.value8[2],
               data.value8[3]);

        retval = false;
    }

    if (retval == false) {
        // Last operation did not succeed
        printf("Last opcode failed to execute @ 0x%08x\n",
               oldPC);
    }

    return retval;
}

