FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i 's|http://archive.ubuntu.com/ubuntu/|http://mirrors.kernel.org/ubuntu/|g' /etc/apt/sources.list

RUN apt-get update

RUN apt-get install -y git
RUN apt-get install -y cmake
RUN apt-get install -y build-essential
RUN apt-get install -y clang
RUN apt-get install -y libx11-dev
RUN apt-get install -y xorg-dev
RUN apt-get install -y libglu1-mesa-dev

RUN apt install -y software-properties-common
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt update
RUN apt-get install -y g++-13
RUN apt-get install -y gcc-13

RUN apt-get install -y libgtk-3-dev

# Install Node.js and npm
RUN apt-get install -y curl
RUN curl -sL https://deb.nodesource.com/setup_18.x | bash -
RUN apt-get install -y nodejs

RUN apt-get clean

ENV CXX=g++-13
ENV CC=gcc-13

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
RUN g++ --version
RUN gcc --version
RUN echo "Checking C++13 support..." && g++-13 -dM -E -x c++ /dev/null | grep -i cplusplus

WORKDIR /app

COPY . .

RUN npm install

# Rebuild the addon
RUN npm run install

RUN npm run build

ENV LD_LIBRARY_PATH=/app/cplusplus/lib/bin

CMD [ "node", "dist/index.js" ]
