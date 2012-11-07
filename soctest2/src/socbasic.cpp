/**
 * @author Wayne Moorefield
 * @brief SoC Util Functions
 */

#include <stdio.h>
#include "socbasic.h"



/**
 * @brief Runs the program loaded in to memory using
 *        cpu context passed in.
 * @param ctx CPU Context
 * @param mem Memory to use
 * @return true if success, otherwise error
 */
bool runProgram(CPUContext &ctx, Memory &mem)
{
    bool finished = false;

    while (!finished) {
        if (!executeCPUInstruction(ctx, mem)) {
            printf("Program Finished\n");
            finished = true;
        }
    }

    return true;
}


/**
 * @brief Dumps the current status of the cpu
 * @param ctx CPU Context
 */
void debugDumpCPU(const CPUContext &ctx)
{
    printf("CPU\n");
    printf("\tpc = 0x%08x\n", ctx.reg[REG_PC]);
    for (int i=0; i<MAX_CPU_REGISTERS - 1; ++i) {
        printf("\treg[%d] = 0x%08x\n", i, ctx.reg[i]);
    }
}


/**
 * @brief Dumps the current status of the cpu
 * @param mem Memory to use
 * @param first first address to dump
 * @param last last address to dump
 */
void debugDumpMemory(const Memory &mem, U32 first=0, U32 last=MEMORY_SIZE-1)
{
    int alignment = 16;

    // Align memory for debug printing
    first = first & (~(alignment-1));
    last = (last + (alignment-1)) & (~(alignment-1));

    // Print memory
    printf("Memory (0x%08x-0x%08x)\n", first, last);
    for (U32 addr=first; addr<last; addr+= alignment) {
        printf("0x%08x", addr);
        for (int i=0; i<alignment; ++i) {
            if ((i&(alignment/2 - 1)) == 0) {
                printf(" ");
            }

            printf("%02x ", mem.data[addr+i]);
        }

        for (int i=0; i<alignment; ++i) {
            if ((i&(alignment/2 - 1)) == 0) {
                printf(" ");
            }

            if ((mem.data[addr+i] >= 32) && (mem.data[addr+i] < 127)) {
                printf("%c", mem.data[addr+i]);
            } else if ((mem.data[addr+i] >= 129) && (mem.data[addr+i] < 255)) {
                printf("%c", mem.data[addr+i]);
            } else {
                printf(".");
            }
        }

        printf("\n");
    }
}

/**
 * @brief Dumps the current status of the SoC
 * @param ctx CPU Context
 * @param mem Memory to use
 */
void debugDumpSocStatus(const CPUContext &ctx, const Memory &mem)
{
    // Dump contents of CPU
    debugDumpCPU(ctx);

    // Add a line between them
    printf("\n");

    // Dump contents of Memory
    debugDumpMemory(mem);
}


/**
 * @brief reads 8-bit value memory at address and stores it in value
 * @param mem memory to read from
 * @param address location to read from
 * @param value location to store value read
 * @return true if success, otherwise false
 */
bool read8Memory(const Memory &mem, U32 address, U8 &value)
{
    bool retval;

    if (address < MEMORY_SIZE) {
        value = mem.data[address];
        retval = true;
    } else {
        printf("READ ERROR: Address 0x%08x >= 0x%08x\n",
               address, MEMORY_SIZE);
        retval = false;
    }

    return retval;
}


/**
 * @brief reads 16-bit value memory at address and stores it in value
 * @param mem memory to read from
 * @param address location to read from
 * @param value location to store value read
 * @return true if success, otherwise false
 */
bool read16Memory(const Memory &mem, U32 address, U16 &value)
{
    bool retval;

    // Check alignment
    if ((address & 0x1) == 0) {
        if (address < MEMORY_SIZE) {
            U16 *memptr = (U16*)&mem.data[address];
            value = *memptr;
            value = byteswap16(value);
            retval = true;
        } else {
            printf("READ ERROR: Address 0x%08x >= 0x%08x\n",
                   address, MEMORY_SIZE);
            retval = false;
        }
    } else {
        printf("READ ERROR: Address 0x%08x is not 16-bit aligned\n", address);
        retval = false;
    }

    return retval;
}


/**
 * @brief reads 32-bit value memory at address and stores it in value
 * @param mem memory to read from
 * @param address location to read from
 * @param value location to store value read
 * @return true if success, otherwise false
 */
