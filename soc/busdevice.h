/**
 * @author Wayne Moorefield
 * @brief This file describes a device that connects to a bus
 */

#ifndef _SOC_BUS_DEVICE_H
#define _SOC_BUS_DEVICE_H

#include "device.h"
#include "bus.h"

namespace soc {

/**
 * @class BusDevice
 * @author Wayne Moorefield
 * @date 10/10/2012
 * @file busdevice.h
 * @brief Allows the device to talk to the bus
 */
class BusDevice : public Device
{
private:
    Bus *mBus;


protected:
    /**
     * @brief Returns bus
     * @return ptr to bus
     */
    Bus* getBus()
    {
        return mBus;
    }


public:
    /**
     * @brief Attaches device to specific bus
     * @param bus specific bus to connect to
     * @param devType master or slave
     * @param addrRange addressable range of device
     * @return true if success, otherwise false
     */
    virtual bool attachToBus(Bus *bus,
                             Bus::BusDeviceType devType,
                             const Bus::AddressRange *addrRange)
    {
        if (bus != NULL) {
            // Not connected, connect the device
            if (bus->attachDevice(devType, this, addrRange)) {
                // Success
                mBus = bus;
                return true;
            }
        } else {
            // Already connected, return error
        }

        // unable to attach bus
        return false;
    }
};

} // soc

#endif
