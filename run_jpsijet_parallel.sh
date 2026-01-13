#!/bin/bash
# =============================================================================
# run_jpsijet_parallel.sh - Parallel J/psi in Jet Analysis Pipeline
# =============================================================================
# Usage: bash run_jpsijet_parallel.sh [total_events] [n_jobs] [mode]
# Example: bash run_jpsijet_parallel.sh 1000000 8 prompt

set -e

# Default settings
TOTAL_EVENTS=${1:-100000}
N_JOBS=${2:-4}
MODE=${3:-prompt}

EVENTS_PER_JOB=$((TOTAL_EVENTS / N_JOBS))
IMAGE_GEN="cmsana-gen:py8313-evtgen200"
IMAGE_RIVET="cmsana-rivet:latest"
ANALYSIS_FILE="rivet/JpsiJet_RivetAnalyzer.cc"
ANALYSIS_NAME="JpsiJet_RivetAnalyzer"
BASE_SEED=12345
WORKDIR=$(pwd)

echo "=== Parallel J/psi in Jets Pipeline ==="
echo "Total Events: $TOTAL_EVENTS"
echo "Jobs: $N_JOBS"
echo "Events/Job: $EVENTS_PER_JOB"
echo "Mode: $MODE"
echo "========================================"

# Cleanup old files
rm -f events_*.fifo results_${MODE}_*.yoda
mkdir -p logs

# Determine which generator to use
if [ "$MODE" == "prompt" ]; then
    GEN_EXEC="/work/build/gen_prompt_jpsi"
else
    GEN_EXEC="/work/build/gen_bpkjpsi"
fi

# Pre-compile Rivet analysis ONCE before parallel jobs (avoid race condition)
echo ""
echo "Pre-compiling Rivet analysis..."
rm -f RivetAnalysis.so
docker run --rm \
    -v "${WORKDIR}:/work" \
    --entrypoint rivet-build \
    "$IMAGE_RIVET" \
    -o RivetAnalysis.so "$ANALYSIS_FILE"

if [ ! -f "RivetAnalysis.so" ]; then
    echo "ERROR: Rivet compilation failed!"
    exit 1
fi
echo "Rivet compilation successful: RivetAnalysis.so"

# Function to run a single job
run_job() {
    local JOB_ID=$1
    local SEED=$((BASE_SEED + JOB_ID))
    local FIFO="events_${JOB_ID}.fifo"
    local OUTPUT="results_${MODE}_${JOB_ID}.yoda"
    local CONTAINER_RIVET="rivet_service_${JOB_ID}"
    local CONTAINER_GEN="gen_service_${JOB_ID}"
    
    echo "[Job $JOB_ID] Starting (seed=$SEED, events=$EVENTS_PER_JOB)"
    
    # Create FIFO
    rm -f "$FIFO"
    mkfifo "$FIFO"
    
    # Start Rivet Service (Background) - capture full logs
    docker run --rm -d \
        --name "$CONTAINER_RIVET" \
        -v "${WORKDIR}:/work" \
        "$IMAGE_RIVET" \
        "$ANALYSIS_FILE" "$ANALYSIS_NAME" "$FIFO" "$OUTPUT"
    
    # Follow Rivet logs in background
    docker logs -f "$CONTAINER_RIVET" > "logs/rivet_${JOB_ID}.log" 2>&1 &
    local RIVET_LOG_PID=$!
    
    # Start Generator (writes to FIFO)
    docker run --rm \
        --name "$CONTAINER_GEN" \
        -v "${WORKDIR}:/work" \
        -v "${WORKDIR}/lhapdf_data:/work/lhapdf_data" \
        "$IMAGE_GEN" \
        "$GEN_EXEC" "$EVENTS_PER_JOB" "/work/$FIFO" "$SEED" \
        > "logs/gen_${JOB_ID}.log" 2>&1
    
    # Wait for Rivet to finish
    docker wait "$CONTAINER_RIVET" > /dev/null 2>&1 || true
    
    # Stop log following
    kill $RIVET_LOG_PID 2>/dev/null || true
    
    # Cleanup FIFO
    rm -f "$FIFO"
    
    echo "[Job $JOB_ID] Complete → $OUTPUT (logs: logs/gen_${JOB_ID}.log, logs/rivet_${JOB_ID}.log)"
}

# Export function and variables for parallel
export -f run_job
export BASE_SEED EVENTS_PER_JOB MODE IMAGE_GEN IMAGE_RIVET ANALYSIS_FILE ANALYSIS_NAME GEN_EXEC WORKDIR

# Run jobs in parallel
echo ""
echo "Starting $N_JOBS parallel jobs..."
START_TIME=$(date +%s)

# Use background processes with explicit job control
PIDS=()
for i in $(seq 0 $((N_JOBS - 1))); do
    run_job $i &
    PIDS+=($!)
done

# Wait for all jobs to complete
echo "Waiting for all jobs to complete..."
for pid in "${PIDS[@]}"; do
    wait $pid
done

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo ""
echo "All jobs complete! Elapsed time: ${ELAPSED}s"
echo ""

# List generated files
echo "Generated YODA files:"
ls -la results_${MODE}_*.yoda 2>/dev/null || echo "  (no files found)"

# Merge YODA files
echo ""
echo "Merging YODA files..."
YODA_FILES=$(ls results_${MODE}_*.yoda 2>/dev/null | tr '\n' ' ')

if [ -n "$YODA_FILES" ]; then
    docker run --rm \
        -v "${WORKDIR}:/work" \
        --entrypoint yodamerge \
        "$IMAGE_RIVET" \
        $YODA_FILES -o "results_${MODE}_merged.yoda"
    
    echo ""
    echo "=== Pipeline Complete ==="
    echo "Merged output: results_${MODE}_merged.yoda"
    echo "Individual files preserved: results_${MODE}_*.yoda"
    echo "Logs: logs/gen_*.log"
    
    # Show statistics
    echo ""
    echo "Summary:"
    for log in logs/gen_*.log; do
        JPSI=$(grep "Total J/psi produced" "$log" 2>/dev/null | tail -1 | awk '{print $NF}')
        echo "  $(basename $log .log): $JPSI J/psi"
    done
else
    echo "ERROR: No YODA files generated!"
    exit 1
fi
