.. _tutorial:

User guide
=================

Roc provides easy-to-use and flexible C API. Overall complexity is hidden behind two methods **write** and **read**.

Besides the fact that Roc is written in C++, it provides API in terms of plain old C in sake of portability and simplicity.

This guide covers building audio transmission into your application. It is supposed to show a main concept. For detailed description of functions and structures refer to :ref:`api`.

.. _tutorial_build:


Example
------

As a main concept of Roc is straightforward, I'll put the example of transmitting 5 seconds of a sine-wave before any explanation:

``
const char* dst_addr_str = "127.0.0.1:12345";

roc_config conf;
memset(&conf, 0, sizeof(roc_config));
conf.options = 0; // Synchronous variant is a default.
conf.FEC_scheme = roc_config::ReedSolomon2m; // Enable Forward Error Correction.

conf.samples_per_packet = 320; 	// Each packet consists 320 samples of left channel 
								// and 320 samples of right channel.

conf.n_source_packets = 10;		// For every 10 packets with audio samples, 
conf.n_repair_packets = 5;		// Roc will send 5 more redundant packets
								// to be able to withstand congestions and losts.


roc_sender* sndr = roc_sender_new(&conf);
// Bind sender to a destination.
// Important: Roc uses RTP packets over UDP, so it doesn't know so far if the receiver
// is listening on the address.
roc_sender_bind(sndr, dst_addr_str);

for (size_t i = 0; i < 100; ++i) {
	roc_sender_write(sndr_, samples_, sz_);
}

roc_sender_delete(sndr);

``

The sender is supposed to send some small chunks of samples to the particular **address:port** with some small overhead (e.g. RTP headers) and *maybe* some redundant packets so as to build loss-proof link.2

Data format
-----------

Roc works with floating-point interleaved PCM samples in native endian ordering. ``roc_sender_write`` and ``roc_receiver_read`` both accept an array of floats with two interleaved channels -- Left-Right-Left-Right-etc.

In future Roc's is going to support other samples format and configurable number of channels.

