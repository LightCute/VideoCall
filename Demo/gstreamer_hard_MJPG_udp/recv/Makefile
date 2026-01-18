CXX = g++
CXXFLAGS = -std=c++11 -Wall $(shell pkg-config --cflags gstreamer-1.0)
LDFLAGS = $(shell pkg-config --libs gstreamer-1.0)

TARGET = gst_recv
SOURCES = gst_recver.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp gst_receiver /usr/local/bin/

.PHONY: all clean install
