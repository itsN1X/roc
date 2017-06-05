/*
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CppUTest/TestHarness.h>

#include "roc_config/config.h"
#include "roc_core/scoped_ptr.h"
#include "roc_datagram/datagram_queue.h"
#include "roc_pipeline/sender.h"
#include "roc_pipeline/receiver.h"

#include "test_sample_stream.h"
#include "test_sample_queue.h"
#include "test_datagram.h"

namespace roc {
namespace test {

using namespace pipeline;

TEST_GROUP(sender_receiver) {
    enum {
        // Sending port.
        SenderPort = 501,

        // Receiving port.
        ReceiverPort = 502,

        // Number of samples in every channel per packet.
        PktSamples = ROC_CONFIG_DEFAULT_PACKET_SAMPLES,

        // Number of samples in input/output buffers.
        BufSamples = SampleStream::ReadBufsz,

        // Number of packets to read per tick.
        PacketsPerTick = 20,

        // Maximum number of sample buffers.
        MaxBuffers = PktSamples * 100 / BufSamples,

        // Percentage of packets to be lost.
        RandomLoss = 1
    };

    SampleQueue<MaxBuffers> input;
    SampleQueue<MaxBuffers> output;

    datagram::DatagramQueue network;
    TestDatagramComposer datagram_composer;

    core::ScopedPtr<Sender> sender;
    core::ScopedPtr<Receiver> receiver;

    void setup() {
    }

    void teardown() {
        LONGS_EQUAL(0, input.size());
        LONGS_EQUAL(0, output.size());
        LONGS_EQUAL(0, network.size());
    }

    void init_sender(int options, size_t random_loss = 0) {
        SenderConfig config;

        config.options = options;
        config.channels = ChannelMask;
        config.samples_per_packet = PktSamples;
        config.random_loss_rate = random_loss;
        config.fec.type = fec::ReedSolomon2m;
        config.fec.n_source_packets = 20;
        config.fec.n_repair_packets = 10;

        sender.reset(new Sender(input, network, datagram_composer, config));

        sender->set_audio_port(new_address(SenderPort), new_address(ReceiverPort),
                               Proto_RTP);

        sender->set_repair_port(new_address(SenderPort), new_address(ReceiverPort),
                                Proto_RTP); // FIXME
    }

    void init_receiver(int options) {
        ReceiverConfig config;

        config.options = options;
        config.channels = ChannelMask;
        config.session_timeout = MaxBuffers * BufSamples;
        config.session_latency = BufSamples;
        config.output_latency = 0;
        config.samples_per_tick = BufSamples;
        config.fec.type = fec::ReedSolomon2m;
        config.fec.n_source_packets = 20;
        config.fec.n_repair_packets = 10;

        receiver.reset(new Receiver(network, output, config));

        receiver->add_port(new_address(ReceiverPort), Proto_RTP);
    }

    void flow_sender_receiver() {
        SampleStream si;

        for (size_t n = 0; n < MaxBuffers; n++) {
            si.write(input, BufSamples);
        }

        LONGS_EQUAL(MaxBuffers, input.size());

        while (input.size() != 0) {
            CHECK(sender->tick());
        }

        sender->flush();

        CHECK(network.size() >= MaxBuffers * BufSamples / PktSamples);

        SampleStream so;

        for (size_t n = 0; n < MaxBuffers; n++) {
            CHECK(receiver->tick());

            LONGS_EQUAL(1, output.size());

            so.read(output, BufSamples);

            LONGS_EQUAL(0, output.size());
        }

        LONGS_EQUAL(0, network.size());
    }
};

TEST(sender_receiver, bare) {
    init_sender(0);
    init_receiver(0);
    flow_sender_receiver();
}

TEST(sender_receiver, interleaving) {
    init_sender(EnableInterleaving);
    init_receiver(0);
    flow_sender_receiver();
}

#ifdef ROC_TARGET_OPENFEC
TEST(sender_receiver, ldpc_only_sender) {
    init_sender(EnableFEC);
    init_receiver(0);
    flow_sender_receiver();
}

TEST(sender_receiver, ldpc_only_receiver) {
    init_sender(0);
    init_receiver(EnableFEC);
    flow_sender_receiver();
}

TEST(sender_receiver, ldpc) {
    init_sender(EnableFEC);
    init_receiver(EnableFEC);
    flow_sender_receiver();
}

TEST(sender_receiver, ldpc_interleaving) {
    init_sender(EnableFEC | EnableInterleaving);
    init_receiver(EnableFEC);
    flow_sender_receiver();
}

IGNORE_TEST(sender_receiver, ldpc_random_loss) {
    init_sender(EnableFEC, RandomLoss);
    init_receiver(EnableFEC);
    flow_sender_receiver();
}

IGNORE_TEST(sender_receiver, ldpc_interleaving_random_loss) {
    init_sender(EnableFEC | EnableInterleaving, RandomLoss);
    init_receiver(EnableFEC);
    flow_sender_receiver();
}
#endif // ROC_TARGET_OPENFEC

} // namespace test
} // namespace roc
