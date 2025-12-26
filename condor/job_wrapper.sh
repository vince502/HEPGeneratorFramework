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

echo "Starting job on $(hostname)"
echo "Events: $EVENTS"
echo "Seed: $SEED"
echo "Output: $OUTFILE"

# Make sure we can find the executable
# If we transferred the 'build' folder, it will be in the landing directory
chmod +x ./build/gen_d0_study

# Run the generator
./build/gen_d0_study $EVENTS $SEED $OUTFILE

echo "Job finished with exit code $?"
