/**
 * @author Your Name Here
 * @brief This file contains functions for the soc to run
 */

#include <stdio.h>
#include "socbasic.h"

// All known CPU OPCodes
enum {
    OPCODE_LOADLI = 0x01,
    OPCODE_LOADHI = 0x02,
    OPCODE_ADD    = 0x03,
    OPCODE_SUB    = 0x04
};


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

    // Load Instructions

    // Example: A = 0xABCD1234
    {
        // reg[0].low = 0x1234
        instruction = buildCPUInstructionFmt1(0x01, 0x00, 0x1234);
        write32Memory(mem, 0x0000, instruction.value32);

        // reg[0].high = 0xABCD
        instruction = buildCPUInstructionFmt1(0x02, 0x00, 0xABCD);
        write32Memory(mem, 0x0004, instruction.value32);
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
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x Data: 0x%04x\n",
                   oldPC,
                   "LOADLI",
                   data.format1.opcode,
                   data.format1.regIndex,
                   data.format1.data);

        if (data.format1.regIndex >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            U32 value = ctx.reg[data.format1.regIndex];

            value = (value&0xFFFF0000) | data.format1.data;

            ctx.reg[data.format1.regIndex] = value;
        }
        break;
    case OPCODE_LOADHI: // LOAD HIGH Immediate data into register
        printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x Data: 0x%04x\n",
                   oldPC,
                   "LOADHI",
                   data.format1.opcode,
                   data.format1.regIndex,
                   data.format1.data);

        if (data.format1.regIndex >= MAX_CPU_REGISTERS) {
            retval = false;
        } else {
            U32 value = ctx.reg[data.format1.regIndex];

            value = (value&0x00000FFFF) | (data.format1.data << 16);

            ctx.reg[data.format1.regIndex] = value;
        }
        break;
    case OPCODE_ADD: // ADD reg1 + reg2 -> reg3
        break;
    case OPCODE_SUB: // SUB reg1 - reg2 -> reg3
        break;
    default:
        retval = false;
    }

    if (retval == false) {
        // Invalid operation
        printf("0x%08x: OpCode: %8s(0x%02x) Byte1: 0x%02x Byte2: 0x%02x Byte3: 0x%02x\n",
               oldPC,
               "INVALID",
               data.format1.opcode,
               data.value8[1],
               data.value8[2],
               data.value8[3]);
    }

    return retval;
}
