# Configuration Guide

Detailed breakdown of all configurable parameters in the generator framework.

---

## Table of Contents
1. [Pythia Tunes](#pythia-tunes)
2. [Charmonium Production](#charmonium-production)
3. [Color Reconnection](#color-reconnection)
4. [Parton Showers](#parton-showers)
5. [Heavy Ion Settings](#heavy-ion-settings)
6. [Decay Configuration](#decay-configuration)

---

## Pythia Tunes

### CMS CP5 (Default - Recommended)

The CMS CP5 tune is optimized for LHC Run 2/3 data at mid-rapidity.

```cpp
// PDF
pythia.readString("PDF:pSet = LHAPDF6:NNPDF31_nnlo_as_0118");

// MPI parameters
pythia.readString("MultipartonInteractions:pT0Ref = 1.41");
pythia.readString("MultipartonInteractions:ecmPow = 0.03344");
pythia.readString("MultipartonInteractions:coreFraction = 0.758");
pythia.readString("MultipartonInteractions:coreRadius = 0.63");

// Color Reconnection
pythia.readString("ColourReconnection:reconnect = on");
pythia.readString("ColourReconnection:range = 5.176");

// ISR/FSR
pythia.readString("SpaceShower:alphaSorder = 2");
pythia.readString("SpaceShower:alphaSvalue = 0.118");
pythia.readString("SpaceShower:pT0Ref = 1.56");
pythia.readString("SpaceShower:ecmPow = 0.033");
pythia.readString("TimeShower:alphaSorder = 2");
pythia.readString("TimeShower:alphaSvalue = 0.118");

// Beam Remnants
pythia.readString("BeamRemnants:primordialKThard = 1.88");
pythia.readString("BeamRemnants:halfScaleForKT = 1.033");
```

### Available Pre-built Tunes

| Tune ID | Name | Description | Best For |
|---------|------|-------------|----------|
| 14 | Monash 2013 | Pythia default, general purpose | General studies |
| 21 | A14 | ATLAS tune | ATLAS comparisons |
| - | CP5 | CMS Run 2/3 standard | CMS analyses |
| - | CP1 | CMS older tune | Legacy comparisons |

```cpp
// Using pre-built tunes (instead of manual CP5)
pythia.readString("Tune:pp = 14");  // Monash 2013
pythia.readString("Tune:pp = 21");  // A14
```

### Tune Variations (CP5)

| Variation | Description |
|-----------|-------------|
| CP5 | Central value |
| CP5-CR1 | Color reconnection variation 1 |
| CP5-CR2 | Color reconnection variation 2 |
| CP5-ERD | Early resonance decays |

---

## Charmonium Production

### Master Switch
```cpp
pythia.readString("Charmonium:all = on");  // All charmonium processes
```

### Individual Processes

| Process | String | Description |
|---------|--------|-------------|
| g g → J/ψ g (CS) | `Charmonium:gg2ccbar(3S1)[3S1(1)]g = on` | Color-singlet |
| g g → J/ψ g (CO) | `Charmonium:gg2ccbar(3S1)[3S1(8)]g = on` | Color-octet 3S1 |
| g g → J/ψ g (CO) | `Charmonium:gg2ccbar(3S1)[1S0(8)]g = on` | Color-octet 1S0 |
| g g → J/ψ g (CO) | `Charmonium:gg2ccbar(3S1)[3PJ(8)]g = on` | Color-octet 3PJ |
| q g → J/ψ q | `Charmonium:qg2ccbar(3S1)[3S1(8)]q = on` | Quark-gluon |
| q q̄ → J/ψ g | `Charmonium:qqbar2ccbar(3S1)[3S1(8)]g = on` | Quark-antiquark |

### NRQCD Matrix Elements

Long-distance matrix elements control cross-section normalization:

```cpp
// Color-singlet
pythia.readString("Charmonium:OJpsi(3S1)[3S1(1)] = 1.16");  // GeV^3

// Color-octet (values from NRQCD fits)
pythia.readString("Charmonium:OJpsi(3S1)[3S1(8)] = 0.0119");  // GeV^3
pythia.readString("Charmonium:OJpsi(3S1)[1S0(8)] = 0.01");    // GeV^3
pythia.readString("Charmonium:OJpsi(3S1)[3P0(8)] = 0.01");    // GeV^3/m_c^2
```

### Other Charmonium States

```cpp
pythia.readString("Charmonium:gg2ccbar(3PJ)[3PJ(1)]g = on");  // χ_c0,1,2
pythia.readString("Charmonium:gg2ccbar(3S1)[3D1(1)]g = on");  // ψ(2S)
```

---

## Color Reconnection

### Mode Selection

```cpp
pythia.readString("ColourReconnection:reconnect = on");
pythia.readString("ColourReconnection:mode = X");
```

| Mode | Name | Description |
|------|------|-------------|
| 0 | MPI-based | Classic model, used by CP5 |
| 1 | QCD-based | Monash default, more physical |
| 2 | Gluon-move | Allows gluon exchange between strings |
| 3 | SK I | Rope-like QCD-inspired |
| 4 | SK II | Extended rope model |

### Mode-Specific Parameters

**Mode 0 (MPI-based):**
```cpp
pythia.readString("ColourReconnection:range = 5.176");  // CP5 value
```

**Mode 1 (QCD-based):**
```cpp
pythia.readString("ColourReconnection:range = 1.8");
pythia.readString("ColourReconnection:junctionCorrection = 1.2");
```

**Mode 2 (Gluon-move):**
```cpp
pythia.readString("ColourReconnection:m0 = 0.3");        // Effective gluon mass
pythia.readString("ColourReconnection:fracGluon = 1.0"); // Probability to move gluon
```

**Mode 3/4 (QCD-inspired):**
```cpp
pythia.readString("ColourReconnection:m2Lambda = 1.0");
pythia.readString("ColourReconnection:nColours = 9");
```

---

## Parton Showers

### Global Switches
```cpp
pythia.readString("PartonLevel:MPI = on");   // Multi-parton interactions
pythia.readString("PartonLevel:ISR = on");   // Initial state radiation
pythia.readString("PartonLevel:FSR = on");   // Final state radiation
```

### ISR Parameters
```cpp
pythia.readString("SpaceShower:alphaSorder = 2");     // NLO running
pythia.readString("SpaceShower:alphaSvalue = 0.118"); // α_s(M_Z)
pythia.readString("SpaceShower:pT0Ref = 1.56");       // IR regularization
pythia.readString("SpaceShower:ecmPow = 0.033");     // Energy dependence
pythia.readString("SpaceShower:pTmaxFudge = 0.91");   // Max pT fudge factor
```

### FSR Parameters
```cpp
pythia.readString("TimeShower:alphaSorder = 2");      // NLO running
pythia.readString("TimeShower:alphaSvalue = 0.118");  // α_s(M_Z)
pythia.readString("TimeShower:pTmin = 0.5");          // Cutoff scale
```

---

## Heavy Ion Settings

### Angantyr Mode
```cpp
pythia.readString("HeavyIon:mode = 1");  // Enable Angantyr
```

### Beam Configuration

| System | idA | idB |
|--------|-----|-----|
| p-p | 2212 | 2212 |
| p-Pb | 2212 | 1000822080 |
| Pb-Pb | 1000822080 | 1000822080 |

```cpp
pythia.readString("Beams:idA = 1000822080");  // Pb-208
pythia.readString("Beams:idB = 1000822080");  // Pb-208
pythia.readString("Beams:eCM = 5020.");       // √sNN in GeV
```

### Nuclear PDF (Optional)
```cpp
pythia.readString("PDF:useHardNPDFA = on");
pythia.readString("PDF:useHardNPDFB = on");
pythia.readString("PDF:nPDFSetA = 2");  // EPPS16
pythia.readString("PDF:nPDFSetB = 2");
```

---

## Decay Configuration

### Forcing Specific Decays (Pythia)
```cpp
// Turn off all J/psi decays, enable only μμ
pythia.readString("443:onMode = off");
pythia.readString("443:onIfMatch = 13 -13");
```

### EvtGen Integration
```cpp
// Initialize with external generators
EvtExternalGenList genList;
auto evtgen = std::make_shared<EvtGenDecays>(
    &pythia, decFile, pdlFile, &genList, nullptr, 
    1, false, false, true, true
);
evtgen->readDecayFile("user_decay.dec");
```

### User Decay Files

Located in `decays/`:
- `BpKJpsi.dec` - B+ → K+ J/ψ(μμ)
- `D0SpinAlignment.dec` - D* → D0 π (VSS model)

---

## Phase Space Cuts

```cpp
pythia.readString("PhaseSpace:pTHatMin = 5.0");   // Min pT of hard process
pythia.readString("PhaseSpace:pTHatMax = 100.0"); // Max pT
pythia.readString("PhaseSpace:mHatMin = 2.5");    // Min invariant mass
pythia.readString("PhaseSpace:mHatMax = 100.0");  // Max invariant mass
```

---

## Output Control

```cpp
pythia.readString("Next:numberShowInfo = 0");     // Suppress info
pythia.readString("Next:numberShowProcess = 0"); // Suppress process
pythia.readString("Next:numberShowEvent = 0");   // Suppress event listing
pythia.readString("Next:numberCount = 1000");    // Progress every N events
```
