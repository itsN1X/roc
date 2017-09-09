/*
 * Copyright (c) 2017 Mikhail Baranov
 * Copyright (c) 2017 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_packet/rtp.h"
#include "roc_core/macros.h"

namespace roc {
namespace packet {

RTP::RTP()
    : source(0)
    , seqnum(0)
    , timestamp(0)
    , duration(0)
    , marker(false)
    , payload_type(0) {
}

int RTP::compare(const RTP& other) const {
    if (ROC_UNSIGNED_LT(signed_seqnum_t, seqnum, other.seqnum)) {
        return -1;
    } else if (seqnum == other.seqnum) {
        return 0;
    } else {
        return +1;
    }
}

timestamp_t RTP::distance(const RTP& other) const {
    const signed_timestamp_t dist =
        ROC_UNSIGNED_SUB(signed_timestamp_t, timestamp, other.timestamp);

    if (dist >= 0) {
        return (timestamp_t)dist + other.duration;
    } else {
        return 0;
    }
}

} // namespace packet
} // namespace roc
