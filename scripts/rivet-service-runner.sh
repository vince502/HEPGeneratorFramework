#!/bin/bash
# =============================================================================
# rivet-service-runner.sh - Automated Build & Run Service
# =============================================================================

# Usage: ./rivet-service-runner.sh <AnalysisSource.cc> <AnalysisName> <InputFIFO> <OutputYoda>

# Check if we are running the service pipeline OR a standard command
if [[ "$1" == "yodals" || "$1" == "rivet-mkhtml" || "$1" == "yodadiff" || "$1" == "rivet" || "$1" == "bash" ]]; then
    exec "$@"
fi

SOURCE_CC=$1
ANALYSIS_NAME=$2
INPUT_FIFO=$3
OUTPUT_YODA=$4

echo "=== Rivet Service Started ==="

# 1. Check if pre-compiled .so exists, skip compilation if so (avoids race condition in parallel jobs)
if [ -f "RivetAnalysis.so" ]; then
    echo "Using pre-compiled RivetAnalysis.so (skipping build)"
else
    echo "Building analysis plugin: $SOURCE_CC..."
    if rivet-build RivetAnalysis.so "$SOURCE_CC"; then
        echo "Build Successful: RivetAnalysis.so created."
    else
        echo "ERROR: Compilation failed!"
        exit 1
    fi
fi

# 2. Tell Rivet to look in the current directory for the new plugin
export RIVET_ANALYSIS_PATH=$PWD

# 3. Wait for the FIFO to exist (blocking wait)
echo "Waiting for data stream on $INPUT_FIFO..."
while [ ! -p "$INPUT_FIFO" ]; do
    sleep 1
done

# 4. Run the analysis
echo "Starting analysis $ANALYSIS_NAME..."
rivet -a "$ANALYSIS_NAME" "$INPUT_FIFO" -o "$OUTPUT_YODA"

echo "=== Analysis Complete! Results saved to $OUTPUT_YODA ==="
