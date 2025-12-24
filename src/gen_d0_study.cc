#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/EvtGen.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

using namespace Pythia8;

// Boost 4-vector to rest frame of parent
Vec4 boostToRest(const Vec4 &p, const Vec4 &pParent) {
  double m = pParent.mCalc();
  if (m <= 0)
    return p;
  double e = pParent.e();
  double bx = pParent.px() / e;
  double by = pParent.py() / e;
  double bz = pParent.pz() / e;
  double b2 = bx * bx + by * by + bz * bz;
  double gamma = e / m;
  double p_dot_b = p.px() * bx + p.py() * by + p.pz() * bz;
  double gamma2 = (b2 > 0) ? (gamma - 1.0) / b2 : 0.0;
  return Vec4(p.px() + gamma2 * p_dot_b * bx - gamma * bx * p.e(),
              p.py() + gamma2 * p_dot_b * by - gamma * by * p.e(),
              p.pz() + gamma2 * p_dot_b * bz - gamma * bz * p.e(),
              gamma * (p.e() - p_dot_b));
}

// Check if particle originates from a b-hadron
bool isNonPrompt(int idx, const Event &event) {
  int mother = event[idx].mother1();
  while (mother > 0) {
    int aid = std::abs(event[mother].id());
    if ((aid >= 500 && aid < 600) || (aid >= 5000 && aid < 6000))
      return true;
    mother = event[mother].mother1();
  }
  return false;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <nEvents> <seed> <outputFile.txt>"
              << std::endl;
    return 1;
  }
  int nEvents = std::atoi(argv[1]);
  int seed = std::atoi(argv[2]);
  std::string outFile = argv[3];

  Pythia pythia;

  // Pb-Pb @ 5.02 TeV with Angantyr
  pythia.readString("HeavyIon:mode = 1");
  pythia.readString("Beams:idA = 1000822080");
  pythia.readString("Beams:idB = 1000822080");
  pythia.readString("Beams:eCM = 5020.");

  // Enable charm and beauty
  pythia.readString("HardQCD:hardccbar = on");
  pythia.readString("HardQCD:hardbbbar = on");
  pythia.readString("PhaseSpace:pTHatMin = 3.0");

  // Reduce output verbosity
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 0");

  // Set random seed
  pythia.readString("Random:setSeed = on");
  pythia.readString("Random:seed = " + std::to_string(seed));

  if (!pythia.init())
    return 1;

  // EvtGen Setup
  std::string evtGenDec = "/opt/hep/share/EvtGen/DECAY.DEC";
  std::string evtGenPdt = "/opt/hep/share/EvtGen/evt.pdl";
  std::string userDec = "../decays/D0SpinAlignment.dec";
  EvtExternalGenList genList;
  auto evtgen =
      std::make_shared<EvtGenDecays>(&pythia, evtGenDec, evtGenPdt, &genList,
                                     nullptr, 1, false, false, true, true);
  evtgen->readDecayFile(userDec);

  // Output file for measurements only
  std::ofstream fout(outFile);

  // Random generator for event plane
  std::mt19937 rand_gen(seed);
  std::uniform_real_distribution<> runif(0, M_PI);

  int countPrompt = 0, countNonPrompt = 0;

  std::cout << "Starting generation (Seed: " << seed << ", Events: " << nEvents
            << ")..." << std::endl;

  for (int iEvent = 0; iEvent < nEvents; ++iEvent) {
    if (!pythia.next())
      continue;

    // Random event plane angle
    double psi_RP = runif(rand_gen);

    // Perform EvtGen decays
    evtgen->decay();

    // RP normal in lab frame (perpendicular to beam, B-field direction)
    Vec4 nLab(-std::sin(psi_RP), std::cos(psi_RP), 0.0, 0.0);

    // Loop over particles to find D*
    for (int i = 0; i < pythia.event.size(); ++i) {
      if (pythia.event[i].idAbs() != 413)
        continue; // D*+/-

      // Find D0 daughter
      int d0_idx = -1;
      for (int d = pythia.event[i].daughter1();
           d <= pythia.event[i].daughter2(); ++d) {
        if (d > 0 && pythia.event[d].idAbs() == 421) {
          d0_idx = d;
          break;
        }
      }
      if (d0_idx < 0)
        continue;

      Vec4 pStar = pythia.event[i].p();
      Vec4 pD0 = pythia.event[d0_idx].p();
      double pt = pStar.pT();

      // Boost to D* rest frame
      Vec4 pD0_rf = boostToRest(pD0, pStar);
      Vec4 n_rf = boostToRest(nLab, pStar);

      double pD0Mag = pD0_rf.pAbs();
      double nMag = n_rf.pAbs();

      if (pD0Mag > 1e-6 && nMag > 1e-6) {
        double cosTheta = (pD0_rf.px() * n_rf.px() + pD0_rf.py() * n_rf.py() +
                           pD0_rf.pz() * n_rf.pz()) /
                          (pD0Mag * nMag);
        bool nonPrompt = isNonPrompt(i, pythia.event);

        // Output: type(0=prompt,1=nonprompt) pt cosTheta
        fout << (nonPrompt ? 1 : 0) << " " << pt << " " << cosTheta << "\n";

        if (nonPrompt)
          countNonPrompt++;
        else
          countPrompt++;
      }
    }

    if (iEvent % 500 == 0) {
      std::cout << "  Event " << iEvent << "/" << nEvents
                << " (Prompt: " << countPrompt
                << ", NonPrompt: " << countNonPrompt << ")" << std::endl;
    }
  }

  pythia.stat();
  std::cout << "\nGeneration complete!" << std::endl;
  std::cout << "  Prompt D*: " << countPrompt << std::endl;
  std::cout << "  Non-prompt D*: " << countNonPrompt << std::endl;
  std::cout << "  Output: " << outFile << std::endl;

  return 0;
}
