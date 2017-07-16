.. _sende_n_receiver:

roc-send and roc-recv 
=====================

Roc provides two command-line utilities for transmitting audio-content via LAN/WLAN. They're pretty simple but very useful.

roc-send [OPTIONS] ADDRESS
------------------------------
Sends music content from the file to the ADDRESS.

Available options:
^^^^^^^^^^^^^^^^^^

-v, --verbose
	Increase verbosity level (may be used multiple times)

-s, --source=ADRESS
    Source address (default is 0.0.0.0:0, i.e. INADDR_ANY and random port)

-i, --input=NAME
    Input file or device

-t, --type=TYPE
    Input codec or driver

--fec=ENUM
	FEC scheme  (possible values="rs", "ldpc", "none" default=`rs')

--nbsrc=INT
    Number of source packets in FEC block

--nbrpr=INT
    Number of repair packets in FEC block

--interleaving=ENUM
	Enable/disable packet interleaving  (possible values="yes", "no" default=`yes')

--timing=ENUM
	Enable/disable pipeline timing  (possible values="yes", "no" default=`yes')

--rate=INT
	Sample rate (Hz)

--loss-rate=INT
    Set percentage of packets to be randomly lost, [0; 100]

--delay-rate=INT
	Set percentage of packets to be randomly delayed, [0; 100]

--delay=INT
	Set delay time, milliseconds

ADDRESS should be in form of [IP]:PORT. IP defaults to 0.0.0.0.

Output:
^^^^^^^

Arguments for '--input' and '--type' options are passed to SoX:

* NAME specifies file or device name

* TYPE specifies file or device type

Examples:
^^^^^^^^^

Send wav file:

	``$ roc-send -vv 192.168.0.3:12345 -i song.wav``

or

	``$ roc-send -vv 192.168.0.3:12345 -i song.wav -t wav``

Capture sound from default driver and device:

	``$ roc-send -vv 192.168.0.3:12345``

Capture sound from default ALSA device:

	``$ roc-send -vv 192.168.0.3:12345 -t alsa``

or

	``$ roc-send -vv 192.168.0.3:12345 -t alsa -i default``

Capture sound from specific pulseaudio device:

	``$ roc-send -vv 192.168.0.3:12345 -t pulseaudio -i <device>``

roc-recv [OPTIONS] ADDRESS
--------------------------

Repairs lost packets, adjusts sampling frequincy to the sender, and plays content with SoX or ALSA.

Available options:
^^^^^^^^^^^^^^^^^^

-v, --verbose              
	Increase verbosity level (may be used multiple times).

-o, --output=NAME
	Output file or device.

-t, --type=TYPE
	Output codec or driver.

--fec=ENUM             
	FEC scheme  (possible values="rs", "ldpc", "none" default=`rs')

--nbsrc=INT            
	Number of source packets in FEC block.

--nbrpr=INT            
	Number of repair packets in FEC block.

--resampling=ENUM      
	Enabled/disable resampling  (possible values="yes", "no" default=`yes').

--timing=ENUM          
	Enabled/disable pipeline timing  (possible values="yes", "no" default=`yes').

-1, --oneshot
	Exit when last connected client disconnects (default=off).

--beep                 
	Enable beep on packet loss  (default=off).

--rate=INT             
	Sample rate (Hz).

--session-timeout=INT  
	Session timeout as number of samples.

--session-latency=INT  
	Session latency as number of samples.

--output-latency=INT   
	Output latency as number of samples.

--output-frame=INT     
	Number of samples per output frame.

--resampler-frame=INT  
	Number of samples per resampler frame.

ADDRESS should be in form of [IP]:PORT. IP defaults to 0.0.0.0.

Output:
^^^^^^^

Arguments for '--output' and '--type' options are passed to SoX:

* NAME specifies file or device name

* TYPE specifies file or device type

Examples:
^^^^^^^^^

Start receiver listening on all interfaces on UDP port 12345:

    ``$ roc-recv -vv :12345``

Start receiver listening on particular interface:

    ``$ roc-recv -vv 192.168.0.3:12345``

Output to ALSA default device:

    ``$ roc-recv -vv :12345 -t alsa``

or

    ``$ roc-recv -vv :12345 -t alsa -o default``

Output to file:

	``$ roc-recv -vv :12345 -o record.wav``

or

    ``$ roc-recv -vv :12345 -o record.wav -t wav``

