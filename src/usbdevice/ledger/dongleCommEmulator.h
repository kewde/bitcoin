// Copyright (c) 2018-2019 The Particl Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

int sendApduEmulator(int emulator_handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw);