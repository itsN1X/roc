/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_packet/delayed_reader.h"
#include "roc_core/log.h"

namespace roc {
namespace packet {

DelayedReader::DelayedReader(IReader& reader, timestamp_t delay)
    : reader_(reader)
    , queue_(0)
    , delay_(delay) {
}

PacketPtr DelayedReader::read() {
    if (delay_ != 0) {
        while (PacketPtr packet = reader_.read()) {
            queue_.write(packet);
        }

        const timestamp_t qs = queue_size_();
        if (qs < delay_) {
            return NULL;
        }

        roc_log(LogInfo, "delayed reader: "
                         "received enough packets: delay=%lu samples=%lu packets=%lu",
                (unsigned long)delay_, (unsigned long)qs, (unsigned long)queue_.size());

        delay_ = 0;
    }

    if (queue_.size() != 0) {
        return queue_.read();
    } else {
        return reader_.read();
    }
}

timestamp_t DelayedReader::queue_size_() const {
    if (queue_.size() == 0) {
        return 0;
    } else {
        return queue_.tail()->distance(*queue_.head());
    }
}

} // namespace packet
} // namespace roc
