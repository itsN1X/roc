/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @brief C API.

 #include "roc_core/log.h"
 #include "roc_datagram/address_to_str.h"
 #include "roc_datagram/datagram_queue.h"
 #include "roc_audio/sample_buffer_queue.h"
 #include "roc_pipeline/client.h"
 #include "roc_rtp/composer.h"
 #include "roc_sndio/reader.h"
 #include "roc_netio/transceiver.h"
 #include "roc_netio/inet_address.h"

#include "roc_capi/c_api.h"

using namespace roc ;

struct Roc_Handler
{
    pipeline::ClientConfig config;
    audio::SampleBufferQueue sample_queue;
    rtp::Composer rtp_composer_;

    sndio::Reader reader;

    audio::ISampleBufferPtr buffer_;
    size_t buffer_pos_;
    size_t buffer_size_;
    size_t n_bufs_;
};

void roc_initialize(const unsigned int verbosity)
{
    core::set_log_level(LogLevel(LOG_ERROR + verbosity));
}

int roc_initialize_transmitter(Roc_Handler **handler,
         const char *destination_adress, uint32_t options)
{
    datagram::Address src_addr;
    if (!netio::parse_address(destination_adress, src_addr)) {
        roc_log(LOG_ERROR, "can't parse source address: %s", destination_adress);
        return ROC_INVALID;
    }

    *handler = (Roc_Handler*)malloc( sizeof(Roc_Handler) );
    if (!*handler) {
        return 1;
    }

    if(options & ROC_TRANSMITTER_OPTIONS_FEC){
        (*handler)->config.options |= pipeline::EnableFEC;
    }

    (*handler)->config = pipeline::ClientConfig();
    (*handler)->config.options |= pipeline::EnableInterleaving;
    (*handler)->config.options |= pipeline::EnableTiming;

    (*handler)->buffer_ = NULL;

    netio::Transceiver trx;
    if (!trx.add_udp_sender(src_addr)) {
        roc_log(LOG_ERROR, "can't register udp sender: %s",
                datagram::address_to_str(src_addr).c_str());
        return 1;
    }

    pipeline::Client client((*handler)->sample_queue, trx.udp_sender(), trx.udp_composer(),
                            (*handler)->rtp_composer_, (*handler)->config);

    client.set_sender(src_addr);

    trx.start();

    client.start();

    return ROC_VALID ;
}

size_t roc_transmit( Roc_Handler *handler, const void *data, size_t data_len )
{
    audio::ISampleBufferComposer&  composer = audio::default_buffer_composer();
    const size_t buffer_size = handler->config.samples_per_packet / 2
                        * packet::num_channels(handler->config.channels);
    if (!handler->buffer_) {
        if (!(handler->buffer_ = composer.compose())) {
            roc_log(LOG_ERROR, "reader: can't compose buffer");
            return data_len;
        }

        if (buffer_size > handler->buffer_->max_size()) {
            roc_panic(
                "reader:"
                " maximum buffer size should be at least n_channels * n_samples:"
                " decoder_bufsz=%lu, max_bufsz=%lu, n_channels=%lu",
                (unsigned long)buffer_size,        //
                (unsigned long)handler->buffer_->max_size(), //
                (size_t)2);
        }

        handler->buffer_->set_size(buffer_size);
    }

    packet::sample_t* samples = handler->buffer_->data();
    uint32_t *psample = (uint32_t*)data ;
    for (; handler->buffer_pos_ < handler->buffer_size_; handler->buffer_pos_++) {
        if (data_len == 0) {
            break;
        }
        samples[handler->buffer_pos_] = (float)(int16_t)(*psample & 0xFFFF);
        if(data_len >= sizeof(*psample)) {
            data_len -= sizeof(*psample);
            psample++;
        } else {
            data_len = 0;
        }
    }

    if (handler->buffer_pos_ == handler->buffer_size_) {
        handler->sample_queue.write(*handler->buffer_);

        handler->buffer_ = NULL;
        handler->buffer_pos_ = 0;

        handler->n_bufs_++;
    }

    return 0 ;
}

void roc_close( Roc_Handler *handler )
{
    (void)handler ;
    free( handler );
}

uint32_t roc_trasnmitter_latency( Roc_Handler *handler )
{
    (void)handler ;
    return 0 ;
}
