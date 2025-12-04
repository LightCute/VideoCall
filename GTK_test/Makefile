# ---------------------------
#  Makefile for GTK + GStreamer + C++
# ---------------------------

CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17

# Use pkg-config to get GTK and GStreamer compile/link flags
PKG_FLAGS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 gstreamer-video-1.0`

# Target executable name
TARGET = app

# Source files
SRCS = gtk_test.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(PKG_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< $(PKG_FLAGS)

clean:
	rm -f $(TARGET) *.o

.PHONY: clean
