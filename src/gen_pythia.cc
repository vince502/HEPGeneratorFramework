#include "Pythia8/Pythia.h"
#include <iostream>

int main() {
  Pythia8::Pythia p;

  p.readString("Beams:idA = 2212");
  p.readString("Beams:idB = 2212");
  p.readString("Beams:eCM = 13600.");
  p.readString("HardQCD:all = on");
  p.readString("Random:setSeed = on");
  p.readString("Random:seed = 12345");

  if (!p.init()) {
    std::cerr << "Pythia init failed\n";
    return 1;
  }

  const int nEvent = 100;
  for (int i = 0; i < nEvent; ++i) {
    if (!p.next()) continue;
    // Placeholder: print number of final-state particles
    int nFinal = 0;
    for (int j = 0; j < p.event.size(); ++j)
      if (p.event[j].isFinal()) ++nFinal;
    std::cout << "event " << i << " nFinal=" << nFinal << "\n";
  }

  p.stat();
  return 0;
}
