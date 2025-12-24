# syntax=docker/dockerfile:1.6
FROM almalinux:9 AS builder
ENV GIT_TERMINAL_PROMPT=0
ENV PREFIX=/opt/hep
ENV PATH=${PREFIX}/bin:$PATH
ENV LD_LIBRARY_PATH=${PREFIX}/lib64:${PREFIX}/lib:$LD_LIBRARY_PATH

ARG PYTHIA_VER=8313
ARG EVTGEN_TAG=R02-02-03   # EvtGen 2.2.3 - latest stable release

RUN dnf -y update && \
  dnf -y install dnf-plugins-core && \
  dnf config-manager --set-enabled crb && \
  dnf -y install \
  git curl-minimal wget tar which rsync \
  gcc gcc-c++ gcc-gfortran make cmake ninja-build autoconf automake libtool \
  python3 python3-pip \
  bzip2 bzip2-devel zlib zlib-devel xz \
  openssl-devel \
  && dnf clean all

RUN ln -s /usr/bin/g++ /usr/bin/g

ENV PREFIX=/opt/hep
ENV PATH=${PREFIX}/bin:$PATH
ENV LD_LIBRARY_PATH=${PREFIX}/lib64:${PREFIX}/lib:$LD_LIBRARY_PATH
RUN mkdir -p ${PREFIX}/src

# --- HepMC3 ---
WORKDIR ${PREFIX}/src
RUN git clone --depth 1 https://github.com/hep-mirrors/hepmc3.git && \
  cmake -S hepmc3 -B hepmc3/build -G Ninja \
  -DCMAKE_INSTALL_PREFIX=${PREFIX} \
  -DHEPMC3_ENABLE_TEST=OFF \
  -DHEPMC3_ENABLE_ROOTIO=OFF \
  -DHEPMC3_ENABLE_PYTHON=OFF && \
  cmake --build hepmc3/build && \
  cmake --install hepmc3/build

# --- LHAPDF6 (autotools build) ---
WORKDIR ${PREFIX}/src
RUN git clone --depth 1 https://github.com/hep-mirrors/lhapdf.git && \
  cd lhapdf && \
  autoreconf -i && \
  ./configure --prefix=${PREFIX} --disable-python && \
  make -j"$(nproc)" && make install

# --- Photos++ 3.64 ---
WORKDIR ${PREFIX}/src
RUN git clone --depth 1 -b v3.64 https://gitlab.cern.ch/photospp/photospp.git && \
  cd photospp && \
  mkdir -p lib include && \
  CXX=g++ CC=g++ F77=gfortran ./configure --prefix=${PREFIX} --with-hepmc3=${PREFIX} --without-hepmc && \
  make CXX=g++ CC=g++ F77=gfortran && make install

# --- Tauola++ 1.1.8 ---
WORKDIR ${PREFIX}/src
RUN git clone --depth 1 -b v1.1.8 https://gitlab.cern.ch/tauolapp/tauolapp.git && \
  cd tauolapp && \
  mkdir -p lib include && \
  CXX=g++ CC=g++ F77=gfortran ./configure --prefix=${PREFIX} --with-hepmc3=${PREFIX} --without-hepmc && \
  make CXX=g++ CC=g++ F77=gfortran && make install

# --- Pythia 8.313 from official release tarball ---
WORKDIR ${PREFIX}/src
RUN curl -L -o pythia${PYTHIA_VER}.tgz https://pythia.org/download/pythia83/pythia${PYTHIA_VER}.tgz && \
  tar -xzf pythia${PYTHIA_VER}.tgz && \
  cd pythia${PYTHIA_VER} && \
  ./configure --prefix=${PREFIX} --with-hepmc3=${PREFIX} --with-lhapdf6=${PREFIX} && \
  make -j"$(nproc)" && make install

# --- EvtGen 2.2.3 from CERN GitLab ---
WORKDIR ${PREFIX}/src
RUN git clone https://gitlab.cern.ch/evtgen/evtgen.git && \
  cd evtgen && git checkout ${EVTGEN_TAG} && \
  cmake -S . -B build -G Ninja \
  -DCMAKE_INSTALL_PREFIX=${PREFIX} \
  -DEVTGEN_HEPMC3=ON \
  -DEVTGEN_PYTHIA=ON \
  -DEVTGEN_PHOTOS=ON \
  -DEVTGEN_TAUOLA=ON \
  -DHEPMC3_DIR=${PREFIX} \
  -DPYTHIA8_DIR=${PREFIX} \
  -DPHOTOSPP_DIR=${PREFIX} \
  -DTAUOLAPP_DIR=${PREFIX} \
  -DEVTGEN_TESTS=OFF && \
  cmake --build build && \
  cmake --install build

# Runtime image
FROM almalinux:9

RUN dnf -y update && dnf -y install --allowerasing \
  gcc-c++ gcc-gfortran libstdc++ \
  zlib bzip2 xz \
  make cmake which coreutils rsync \
  && dnf clean all

ENV PREFIX=/opt/hep
ENV PATH=${PREFIX}/bin:$PATH
ENV LD_LIBRARY_PATH=${PREFIX}/lib64:${PREFIX}/lib:$LD_LIBRARY_PATH

COPY --from=builder /opt/hep /opt/hep

WORKDIR /work
CMD ["/bin/bash"]
