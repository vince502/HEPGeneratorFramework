// =============================================================================
// gen_bpkjpsi.cc
//
// B+ -> K+ J/psi (J/psi -> mu+ mu-) generator
// Using PYTHIA8 (CMS CP5 tune) + EvtGen 2.2
//
// Usage: ./gen_bpkjpsi [nEvents] [outputFile.hepmc3]
// =============================================================================

#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/EvtGen.h"
#include "Pythia8Plugins/HepMC3.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/WriterAscii.h"

#include "EvtGenExternal/EvtExternalGenList.hh"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

using namespace Pythia8;

int main(int argc, char *argv[]) {

  // Parse command line arguments
  int nEvents = 10000;
  std::string outputFile = "bpkjpsi.hepmc3";

  if (argc > 1)
    nEvents = std::atoi(argv[1]);
  if (argc > 2)
    outputFile = argv[2];

  std::cout << "========================================\n";
  std::cout << "B+ -> K+ J/psi (mu+mu-) Generator\n";
  std::cout << "========================================\n";
  std::cout << "Events to generate: " << nEvents << "\n";
  std::cout << "Output file: " << outputFile << "\n";
  std::cout << "========================================\n\n";

  // Initialize Pythia
  Pythia pythia;

  // =========================================================================
  // Beam settings: pp at 13.6 TeV (Run 3)
  // =========================================================================
  pythia.readString("Beams:idA = 2212");
  pythia.readString("Beams:idB = 2212");
  pythia.readString("Beams:eCM = 13600.");

  // =========================================================================
  // CMS CP5 Tune (Monash base with CMS adjustments)
  // Reference: CMS-PAS-GEN-17-001
  // =========================================================================
  pythia.readString("Tune:pp = 14"); // Monash 2013 as base

  // CP5 specific parameters
  pythia.readString("Tune:ee = 7");
  pythia.readString("MultipartonInteractions:pT0Ref = 1.41");
  pythia.readString("MultipartonInteractions:ecmPow = 0.03344");
  pythia.readString("MultipartonInteractions:coreFraction = 0.63");
  pythia.readString("MultipartonInteractions:coreRadius = 0.4");
  pythia.readString("ColourReconnection:range = 5.176");
  pythia.readString("SigmaTotal:zeroAXB = off");
  pythia.readString("SpaceShower:alphaSorder = 2");
  pythia.readString("SpaceShower:alphaSvalue = 0.118");
  pythia.readString("SpaceShower:rapidityOrder = on");
  pythia.readString("TimeShower:alphaSorder = 2");
  pythia.readString("TimeShower:alphaSvalue = 0.118");

  // =========================================================================
  // Hard process: b-quark production
  // =========================================================================
  pythia.readString("HardQCD:hardbbbar = on"); // bb-bar production

  // Phase space cuts to enhance B meson production efficiency
  pythia.readString("PhaseSpace:pTHatMin = 5.0"); // Minimum pT for hard process

  // =========================================================================
  // Random seed
  // =========================================================================
  pythia.readString("Random:setSeed = on");
  pythia.readString("Random:seed = 0"); // 0 = use system time

  // =========================================================================
  // Suppress unnecessary output
  // =========================================================================
  pythia.readString("Init:showChangedSettings = on");
  pythia.readString("Init:showChangedParticleData = off");
  pythia.readString("Next:numberShowEvent = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowInfo = 0");

  // =========================================================================
  // Turn off PYTHIA's internal B decays - EvtGen will handle them
  // =========================================================================
  // B+ and B- should not decay in PYTHIA
  pythia.readString("521:mayDecay = off");  // B+
  pythia.readString("-521:mayDecay = off"); // B-
  pythia.readString("511:mayDecay = off");  // B0
  pythia.readString("-511:mayDecay = off"); // B0bar
  pythia.readString("531:mayDecay = off");  // Bs
  pythia.readString("-531:mayDecay = off"); // Bsbar
  pythia.readString("541:mayDecay = off");  // Bc+
  pythia.readString("-541:mayDecay = off"); // Bc-

  // =========================================================================
  // Initialize EvtGen
  // =========================================================================
  // Paths to EvtGen data files (installed with EvtGen)
  std::string evtGenDecFile = "/opt/hep/share/EvtGen/DECAY.DEC";
  std::string evtGenPdtFile = "/opt/hep/share/EvtGen/evt.pdl";

  // User decay file for B+ -> K+ J/psi (mu+mu-)
  std::string userDecFile = "run/BpKJpsi.dec";

  // Create EvtGen object with external generator list (for
  // Pythia/Photos/Tauola)
  EvtExternalGenList genList;

  // Arguments: pythia, decayFile, particleDataFile, extPtr, fsrPtr, mixing,
  // xml, limit, extUse, fsrUse
  auto evtgen = std::make_shared<EvtGenDecays>(&pythia, evtGenDecFile,
                                               evtGenPdtFile, &genList, nullptr,
                                               1, false, false, true, true);

  // Read user decay file
  evtgen->readDecayFile(userDecFile);

  std::cout << "EvtGen initialized with:\n";
  std::cout << "  Decay file: " << evtGenDecFile << "\n";
  std::cout << "  PDT file: " << evtGenPdtFile << "\n";
  std::cout << "  User decay: " << userDecFile << "\n\n";

  // =========================================================================
  // Initialize Pythia
  // =========================================================================
  if (!pythia.init()) {
    std::cerr << "Pythia initialization failed!\n";
    return 1;
  }

  // =========================================================================
  // Set up HepMC3 output
  // =========================================================================
  HepMC3::WriterAscii hepmcWriter(outputFile);
  HepMC3::Pythia8ToHepMC3 toHepMC;

  // =========================================================================
  // Event loop
  // =========================================================================
  int nBplusFound = 0;
  int nBplusKJpsi = 0;
  int nEventsGenerated = 0;
  int nEventsTotal = 0;

  std::cout << "Starting event generation...\n";

  while (nBplusKJpsi < nEvents) {
    nEventsTotal++;

    // Generate event
    if (!pythia.next())
      continue;
    nEventsGenerated++;

    // Check for B+ in the event
    bool hasBplus = false;
    for (int i = 0; i < pythia.event.size(); ++i) {
      int absId = std::abs(pythia.event[i].id());
      if (absId == 521) { // B+ or B-
        hasBplus = true;
        nBplusFound++;
        break;
      }
    }

    if (!hasBplus)
      continue;

    // Apply EvtGen decays to all B hadrons
    evtgen->decay();

    // Check if we have B+ -> K+ J/psi (mu+mu-)
    bool hasSignal = false;
    for (int i = 0; i < pythia.event.size(); ++i) {
      const Particle &p = pythia.event[i];

      // Look for J/psi from B+
      if (std::abs(p.id()) == 443) { // J/psi
        // Check if J/psi comes from B+
        int mother1 = p.mother1();
        if (mother1 > 0 && std::abs(pythia.event[mother1].id()) == 521) {
          // Check if J/psi decays to mu+mu-
          int d1 = p.daughter1();
          int d2 = p.daughter2();
          if (d1 > 0 && d2 > 0) {
            int id1 = std::abs(pythia.event[d1].id());
            int id2 = std::abs(pythia.event[d2].id());
            if ((id1 == 13 && id2 == 13)) { // mu+ mu-
              hasSignal = true;
              break;
            }
          }
        }
      }
    }

    if (!hasSignal)
      continue;

    nBplusKJpsi++;

    // Convert to HepMC3 and write
    HepMC3::GenEvent hepmcEvent;
    toHepMC.fill_next_event(pythia, &hepmcEvent);
    hepmcWriter.write_event(hepmcEvent);

    // Progress report
    if (nBplusKJpsi % 1000 == 0) {
      std::cout << "  Generated " << nBplusKJpsi << "/" << nEvents
                << " signal events (efficiency: "
                << 100.0 * nBplusKJpsi / nEventsTotal << "%)\n";
    }
  }

  // =========================================================================
  // Final statistics
  // =========================================================================
  std::cout << "\n========================================\n";
  std::cout << "Generation complete!\n";
  std::cout << "========================================\n";
  std::cout << "Total events tried: " << nEventsTotal << "\n";
  std::cout << "Events with B+/-: " << nBplusFound << "\n";
  std::cout << "Signal events (B+ -> K+ J/psi -> mu+mu-): " << nBplusKJpsi
            << "\n";
  std::cout << "Overall efficiency: " << 100.0 * nBplusKJpsi / nEventsTotal
            << "%\n";
  std::cout << "Output written to: " << outputFile << "\n";
  std::cout << "========================================\n";

  pythia.stat();

  return 0;
}
