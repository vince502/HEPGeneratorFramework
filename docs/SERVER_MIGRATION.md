# Server Migration Plan: Massive D0 Production

To move the current analysis to a server for high-statistics production, follow these steps:

## 1. Prepare the Server
The server should have:
- Ubuntu 22.04 or similar Linux distribution.
- Docker installed and configured ($USER added to the `docker` group).
- Sufficient disk space for generated data.

## 2. Deploy Code & Environment
Run the following commands on the server to clone the latest code and setup the environment:

```bash
# Clone the repository
git clone https://github.com/vince502/HEPGeneratorFramework.git
cd HEPGeneratorFramework

# Pull the pre-built multi-platform image from GHCR
docker pull ghcr.io/vince502/hepgeneratorframework:main
docker tag ghcr.io/vince502/hepgeneratorframework:main cmsana-gen:py8313-evtgen200
```

## 3. Setup LHAPDF for CP5 Tune
The CP5 tune requires the `NNPDF31_nnlo_as_0118` PDF set. Run this on the server to prepare the data:

```bash
mkdir -p lhapdf_data
cd lhapdf_data
wget https://lhapdfsets.web.cern.ch/lhapdfsets/current/NNPDF31_nnlo_as_0118.tar.gz
tar -xf NNPDF31_nnlo_as_0118.tar.gz
rm NNPDF31_nnlo_as_0118.tar.gz
cd ..
```

## 4. Run Massive Production
I've updated `run_cp5_parallel.sh` (or `run_parallel_server.sh`) to automatically detect the number of CPU cores.

```bash
# Example: Generate 1M events using all available cores
TOTAL_EVENTS=1000000 bash run_cp5_parallel.sh
```
