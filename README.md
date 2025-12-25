# HEP Event Generation with Pythia8 + EvtGen + Angantyr

[![Docker Build](https://github.com/vince502/HEPGeneratorFramework/actions/workflows/docker-build.yml/badge.svg)](https://github.com/vince502/HEPGeneratorFramework/actions/workflows/docker-build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A containerized Monte Carlo event generation framework for Heavy Ion and Heavy Flavor physics studies. Features:

- **Pythia 8.313** - Hard scattering, parton shower, hadronization, OniaShower
- **EvtGen 2.2.3** - Heavy flavor decays with proper angular distributions
- **Angantyr** - Heavy Ion collisions (pPb, PbPb)
- **Photos++ 3.64** - QED final state radiation
- **Tauola++ 1.1.8** - Polarized tau decays
- **HepMC3 3.3.0** - Standard event record format

## Quick Start

### Pull Pre-built Image
```bash
docker pull ghcr.io/vince502/hepgeneratorframework:latest
```

### Build Locally
```bash
git clone https://github.com/vince502/HEPGeneratorFramework.git
cd HEPGeneratorFramework

# Build for current platform
docker build -f docker/Dockerfile -t hep-generation:latest .

# Cross-platform build (requires buildx)
docker buildx build --platform linux/amd64,linux/arm64 \
    -f docker/Dockerfile -t hep-generation:latest .
```

## Usage Examples

### 1. Prompt J/ψ Production (OniaShower)
```bash
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    ./build/gen_prompt_jpsi 10000 prompt_jpsi.hepmc3
```

### 2. B+ → K+ J/ψ(μμ) Signal
```bash
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    ./build/gen_bpkjpsi 10000 signal_events.hepmc3
```

### 3. Heavy Ion Collisions (Pb-Pb)
```bash
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    ./build/gen_angantyr 100 pbpb_events.hepmc3
```

### 4. D0 Spin Alignment Study (Streamlined)
```bash
docker run --rm -v $(pwd)/output:/work hep-generation:latest \
    bash -c "cd build && ./gen_d0_study 10000 1234 /work/results.txt"

python3 scripts/plot_spin_pt.py output/results.txt
```

## Generators

| Generator | Description | Output |
|-----------|-------------|--------|
| `gen_prompt_jpsi` | Prompt J/ψ via OniaShower (charmonium) | HepMC3 |
| `gen_bpkjpsi` | B+ → K+ J/ψ(μμ) signal | HepMC3 |
| `gen_angantyr` | Heavy Ion test (pPb, PbPb) | HepMC3 |
| `gen_d0_study` | D0 spin alignment (Pb-Pb) | Text |
| `analyze_spin` | HepMC3 analyzer for D* | Text |

## Tweaking Pythia Parameters

The `gen_prompt_jpsi.cc` file has a clearly marked **CONFIGURATION** section where you can modify:

```cpp
// --- Beam Settings ---
double sqrtS = 13600.0;  // Center-of-mass energy [GeV]

// --- Charmonium Production ---
pythia.readString("Charmonium:all = on");  // All charmonium processes

// --- Phase Space Cuts ---
double pTHatMin = 0.0;   // Minimum pT of hard process

// --- Color-Octet Matrix Elements ---
// pythia.readString("Charmonium:OJpsi(3S1)[3S1(8)] = 0.0119");

// --- J/psi Decay ---
pythia.readString("443:onIfMatch = 13 -13");  // Force μ+μ-
```

## Physics Models

### Prompt J/ψ (OniaShower)
- Color-singlet and color-octet contributions
- Configurable NRQCD matrix elements
- pp collisions at LHC energies

### D0 Spin Alignment
- Quantization axis: Event plane normal (B-field direction)
- Lorentz boost to D* rest frame
- Prompt/Non-prompt classification

## Multi-Platform Support

GitHub Actions builds Docker images for:
- `linux/amd64` (Intel/AMD x86_64)
- `linux/arm64` (Apple Silicon M1/M2/M3)

## Development

```bash
docker run -it --rm -v $(pwd):/work hep-generation:latest bash
cd build && cmake .. && make -j$(nproc)
```

## Citation

- Pythia 8: [arXiv:2203.11601](https://arxiv.org/abs/2203.11601)
- EvtGen: [Comput.Phys.Commun. 181 (2010) 1721](https://doi.org/10.1016/j.cpc.2010.05.025)

## License

MIT License - see [LICENSE](LICENSE)
