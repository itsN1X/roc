/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_packet/sorted_queue.h"
#include "roc_core/log.h"
#include "roc_core/panic.h"

namespace roc {
namespace packet {

SortedQueue::SortedQueue(size_t max_size)
    : max_size_(max_size) {
}

PacketPtr SortedQueue::read() {
    if (PacketPtr packet = list_.back()) {
        list_.remove(*packet);
        return packet;
    }

    return NULL;
}

void SortedQueue::write(const PacketPtr& packet) {
    if (!packet) {
        roc_panic("sorted queue: attempting to add null packet");
    }

    if (max_size_ > 0 && list_.size() == max_size_) {
        roc_log(LogDebug, "sorted queue: queue is full, dropping packet:"
                          " max_size=%u",
                (unsigned)max_size_);
        return;
    }

    PacketPtr pos = list_.front();

    for (; pos; pos = list_.nextof(*pos)) {
        const int cmp = packet->compare(*pos);

        if (cmp < 0) {
            continue;
        }

        if (cmp == 0) {
            roc_log(LogDebug, "sorted queue: dropping duplicate packet");
            return;
        }

        break;
    }

    if (pos) {
        list_.insert_before(*packet, *pos);
    } else {
        list_.push_back(*packet);
    }
}

size_t SortedQueue::size() const {
    return list_.size();
}

PacketPtr SortedQueue::head() const {
    return list_.back();
}

PacketPtr SortedQueue::tail() const {
    return list_.front();
}

} // namespace packet
} // namespace roc
