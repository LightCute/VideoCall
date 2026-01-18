# 基础镜像
FROM crpi-2isg1kzq2gewzf4m.cn-shenzhen.personal.cr.aliyuncs.com/light_image/ubuntu:22.04

# 设置非交互模式
ENV DEBIAN_FRONTEND=noninteractive

# 安装基础开发工具
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    curl \
    ca-certificates \
    sudo \
    locales \
    vim \
    net-tools \
    iproute2 \
    && locale-gen en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

# 安装 Qt5 及开发库
RUN apt-get update && apt-get install -y --no-install-recommends \
    qtbase5-dev \
    qtbase5-dev-tools \
    qtchooser \
    qt5-qmake \
    qt5-qmake-bin \
    qtwayland5 \
    qt5-style-plugins \
    libqt5widgets5 \
    libqt5gui5 \
    libqt5core5a \
    && rm -rf /var/lib/apt/lists/*


# 安装 GStreamer 及插件
RUN apt-get update && apt-get install -y --no-install-recommends \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-gl \
    gstreamer1.0-qt5 \
    v4l-utils \
    && rm -rf /var/lib/apt/lists/*

# 安装 X11 依赖（用于 X11 GUI 转发）
RUN apt-get update && apt-get install -y --no-install-recommends \
    libx11-xcb1 \
    libxrender1 \
    libxrandr2 \
    libxi6 \
    libxext6 \
    libxcb-xinerama0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-shape0 \
    libxcb-shm0 \
    libxcb-sync1 \
    libxcb-xfixes0 \
    libxcb-render-util0 \
    libxcb-render0 \
    libxcb1 \
    libx11-6 \
    libxkbcommon0 \
    && rm -rf /var/lib/apt/lists/*

# 创建工作目录
WORKDIR /workspace

# 设置默认入口，bash
CMD ["/bin/bash"]
