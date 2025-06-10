# Use Ubuntu 22.04 as the base image
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    libtool \
    autotools-dev \
    automake \
    pkg-config \
    bsdmainutils \
    curl \
    git \
    libevent-dev \
    libboost-all-dev \
    libssl-dev \
    libzmq3-dev \
    libminiupnpc-dev \
    python3 \
    ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Clone Shahcoin (Shahcoin) repo
WORKDIR /root
RUN git clone https://github.com/SHAHCoinvip/shahcoin.git shahcoin

# Build Shahcoin
WORKDIR /root/shahcoin
RUN ./autogen.sh && \
    ./configure --disable-tests --with-incompatible-bdb && \
    make -j$(nproc)

# Default command to show version
CMD ["/root/shahcoin/src/shahcoind", "--version"]

