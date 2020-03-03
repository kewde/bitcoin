// Copyright (c) 2018-2019 The Particl Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <util/system.h>
#include <usbdevice/ledger/btchipApdu.h>

int SendToEmulator(int emulator_handle, const unsigned char *apdu, size_t apduLength) {
    LogPrintf("%s: calling apdu length send() \n", __func__);
    int apduLengthInt = htonl((int) apduLength);
    int result = send(emulator_handle, &apduLengthInt, sizeof(apduLengthInt) , 0);
    if(result < 0) {
        // Failed write, won't read then
        LogPrintf("%s: apdu length send() result errored: %ld\n", __func__, errno);
        return 0;
    }

    LogPrintf("%s: calling apdu send() \n", __func__);
    result = send(emulator_handle, apdu, apduLength , 0);
    if(result < 0) {
        // Failed write, won't read then
        LogPrintf("%s: apdu send() result errored: %ld\n", __func__, errno);
        return 0;
    }
}

int ReceiveFromEmulator(int emulator_handle, unsigned char *out, size_t outLength, int *sw) {
    // The first 4 bytes sent back are the length of the APDU that follows.
    int response_length = 0;
    // ledger will always return 4 bytes, sizeof(int) always 4 on all arch? 
    int result = recv(emulator_handle, &response_length, 4, 0);
    if (result < 0){
        LogPrintf("%s: recv response_length failed, errno: %ld\n", __func__, errno);
        return result;
    }

    response_length = ntohl(response_length);
    LogPrintf("%s: response_length: %ld\n", __func__, response_length);

    // Make sure that we can fit the data in the out buffer without overflow
    if (outLength < response_length) {
        LogPrintf("%s: can't fit the returned apdus into the buffer : %ld < %ld\n", __func__, outLength, response_length);
        return 0;
    }

    // Fetch the APDU bytes
    result = recv(emulator_handle, out, response_length, 0);
    if (result < 0){
        LogPrintf("%s: recv actual response failed, errno: %ld\n", __func__, errno);
        return result;
    }

    // The last 2 bytes are the SW code
    unsigned char sw_short[2];
    result = recv(emulator_handle, sw_short, 2, 0);
    *sw = (sw_short[0]<<8)+sw_short[1]; // Fix endianness & turn into int

    return response_length;
}

int sendApduEmulator(int emulator_handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) {
    int result = SendToEmulator(emulator_handle, apdu, apduLength);
    if (result == 0) {
        LogPrintf("%s: SendToEmulator() result errored: %ld\n", __func__, result);
        return 0;
    }

    int totalResponseLength = 0;
    bool more = true;
    while(more) {
        int spaceLeftOut = outLength - totalResponseLength;
        int reponseLength = ReceiveFromEmulator(emulator_handle, out + totalResponseLength, spaceLeftOut, sw);
        if (reponseLength < 0) {
            LogPrintf("%s: ReceiveFromEmulator() result errored: %ld\n", __func__, result);
            return 0;
        }
        totalResponseLength += reponseLength;

        // SW indicates we need to fetch more data
        if ((*sw & 0xFF00) == 0x6100) {
            LogPrintf("%s: Fetching more data from the emulator!\n", __func__);
            uint8_t apduMoreData[5];
            apduMoreData[0] = 0x00;
            apduMoreData[1] = BTCHIP_INS_GET_RANDOM;
            apduMoreData[2] = 0x00;
            apduMoreData[3] = 0x00;
            apduMoreData[4] = 0x00;
            int result = SendToEmulator(emulator_handle, apduMoreData, 5);
            if (result == 0) {
                LogPrintf("%s: SendToEmulator() request more data errored: %ld\n", __func__, result);
                return 0;
            }
        } else {
            more = false;
        }
    }

    out[totalResponseLength] = '\0';
    return totalResponseLength;
}