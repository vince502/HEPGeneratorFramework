#!/bin/bash
# =============================================================================
# run_jpsijet_pipeline.sh - J/psi in Jet Analysis Pipeline
# =============================================================================

# Default settings
EVENTS=${1:-5000}
MODE=${2:-prompt} # prompt or nonprompt
IMAGE_GEN="cmsana-gen:py8313-evtgen200"
IMAGE_RIVET="cmsana-rivet:latest"
ANALYSIS_FILE="rivet/JpsiJet_RivetAnalyzer.cc"
ANALYSIS_NAME="JpsiJet_RivetAnalyzer"
FIFO_NAME="events.fifo"
OUTPUT_YODA="results_${MODE}.yoda"

echo "=== Starting J/psi in Jets Pipeline ($MODE mode) ==="
echo "Events: $EVENTS"

# 1. Cleanup old files
rm -f $FIFO_NAME
mkfifo $FIFO_NAME

# 2. Determine which generator to use
if [ "$MODE" == "prompt" ]; then
    GEN_EXEC="/work/build/gen_prompt_jpsi"
else
    GEN_EXEC="/work/build/gen_bpkjpsi"
fi

# 3. Start Rivet Service (Background)
echo "Starting Rivet Service Container..."
docker run --rm -d \
    --name rivet_service \
    -v "$(pwd):/work" \
    "$IMAGE_RIVET" \
    "$ANALYSIS_FILE" "$ANALYSIS_NAME" "$FIFO_NAME" "$OUTPUT_YODA"

# 4. Start Generator (Foreground)
echo "Starting Generator: $GEN_EXEC"
docker run --rm \
    -v "$(pwd):/work" \
    -v "$(pwd)/lhapdf_data:/work/lhapdf_data" \
    "$IMAGE_GEN" \
    "$GEN_EXEC" "$EVENTS" "/work/$FIFO_NAME"

# 5. Wait for Rivet to finish
echo "Waiting for Rivet to finalize..."
docker wait rivet_service > /dev/null

echo "Pipeline Finished! Results in $OUTPUT_YODA"
rm -f $FIFO_NAME
