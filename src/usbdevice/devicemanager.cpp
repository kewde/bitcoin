
// Copyright (c) 2018-2019 The Particl Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <usbdevice/devicemanager.h>

#include <chainparams.h>
#include <logging.h>

namespace usb_device {

std::vector<CEmulatorDevice *> emulators;

int AddEmulatorDevice(DeviceTypeID type, std::string ip, int port, std::unique_ptr<CUSBDevice> &device)
{
    CEmulatorDevice *factory = new CEmulatorDevice(type, ip, port);
    emulators.push_back(factory);
    if(factory->getNewUsbDevice(device) == 0) {
        return 0;
    }
    return 1;
}

int RemoveEmulatorDevice(DeviceTypeID type, std::string ip, int port) {
    int removed = 0;
    LogPrintf("%s: iterating to find the emulator. \n", __func__);

    for(int i = 0; i < emulators.size(); i++) {
        CEmulatorDevice* factory = emulators[i];

        // Find the emulators that match the description
        // & delete them from the list
        if (factory->type == type
            && factory->ip == ip
            && factory->port == port) {
                delete factory;
                emulators.erase(emulators.begin() + i);
                removed++;
                LogPrintf("%s: remove an emulator %d\n", __func__, removed);
        }
    }

    return removed;
}

void ListEmulatorDevices(std::vector<std::unique_ptr<CUSBDevice> > &vDevices)
{
    // vDevices.insert(vDevices.end(), emulators.begin(), emulators.end());
    // std::copy(emulators.begin(), emulators.end(), std::back_inserter(vDevices));
    for (int i = 0; i < emulators.size(); i++) {
        usb_device::CEmulatorDevice *factory = emulators[i];
        std::unique_ptr<CUSBDevice> device;
        int r = factory->getNewUsbDevice(device);
        LogPrintf("%s: return value getNewUsbDevice %d\n", __func__, r);
        if(0 == r) {
            vDevices.push_back(move(device));
        }
    }
    return;
};

void ListAllDevices(std::vector<std::unique_ptr<CUSBDevice> > &vDevices)
{
    if (Params().NetworkIDString() == "regtest") {
        vDevices.push_back(std::unique_ptr<CUSBDevice>(new CDebugDevice()));
        return;
    }

    ListHIDDevices(vDevices);
    ListWebUSBDevices(vDevices);
    ListEmulatorDevices(vDevices);

    return;
};

CUSBDevice *SelectDevice(std::vector<std::unique_ptr<CUSBDevice> > &vDevices, std::string &sError)
{
    if (Params().NetworkIDString() == "regtest") {
        vDevices.push_back(std::unique_ptr<CUSBDevice>(new CDebugDevice()));
        return vDevices[0].get();
    }

    ListAllDevices(vDevices);
    if (vDevices.size() < 1) {
        sError = "No device found.";
        return nullptr;
    }
    if (vDevices.size() > 1) { // TODO: Select device
        sError = "Multiple devices found.";
        return nullptr;
    }

    return vDevices[0].get();
};

};