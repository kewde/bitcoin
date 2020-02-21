// Copyright (c) 2018-2019 The Particl Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <usbdevice/usbdevice.h>
#include <key/extkey.h>

#include <memory>

namespace usb_device {

class CEmulatorDevice
{
public:
    CEmulatorDevice(
        DeviceTypeID type_, const char* ip_, int port_)
        : type(type_), ip(ip_), port(port_)
        {};

    DeviceTypeID type = USBDEVICE_UNKNOWN;
    const char* ip = "";
    int port = 0;
    
    int getNewUsbDevice(CUSBDevice &device) {
        sockaddr_in emulator_destination;
        emulator_destination.sin_family = AF_INET;
        emulator_destination.sin_port = htons(port);
        if (inet_aton(ip, &emulator_destination.sin_addr)==0) {
            return 1;
        }

        if (type == EMULATOR_LEDGER) {
            // 127.0.0.1:40000
            CUSBDevice d = new usb_device::CLedgerDevice(emulator_destination);
        
        } else if (type == EMULATOR_TREZOR) {
            // 127.0.0.1:21324
            CUSBDevice d = new usb_device::CTrezorDevice(emulator_destination);
        } else {
            // Unknown emulator type
            return 2;
        
        }

        return 0;
    }
};

} // usb_device
