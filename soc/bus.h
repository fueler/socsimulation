/**
 * @author Wayne Moorefield
 * @brief This file describes a Bus
 */

#ifndef _SOC_BUS_H
#define _SOC_BUS_H

#include <vector>
#include <algorithm>
#include "types.h"
#include "device.h"

namespace soc {

/**
 * @class Bus
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file bus.h
 * @brief Describes a system bus for the SoC
 */
class Bus
{
public:
    enum BusDeviceType {
        BUSDEVICE_SLAVE,
        BUSDEVICE_MASTER
    };

    enum BusOperationType {
        BUSOP_RESET,
        BUSOP_READ,
        BUSOP_WRITE
    };

    struct AddressRange
    {
        BusAddressType start;
        BusAddressType end;
    };


private:
    struct DeviceContext
    {
        BusDeviceType type;
        Device *device;
        bool addressable;
        AddressRange addrRange;
    };

    std::vector<DeviceContext> mDevices;


protected:
    /**
     * @brief Finds a device context based on an address
     * @param address address to use when searching
     * @param ctx device context is returned through this
     * @return true if success, otherwise false
     */
    bool findDevice(BusAddressType address, DeviceContext &ctx)
    {
        std::vector<DeviceContext>::const_iterator it;

        // Iterate through devices connected to the bus
        for (it=mDevices.begin(); it != mDevices.end(); ++it) {
            // Check to see if the device is addressable
            if (!it->addressable) {
                // device is not addressable on the bus
                // continue searching
                continue;
            }

            // Check if address matches device
            if (address >= it->addrRange.start) {
                if (address <= it->addrRange.end) {
                    // found device
                    ctx = *it;
                    return true;
                }
            }
        }

        // unable to find device
        return false;
    }

    /**
     * @brief Finds device context based on device
     * @param device device to search for
     * @param ctx device context to return
     * @return true if success, otherwise false
     */
    bool findDevice(Device *device, DeviceContext &ctx)
    {
        std::vector<DeviceContext>::const_iterator it;

        // Iterate through devices connected to the bus
        for (it=mDevices.begin(); it != mDevices.end(); ++it) {
            // Check to see if device matches
            if (device == it->device) {
                // found device
                ctx = *it;
                return true;
            }
        }

        // unable to find device
        return false;
    }

    /**
     * @brief Resets every device attached to the bus
     * @return true if no error reseting devices, otherwise false
     */
    bool resetAll()
    {
        std::vector<DeviceContext>::const_iterator it;

        // Iterate through devices
        for (it=mDevices.begin(); it != mDevices.end(); ++it) {
            // For each device reset it
            if (!it->device->reset()) {
                // unable to reset device
                return false;
            }
        }

        return true;
    }


public:
    /**
     * @brief Constructor, initializes bus
     * @return nothing
     */
    Bus()
    {
        mDevices.clear();
    }

    /**
     * @brief Deconstructor, removes all devices from bus
     * @return nothing
     */
    virtual ~Bus()
    {
        mDevices.clear();
    }

    /**
     * @brief Performs a system reset
     * @return true if success, otherwise false
     */
    bool systemReset()
    {
        return resetAll();
    }

    /**
     * @brief Attaches a device to the bus
     * @param type either master or slave
     * @param device actual device to connect
     * @param addrRange if addressable, contains a valid range of addresses
     *                  device will respond to.
     * @return true if success, otherwise false
     */
    bool attachDevice(BusDeviceType type,
                      Device *device,
                      const AddressRange *addrRange)
    {
        DeviceContext dev;

        if (!findDevice(device, dev)) {
            // new device
            dev.type = type;
            dev.device = device;
            if (addrRange) {
                dev.addressable = true;
                dev.addrRange = *addrRange;
            } else {
                dev.addressable = false;
                dev.addrRange.start = 0;
                dev.addrRange.end = 0;
            }

            mDevices.push_back(dev);

            return true;
        } else {
            // device already attached

            return false;
        }
    }

    /**
     * @brief Removes a device from the bus
     * @param device actual device to remove
     * @return true if success, otherwise false
     */
    bool removeDevice(Device *device)
    {
        std::vector<DeviceContext>::iterator it;

        // Iternate through devices connected to the bus
        for (it = mDevices.begin(); it != mDevices.end(); ++it) {
            // Check to see if a match
            if (it->device == device) {
                // Found it, remove it from device list
                mDevices.erase(it);
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Performs a request put on the bus
     * @param op Bus Operation
     * @param address Address, will identify a device
     * @param data data to either read or write
     * @return true if success, otherwise false
     */
    bool request(BusOperationType op, BusAddressType address, BusDataType &data)
    {
        bool retval = false; // default
        DeviceContext dev;

        if (op == BUSOP_RESET) {
            // Request to reset system
            retval = resetAll();
        } else if (findDevice(address, dev)) {
            // found device that corresponds to address given
            if (op == BUSOP_WRITE) {
                // write operation was requested, perform
                // write action on device
                if (dev.device->write(address, data)) {
                    // success
                    retval = true;
                } else {
                    // device error
                }
            } else if (op == BUSOP_READ) {
                // read operation was requested, perform
                // read action on device
                if (dev.device->read(address, data)) {
                    // success
                    retval = true;
                } else {
                    // device error
                }
            }
        } else {
            // unable to find device associated
            // with the address, bus error
        }

        return retval;
    }
};

} // namespace soc

#endif
