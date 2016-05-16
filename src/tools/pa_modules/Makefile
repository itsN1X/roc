CXX = cc
CFLAGS = -shared -g -fPIC -lgdbm -lcap -lpthread -lrt -ldl -lm  -lc -Wl,-soname -L/usr/lib/x86_64-linux-gnu/pulseaudio/ -lpulsecore-4.0 -lpulsecommon-4.0 -lpulse -I /home/mkh/Coding/pulseaudio/src/ -I/home/mkh/Coding/pulseaudio/  
OUT = module-roc-sink.so
SRC = sink.c

$(OUT): $(SRC)
	$(CXX) $(CFLAGS) -o $(OUT) $(SRC)

install:
	cp $(OUT) /usr/lib/pulse-4.0/modules