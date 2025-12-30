CXX = g++
CXXFLAGS = -std=c++17 `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0 gtk+-3.0`
LIBS = `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0 gtk+-3.0`

SRCS =  CameraWidget.cpp  CameraUI.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
