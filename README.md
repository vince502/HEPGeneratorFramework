# HEP Generation & Analysis Framework

A unified framework for Monte Carlo event generation and real-time Rivet analysis, optimized for Heavy Flavor and Heavy Ion studies. This repository integrates Pythia 8, EvtGen, and Rivet 4 into a containerized pipeline designed for both local development and massive scale-out on HTCondor.

## 1. Key Features
- **Generators**: Pythia 8.313 (with OniaShower and Angantyr) & EvtGen 2.2.3.
- **Analysis**: Rivet 4.1.0 & YODA 2.1.2 integrated via real-time FIFO data streaming.
- **Physics**: CMS CP5 Tune synchronized across all production modes.
- **Portability**: Fully containerized using Docker/Singularity for bit-reproducible results.

---

## 2. Setup Guide

### Local Clone & Preparation
```bash
git clone https://github.com/vince502/HEPGeneratorFramework.git
cd HEPGeneratorFramework
```

### Building the Environments
The framework uses two specialized Docker images to isolate generation from analysis:
```bash
# 1. Build the Generator environment
docker build -t cmsana-gen -f docker/Dockerfile .

# 2. Build the Rivet Analysis environment (uses official Rivet 4 base)
docker build -t cmsana-rivet -f docker/Dockerfile.rivet .
```

### Compiling the C++ Generators
The generators are compiled inside the `cmsana-gen` environment. Use the provided setup script to handle the compilation and LHAPDF data:
```bash
bash setup_server.sh
```

---

## 3. Running Analyses

### J/psi in Jets (Real-time Pipeline)
This pipeline streams events directly from Pythia to Rivet via a FIFO pipe, saving disk space by avoiding large intermediate HepMC files.

```bash
# Usage: bash run_jpsijet_pipeline.sh [events] [mode: prompt/nonprompt]
bash run_jpsijet_pipeline.sh 50000 prompt
```
Outputs: `results_prompt.yoda` containing histograms.

### D0 Spin Alignment (Pb-Pb)
Generate D0 candidates in Heavy Ion collisions with the CP5 tune:
```bash
./run_cp5_parallel.sh [total_events] [num_cores]
```

---

## 4. Batch Production (HTCondor)
For large-scale production, use the Python orchestrator to submit jobs to a cluster. It handles unique seeds and output indexing automatically.

```bash
# Submit 100 jobs of 100k events each (Prompt J/psi in Jets)
python3 condor/submit_condor.py \
    --total-events 10000000 \
    --events-per-job 100000 \
    --mode prompt \
    --rivet JpsiJet_RivetAnalyzer
```

---

## 5. Customization Documentation

### Modifying Pythia Settings
The C++ generators in `src/` (e.g., `gen_prompt_jpsi.cc`) contain a synchronized **CMS CP5 Tune** block. 
- To change the **pT-hat min**, modify `PhaseSpace:pTHatMin`.
- To adjust **MPI/ISR/FSR**, edit the respective `MultipartonInteractions` or `SpaceShower` readString calls.
- **Note**: Ensure you recompile (`bash setup_server.sh`) after modifying C++ source files.

### Modifying Rivet Histograms
Your analysis logic lives in `rivet/JpsiJet_RivetAnalyzer.cc`. 
- **Adding Histograms**: Register them in the `init()` function using `book(_h_myhist, "MyHist", 50, 0, 100)`.
- **Applying Cuts**: Modify the `analyze()` function to update J/psi or Jet selection criteria.
- **Real-time Build**: The `cmsana-rivet` container automatically re-compiles your `.cc` code every time you start the pipeline, so there is no need to manually build the plugin.

### Viewing Results
Use the containerized YODA tools to inspect your output:
```bash
# List histograms
docker run --rm -v $(pwd):/work cmsana-rivet yodals /work/results_prompt.yoda

# Generate HTML gallery
docker run --rm -v $(pwd):/work cmsana-rivet rivet-mkhtml /work/results_prompt.yoda
```
