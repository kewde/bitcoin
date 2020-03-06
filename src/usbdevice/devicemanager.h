// Copyright (c) 2018-2019 The Particl Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <usbdevice/usbdevice.h>
#include <usbdevice/debugdevice.h>
#include <usbdevice/trezordevice.h>
#include <usbdevice/ledgerdevice.h>
#include <key/extkey.h>

#include <memory>

namespace usb_device {

class CEmulatorDevice
{
public:
    CEmulatorDevice(
        DeviceTypeID type_, std::string ip_, int port_)
        : type(type_), ip(ip_), port(port_)
        {};

    DeviceTypeID type = USBDEVICE_UNKNOWN;
    std::string ip = "";
    int port = 0;
    
    int getNewUsbDevice(std::unique_ptr<CUSBDevice> &pDevice) {
        sockaddr_in emulator_destination;
        emulator_destination.sin_family = AF_INET;
        emulator_destination.sin_port = htons(port);
        const char* ip_ = ip.c_str();
        if (inet_aton(ip_, &emulator_destination.sin_addr)==0) {
            return 1;
        }

        if (type == EMULATOR_LEDGER) {
            // 127.0.0.1:40000
            // device = new usb_device::CLedgerDevice(emulator_destination);
             std::unique_ptr<CUSBDevice> device(new CLedgerDevice(emulator_destination));
             pDevice.swap(device);
        } else if (type == EMULATOR_TREZOR) {
            // 127.0.0.1:21324
            std::unique_ptr<CUSBDevice> device(new CTrezorDevice(emulator_destination));
            pDevice.swap(device);
        } else {
            // Unknown emulator type
            return 2;
        
        }

        return 0;
    }
};

int AddEmulatorDevice(DeviceTypeID type, std::string ip, int port, std::unique_ptr<CUSBDevice> &device);
int RemoveEmulatorDevice(DeviceTypeID type, std::string ip, int port);
void ListEmulatorDevices(std::vector<std::unique_ptr<CUSBDevice> > &vDevices);
void ListAllDevices(std::vector<std::unique_ptr<CUSBDevice> > &vDevices);
CUSBDevice *SelectDevice(std::vector<std::unique_ptr<CUSBDevice> > &vDevices, std::string &sError);

} // usb_device
