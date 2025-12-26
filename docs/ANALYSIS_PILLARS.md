# Physics Analysis Pillars

This framework is built around several core physics programs in Heavy Flavor and Heavy Ion collisions. Each analysis uses a specialized C++ generator and an associated analysis workflow.

---

## 1. J/psi in Jets ($z$ Variable Analysis)

**Generators**: `gen_prompt_jpsi`, `gen_bpkjpsi`  
**Tooling**: Rivet 4 + YODA 2

### Physics Overview
This analysis investigates the fragmentation of $J/\psi$ within jets, focusing on the momentum fraction $z = p_{T, J/\psi} / p_{T, \text{jet}}$.
- **Prompt**: Produced via Pythia's OniaShower (NRQCD mechanisms).
- **Non-prompt**: Produced via B-meson decays handled by EvtGen.

### Workflow: The FIFO Pipeline
To avoid storing massive HepMC files on disk, we stream events in real-time:
1. **Generator**: Streams HepMC3 events into a Unix FIFO pipe.
2. **Rivet Service**: Subscribes to the pipe, analyzes events, and populates histograms in memory.
3. **Output**: Only the final `.yoda` histogram file is saved to disk.

### Implementation
- **Selection**: Muons with $|\eta| < 2.4$, $p_T > 6.5$ GeV. Anti-$k_t$ $R=0.4$ jets.
- **Reference**: `rivet/JpsiJet_RivetAnalyzer.cc`

---

## 2. D0 Spin Alignment in Heavy Ions

**Generator**: `gen_d0_study`  
**Tooling**: C++ Analyzer + Python/ROOT

### Physics Overview
Measures the spin density matrix element $\rho_{00}$ of $D^0$ mesons relative to the event plane normal. A value of $\rho_{00} \neq 1/3$ indicates spin alignment, potentially caused by the strong magnetic field or global vorticity in the Quark-Gluon Plasma (QGP).

### Methodology
1. **Collision**: Pb-Pb at $\sqrt{s_{NN}} = 5.02$ TeV via Angantyr.
2. **Reconstruction**: Identify $D^* \to D^0 \pi_{soft}$ decay chains.
3. **Reference Frame**: Perform a Lorentz boost to the $D^*$ rest frame.
4. **Observable**: Extract the decay angle $\theta^*$ relative to the quantization axis.

---

## 3. Heavy Ion Collisions (Angantyr Model)

**Generator**: `gen_angantyr`

### Physics Overview
The master generator for nucleus-nucleus collisions. It models nuclear overlap geometries and handles multiple parton interactions across different nucleon pairs.

### Capability Matrix
- **p-Pb**: Proton-Lead collisions (asymmetric).
- **Pb-Pb**: Symmetric Heavy Ion collisions.
- **nPDFs**: Integrates EPPS16/nCTEQ15 nuclear PDFs via LHAPDF.

---

## 4. Heavy Flavor Decays (EvtGen Integration)

**Generator**: `gen_bpkjpsi`

### Physics Overview
Pythia is used for the hard process and parton shower, while **EvtGen** handles the transition from $b$-quarks to $B$-mesons and their subsequent complex decays. This ensures accurate branching fractions and angular distributions for decay chains like $B^+ \to K^+ J/\psi(\to \mu\mu)$.

### Key Technologies
- **Photos++**: QED final state radiation from muons.
- **Tauola++**: Precision $\tau$-lepton decays.
- **LHA Interface**: Correct translation of color strings to decay-ready hadrons.
