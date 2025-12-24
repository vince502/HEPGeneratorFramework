#include "HepMC3/Attribute.h"
#include "HepMC3/FourVector.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"
#include "HepMC3/ReaderAscii.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace HepMC3;

// Helper to boost a 4-vector to the rest frame of pParent
FourVector boostToRest(const FourVector &p, const FourVector &pParent) {
  double m2 = pParent.e() * pParent.e() - pParent.px() * pParent.px() -
              pParent.py() * pParent.py() - pParent.pz() * pParent.pz();
  double m = (m2 > 0) ? std::sqrt(m2) : 0.0;
  if (m <= 0)
    return p;

  double e = pParent.e();
  double px = pParent.px();
  double py = pParent.py();
  double pz = pParent.pz();

  double bx = px / e;
  double by = py / e;
  double bz = pz / e;
  double b2 = bx * bx + by * by + bz * bz;
  double gamma = e / m;

  double b_dot_p = bx * p.px() + by * p.py() + bz * p.pz();
  double gamma2 = (b2 > 0) ? (gamma - 1.0) / b2 : 0.0;

  double pOutX = p.px() + gamma2 * b_dot_p * bx - gamma * bx * p.e();
  double pOutY = p.py() + gamma2 * b_dot_p * by - gamma * by * p.e();
  double pOutZ = p.pz() + gamma2 * b_dot_p * bz - gamma * bz * p.e();
  double pOutE = gamma * (p.e() - b_dot_p);

  return FourVector(pOutX, pOutY, pOutZ, pOutE);
}

// Check if particle comes from a b-hadron
bool isNonPrompt(const GenParticlePtr &p) {
  if (!p)
    return false;
  auto vtx = p->production_vertex();
  if (!vtx)
    return false;

  for (auto const &parent : vtx->particles_in()) {
    int aid = std::abs(parent->pid());
    // b-hadrons: 500-599 (mesons), 5000-5999 (baryons)
    if ((aid >= 500 && aid < 600) || (aid >= 5000 && aid < 6000))
      return true;
    if (isNonPrompt(parent))
      return true;
  }
  return false;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.hepmc3> [output.txt]"
              << std::endl;
    return 1;
  }

  std::string inputFile = argv[1];
  std::string outputFile = (argc > 2) ? argv[2] : "cos_theta_pt_bins.txt";

  ReaderAscii reader(inputFile);
  std::ofstream fout(outputFile);

  int countPrompt = 0;
  int countNonPrompt = 0;

  while (!reader.failed()) {
    auto evt = std::make_shared<GenEvent>();
    if (!reader.read_event(*evt))
      break;

    double psi_RP = 0;
    auto attr = evt->attribute<DoubleAttribute>("psi_RP");
    if (attr)
      psi_RP = attr->value();

    FourVector nLab(-std::sin(psi_RP), std::cos(psi_RP), 0.0, 0.0);

    for (auto const &p : evt->particles()) {
      if (std::abs(p->pid()) == 413) { // D*+
        auto endVtx = p->end_vertex();
        if (!endVtx)
          continue;

        GenParticlePtr d0 = nullptr;
        for (auto const &d : endVtx->particles_out()) {
          if (std::abs(d->pid()) == 421)
            d0 = d;
        }

        if (d0) {
          FourVector pStar = p->momentum();
          double pt =
              std::sqrt(pStar.px() * pStar.px() + pStar.py() * pStar.py());

          FourVector pD0 = d0->momentum();
          FourVector pD0_rf = boostToRest(pD0, pStar);
          FourVector n_rf = boostToRest(nLab, pStar);

          double pD0Mag =
              std::sqrt(pD0_rf.px() * pD0_rf.px() + pD0_rf.py() * pD0_rf.py() +
                        pD0_rf.pz() * pD0_rf.pz());
          double nMag =
              std::sqrt(n_rf.px() * n_rf.px() + n_rf.py() * n_rf.py() +
                        n_rf.pz() * n_rf.pz());

          if (pD0Mag > 1e-6 && nMag > 1e-6) {
            double cosTheta =
                (pD0_rf.px() * n_rf.px() + pD0_rf.py() * n_rf.py() +
                 pD0_rf.pz() * n_rf.pz()) /
                (pD0Mag * nMag);

            bool nonPrompt = isNonPrompt(p);
            // Output: Type (0=prompt, 1=non-prompt) pt cosTheta
            fout << (nonPrompt ? 1 : 0) << " " << pt << " " << cosTheta << "\n";

            if (nonPrompt)
              countNonPrompt++;
            else
              countPrompt++;
          }
        }
      }
    }
  }

  std::cout << "Analysis complete." << std::endl;
  std::cout << "  Prompt D*: " << countPrompt << std::endl;
  std::cout << "  Non-prompt D*: " << countNonPrompt << std::endl;

  return 0;
}
