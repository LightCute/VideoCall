# 编译器设置
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# OpenCV 和 GStreamer 库的路径设置
OPENCV_INCLUDE = `pkg-config --cflags opencv4`
OPENCV_LIB = `pkg-config --libs opencv4`
GST_INCLUDE = `pkg-config --cflags gstreamer-1.0`
GST_LIB = `pkg-config --libs gstreamer-1.0`

# 目标文件设置
TARGET = udp_recv
SRCS = udp_recv.cpp

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(OPENCV_INCLUDE) $(OPENCV_LIB) $(GST_INCLUDE) $(GST_LIB)

# 清理生成的文件
clean:
	rm -f $(TARGET)

# 运行目标
run: $(TARGET)
	./$(TARGET)

