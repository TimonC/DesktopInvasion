# Base image with C++ compiler and minimal Linux
FROM ubuntu:24.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies: Qt, CMake, g++, Wayland support, inotify-tools
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        qtbase5-dev \
        qtbase5-dev-tools \
        qtwayland5 \
        libwayland-client0 \
        libwayland-cursor0 \
        libwayland-egl1-mesa \
        inotify-tools \
        wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy build script and CMakeLists.txt into image
COPY build_and_run.sh /app/build_and_run.sh
COPY CMakeLists.txt /app/CMakeLists.txt

# Make build script executable
RUN chmod +x /app/build_and_run.sh

# Set default platform to Wayland
ENV QT_QPA_PLATFORM=wayland
ENV XDG_RUNTIME_DIR=/tmp/xdg

