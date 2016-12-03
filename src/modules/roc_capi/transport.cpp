/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "transport.h"

#include "roc_core/log.h"
#include "roc_core/scoped_ptr.h"
#include "roc_datagram/address_to_str.h"
#include "roc_datagram/datagram_queue.h"
#include "roc_audio/sample_buffer_queue.h"
#include "roc_pipeline/client.h"
#include "roc_rtp/composer.h"
#include "roc_sndio/reader.h"
#include "roc_netio/transceiver.h"
#include "roc_netio/inet_address.h"

using namespace roc;

struct roc_transport
{
    pipeline::ClientConfig config;
    audio::SampleBufferQueue sample_queue;
    rtp::Composer rtp_composer;

    audio::ISampleBufferPtr buffer;
    size_t buffer_pos;
    size_t n_bufs;

    core::ScopedPtr<netio::Transceiver> trx;
    core::ScopedPtr<pipeline::Client> client;
};

static size_t roc_transport_send_packet(
    roc_transport *transport, const float *samples, const size_t n_samples);

roc_transport* roc_transport_open(const char *destination_adress)
{
    datagram::Address src_addr;
    datagram::Address dst_addr;
    if (!netio::parse_address(destination_adress, dst_addr)) {
        roc_log(LOG_ERROR, "can't parse source address: %s", destination_adress);
        return NULL;
    }

    core::ScopedPtr<roc_transport> transport(new roc_transport());
    if (!transport) {
        return NULL;
    }

    transport->config = pipeline::ClientConfig();
    transport->config.options = 0;
    transport->config.options |= pipeline::EnableFEC;

    transport->trx.reset(new netio::Transceiver());

    if (!transport->trx->add_udp_sender(src_addr)) {
        roc_log(LOG_ERROR, "can't register udp sender: %s",
                datagram::address_to_str(src_addr).c_str());
        return NULL;
    }

    transport->client.reset(new pipeline::Client(
        transport->sample_queue,
        transport->trx->udp_sender(),
        transport->trx->udp_composer(),
        transport->rtp_composer,
        transport->config));

    transport->client->set_sender(src_addr);
    transport->client->set_receiver(dst_addr);

    transport->trx->start();
    transport->client->start();

    return transport.release();
}

size_t roc_transport_send(roc_transport *transport, const float *samples, size_t n_samples)
{
    size_t sent_samples = 0;

    while (sent_samples < n_samples) {
        size_t n = roc_transport_send_packet(
            transport, samples + sent_samples, n_samples - sent_samples);

        if (n == 0) {
            break;
        }

        sent_samples += n;
    }

    return sent_samples;
}

size_t roc_transport_send_packet(
    roc_transport *transport, const float *samples, size_t n_samples)
{
    audio::ISampleBufferComposer& composer = audio::default_buffer_composer();

    const size_t buffer_size = transport->config.samples_per_packet;

    if (!transport->buffer) {
        if (!(transport->buffer = composer.compose())) {
            roc_log(LOG_ERROR, "reader: can't compose buffer");
            return 0;
        }

        if (buffer_size > transport->buffer->max_size()) {
            roc_panic(
                "reader:"
                " maximum buffer size should be at least n_channels * n_samples:"
                " decoder_bufsz=%lu, max_bufsz=%lu, n_channels=%lu",
                (unsigned long)buffer_size,        //
                (unsigned long)transport->buffer->max_size(), //
                (size_t)2);
        }

        transport->buffer->set_size(buffer_size);
    }

    packet::sample_t* buf_samples = transport->buffer->data();

    size_t tx_samples = 0;

    for (;;) {
        if (tx_samples >= n_samples) {
            break;
        }
        if (transport->buffer_pos == transport->buffer->size()) {
            break;
        }
        buf_samples[transport->buffer_pos++] = *samples++;
        tx_samples++;
    }

    if (transport->buffer_pos == transport->buffer->size()) {
        transport->sample_queue.write(*transport->buffer);

        transport->buffer.reset();
        transport->buffer_pos = 0;

        transport->n_bufs++;
    }

    return tx_samples;
}

void roc_transport_close(roc_transport *transport)
{
    delete transport;
}

uint32_t roc_transport_get_latency(roc_transport *transport)
{
    (void)transport;
    return 0;
}
