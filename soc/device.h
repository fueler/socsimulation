/**
 * @author Wayne Moorefield
 * @brief This file describes a device
 */

#ifndef _SOC_DEVICE_H
#define _SOC_DEVICE_H

#include <string>
#include "types.h"

namespace soc {

/**
 * @class Device
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file device.h
 * @brief Describes a device
 */
class Device
{
public:
    /**
     * @brief If an active device then execute should be called
     * @return true if success, otherwise false
     */
    virtual bool execute() = 0;

    /**
     * @brief Resets the device
     * @return true if success, otherwise false
     */
    virtual bool reset() = 0;

    /**
     * @brief Read operation
     * @param address address to read
     * @param data location where to store value
     * @return true if success, otherwise false
     */
    virtual bool read(BusAddressType address, BusDataType &data) = 0;

    /**
     * @brief Write operation
     * @param address location to write
     * @param data value to store
     * @return true if success, otherwise false
     */
    virtual bool write(BusAddressType address, BusDataType &data) = 0;

    /**
     * @brief Returns name of device
     * @return String containing name
     */
    virtual std::string getName() = 0;
};

} // soc

#endif
