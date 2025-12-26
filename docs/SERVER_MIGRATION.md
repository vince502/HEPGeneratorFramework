# Production Deployment Guide

This document outlines the procedure for deploying the framework to a production server or cluster for high-statistics event generation and analysis.

## 1. Environment Requirements
- **OS**: Linux (Ubuntu 22.04+ or AlmaLinux 9 recommended).
- **Core Dependencies**: Docker (with sudo-less access) and Git.
- **Resource Limits**: Ensure sufficient RAM for Rivet compilation (min 8GB recommended).

## 2. Server Deployment Workflow

### Initial Setup
```bash
# Clone the repository
git clone https://github.com/vince502/HEPGeneratorFramework.git
cd HEPGeneratorFramework

# Automated setup: builds images, downloads PDFs, and compiles generators
bash setup_server.sh
```

### Manual Component Verification
If you need to verify individual components:
1. **LHAPDF**: Ensure `NNPDF31_nnlo_as_0118` is downloaded in `lhapdf_data/`.
2. **Docker**: Verify the generator image (`cmsana-gen`) and rivet image (`cmsana-rivet`) are tagged.
3. **Binaries**: Ensure `build/gen_prompt_jpsi`, etc. exist and are linked correctly.

---

## 3. High-Statistics Production

### Local Parallelization
For runs on a single high-core machine (no cluster), use the optimized parallel runner:
```bash
# Detects cores automatically and runs parallel instances
TOTAL_EVENTS=1000000 bash run_cp5_parallel.sh
```

### Cluster Scale-out (HTCondor)
For production reaching 100M+ events, use the HTCondor orchestration.

**Submission**:
```bash
# Dispatch 10M events for Prompt J/psi analysis
python3 condor/submit_condor.py \
    --total-events 10000000 \
    --events-per-job 100000 \
    --mode prompt \
    --rivet JpsiJet_RivetAnalyzer
```

**Resource Management**:
You can adjust resource requested per job (memory/CPU/disk) via command line:
```bash
python3 condor/submit_condor.py --mode prompt --memory 4000 --cpus 1
```

**Monitoring & Completion**:
```bash
# Check job status
condor_q

# Merger results (for YODA files)
docker run --rm -v $(pwd):/work cmsana-rivet yodamerge -o merged.yoda /work/condor/output/*.yoda
```

---

## 4. Maintenance & Updates
When source code or analysis macros are updated in the repository:
1. Run `git pull origin main` on the server.
2. Re-run `bash setup_server.sh` to ensure C++ binaries are up to date.
3. The Rivet container re-compiles the analysis macro at runtime; no secondary build is needed for it.
