CXX = g++
CXXFLAGS = -std=c++11 -Wall $(shell pkg-config --cflags gstreamer-1.0)
LDFLAGS = $(shell pkg-config --libs gstreamer-1.0)

TARGET = gst_sender
SOURCES = gst_sender.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp gst_sender /usr/local/bin/

.PHONY: all clean install
