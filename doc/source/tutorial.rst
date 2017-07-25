.. _tutorial:

User guide
**********

Roc provides easy-to-use and flexible C API. Overall complexity is hidden behind two methods **write** and **read**.

Besides the fact that roc is written in C++, it provides API in terms of plain old C in sake of portability and simplicity.

This guide covers building audio transmission into your application. It is supposed to show a main concept, so refer to :ref:`api` for detailed description of functions and structures if need so.

.. _tutorial_build:

Sender
======

Example
-------

As a main concept of roc is straightforward, let's see the example of transmitting 100 packets of a sine-wave to **roc-recv** before any explanation:

.. code-block:: C

	// Destination address.
	const char* dst_addr_str = "127.0.0.1:12345";
	// The number of samples in single packet.
	const size_t packet_sz = 640;

	roc_config conf;
	memset(&conf, 0, sizeof(roc_config));
	conf.options = 0; // Synchronous variant is a default.
	conf.FEC_scheme = roc_config::ReedSolomon2m; // Enable Forward Erasure Correction, it is a default variant though.

	conf.samples_per_packet = packet_sz/2; 	// Each packet consists 320 samples of left channel 
						// and 320 samples of right channel.

	conf.n_source_packets = 10;		// For every 10 packets with audio samples, 
	conf.n_repair_packets = 5;		// Roc will send 5 more redundant packets
						// to be able to withstand congestions and losts.


	roc_sender* sndr = roc_sender_new(&conf);
	// Bind sender to a destination.
	// Important: Roc uses RTP packets over UDP, so it isn't informed if the receiver
	// is actually exists.
	roc_sender_bind(sndr, dst_addr_str);

	// Buffer for samples that we'll fill with a sine-waveSen
	float samples[packet_sz];

	// Time counter.
	size_t t = 0;

	// Sampling frequency.
	const double Fs = 44100;

	// Sine-wave frequency.
	const double F = 440;

	// For each packet:
	for (size_t i = 0; i < 100; ++i) {
		// For each sample in i-th packet:
		for (size_t j = 0; j < packet_sz/2; ++j) {
			// Left channel.
			samples[j*2] = sin(2*M_PI * F/Fs * (double)(t));
			// Right channel.
			samples[j*2 + 1] = -samples[j*2];
			t += 1;
		}
		roc_sender_write(sndr, samples, packet_sz);
	}

	roc_sender_delete(sndr);

You can receive that signal by calling **roc-recv**:

	``$ roc-recv :12345``

The source code of the example is in ``roc/examples/sender_sinewave.cpp``.

Forward Erasure Correction Codes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Roc is being designed with an idea of sensible latency minimization in sight. That's why roc transmits audio content in UDP packets and why roc incorporates Forward Erasure Correction Codes.

Roc cuts samples flow into blocks and sends several redundant packets along with them so as to recover lost packets. In the example above, roc adds 5 redundant packets to every 10 data packets, so that the roc-recv is able to recover 5 data packets at maximum if they get lost or delayed. On the other hand the data rate is increased in 15/10 times.

Roc doesn't make FEC on its own: it uses `OpenFEC <http://openfec.org/>`_ for that purpose. OpenFEC provides two FEC schemes: Reed-Solomon and LDPC, the former one is more suitable for relatively small latency and small data-rates, therefore it is a default option.

Moreover, the roc's interface of block codec allows attaching another implementation with ease. Feel free to integrate great opensource and free implementation of some effective code.

Sender timing
^^^^^^^^^^^^^

One of the valuable feature of roc is that a receiver (e.g. **roc-recv**) adjusts its sampling frequency to a sampling rate of a sender. That's the reason why a sender must send packets with steady rate. In other words it's forbidden to send an entire .wav file at once. There're many other ways for that.

As packets must be scheduled carefully, roc could do this job for sender. Unless a user turns on ``ROC_API_CONF_DISABLE_TIMING`` option in ``roc_config::options``, ``roc_sender_write`` will block sender's thread until the samples are sent. This variant could be used in e.g. media player, when there's no other source of timing.

Otherwise, ``roc_sender_write`` stores samples in a queue and returns immediately which could be usefull when sender's thread blocks on acquiring the samples, e.g. in `the module for pulseaudio <https://github.com/roc-project/pulseaudio-roc>`_.

The example didn't disable timing because it generates samples online, so it needs to be blocked so as not to oveflow roc's senders queue.

Data format
^^^^^^^^^^^

Roc works with floating-point interleaved PCM samples in native endian ordering. ``roc_sender_write`` and ``roc_receiver_read`` both accept an array of floats with two interleaved channels -- Left-Right-Left-Right-etc.

In future roc is going to support other samples format and configurable channels set.

