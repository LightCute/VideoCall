CXX = g++
CXXFLAGS = -std=c++11 -O2 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4` -lpthread

TARGET = opencv_video
SRC = opencv_video.cpp

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
