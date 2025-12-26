#!/bin/bash
# =============================================================================
# setup_server.sh - Setup script for Production Server (Ubuntu/CentOS)
# =============================================================================

set -e

echo "=== HEP Generation Framework Server Setup ==="

# 1. Install Docker if missing
if ! command -v docker &> /dev/null; then
    echo "Installing Docker..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y docker.io
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y docker
    fi
    sudo usermod -aG docker $USER
    echo "Docker installed. Please LOG OUT and LOG IN again, then re-run this script."
    exit 0
fi

# 2. Setup Data Directory
mkdir -p lhapdf_data
if [ ! -d "lhapdf_data/NNPDF31_nnlo_as_0118" ]; then
    echo "Downloading LHAPDF set (NNPDF3.1)..."
    cd lhapdf_data
    wget -q https://lhapdfsets.web.cern.ch/lhapdfsets/current/NNPDF31_nnlo_as_0118.tar.gz
    tar -xf NNPDF31_nnlo_as_0118.tar.gz
    rm NNPDF31_nnlo_as_0118.tar.gz
    cd ..
fi

# Ensure basic LHAPDF config files are present to avoid MetadataError
IMAGE_NAME="cmsana-gen:py8313-evtgen200"
if [ ! -f "lhapdf_data/lhapdf.conf" ]; then
    echo "Extracting LHAPDF configuration from image..."
    # Temporary run to grab files
    docker run --rm $IMAGE_NAME cat /opt/hep/share/LHAPDF/lhapdf.conf > lhapdf_data/lhapdf.conf || true
    docker run --rm $IMAGE_NAME cat /opt/hep/share/LHAPDF/pdfsets.index > lhapdf_data/pdfsets.index || true
fi

# 3. Pull or Build Docker Image
IMAGE_NAME="cmsana-gen:py8313-evtgen200"
if ! docker images | grep -q "cmsana-gen"; then
    echo "Pulling Docker image from GHCR..."
    # Replace with your actual GHCR image if public, otherwise build
    if docker pull ghcr.io/vince502/hepgeneratorframework:main; then
        docker tag ghcr.io/vince502/hepgeneratorframework:main $IMAGE_NAME
    else
        echo "GHCR pull failed. Building image locally (this may take 20-30 mins)..."
        docker build -t $IMAGE_NAME -f docker/Dockerfile .
    fi
fi

# 4. Compile Executables
echo "Compiling generators..."
docker run --rm -v "$(pwd):/work" -v "$(pwd)/lhapdf_data:/opt/hep/share/LHAPDF" $IMAGE_NAME \
    bash -c "mkdir -p build && cd build && cmake .. && make -j$(nproc)"

echo ""
echo "=== Setup Complete! ==="
echo "You can now run massive production using:"
echo "  TOTAL_EVENTS=1000000 bash run_cp5_parallel.sh"
echo ""