bool read32Memory(const Memory &mem, U32 address, U32 &value)
{
    bool retval;

    // Check alignment
    if ((address & 0x3) == 0) {
        if (address < MEMORY_SIZE) {
            U32 *memptr = (U32*)&mem.data[address];
            value = *memptr;
            value = byteswap32(value);
            retval = true;
        } else {
            printf("READ ERROR: Address 0x%08x >= 0x%08x\n",
                   address, MEMORY_SIZE);
            retval = false;
        }
    } else {
        printf("READ ERROR: Address 0x%08x is not 32-bit aligned\n", address);
        retval = false;
    }

    return retval;
}


/**
 * @brief writes 8-bit value to memory at address
 * @param mem memory to write to
 * @param address location to write to
 * @param value value to store in memory
 * @return true if success, otherwise false
 */
bool write8Memory(Memory &mem, U32 address, const U8 &value)
{
    bool retval;

    if (address < MEMORY_SIZE) {
        mem.data[address] = value;
        retval = true;
    } else {
        printf("WRITE ERROR: Address 0x%08x >= 0x%08x\n",
               address, MEMORY_SIZE);
        retval = false;
    }

    return retval;
}


/**
 * @brief writes 16-bit value to memory at address
 * @param mem memory to write to
 * @param address location to write to
 * @param value value to store in memory
 * @return true if success, otherwise false
 */
bool write16Memory(Memory &mem, U32 address, const U16 &value)
{
    bool retval;

    // Check alignment
    if ((address & 0x3) == 0) {
        if (address < MEMORY_SIZE) {
            U16 *memptr = (U16*)&mem.data[address];
            *memptr = byteswap16(value);
            retval = true;
        } else {
            printf("WRITE ERROR: Address 0x%08x >= 0x%08x\n",
                   address, MEMORY_SIZE);
            retval = false;
        }
    } else {
        printf("WRITE ERROR: Address 0x%08x is not 16-bit aligned\n", address);
        retval = false;
    }

    return retval;
}


/**
 * @brief writes 32-bit value to memory at address
 * @param mem memory to write to
 * @param address location to write to
 * @param value value to store in memory
 * @return true if success, otherwise false
 */
bool write32Memory(Memory &mem, U32 address, const U32 &value)
{
    bool retval;

    // Check alignment
    if ((address & 0x3) == 0) {
        if (address < MEMORY_SIZE) {
            U32 *memptr = (U32*)&mem.data[address];
            *memptr = byteswap32(value);
            retval = true;
        } else {
            printf("WRITE ERROR: Address 0x%08x >= 0x%08x\n",
                   address, MEMORY_SIZE);
            retval = false;
        }
    } else {
        printf("WRITE ERROR: Address 0x%08x is not 32-bit aligned\n", address);
        retval = false;
    }

    return retval;
}


/**
 * @brief Takes an opcode, register index and data and formats it
 *        in to an instruction format 1 type
 * @param opcode operation to perform
 * @param regIndex register index
 * @param data immediate value
 * @return TestInstruction
 */
TestInstruction buildCPUInstructionFmt1(U8 opcode, U8 regIndex, U16 data)
{
    TestInstruction instruction;

    instruction.format1.opcode = opcode;
    instruction.format1.regIndex = regIndex;
    instruction.format1.data = data;

    return instruction;
}


/**
 * @brief Takes an opcode and three register indexes and formats it
 *        in to an instruction format 2 type
 * @param opcode operation to perform
 * @param regIndex1 register index
 * @param regIndex2 register index
 * @param regIndex3 register index
 * @return TestInstruction
 */
TestInstruction buildCPUInstructionFmt2(U8 opcode,
                                        U8 regIndex1,
                                        U8 regIndex2,
                                        U8 regIndex3)
{
    TestInstruction instruction;

    instruction.format2.opcode = opcode;
    instruction.format2.regIndex1 = regIndex1;
    instruction.format2.regIndex2 = regIndex2;
    instruction.format2.regIndex3 = regIndex3;

    return instruction;
}


/**
 * @brief Takes an opcode, two register indexes and data and formats it
 *        in to an instruction format 3 type
 * @param opcode operation to perform
 * @param regIndex1 register index
 * @param regIndex2 register index
 * @param data data
 * @return TestInstruction
 */
TestInstruction buildCPUInstructionFmt3(U8 opcode,
                                        U8 regIndex1,
                                        U8 regIndex2,
                                        U8 data)
{
    TestInstruction instruction;

    instruction.format3.opcode = opcode;
    instruction.format3.regIndex1 = regIndex1;
    instruction.format3.regIndex2 = regIndex2;
    instruction.format3.data = data;

    return instruction;
}
