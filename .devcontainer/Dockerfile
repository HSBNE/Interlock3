# Base Ubuntu
FROM ubuntu:22.04

# Noninteractive apt
ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools
RUN apt-get update && apt-get install -y \
    bash-completion \
    bison \
    ccache \
    clang \
    clang-format \
    clang-tidy \
    clangd \
    cmake \
    curl \
    dfu-util \
    flex \
    fzf \
    git \
    gperf \
    grep \
    htop \
    libffi-dev \
    libssl-dev \
    locales \
    make \
    nano \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv \
    sudo \
    tar \
    tmux \
    tree \
    unzip \
    usbutils \
    vim \
    wget \
    zip \
    && rm -rf /var/lib/apt/lists/*

# Provide python3 as python
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 1

# Set UTF-8 locale
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Create espdev user with same UID/GID as host
ARG USER_ID=1000
ARG GROUP_ID=1000
RUN
RUN groupadd -g ${GROUP_ID:-1000} espdev && \
    useradd -m -u ${USER_ID:-1000} -g ${GROUP_ID:-1000} espdev && \
    echo "espdev ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
USER espdev

WORKDIR /workspace

# Clone ESP8266 RTOS SDK
RUN mkdir -p /home/espdev/esp && \
    git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git /home/espdev/esp/ESP8266_RTOS_SDK && \
    cd /home/espdev/esp/ESP8266_RTOS_SDK && \
    git checkout v3.4 && \
    git submodule update --init --recursive && \
    ./install.sh

# Configure environment
RUN echo 'export IDF_PATH=/home/espdev/esp/ESP8266_RTOS_SDK' >> ~/.bashrc && \
    echo 'source $IDF_PATH/export.sh' >> ~/.bashrc && \
    echo 'export TERM=xterm-256color' >> ~/.bashrc

SHELL ["/bin/bash", "-l", "-c"]

CMD ["bash"]