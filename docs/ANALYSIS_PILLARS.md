# Analysis Pillars

This document describes the main physics analyses supported by this framework.

---

## 1. Prompt Charmonium Production (OniaShower)

**Generator:** `gen_prompt_jpsi`

### Physics Overview
Prompt J/ψ and other charmonium states are produced through the color-singlet and color-octet mechanisms in pp collisions. This is modeled using Pythia8's OniaShower implementation based on Non-Relativistic QCD (NRQCD).

### Production Mechanisms
| Process | Type | Description |
|---------|------|-------------|
| g g → J/ψ g | Color-Singlet | Leading order QCD |
| g g → ccbar[3S1(8)] g | Color-Octet | Higher pT contribution |
| q g → ccbar[3S1(8)] q | Color-Octet | Important at high pT |
| q qbar → ccbar[3S1(8)] g | Color-Octet | Subdominant |

### Key Observables
- Differential cross-section dσ/dpT
- Polarization λθ, λφ, λθφ (helicity frame)
- Rapidity distribution dσ/dy

### Usage
```bash
./build/gen_prompt_jpsi 10000 prompt_jpsi.hepmc3
```

---

## 2. B+ Signal Production

**Generator:** `gen_bpkjpsi`

### Physics Overview
Production of B+ mesons through bb̄ production, with exclusive decay chain:
```
B+ → K+ J/ψ(→ μ+μ-)
```

Uses EvtGen for proper angular distributions and branching fractions.

### Key Features
- CP5 tune for underlying event
- EvtGen for B meson decay modeling
- Photos++ for QED FSR from muons
- Proper lifetime and decay vertex

### Key Observables
- ct* (proper decay length)
- B+ pT spectrum
- μ+μ- invariant mass resolution

### Usage
```bash
./build/gen_bpkjpsi 10000 bplus_signal.hepmc3
```

---

## 3. D0 Spin Alignment in Heavy Ions

**Generator:** `gen_d0_study`

### Physics Overview
Studies the spin alignment of D0 mesons (from D* decay) in Pb-Pb collisions. The spin density matrix element ρ00 measures the alignment relative to the event plane normal (magnetic field direction).

### Physics Motivation
- In a polarizing medium (QGP with strong B-field), vector mesons may show alignment
- ρ00 ≠ 1/3 indicates spin polarization
- Connects to vorticity and magnetic field in QGP

### Analysis Method
1. Generate Pb-Pb events with Angantyr
2. Assign random event plane ΨRP
3. Find D* → D0 π decays
4. Boost to D* rest frame
5. Calculate cos θ* relative to boosted RP normal
6. Fit cos θ* distribution to extract ρ00

### Key Observables
- ρ00 vs pT
- ρ00 vs centrality
- Prompt vs non-prompt separation

### Usage
```bash
./build/gen_d0_study 10000 1234 output.txt
python3 scripts/plot_spin_pt.py output.txt
```

---

## 4. Heavy Ion Collisions (Angantyr)

**Generator:** `gen_angantyr`

### Physics Overview
Simulates proton-nucleus and nucleus-nucleus collisions using the Angantyr model in Pythia8. Models sub-collisions, wounded nucleons, and collective effects.

### Supported Systems
| System | Beam IDs | Typical √sNN |
|--------|----------|--------------|
| p-Pb | 2212, 1000822080 | 5.02, 8.16 TeV |
| Pb-Pb | 1000822080, 1000822080 | 2.76, 5.02 TeV |
| p-p | 2212, 2212 | 5.02, 13.6 TeV |

### Key Features
- Geometric modeling of nuclear overlap
- Multiple parton interactions per nucleon pair
- String shoving and rope hadronization (optional)

### Usage
```bash
./build/gen_angantyr 100 pbpb_events.hepmc3
```

---

## 5. HepMC3 Analysis Tools

**Analyzer:** `analyze_spin`

### Purpose
Reads HepMC3 event files and extracts physics observables for the D0 spin alignment study.

### Features
- Recursive ancestry check for prompt/non-prompt
- Lorentz boost to D* rest frame
- Event plane extraction from HepMC3 attributes

### Output Format
```
# type(0=prompt,1=nonprompt) pT cosTheta
0 5.234 0.412
1 12.891 -0.156
```

---

## Analysis Workflow Summary

```
┌─────────────────┐     ┌──────────────┐     ┌────────────────┐
│  gen_prompt_jpsi│     │ gen_bpkjpsi  │     │  gen_d0_study  │
│  (OniaShower)   │     │   (EvtGen)   │     │   (Angantyr)   │
└────────┬────────┘     └──────┬───────┘     └───────┬────────┘
         │                     │                     │
         v                     v                     v
    ┌─────────┐          ┌─────────┐           ┌─────────┐
    │ HepMC3  │          │ HepMC3  │           │ Text    │
    │ Output  │          │ Output  │           │ Output  │
    └────┬────┘          └────┬────┘           └────┬────┘
         │                    │                     │
         v                    v                     v
    ┌─────────────────────────────────────────────────────┐
    │                  Analysis Scripts                    │
    │     (plot_spin.py, plot_spin_pt.py, Rivet, ...)    │
    └─────────────────────────────────────────────────────┘
```
