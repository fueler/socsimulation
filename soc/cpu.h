/**
 * @author Wayne Moorefield
 * @brief This file describes a CPU
 */

#ifndef _SOC_CPU_H
#define _SOC_CPU_H

#include "busdevice.h"

namespace soc {

/**
 * @class CPU
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file cpu.h
 * @brief Describes a CPU
 */
class CPU : public BusDevice
{
public:
    enum {
        MAX_CPUREGISTERS = 3,
        REG_PC = (MAX_CPUREGISTERS - 1)
    };

    struct CPUContext
    {
        U32 reg[MAX_CPUREGISTERS];
    };


protected:
    enum {
        RESET_ADDRESS = 0x00000000,
        CPUREGISTER_RESETVALUE = 0x1C1C1B1B,
        INSTRUCTION_SIZE = 4
    };

    CPUContext mContext;


public:
    /**
     * @brief Constructor
     * @return nothing
     */
    CPU()
    {
    }

    /**
     * @brief Deconstructor
     * @return nothing
     */
    virtual ~CPU()
    {
    }

    /**
     * @brief Default execute function
     * @return true if success, otherwise false
     */
    virtual bool execute()
    {
        BusDataType data;

        if (getBus()->request(Bus::BUSOP_READ, mContext.reg[REG_PC], data)) {
            mContext.reg[REG_PC] += INSTRUCTION_SIZE;
            // do something
            return true;
        } else {
            // failure to read next command
            // from location pc
            return false;
        }
    }

    /**
     * @brief Default reset function
     * @return true if success, otherwise false
     */
    virtual bool reset()
    {
        // Set registers to reset value
        for (int i=0; i<MAX_CPUREGISTERS; ++i) {
            mContext.reg[i] = CPUREGISTER_RESETVALUE;
        }

        // Set PC to reset address
        mContext.reg[REG_PC] = RESET_ADDRESS;

        return true;
    }

    /**
     * @brief Default read operation
     * @param address address to read from
     * @param data location to store data
     * @return true if success, otherwise false
     */
    virtual bool read(BusAddressType address, BusDataType &data)
    {
        // not supported
        return false;
    }

    /**
     * @brief Default write operation
     * @param address address to write to
     * @param data value to store
     * @return true if success, otherwise false
     */
    virtual bool write(BusAddressType address, BusDataType &data)
    {
        // not supported
        return false;
    }

    /**
     * @brief Returns current CPU context
     * @return current CPU Context
     */
    virtual CPUContext getCurrentContext()
    {
        return mContext;
    }
};

} // soc

#endif

