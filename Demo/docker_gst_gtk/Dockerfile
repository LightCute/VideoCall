# 基础镜像：Ubuntu 22.04（较新、apt 上的 gstreamer 1.20 系列）
FROM crpi-2isg1kzq2gewzf4m.cn-shenzhen.personal.cr.aliyuncs.com/light_image/ubuntu:resolute-20251130

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

# 安装常用工具、GStreamer、GTK3、开发工具（用于在容器里编译）
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    wget \
    curl \
    build-essential \
    cmake \
    pkg-config \
    git \
    g++ \
    # GTK / X11 库
    libgtk-3-0 libgtk-3-dev \
    # GStreamer core + plugins + GL + GTK sink
    gstreamer1.0-tools \
    gstreamer1.0-libav \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-gl \
    gstreamer1.0-gtk3 \
    libgstreamer1.0-0 \
    libgstreamer-plugins-base1.0-0 \
    # GStreamer 开发包（若需要在容器内编译 gst 程序）
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    # 清理
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# 创建非 root 用户（好习惯），并设置工作目录
# ARG USER=developer
# ARG UID=1000
# RUN useradd -m -u ${UID} -s /bin/bash ${USER} && \
#     mkdir -p /home/${USER}/app && chown -R ${USER}:${USER} /home/${USER}/app
# WORKDIR /home/${USER}/app
# USER ${USER}

# 复制源码（如果你要在构建时编译，取消注释下一行）
# COPY --chown=${USER}:${USER} . /home/${USER}/app

# 默认启动命令（容器启动后你可以覆盖为 ./cam_gui）
CMD ["bash"]
