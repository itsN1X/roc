CXX = g++
CFLAGS = -shared -g -fPIC -lc -Wl,-soname -L/usr/lib/x86_64-linux-gnu/pulseaudio/ -lpulsecore-4.0 -lpulsecommon-4.0 -lpulse -I /home/mkh/Coding/pulseaudio/src/ -I/home/mkh/Coding/pulseaudio/  
OUT = sink.la 
SRC = sink.cpp

$(OUT): $(SRC)
	$(CXX) $(CFLAGS) -o $(OUT) $(SRC)