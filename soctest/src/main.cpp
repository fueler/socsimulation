/**
 * @author Wayne Moorefield
 * @brief This is a test app for simulating a SoC
 */

#include <stdio.h>
#include <soc/bus.h>
#include <soc/cpu.h>
#include <soc/memory.h>

union TestInstruction {
    U32 value;
    struct {
        U8 opcode;
        U8 regIndex;
        U16 data;
    } field;
};

static TestInstruction buildCPUInstruction(U8 opcode,
                                           U8 regIndex,
                                           U16 data);

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
        // 0x01 for now is storel
        // so the instruction is reg[0x00].low = 0x1234
        instruction = buildCPUInstruction(0x01, 0x00, 0x1234);
        writeU32ToMem(0x0000, instruction.value);

        // 0x02 for now is storeh
        // so the instruction is reg[0x00].high = 0xABCD
        instruction = buildCPUInstruction(0x02, 0x00, 0xABCD);
        writeU32ToMem(0x0004, instruction.value);

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

        if (getBus()->request(soc::Bus::BUSOP_READ, mContext.reg[REG_PC], data.value)) {
            printf("0x%08x: OpCode: 0x%02x RegIndex: 0x%02x Data: 0x%04x\n",
                       mContext.reg[REG_PC],
                       data.field.opcode,
                       data.field.regIndex,
                       data.field.data);

            // Increment Program Counter
            mContext.reg[REG_PC] += INSTRUCTION_SIZE;

            switch (data.field.opcode) {
            case 0x01: // LOAD LOW data into register
                if (data.field.regIndex >= MAX_CPUREGISTERS) {
                    retval = false;
                } else {
                    U32 value = mContext.reg[data.field.regIndex];

                    value = (value&0xFFFF0000) | data.field.data;

                    mContext.reg[data.field.regIndex] = value;
                }
                break;
            case 0x02: // LOAD HIGH data into register
                if (data.field.regIndex >= MAX_CPUREGISTERS) {
                    retval = false;
                } else {
                    U32 value = mContext.reg[data.field.regIndex];

                    value = (value&0x00000FFFF) | (data.field.data << 16);

                    mContext.reg[data.field.regIndex] = value;
                }
                break;
            default:
                retval = false;
            }

            if (retval == false) {
                // Invalid operation
                printf("Invalid Instruction: OpCode: 0x%02x RegIndex: 0x%02x Data: 0x%04x\n",
                       data.field.opcode,
                       data.field.regIndex,
                       data.field.data);
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
 *        in to an instruction
 * @param opcode operation to perform
 * @param regIndex register index
 * @param data register index
 * @return TestInstruction
 */
TestInstruction buildCPUInstruction(U8 opcode, U8 regIndex, U16 data)
{
    TestInstruction instruction;

    instruction.field.opcode = opcode;
    instruction.field.regIndex = regIndex;
    instruction.field.data = data;

    return instruction;
}
