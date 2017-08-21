/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc/receiver.h"

#include "roc_core/heap_allocator.h"
#include "roc_core/log.h"
#include "roc_netio/inet_address.h"
#include "roc_netio/transceiver.h"
#include "roc_packet/address_to_str.h"
#include "roc_pipeline/receiver.h"

using namespace roc;

namespace {

enum { MaxPacketSize = 2048, MaxFrameSize = 65 * 1024 };

bool make_config(pipeline::ReceiverConfig& out, const roc_config* in) {
    out.default_session.latency = in->latency;
    out.default_session.timeout = in->timeout;

    out.default_session.resampling = !(in->options & ROC_API_CONF_RESAMPLER_OFF);

    switch (in->FEC_scheme) {
    case roc_config::ReedSolomon2m:
        out.default_session.fec.codec = fec::ReedSolomon2m;
        break;
    case roc_config::LDPC:
        out.default_session.fec.codec = fec::LDPCStaircase;
        break;
    case roc_config::NO_FEC:
        out.default_session.fec.codec = fec::NoCodec;
        break;
    }

    out.default_session.fec.n_source_packets = in->n_source_packets;
    out.default_session.fec.n_repair_packets = in->n_repair_packets;

    out.timing = !(in->options & ROC_API_CONF_DISABLE_TIMING);

    return true;
}

} // namespace

struct roc_receiver {
    core::HeapAllocator allocator;

    packet::PacketPool packet_pool;
    core::BufferPool<uint8_t> byte_buffer_pool;
    core::BufferPool<audio::sample_t> sample_buffer_pool;

    rtp::FormatMap format_map;

    pipeline::Receiver receiver;
    netio::Transceiver trx;

    roc_receiver(pipeline::ReceiverConfig& config)
        : packet_pool(allocator, 1)
        , byte_buffer_pool(allocator, MaxPacketSize, 1)
        , sample_buffer_pool(allocator, MaxFrameSize, 1)
        , receiver(config,
                   format_map,
                   packet_pool,
                   byte_buffer_pool,
                   sample_buffer_pool,
                   allocator)
        , trx(packet_pool, byte_buffer_pool, allocator) {
    }
};

roc_receiver* roc_receiver_new(const roc_config* config) {
    pipeline::ReceiverConfig c;

    if (!make_config(c, config)) {
        return NULL;
    }

    roc_log(LogInfo, "roc receiver: creating receiver");
    return new roc_receiver(c);
}

void roc_receiver_delete(roc_receiver* receiver) {
    roc_panic_if(receiver == NULL);

    roc_log(LogInfo, "roc receiver: deleting receiver");
    delete receiver;
}

bool roc_receiver_bind(roc_receiver* receiver, const char* address) {
    roc_panic_if(receiver == NULL);
    roc_panic_if(address == NULL);

    roc_log(LogInfo, "roc receiver: binding to %s", address);

    packet::Address addr;
    if (!netio::parse_address(address, addr)) {
        return false;
    }

    if (!receiver->trx.add_udp_receiver(addr, receiver->receiver)) {
        return false;
    }

    return true;
}

ssize_t
roc_receiver_read(roc_receiver* receiver, float* samples, const size_t n_samples) {
    roc_panic_if(receiver == NULL);
    roc_panic_if(samples == NULL && n_samples != 0);

    audio::Frame frame;
    frame.samples = new (receiver->sample_buffer_pool)
        core::Buffer<audio::sample_t>(receiver->sample_buffer_pool);

    frame.samples.resize(n_samples);
    receiver->receiver.read(frame);

    roc_panic_if(sizeof(float) != sizeof(audio::sample_t));
    memcpy(samples, frame.samples.data(), n_samples * sizeof(audio::sample_t));

    return (ssize_t)n_samples;
}
