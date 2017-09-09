/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_packet/address.h
//! @brief Network address.

#ifndef ROC_PACKET_ADDRESS_H_
#define ROC_PACKET_ADDRESS_H_

#include "roc_core/stddefs.h"

namespace roc {
namespace packet {

//! TCP or UDP port.
typedef uint16_t port_t;

//! Network address.
struct Address {
    //! IPv4 address.
    uint8_t ip[4];

    //! TCP or UDP port.
    port_t port;

    //! Construct zero address.
    Address() {
        memset(ip, 0, sizeof(ip));
        port = 0;
    }

    //! Compare addresses.
    bool operator==(const Address& other) const {
        return (memcmp(ip, other.ip, sizeof(ip)) == 0 && port == other.port);
    }

    //! Compare addresses.
    bool operator!=(const Address& other) const {
        return !(*this == other);
    }
};

} // namespace packet
} // namespace roc

#endif // ROC_PACKET_ADDRESS_H_
