# Use a prebuilt Qt6 image with QML modules
FROM stateoftheartio/qt6:6.6-gcc-aqt

# Use root to install build dependencies
USER root
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools, CMake, git, X11/OpenGL/Vulkan support, inotify
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libx11-xcb-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-render-util0-dev \
    libxcb-xinerama0-dev \
    libgl1-mesa-dev \
    libxkbcommon-dev \
    libvulkan-dev \
    vulkan-tools \
    inotify-tools \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy your project files
COPY build_and_run.sh ./build_and_run.sh
COPY CMakeLists.txt ./CMakeLists.txt
COPY assets ./assets
COPY sprites ./sprites
COPY src ./src
COPY resources.qrc ./resources.qrc

# Make build script executable
RUN chmod +x ./build_and_run.sh

# Use root for development / X11 forwarding
USER root

# Entrypoint
ENTRYPOINT ["./build_and_run.sh"]

