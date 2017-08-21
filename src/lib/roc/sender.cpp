/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc/sender.h"

#include "roc_core/heap_allocator.h"
#include "roc_core/log.h"
#include "roc_netio/inet_address.h"
#include "roc_netio/transceiver.h"
#include "roc_packet/address_to_str.h"
#include "roc_pipeline/sender.h"

using namespace roc;

namespace {

enum { MaxPacketSize = 2048, MaxFrameSize = 65 * 1024 };

bool make_config(pipeline::SenderConfig& out, const roc_config* in) {
    out.samples_per_packet = in->samples_per_packet;

    switch (in->FEC_scheme) {
    case roc_config::ReedSolomon2m:
        out.fec.codec = fec::ReedSolomon2m;
        break;
    case roc_config::LDPC:
        out.fec.codec = fec::LDPCStaircase;
        break;
    case roc_config::NO_FEC:
        out.fec.codec = fec::NoCodec;
        break;
    }

    out.fec.n_source_packets = in->n_source_packets;
    out.fec.n_repair_packets = in->n_repair_packets;

    out.interleaving = !(in->options & ROC_API_CONF_INTERLEAVER_OFF);
    out.timing = !(in->options & ROC_API_CONF_DISABLE_TIMING);

    return true;
}

} // namespace

struct roc_sender {
    core::HeapAllocator allocator;

    packet::PacketPool packet_pool;
    core::BufferPool<uint8_t> byte_buffer_pool;
    core::BufferPool<audio::sample_t> sample_buffer_pool;

    rtp::FormatMap format_map;

    pipeline::SenderConfig config;

    netio::Transceiver trx;
    core::UniquePtr<pipeline::Sender> sender;

    roc_sender(pipeline::SenderConfig& cfg)
        : packet_pool(allocator, 1)
        , byte_buffer_pool(allocator, MaxPacketSize, 1)
        , sample_buffer_pool(allocator, MaxFrameSize, 1)
        , config(cfg)
        , trx(packet_pool, byte_buffer_pool, allocator) {
    }
};

roc_sender* roc_sender_new(const roc_config* config) {
    pipeline::SenderConfig c;

    if (!make_config(c, config)) {
        return NULL;
    }

    roc_log(LogInfo, "roc sender: creating sender");
    return new roc_sender(c);
}

void roc_sender_delete(roc_sender* sender) {
    roc_panic_if(sender == NULL);

    roc_log(LogInfo, "roc sender: deleting sender");
    delete sender;
}

bool roc_sender_bind(roc_sender* sender, const char* address) {
    roc_panic_if(sender == NULL);

    roc_log(LogInfo, "roc sender: binding to %s", address);

    packet::Address addr;
    if (!netio::parse_address(address, addr)) {
        return false;
    }

    packet::IWriter *writer = sender->trx.add_udp_sender(addr);
    if (!writer) {
        return false;
    }

    sender->sender.reset(new (sender->allocator) pipeline::Sender(
                             sender->config, *writer, *writer, sender->format_map,
                             sender->packet_pool, sender->byte_buffer_pool,
                             sender->allocator),
                         sender->allocator);

    return true;
}

ssize_t
roc_sender_write(roc_sender* sender, const float* samples, const size_t n_samples) {
    roc_panic_if(sender == NULL);
    roc_panic_if(samples == NULL && n_samples != 0);

    audio::Frame frame;
    frame.samples = new (sender->sample_buffer_pool)
        core::Buffer<audio::sample_t>(sender->sample_buffer_pool);

    frame.samples.resize(n_samples);
    roc_panic_if(sizeof(float) != sizeof(audio::sample_t));
    memcpy(frame.samples.data(), samples, n_samples * sizeof(audio::sample_t));

    sender->sender->write(frame);

    return (ssize_t)n_samples;
}
