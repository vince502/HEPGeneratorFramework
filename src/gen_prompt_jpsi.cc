// =============================================================================
// gen_prompt_jpsi.cc
// -----------------------------------------------------------------------------
// Prompt J/psi production using Pythia8 OniaShower mechanism.
// Includes color-octet contributions for realistic charmonium production.
//
// Tweakable parameters are clearly marked in the CONFIGURATION section below.
// =============================================================================

#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC3.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace Pythia8;

int main(int argc, char *argv[]) {
  // =========================================================================
  // COMMAND LINE ARGUMENTS
  // =========================================================================
  int nEvents = (argc > 1) ? std::atoi(argv[1]) : 10000;
  std::string outFile = (argc > 2) ? argv[2] : "prompt_jpsi.hepmc3";

  Pythia pythia;

  // =========================================================================
  // CONFIGURATION - TWEAK THESE PARAMETERS
  // =========================================================================

  // --- Beam Settings ---
  double sqrtS = 13600.0; // Center-of-mass energy [GeV]
  pythia.readString("Beams:eCM = " + std::to_string(sqrtS));

  // --- Charmonium Production (OniaShower) ---
  // Main switch for charmonium production
  pythia.readString("Charmonium:all = on");

  // Or select specific channels:
  // pythia.readString("Charmonium:gg2ccbar(3S1)[3S1(1)]g = on");  //
  // Color-singlet g g -> J/psi g
  // pythia.readString("Charmonium:gg2ccbar(3S1)[3S1(8)]g = on");  //
  // Color-octet g g -> J/psi g
  // pythia.readString("Charmonium:qg2ccbar(3S1)[3S1(8)]q = on");  //
  // Color-octet q g -> J/psi q
  // pythia.readString("Charmonium:qqbar2ccbar(3S1)[3S1(8)]g = on"); //
  // Color-octet q qbar -> J/psi g

  // --- Phase Space Cuts ---
  double pTHatMin = 0.0;  // Minimum pT of hard process [GeV]
  double pTHatMax = -1.0; // Maximum pT (-1 = no limit) [GeV]
  pythia.readString("PhaseSpace:pTHatMin = " + std::to_string(pTHatMin));
  if (pTHatMax > 0)
    pythia.readString("PhaseSpace:pTHatMax = " + std::to_string(pTHatMax));

  // --- Non-perturbative QCD Parameters (Long-Distance Matrix Elements) ---
  // These affect the normalization of color-octet contributions
  // Default values are from NRQCD fits
  // pythia.readString("Charmonium:OJpsi(3S1)[3S1(1)] = 1.16");  //
  // <O^{J/psi}(^3S_1^{(1)})> pythia.readString("Charmonium:OJpsi(3S1)[3S1(8)] =
  // 0.0119"); // <O^{J/psi}(^3S_1^{(8)})>
  // pythia.readString("Charmonium:OJpsi(3S1)[1S0(8)] = 0.01");   //
  // <O^{J/psi}(^1S_0^{(8)})> pythia.readString("Charmonium:OJpsi(3S1)[3P0(8)] =
  // 0.01");   // <O^{J/psi}(^3P_0^{(8)})>/m_c^2

  // --- Rapidity Cuts (optional) ---
  // pythia.readString("PhaseSpace:mHatMin = 2.5");  // Minimum invariant mass
  // pythia.readString("PhaseSpace:mHatMax = 4.0");  // Maximum invariant mass

  // --- Tune Selection ---
  pythia.readString("Tune:pp = 14"); // Monash 2013 tune (default)
  // pythia.readString("Tune:pp = 21"); // A14 tune

  // --- Parton Shower Settings ---
  pythia.readString("PartonLevel:MPI = on"); // Multi-parton interactions
  pythia.readString("PartonLevel:ISR = on"); // Initial state radiation
  pythia.readString("PartonLevel:FSR = on"); // Final state radiation

  // --- Color Reconnection ---
  // Mode selection (ColourReconnection:mode):
  //   0 = MPI-based (old default)
  //   1 = New more QCD-based scheme (default in Monash)
  //   2 = Gluon-move model
  //   3 = QCD-inspired (SK I)
  //   4 = QCD-inspired (SK II)
  pythia.readString("ColourReconnection:reconnect = on");
  pythia.readString(
      "ColourReconnection:mode = 1"); // QCD-based (Monash default)

  // Fine-tuning parameters for mode 1:
  // pythia.readString("ColourReconnection:range = 1.8");  // Range of CR
  // pythia.readString("ColourReconnection:junctionCorrection = 1.2");

  // For gluon-move model (mode 2):
  // pythia.readString("ColourReconnection:m0 = 0.3");     // Effective gluon
  // mass pythia.readString("ColourReconnection:fracGluon = 1.0");

  // For QCD-inspired models (mode 3,4):
  // pythia.readString("ColourReconnection:m2Lambda = 1.0");
  // pythia.readString("ColourReconnection:nColours = 9");

  // --- J/psi Decay ---
  // Force J/psi -> mu+ mu- for easier analysis
  pythia.readString("443:onMode = off");       // Turn off all J/psi decays
  pythia.readString("443:onIfMatch = 13 -13"); // Enable only mu+ mu-

  // --- Output Control ---
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 0");
  pythia.readString("Next:numberCount = 1000");

  // =========================================================================
  // INITIALIZATION
  // =========================================================================

  if (!pythia.init()) {
    std::cerr << "Pythia initialization failed!" << std::endl;
    return 1;
  }

  // HepMC3 output
  HepMC3::Pythia8ToHepMC3 toHepMC;
  HepMC3::WriterAscii writer(outFile);

  std::cout << "\n=== Prompt J/psi Generation ===" << std::endl;
  std::cout << "sqrt(s) = " << sqrtS << " GeV" << std::endl;
  std::cout << "Events: " << nEvents << std::endl;
  std::cout << "Output: " << outFile << std::endl;
  std::cout << "================================\n" << std::endl;

  // =========================================================================
  // EVENT GENERATION
  // =========================================================================

  int nJpsi = 0;

  for (int iEvent = 0; iEvent < nEvents; ++iEvent) {
    if (!pythia.next())
      continue;

    // Count J/psi in event
    for (int i = 0; i < pythia.event.size(); ++i) {
      if (pythia.event[i].id() == 443)
        nJpsi++;
    }

    // Write to HepMC3
    HepMC3::GenEvent hepmcEvent;
    toHepMC.fill_next_event(pythia, &hepmcEvent);
    writer.write_event(hepmcEvent);

    if (iEvent % 1000 == 0) {
      std::cout << "Event " << iEvent << " / " << nEvents
                << " (J/psi count: " << nJpsi << ")" << std::endl;
    }
  }

  // =========================================================================
  // STATISTICS
  // =========================================================================

  pythia.stat();

  std::cout << "\n=== Generation Complete ===" << std::endl;
  std::cout << "Total J/psi produced: " << nJpsi << std::endl;
  std::cout << "Output file: " << outFile << std::endl;

  return 0;
}
