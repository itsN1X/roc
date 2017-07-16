.. _tutorial:

User guide
=================

Roc provides easy-to-use and flexible C API. Overall complexity is hidden behind two methods write and read.

This guide covers building audio transmission into your application


.. _tutorial_build:


Sender
------

The sender is supposed to send some small chunks of samples to the particular address:port with some small overhead (e.g. RTP headers) and *maybe* some redundant packets so as to build loss-proof link.2


Receiver
--------

