/**
 * @author Wayne Moorefield
 * @brief This file describes a System Bus
 */

#ifndef _SOC_MEMORY_H
#define _SOC_MEMORY_H

#include "types.h"
#include "busdevice.h"

namespace soc {

/**
 * @class Memory
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file memory.h
 * @brief Descibes basic memory that's addressable up to 16KB
 */
template <int MemorySizeInBytes>
class Memory : public BusDevice
{
private:
    enum {
        DEFAULT_MEMORYVALUE = 0,
        DEFAULT_ADDRESSMASK = 0xFFFF
    };


protected:
    U8 mData[MemorySizeInBytes];


protected:
    /**
     * @brief Converts a physical address to local address
     * @param address physical address to convert
     * @return local address
     */
    virtual BusAddressType convertToLocalAddress(BusAddressType address)
    {
        // TODO: find better way of doing this
        return (address&DEFAULT_ADDRESSMASK);
    }

    /**
     * @brief Writes an 32-bit unsigned int to memory, address
     *        must already be validated
     * @param address location to write data to
     * @param value data to write
     */
    void writeU32ToMem(U16 address, U32 value)
    {
        U32 *mem32;

        // First get U32 ptr to memory
        mem32 = (U32*)&mData[address];

        // Store value in memory
        *mem32 = value;
    }


public:
    /**
     * @brief Constructor
     * @return nothing
     */
    Memory()
    {
    }

    /**
     * @brief Deconstructor
     * @return nothing
     */
    virtual ~Memory()
    {
    }

    /**
     * @brief Default execute for memory devices
     * @return true if success, otherwise false
     */
    virtual bool execute()
    {
        // nothing to do
        return true;
    }

    /**
     * @brief Default reset behavior for a memory device
     * @return true if success, otherwise false
     */
    virtual bool reset()
    {
        // Set all data to the default value
        for (int i=0; i<MemorySizeInBytes; ++i) {
            mData[i] = DEFAULT_MEMORYVALUE;
        }

        return true;
    }

    /**
     * @brief Default read operation handler
     * @param address location to read from
     * @param data location to store read value
     * @return true if success, otherwise false
     */
    virtual bool read(BusAddressType address, BusDataType& data)
    {
        BusAddressType localAddress;

        // First convert physical address to local address
        localAddress = convertToLocalAddress(address);

        // TODO: Update 4 to define that's read/write size
        if ((localAddress+4) < MemorySizeInBytes) {
            // valid address
            U32 *readLoc = (U32*)&mData[localAddress];
            data = *readLoc;
            return true;
        } else {
            // invalid address
            return false;
        }
    }

    /**
     * @brief Default write operation handler
     * @param address location to write to
     * @param data value to store
     * @return true if success, otherwise false
     */
    virtual bool write(BusAddressType address, BusDataType& data)
    {
        BusAddressType localAddress;

        // First convert physical address to local address
        localAddress = convertToLocalAddress(address);

        // TODO: Update 4 to define that's read/write size
        if ((localAddress+4) < MemorySizeInBytes) {
            // valid address
            U32 *writeLoc = (U32*)&mData[localAddress];
            *writeLoc = data;
            return true;
        } else {
            // invalid address
            return false;
        }
    }
};


} // soc

#endif
