/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_packet/target_stdio/roc_packet/print.h
//! @brief Print packet to stdout.

#ifndef ROC_PACKET_PRINT_H_
#define ROC_PACKET_PRINT_H_

#include "roc_packet/packet.h"

namespace roc {
namespace packet {

//! Print flags.
enum {
    PrintPayload = (1 << 0) //!< Print packet payload.
};

//! Print packet to stderr.
void print(const Packet& packet, int flags);

} // namespace packet
} // namespace roc

#endif // ROC_PACKET_PRINT_H_
