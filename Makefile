# Makefile for OpenCV C++ project

# 编译器
CXX = g++
# 编译选项
CXXFLAGS = -std=c++11 -O2 `pkg-config --cflags opencv4`
# 链接选项
LDFLAGS = `pkg-config --libs opencv4`

# 源文件
SRC = opencv_video.cpp
# 输出可执行文件
TARGET = opencv_video

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
