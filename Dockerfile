FROM ubuntu:22.04

# Avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install basic dependencies
RUN apt update && \
    apt install -y software-properties-common wget gnupg

# Add LLVM repository for clang-14
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-14 main"

# Install system dependencies
RUN apt update && \
    apt install -y \
    clang \
    clang-14 \
    make \
    cmake \
    libclang-common-14-dev \
    libclang-14-dev \
    llvm-14-dev \
    gcc-11 \
    g++-11 \
    python3 \
    python3-pip \
    git \
    build-essential \
    libstdc++-11-dev

# Set clang-14 and g++-11 as default compilers
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-14 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-14 100 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

# Install Python dependencies
RUN pip3 install --upgrade pip && \
    pip3 install pytest deepdiff

# Set working directory
WORKDIR /app

# Default command
CMD ["/bin/bash"]
