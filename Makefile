# ============================
# Makefile for opencv_video.cpp
# ============================

# 使用的编译器
CXX = g++

# C++ 标准
CXX_STD = -std=c++11

# 编译优化
CXX_OPT = -O2

# 通过 pkg-config 获取 OpenCV 4 的头文件路径（-I...）
OPENCV_CFLAGS = $(shell pkg-config --cflags opencv4)

# 通过 pkg-config 获取 OpenCV 4 的库文件（-lopencv_xxx）
OPENCV_LIBS = $(shell pkg-config --libs opencv4)

# 需要额外链接的库：pthread 和 dl
EXTRA_LIBS = -lpthread -ldl

# 源文件
SRC = udp_send.cpp

# 输出的可执行文件名
TARGET = img_show

# 默认命令：编译全部
all: $(TARGET)

# 构建规则
# $@ 表示目标（TARGET）
# $< 表示第一个依赖（SRC）
$(TARGET): $(SRC)
	$(CXX) $(CXX_STD) $(CXX_OPT) $(OPENCV_CFLAGS) -o $@ $< $(OPENCV_LIBS) $(EXTRA_LIBS)

# 清理命令
clean:
	rm -f $(TARGET)

# 声明伪目标（防止与文件重名）
.PHONY: all clean
