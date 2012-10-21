/**
 * @author Wayne Moorefield
 * @brief This is a test app for simulating a SoC
 */

#include <stdio.h>
#include <soc/bus.h>
#include <soc/cpu.h>
#include <soc/memory.h>

union TestInstruction {
    U32 value32;
    U16 value16[2];
    U8  value8[4];

    struct {
        U8 opcode;
        U8 regIndex;
        U16 data;
    } format1;

    struct {
        U8 opcode;
        U8 regIndex1;
        U8 regIndex2;
        U8 regIndex3;
    } format2;
};

static TestInstruction buildCPUInstructionFmt1(U8 opcode,
                                               U8 regIndex,
                                               U16 data);
static TestInstruction buildCPUInstructionFmt2(U8 opcode,
                                               U8 regIndex1,
                                               U8 regIndex2,
                                               U8 regIndex3);


#define MYMEMORY_SIZE   0x0100

/**
 * @class MyMemory
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file main.cpp
 * @brief Test harness for memory, preloads data in to memory
 */
class MyMemory : public soc::Memory<MYMEMORY_SIZE>
{
public:
    /**
     * @brief Puts device back in reset state
     * @return true if success, otherwise false
     */
    virtual bool reset()
    {
        // Set all data to the default value
        for (int i=0; i<MYMEMORY_SIZE; ++i) {
            mData[i] = 0xFF;
        }

        // Put preloaded memory here
        TestInstruction instruction;

        // Store temp data
        // 0x01 for now is loadli
        // so the instruction is reg[0x00].low = 0x1234
        instruction = buildCPUInstructionFmt1(0x01, 0x00, 0x1234);
        writeU32ToMem(0x0000, instruction.value32);

        // 0x02 for now is loadhi
        // so the instruction is reg[0x00].high = 0xABCD
        instruction = buildCPUInstructionFmt1(0x02, 0x00, 0xABCD);
        writeU32ToMem(0x0004, instruction.value32);

        return true;
    }

    /**
     * @brief Returns name of device
     * @return String containing name
     */
    virtual std::string getName()
    {
        return std::string("MyMemory");
    }
};


/**
 * @class MyCPU
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file main.cpp
 * @brief Test harness for cpu
 */
class MyCPU : public soc::CPU
{
public:
    /**
     * @brief Returns device name
     * @return String containing device name
     */
    virtual std::string getName()
    {
        return std::string("MyCPU");
    }

    /**
     * @brief Executes one CPU cycle
     * @return true if success, otherwise false
     */
    virtual bool execute()
    {
        bool retval = true;
        TestInstruction data;
        U32 oldPC = mContext.reg[REG_PC];

        if (getBus()->request(soc::Bus::BUSOP_READ, mContext.reg[REG_PC], data.value32)) {
            // Increment Program Counter
            mContext.reg[REG_PC] += INSTRUCTION_SIZE;

            switch (data.format1.opcode) {
            case 0x01: // LOAD LOW Immediate data into register
                printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x Data: 0x%04x\n",
                           oldPC,
                           "LOADLI",
                           data.format1.opcode,
                           data.format1.regIndex,
                           data.format1.data);

                if (data.format1.regIndex >= MAX_CPUREGISTERS) {
                    retval = false;
                } else {
                    U32 value = mContext.reg[data.format1.regIndex];

                    value = (value&0xFFFF0000) | data.format1.data;

                    mContext.reg[data.format1.regIndex] = value;
                }
                break;
            case 0x02: // LOAD HIGH Immediate data into register
                printf("0x%08x: OpCode: %8s(0x%02x) RegIndex: 0x%02x Data: 0x%04x\n",
                           oldPC,
                           "LOADHI",
                           data.format1.opcode,
                           data.format1.regIndex,
                           data.format1.data);

                if (data.format1.regIndex >= MAX_CPUREGISTERS) {
                    retval = false;
                } else {
                    U32 value = mContext.reg[data.format1.regIndex];

                    value = (value&0x00000FFFF) | (data.format1.data << 16);

                    mContext.reg[data.format1.regIndex] = value;
                }
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
        } else {
            // failure to read next command
            // from location pc
            printf("Bus failure for address: 0x%08x\n", mContext.reg[REG_PC]);
            retval = false;
        }

        return retval;
    }
};


/**
 * @brief Program Entry Point
 * @param argc number of arguments passed
 * @param argv ptr to arguments
 * @return 0 if success, otherwise error
 */
int main(int argc, char *argv[])
{
    printf("Creating SoC\n");

    soc::Bus bus;
    MyMemory mem;
    MyCPU cpu;

    soc::Bus::AddressRange addrRange;

    // set up memory
    addrRange.start = 0x0000;
    addrRange.end = 0x1000;
    if (!mem.attachToBus(&bus, soc::Bus::BUSDEVICE_SLAVE, &addrRange)) {
        printf("Failed to attach memory to bus\n");
    }

    // set up cpu
    if (!cpu.attachToBus(&bus, soc::Bus::BUSDEVICE_MASTER, &addrRange)) {
        printf("Failed to attach cpu to bus\n");
    }

    printf("SoC Created\n");

    // Send Reset Signal
    if (!bus.systemReset()) {
        printf("Failed to reset system\n");
    }

    // execute until error
    while (cpu.execute()) {
        // Any debugging per execute do here
    }
    printf("CPU has stopped\n");

    printf("Verification begin\n");
    // Do something
    // - check cpu
    // - check memory

    // CPU
    soc::CPU::CPUContext cpuctx;
    cpuctx = cpu.getCurrentContext();

    // Dump contents of CPU
    printf("CPU\n");
    printf("\tpc = 0x%08x\n", cpuctx.reg[soc::CPU::REG_PC]);
    for (int i=0; i<soc::CPU::MAX_CPUREGISTERS - 1; ++i) {
        printf("\treg[%d] = 0x%08x\n", i, cpuctx.reg[i]);
    }

    // Verify contents of Registers
    // PC correct?
    // Registers correct?

    // Memory
    // Is Memory correct?

    printf("Verification complete\n");

    return 0;
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
