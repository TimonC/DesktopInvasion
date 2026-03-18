# Use a prebuilt Qt6 image with QML modules
FROM stateoftheartio/qt6:6.6-gcc-aqt

# Use root to install build dependencies
USER root
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    inotify-tools \
    libx11-xcb-dev \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# Set UTF-8 locale
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

WORKDIR /app
COPY build_and_run.sh ./build_and_run.sh
COPY CMakeLists.txt ./CMakeLists.txt
COPY assets ./assets
COPY sprites ./sprites
COPY src ./src
COPY resources.qrc ./resources.qrc

# Make build script executable
RUN chmod +x ./build_and_run.sh
# Entrypoint
ENTRYPOINT ["./build_and_run.sh"]

