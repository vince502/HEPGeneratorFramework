# HEP Event Generation with Pythia8 + EvtGen + Angantyr

[![Docker Build](https://github.com/YOUR_USERNAME/hep-generation/actions/workflows/docker-build.yml/badge.svg)](https://github.com/YOUR_USERNAME/hep-generation/actions/workflows/docker-build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A containerized Monte Carlo event generation framework for Heavy Ion and Heavy Flavor physics studies. Features:

- **Pythia 8.313** - Hard scattering, parton shower, hadronization
- **EvtGen 2.2.3** - Heavy flavor decays with proper angular distributions
- **Angantyr** - Heavy Ion collisions (pPb, PbPb)
- **Photos++ 3.64** - QED final state radiation
- **Tauola++ 1.1.8** - Polarized tau decays
- **HepMC3 3.3.0** - Standard event record format

## Quick Start

### Pull Pre-built Image
```bash
# For Linux x86_64
docker pull ghcr.io/vince502/hep-generation:latest

# For Mac M1/M2/M3
docker pull ghcr.io/vince502/hep-generation:latest
```

### Build Locally
```bash
git clone https://github.com/YOUR_USERNAME/hep-generation.git
cd hep-generation

# Build for current platform
docker build -f docker/Dockerfile -t hep-generation:latest .

# Cross-platform build (requires buildx)
docker buildx build --platform linux/amd64,linux/arm64 \
    -f docker/Dockerfile -t hep-generation:latest .
```

## Usage Examples

### 1. B+ → K+ J/ψ(μμ) Signal
```bash
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    ./build/gen_bpkjpsi 10000 signal_events.hepmc3
```

### 2. Heavy Ion Collisions (Pb-Pb)
```bash
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    ./build/gen_angantyr 100 pbpb_events.hepmc3
```

### 3. D0 Spin Alignment Study (Streamlined)
```bash
# Direct analysis output - no large HepMC3 files
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    bash -c "cd build && ./gen_d0_study 10000 1234 /work/results.txt"

# Generate plot
python3 scripts/plot_spin_pt.py output/results.txt
```

## Generators

| Generator | Description | Output |
|-----------|-------------|--------|
| `gen_bpkjpsi` | B+ → K+ J/ψ(μμ) signal | HepMC3 |
| `gen_angantyr` | Heavy Ion test (pPb, PbPb) | HepMC3 |
| `gen_d0_study` | D0 spin alignment (Pb-Pb) | Text (type, pT, cosθ*) |
| `analyze_spin` | HepMC3 analyzer for D* | Text |

## Physics Models

### D0 Spin Alignment
- **Quantization axis**: Event plane normal (B-field direction)
- **Lorentz boost**: Axis properly transformed to D* rest frame
- **Prompt/Non-prompt**: Automatic classification via ancestry

### Decay Files
- `decays/BpKJpsi.dec` - B+ signal decay chain
- `decays/D0SpinAlignment.dec` - D* → D0π with VSS model

## Multi-Platform Support

This repository uses GitHub Actions to build Docker images for:
- `linux/amd64` (Intel/AMD x86_64)
- `linux/arm64` (Apple Silicon M1/M2/M3, AWS Graviton)

## Development

```bash
# Enter container for development
docker run -it --rm -v $(pwd):/work hep-generation:latest bash

# Inside container
cd build && cmake .. && make -j$(nproc)
```

## Citation

If you use this framework, please cite:
- Pythia 8: [arXiv:2203.11601](https://arxiv.org/abs/2203.11601)
- EvtGen: [Comput.Phys.Commun. 181 (2010) 1721](https://doi.org/10.1016/j.cpc.2010.05.025)

## License

MIT License - see [LICENSE](LICENSE)
