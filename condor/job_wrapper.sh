#!/bin/bash
# =============================================================================
# job_wrapper.sh - Entry point for HTCondor jobs
# =============================================================================

# 1. Setup environment (if not running in Docker Universe)
# If running in Docker Universe, the container already has the libs in LD_LIBRARY_PATH.
# However, we need to ensure local directories (decays, build) are visible.

EVENTS=$1
SEED=$2
OUTFILE=$3
MODE=${4:-prompt} # prompt/nonprompt/d0
ANALYSIS=${5:-none}

echo "Starting job on $(hostname)"
echo "Events: $EVENTS"
echo "Seed: $SEED"
echo "Mode: $MODE"
echo "Analysis: $ANALYSIS"

# Define executables
if [ "$MODE" == "prompt" ]; then
    GEN_EXEC="./build/gen_prompt_jpsi"
elif [ "$MODE" == "nonprompt" ]; then
    GEN_EXEC="./build/gen_bpkjpsi"
else
    GEN_EXEC="./build/gen_d0_study"
fi

if [ "$ANALYSIS" != "none" ]; then
    echo "Running with Rivet Pipeline..."
    FIFO="events.fifo"
    rm -f $FIFO && mkfifo $FIFO
    
    # Run Rivet in background
    # Note: Using the built-in rivet-service runner if available
    rivet-service "rivet/${ANALYSIS}.cc" "$ANALYSIS" "$FIFO" "$OUTFILE" &
    RIVET_PID=$!
    
    # Run Generator in foreground
    $GEN_EXEC $EVENTS $FIFO
    
    wait $RIVET_PID
else
    echo "Running Standard Generation..."
    $GEN_EXEC $EVENTS $SEED $OUTFILE
fi

echo "Job finished with exit code $?"
