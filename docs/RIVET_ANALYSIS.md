# Customizing Rivet Analyses

This guide explains how to modify the analysis logic, add histograms, and update selection cuts in the Rivet-based pipeline.

## 1. Analysis Architecture
The framework uses a specialized Docker container (`cmsana-rivet`) that acts as an analysis service. It supports:
- **On-the-fly Compilation**: Your C++ analysis is compiled at runtime inside the container.
- **Data Streaming**: Events are read from a FIFO pipe, eliminating the need for large disk storage.

The primary analysis file for the $J/\psi$ study is:
`rivet/JpsiJet_RivetAnalyzer.cc`

---

## 2. Modifying the Analysis Code

### Registering Histograms
In the `init()` function of the C++ file, book your histograms. Rivet uses the `_h_name` naming convention for internal pointers.
```cpp
void init() {
    // Standard histogram booking
    // Arguments: pointer, name, num_bins, min, max
    book(_h_jpsi_pt, "Jpsi_pT", 50, 0, 50);
    book(_h_jet_pt, "Jet_pT", 50, 0, 100);
    book(_h_z, "Fragment_z", 20, 0, 1.0);
}
```

### Updating Selection Cuts
The `analyze(const Event& event)` function is where you define your physics logic.

**Example: Modifying pT or Acceptance cuts**
```cpp
// Inside analyze()
// Selection for muons (J/psi final state)
const Particles muons = apply<DileptonFinder>(event, "JpsiFinder").particles();

// Update J/psi selection:
for (const Particle& p : muons) {
    if (p.pt() < 6.5*GeV || abs(p.rap()) > 2.4) continue;
    // ... logic continues ...
}
```

**Example: Jet Finder parameters**
```cpp
// Inside init()
// Change Anti-kt radius or jet pT threshold
FastJets fj(FinalState(), FastJets::ANTIKT, 0.4);
declare(fj, "Jets");
```

---

## 3. Real-time Compilation
One of the key features of this framework is that **you do not need to manually compile the Rivet plugin**.

When you run `bash run_jpsijet_pipeline.sh`, the following sequence occurs:
1. The script mounts the `rivet/` directory into the container.
2. The container entrypoint executes `rivet-build RivetAnalysis.so JpsiJet_RivetAnalyzer.cc`.
3. Rivet loads the resulting `.so` file and begins processing events from the FIFO.

If you make a change to the `.cc` file, simply restart the pipeline script, and your changes will be applied instantly.

---

## 4. Inspection and Post-Processing

### Listing Histograms
To see what histograms were generated in your `.yoda` file:
```bash
docker run --rm -v $(pwd):/work cmsana-rivet yodals /work/results_prompt.yoda
```

### Converting to Plots
Generating high-quality plots and an HTML gallery:
```bash
docker run --rm -v $(pwd):/work cmsana-rivet rivet-mkhtml /work/results_prompt.yoda
```
This creates a `rivet-plots/` directory with `.pdf`, `.png`, and a web interface.
