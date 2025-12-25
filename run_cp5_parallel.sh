#!/bin/bash

# Configuration
TOTAL_EVENTS=100000
NUM_CORES=5
EVENTS_PER_CORE=$((TOTAL_EVENTS / NUM_CORES))
IMAGE_NAME="cmsana-gen:py8313-evtgen200"
OUTPUT_DIR="$(pwd)/output_cp5"
FINAL_OUTPUT="output_cp5_combined.txt"

mkdir -p "$OUTPUT_DIR"

echo "=================================================="
echo "Starting Parallel CP5 D0 Study"
echo "Total Events: $TOTAL_EVENTS"
echo "Cores: $NUM_CORES ($EVENTS_PER_CORE events/core)"
echo "Output Directory: $OUTPUT_DIR"
echo "=================================================="

# Build before running to ensure CP5 changes are included
echo "Rebuilding gen_d0_study..."
docker run --rm -v "$(pwd):/work" -v "$(pwd)/lhapdf_data:/opt/hep/share/LHAPDF" "$IMAGE_NAME" bash -c "cd /work/build && cmake .. && make -j4 gen_d0_study"

# Launch parallel jobs
pids=()
for i in $(seq 1 $NUM_CORES); do
    SEED=$((1234 + i))
    CORE_OUT="/work/output_cp5/out_core_$i.txt"
    echo "Launching Core $i (Seed: $SEED)..."
    
    docker run --rm -v "$(pwd):/work" -v "$(pwd)/lhapdf_data:/opt/hep/share/LHAPDF" "$IMAGE_NAME" \
        /work/build/gen_d0_study $EVENTS_PER_CORE $SEED "$CORE_OUT" > "$OUTPUT_DIR/log_core_$i.log" 2>&1 &
    
    pids+=($!)
done

echo "Jobs running. Waiting for completion..."

# Wait for all jobs
for pid in "${pids[@]}"; do
    wait "$pid"
done

echo "All jobs finished. Merging results..."

# Merge all text files (skipping any potential headers if added later)
rm -f "$FINAL_OUTPUT"
for i in $(seq 1 $NUM_CORES); do
    cat "$OUTPUT_DIR/out_core_$i.txt" >> "$FINAL_OUTPUT"
done

echo "=================================================="
echo "SUCCESS: Merged output saved to $FINAL_OUTPUT"
echo "Total lines: $(wc -l < "$FINAL_OUTPUT")"
echo "Run analysis with:"
echo "python3 Generation-clean/scripts/plot_d0_combined.py $FINAL_OUTPUT"
echo "=================================================="
