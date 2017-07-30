/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CppUTest/TestHarness.h>

#include "roc_core/buffer_pool.h"
#include "roc_core/heap_allocator.h"
#include "roc_netio/transceiver.h"
#include "roc_packet/concurrent_queue.h"
#include "roc_packet/packet_pool.h"

namespace roc {
namespace netio {

namespace {

core::HeapAllocator allocator;
core::BufferPool<uint8_t> buffer_pool(allocator, 1000, 1);
packet::PacketPool packet_pool(allocator, 1);

} // namespace

TEST_GROUP(transceiver){};

TEST(transceiver, no_thread) {
    Transceiver trx(packet_pool, buffer_pool, allocator);

    CHECK(trx.valid());
}

TEST(transceiver, start_stop) {
    Transceiver trx(packet_pool, buffer_pool, allocator);

    CHECK(trx.valid());

    trx.start();

    trx.stop();
    trx.join();
}

TEST(transceiver, stop_start) {
    Transceiver trx(packet_pool, buffer_pool, allocator);

    CHECK(trx.valid());

    trx.stop();

    trx.start();
    trx.join();
}

TEST(transceiver, add_start_stop) {
    packet::ConcurrentQueue queue(0, true);

    Transceiver trx(packet_pool, buffer_pool, allocator);

    CHECK(trx.valid());

    packet::Address tx_addr;
    packet::Address rx_addr;

    CHECK(trx.add_udp_sender(tx_addr));
    CHECK(trx.add_udp_receiver(rx_addr, queue));

    trx.start();

    trx.stop();
    trx.join();
}

TEST(transceiver, bind_any) {
    packet::ConcurrentQueue queue(0, true);

    Transceiver trx(packet_pool, buffer_pool, allocator);

    CHECK(trx.valid());

    packet::Address tx_addr;
    packet::Address rx_addr;

    CHECK(trx.add_udp_sender(tx_addr));
    CHECK(trx.add_udp_receiver(rx_addr, queue));

    CHECK(tx_addr.port != 0);
    CHECK(rx_addr.port != 0);
}

TEST(transceiver, bind_lo) {
    packet::ConcurrentQueue queue(0, true);

    Transceiver trx(packet_pool, buffer_pool, allocator);

    CHECK(trx.valid());

    packet::Address tx_addr;
    tx_addr.ip[0] = 127;
    tx_addr.ip[1] = 0;
    tx_addr.ip[2] = 0;
    tx_addr.ip[3] = 1;

    packet::Address rx_addr;
    rx_addr.ip[0] = 127;
    rx_addr.ip[1] = 0;
    rx_addr.ip[2] = 0;
    rx_addr.ip[3] = 1;

    CHECK(trx.add_udp_sender(tx_addr));
    CHECK(trx.add_udp_receiver(rx_addr, queue));

    CHECK(tx_addr.port != 0);
    CHECK(rx_addr.port != 0);
}

} // namespace netio
} // namespace roc
