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
    OPCODE_LOAD   = 0x11  // Fmt3
};

#define LOWVALUE(_value) (_value&0x0000FFFF)
#define HIGHVALUE(_value) ((_value&0xFFFF0000) >> 16)

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
    U32 stackAddr = MEMORY_SIZE - 4;

    // Load Instructions

    // Example: A = B - C
    {
        // B = 10... reg[0] = 4
        instruction = buildCPUInstructionFmt1(0x01, 0x00, LOWVALUE(4));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x00, HIGHVALUE(4));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // C = 3... reg[1] = -3
        instruction = buildCPUInstructionFmt1(0x01, 0x01, LOWVALUE(-3));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x01, HIGHVALUE(-3));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // A = B - C... reg[1] = reg[0] - reg[1]
        instruction = buildCPUInstructionFmt2(0x4, 0x00, 0x01, 0x01);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr +=4;
    }
    // Store Answer
    {
        // memory[STACKADDR] = reg[1]

        // reg[0] = stackAddr
        instruction = buildCPUInstructionFmt1(0x01, 0x00, LOWVALUE(stackAddr));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x00, HIGHVALUE(stackAddr));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // store reg1 @ memory[reg0 + offset]
        instruction = buildCPUInstructionFmt3(0x10, 0x00, 0x01, 0);
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // update stack address
        stackAddr -= 4;
    }

    // Load Answer
    {
        // reg[1] = memory[STACKADDR]

        // reg[0] =  stackAddr
        stackAddr += 4;

        instruction = buildCPUInstructionFmt1(0x01, 0x00, LOWVALUE(stackAddr));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;
        instruction = buildCPUInstructionFmt1(0x02, 0x00, HIGHVALUE(stackAddr));
        write32Memory(mem, instrAddr, instruction.value32);
        instrAddr += 4;

        // load reg0 @memory[reg0 + offset]
        instruction = buildCPUInstructionFmt3(0x11, 0x00, 0x00, 0x0);
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
