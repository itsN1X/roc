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

#include <stdio.h>
FILE *flog = NULL ;

using namespace roc ;

void loger(LogLevel level, const char* module, const char* message);
static size_t roc_send_packet( Roc_Handler *handler, void *data, const size_t data_len );

struct Roc_Handler
{
    pipeline::ClientConfig config;
    audio::SampleBufferQueue sample_queue;
    rtp::Composer rtp_composer_;

    audio::ISampleBufferPtr buffer_;
    size_t buffer_pos_;
    size_t n_bufs_;

    netio::Transceiver *trx;
    pipeline::Client *client ;
};

void roc_initialize(const unsigned int verbosity)
{
    flog = fopen( "/tmp/roc.log", "wa" );
    core::set_log_handler( loger );
    core::set_log_level(LogLevel( LOG_FLOOD ));
}

int roc_initialize_transmitter(Roc_Handler **handler,
         const char *destination_adress, uint32_t options)
{
    datagram::Address src_addr;
    datagram::Address dst_addr;
    if (!netio::parse_address(destination_adress, dst_addr)) {
        roc_log(LOG_ERROR, "can't parse source address: %s", destination_adress);
        return ROC_INVALID;
    }

    *handler = new Roc_Handler();
    if (!*handler) {
        return 1;
    }

    if(options & ROC_TRANSMITTER_OPTIONS_FEC){
        (*handler)->config.options |= pipeline::EnableFEC;
    }

    (*handler)->config = pipeline::ClientConfig();
    // (*handler)->config.options |= pipeline::EnableInterleaving;
    (*handler)->config.options |= pipeline::EnableTiming;

    (*handler)->buffer_ = NULL;

    (*handler)->trx = new netio::Transceiver();
    if (!(*handler)->trx->add_udp_sender(src_addr)) {
        roc_log(LOG_ERROR, "can't register udp sender: %s",
                datagram::address_to_str(src_addr).c_str());
        return 1;
    }

    (*handler)->client = new pipeline::Client((*handler)->sample_queue, (*handler)->trx->udp_sender(), (*handler)->trx->udp_composer(),
                            (*handler)->rtp_composer_, (*handler)->config);

    (*handler)->client->set_sender(src_addr);
    (*handler)->client->set_receiver(dst_addr);

    (*handler)->trx->start();
    (*handler)->client->start();

    return ROC_VALID ;
}

size_t roc_transmit( Roc_Handler *handler, const void *data, size_t data_len )
{
    uint8_t *p = (uint8_t*)data;
    size_t total_sent_len = 0;
    while(total_sent_len < data_len){
        total_sent_len += roc_send_packet( handler,
                (uint8_t*)data + total_sent_len, data_len - total_sent_len );
    }
    return total_sent_len;
}

size_t roc_send_packet( Roc_Handler *handler, void *data, const size_t data_len )
{
    size_t tx_len = 0 ; // Transmitted bytes.
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
    int16_t *psample = (int16_t*)data ;
    for (; handler->buffer_pos_ < handler->buffer_->size(); handler->buffer_pos_++) {
        if (tx_len == data_len) {
            break;
        }
        samples[handler->buffer_pos_] = (float)(int16_t)(*psample & 0xFFFF);
        if((data_len - tx_len) >= sizeof(*psample)) {
            tx_len += sizeof(*psample);
            psample++;
        } else {
            tx_len = data_len;
        }
    }

    if (handler->buffer_pos_ == handler->buffer_->size()) {
        handler->sample_queue.write(*handler->buffer_);

        handler->buffer_ = NULL;
        handler->buffer_pos_ = 0;

        handler->n_bufs_++;
    }

    return tx_len ;
}

void roc_close( Roc_Handler *handler )
{
    (void)handler ;

    if( flog )
        fclose( flog );
    // XXX:
    handler->client->join();
    handler->trx->join();
    delete handler->trx;
    delete handler->client;
    free( handler );
}

uint32_t roc_trasnmitter_latency( Roc_Handler *handler )
{
    (void)handler ;
    return 0 ;
}

void loger( LogLevel level, const char* module, const char* message)
{
    const char slevel[][16] = {
        "LOG_NONE",
        "LOG_ERROR",
        "LOG_DEBUG",
        "LOG_TRACE",
        "LOG_FLOOD"
    };
    if( !flog )
        return ;
    fprintf( flog, "[%s]: %s: %s\n", slevel[level], module, message );
}
