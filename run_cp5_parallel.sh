#!/bin/bash

# =============================================================================
# run_cp5_parallel.sh - Parallel D0 Producion with CMS CP5 Tune
# Supports TOTAL_EVENTS and NUM_CORES environment variables
# =============================================================================

# Configuration (Use env vars if set, otherwise defaults)
TOTAL_EVENTS=${TOTAL_EVENTS:-100000}
NUM_CORES=${NUM_CORES:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}
EVENTS_PER_CORE=$((TOTAL_EVENTS / NUM_CORES))
IMAGE_NAME="cmsana-gen:py8313-evtgen200"
OUTPUT_DIR="$(pwd)/output_cp5"
FINAL_OUTPUT="output_cp5_combined.txt"

# Ensure we have the LHAPDF data directory or mount point
LHAPDF_DIR="$(pwd)/lhapdf_data"
if [ ! -d "$LHAPDF_DIR/NNPDF31_nnlo_as_0118" ]; then
    echo "Warning: LHAPDF directory $LHAPDF_DIR/NNPDF31_nnlo_as_0118 not found!"
    echo "Please download the PDF set first (see docs/SERVER_MIGRATION.md)."
    # Attempt to continue if the user knows what they are doing
fi

mkdir -p "$OUTPUT_DIR"

echo "=================================================="
echo "Starting Parallel CP5 D0 Study"
echo "Total Events: $TOTAL_EVENTS"
echo "Cores: $NUM_CORES ($EVENTS_PER_CORE events/core)"
echo "Output Directory: $OUTPUT_DIR"
echo "=================================================="

# Build before running to ensure latest changes are included
echo "Rebuilding gen_d0_study..."
docker run --rm -v "$(pwd):/work" -v "$LHAPDF_DIR:/opt/hep/share/LHAPDF" "$IMAGE_NAME" \
    bash -c "mkdir -p /work/build && cd /work/build && cmake .. && make -j$NUM_CORES gen_d0_study"

# Launch parallel jobs
pids=()
for i in $(seq 1 $NUM_CORES); do
    SEED=$((1234 + i))
    CORE_OUT="/work/output_cp5/out_core_$i.txt"
    echo "Launching Core $i (Seed: $SEED)..."
    
    docker run --rm -v "$(pwd):/work" -v "$LHAPDF_DIR:/opt/hep/share/LHAPDF" "$IMAGE_NAME" \
        /work/build/gen_d0_study $EVENTS_PER_CORE $SEED "$CORE_OUT" > "$OUTPUT_DIR/log_core_$i.log" 2>&1 &
    
    pids+=($!)
done

echo "Jobs running. Waiting for completion..."

# Wait for all jobs
for pid in "${pids[@]}"; do
    wait "$pid"
done

echo "All jobs finished. Merging results..."

# Merge and clean (ensure exactly 5 columns)
rm -f "$FINAL_OUTPUT"
for i in $(seq 1 $NUM_CORES); do
    if [ -f "$OUTPUT_DIR/out_core_$i.txt" ]; then
        awk 'NF == 5' "$OUTPUT_DIR/out_core_$i.txt" >> "$FINAL_OUTPUT"
    fi
done

echo "=================================================="
echo "SUCCESS: Merged and cleaned output saved to $FINAL_OUTPUT"
echo "Total candidates: $(wc -l < "$FINAL_OUTPUT")"
echo "Run analysis with:"
echo "python3 scripts/plot_d0_combined.py $FINAL_OUTPUT"
echo "=================================================="
