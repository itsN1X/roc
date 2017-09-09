/*
 * Copyright (c) 2017 Mikhail Baranov
 * Copyright (c) 2017 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_packet/icomposer.h
//! @brief Packet composer interface.

#ifndef ROC_PACKET_ICOMPOSER_H_
#define ROC_PACKET_ICOMPOSER_H_

#include "roc_core/slice.h"
#include "roc_packet/packet.h"

namespace roc {
namespace packet {

//! Packet composer interface.
class IComposer {
public:
    virtual ~IComposer();

    //! Adjust buffer to align payload.
    //! @remarks
    //!  Adjusts the given @p buffer so that the payload of the most inner composer
    //!  will have @p payload_alignment. The @p header_size parameter defines the
    //!  total size of all headers before the payload.
    //! @returns
    //!  true if the buffer was successfully adjusted or false if the @p buffer
    //!  capacity is not enough.
    virtual bool
    align(core::Slice<uint8_t>& buffer, size_t header_size, size_t payload_alignment) = 0;

    //! Prepare buffer for composing a packet.
    //! @remarks
    //!  Resizes the given @p buffer so that it can fit the @p packet headers and
    //!  payload. If the packet payload contains an inner packet, calls the inner
    //!  composer as well The @p payload_size referes to the payload of the most
    //!  inner packet. Modifies the @p packet so that its payload fields point to
    //!  the appropriate parts of the @p buffer.
    //! @returns
    //!  true if the packet was successfully prepared or false if the @p buffer
    //!  capacity is not enough.
    virtual bool
    prepare(Packet& packet, core::Slice<uint8_t>& buffer, size_t payload_size) = 0;

    //! Compose packet to buffer.
    //! @remarks
    //!  Formats @p packet headers and payloads to the buffer attached to it during
    //!  a previous prepare() call.
    //! @returns
    //!  true if the packet was successfully composed or false if an error occured.
    virtual bool compose(Packet& packet) = 0;
};

} // namespace packet
} // namespace roc

#endif // ROC_PACKET_ICOMPOSER_H_
