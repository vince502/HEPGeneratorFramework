#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC3.h"
#include <iostream>

using namespace Pythia8;

int main(int argc, char *argv[]) {
  int nEvents = (argc > 1) ? std::atoi(argv[1]) : 10;
  std::string outFile = (argc > 2) ? argv[2] : "angantyr_test.hepmc3";

  Pythia pythia;

  // --- Angantyr Settings for pPb @ 5.02 TeV ---
  pythia.readString("HeavyIon:mode = 1"); // Projectile: Proton
  pythia.readString("Beams:idA = 2212");  // Target: Lead-208 (PDG ID: 100ZAA0)
  pythia.readString("Beams:idB = 1000822080");
  pythia.readString("Beams:eCM = 5020.");
  pythia.readString("Beams:frameType = 1");

  // Enable soft QCD processes (standard for HI)
  pythia.readString("SoftQCD:nonDiffractive = on");

  // Reduce output
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 0");

  if (!pythia.init()) {
    std::cerr << "Pythia failed to initialize Angantyr!" << std::endl;
    return 1;
  }

  // HepMC3 Writer
  HepMC3::Pythia8ToHepMC3 hepmcWriter;
  hepmcWriter.set_print_inconsistency(false);
  HepMC3::WriterAscii asciiWriter(outFile);

  std::cout << "Generating " << nEvents << " pPb events with Angantyr..."
            << std::endl;

  for (int i = 0; i < nEvents; ++i) {
    if (!pythia.next())
      continue;

    HepMC3::GenEvent hepmcevt;
    hepmcWriter.fill_next_event(pythia, &hepmcevt);
    asciiWriter.write_event(hepmcevt);

    if (i % 2 == 0)
      std::cout << "  Event " << i << std::endl;
  }

  pythia.stat();
  std::cout << "Done! Output saved to " << outFile << std::endl;

  return 0;
}
