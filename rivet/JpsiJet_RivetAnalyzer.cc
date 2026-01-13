// -*- C++ -*-
#include "Rivet/Analysis.hh"
// #include "Rivet/Projections/DressedLeptons.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/MissingMomentum.hh"
#include "Rivet/Projections/PromptFinalState.hh"
#include <cmath> // Inclure la bibliothèque cmath pour std::pow()

#include "Rivet/Analysis.hh"
#include "Rivet/Particle.hh"
#include "Rivet/Projections/ChargedFinalState.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/UnstableParticles.hh"
#include "Rivet/Projections/VetoedFinalState.hh"

#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/contrib/LundGenerator.hh"

#define JpsiDEBUG

namespace Rivet {

/// @brief Add a short analysis description here
class JpsiJet_RivetAnalyzer : public Analysis {
public:
  // Constructor
  RIVET_DEFAULT_ANALYSIS_CTOR(JpsiJet_RivetAnalyzer);

  void init() {

    // Projections
    FinalState fs(Cuts::abseta < 2.4);
    FastJets jet4(fs, JetAlg::ANTIKT, 0.4);
    declare(jet4, "jet4");

    // Particles for the jets
    VetoedFinalState jet_input(fs);
    const UnstableParticles jpsi_fs = UnstableParticles(
        Cuts::abspid == PID::JPSI && Cuts::rap < 2.4 && Cuts::rap > -2.4 &&
        // FinalState jpsi_fs =
        //     FinalState(Cuts::abspid == PID::JPSI && Cuts::abseta < 2.4 &&
        Cuts::pT > 6.5 * GeV &&
        Cuts::pT < 30 * GeV); // Promptfinalstate for jpsi
    declare(jpsi_fs, "Jpsi");
    jet_input.vetoNeutrinos(); // veto neutrinos
    declare(jet_input, "JET_INPUT");

    // I kept only the histogram related to the observable z in the first
    // instance
    book(_njets, "_njets");

    // book(_hZ, "zJpsi", {0.16, 0.22, 0.376, 0.532,  0.688, 0.844, 1.0});
    book(_hnJpsi, "nJpsi", {0, 1, 2, 3, 4});
    book(_hZ, "zJpsi",
         {0.16, 0.22, 0.298, 0.376, 0.454, 0.532, 0.610, 0.688, 0.766, 0.844,
          0.922, 1.0});
    book(_hZqqcc, "zJpsiFromqqbar",
         {0.16, 0.22, 0.298, 0.376, 0.454, 0.532, 0.610, 0.688, 0.766, 0.844,
          0.922, 1.0});
    book(_hZgcc, "zJpsiFromgtocc",
         {0.16, 0.22, 0.298, 0.376, 0.454, 0.532, 0.610, 0.688, 0.766, 0.844,
          0.922, 1.0});

    book(_hJpsipT, "JpsipT", {5, 6, 8, 10, 15, 20, 25, 30});
    book(_hJetpT, "JetpT", {5, 10, 15, 20, 30, 40, 50, 70, 100});

    // New histograms: Fixed J/psi pT (6.5-10 GeV), freed jet pT (>=10 GeV)
    book(_hZ_lowPtJpsi, "zJpsi_lowPtJpsi",
         {0.16, 0.22, 0.298, 0.376, 0.454, 0.532, 0.610, 0.688, 0.766, 0.844,
          0.922, 1.0});
    book(_hZqqcc_lowPtJpsi, "zJpsiFromqqbar_lowPtJpsi",
         {0.16, 0.22, 0.298, 0.376, 0.454, 0.532, 0.610, 0.688, 0.766, 0.844,
          0.922, 1.0});
    book(_hZgcc_lowPtJpsi, "zJpsiFromgtocc_lowPtJpsi",
         {0.16, 0.22, 0.298, 0.376, 0.454, 0.532, 0.610, 0.688, 0.766, 0.844,
          0.922, 1.0});
    book(_hJetpT_lowPtJpsi, "JetpT_lowPtJpsi",
         {10, 20, 30, 40, 50, 70, 100, 150, 200});
  }

  void analyze(const Event &event) {
    // const UnstableParticles& unstable = apply<UnstableParticles>(event,
    // "UFS");

    const VetoedFinalState &fs = apply<VetoedFinalState>(event, "JET_INPUT");
    const Particles &fsParticles = fs.particles();
    //    const Particles &jpsi_particles =
    //        apply<FinalState>(event, "Jpsi").particles();

    const Particles &jpsi_particles =
        apply<UnstableParticles>(event, "Jpsi").particles();
    if (jpsi_particles.size() == 0)
      vetoEvent;
#ifdef JpsiDEBUG
    printf("> 1 Jpsi\n");
    fflush(stdout);
#endif
    bool isNonPrompt = jpsi_particles[0].fromBottom();
    if (isNonPrompt)
      vetoEvent;
#ifdef JpsiDEBUG
    printf("Selected Prompt Jpsi\n");
    fflush(stdout);
#endif

    //    Jets jets = apply<FastJets>(event, "jet4").jetsByPt(Cuts::pT > 30 *
    //    GeV).jetsByPt(Cuts::pT < 40 * GeV);

#ifdef JpsiDEBUG
    printf("Pass Onia Veto!!\n");
    fflush(stdout);
#endif
    _hnJpsi->fill(std::min(jpsi_particles.size(), (long unsigned int)5));
    double pTj = jpsi_particles[0].pt();
    _hJpsipT->fill(pTj);

    vector<PseudoJet> toCheck;
    for (const Particle &daughter : jpsi_particles) {
#ifdef JpsiDEBUG
      printf("J/psi has %lu daughters: ", daughter.children().size());
      fflush(stdout);
#endif
      for (const Particle &desc : daughter.children()) {
        toCheck.push_back(desc.pseudojet());
#ifdef JpsiDEBUG
        printf("pdg=%d (eta=%.3f, phi=%.3f) ", desc.pid(), desc.eta(),
               desc.phi());
#endif
      }
#ifdef JpsiDEBUG
      printf("\n");
      fflush(stdout);
#endif
    }

    vector<PseudoJet> particles;
    particles.reserve(fsParticles.size());
    int nTaggedMuons = 0;
    for (uint iFS = 0; iFS < fsParticles.size(); iFS++) {
      PseudoJet p = fsParticles[iFS].pseudojet();
      for (auto dau : toCheck) {
        // FIX: fabs should wrap the difference, not the comparison
        if (fabs(fsParticles[iFS].eta() - dau.eta()) < 1e-4 &&
            fabs(fsParticles[iFS].phi() - dau.phi()) < 1e-4) {
          p.set_user_index(1313); // FastJet index used to keep track of J/psi
          nTaggedMuons++;
        }
      }
      particles.push_back(p);
    }
#ifdef JpsiDEBUG
    printf("Tagged %d muons in FinalState (toCheck size=%lu, fsParticles "
           "size=%lu)\n",
           nTaggedMuons, toCheck.size(), fsParticles.size());
    fflush(stdout);
#endif

    JetDefinition jetDefAKT_Sig(fastjet::antikt_algorithm,
                                0.4); // jet distance parameter
    ClusterSequence antikTjets(particles, jetDefAKT_Sig); // cluster anti-kT
    vector<PseudoJet> jets = fastjet::sorted_by_pt(antikTjets.inclusive_jets(
        30)); // Jet projection : we only keep jets with pT > 30 Gev

    //  printf("Pseudo jet size : %d, Clustered jet size : %d\n", (int)
    //  particles.size(), (int) jets.size());

    if (jets.size() < 1)
      vetoEvent; // at least 1 jet in the event
    _hJetpT->fill(jets[0].pt());
    printf("Pass Jet Veto!!\n");
    fflush(stdout);
    if (jpsi_particles.size() != 1)
      vetoEvent; // 1 Jpsi in the final state (do we allow the finalstate to
                 // have more than 1 Jpsi ? Maybe it will be interesting to
                 // modify the routine so that you can treat the case of
                 // multiple Jpsi productions)
#ifdef JpsiDEBUG
    printf("Pass Unique Onia Veto!!\n");
    fflush(stdout);
#endif
    int nJets = 0;
    const double maxDeltaR = 0.4; // J/psi must be within this dR of jet axis

    for (size_t i = 0; i < jets.size(); ++i) // loop over all jets
    {
      // Calculate deltaR between jet axis and J/psi
      double dPhi = jets[i].phi() - jpsi_particles[0].phi();
      // Handle phi wraparound
      while (dPhi > M_PI)
        dPhi -= 2 * M_PI;
      while (dPhi < -M_PI)
        dPhi += 2 * M_PI;
      double dEta = jets[i].eta() - jpsi_particles[0].eta();
      double deltaR = sqrt(dPhi * dPhi + dEta * dEta);

      // J/psi must be within jet cone
      if (deltaR > maxDeltaR)
        continue;
#ifdef JpsiDEBUG
      printf("Jet %lu: dR(jet,Jpsi)=%.3f < %.1f\n", i, deltaR, maxDeltaR);
      fflush(stdout);
#endif

      // Count matched muons in jet constituents and compute their 4-momentum
      std::vector<PseudoJet> constituents = jets[i].constituents();
      int nMatchedMuons = 0;
      PseudoJet muonSum(0, 0, 0, 0);
      for (const auto &constituent : constituents) {
        if (constituent.user_index() == 1313) {
          nMatchedMuons++;
          muonSum += constituent;
        }
      }
#ifdef JpsiDEBUG
      printf("  Matched muons in jet: %d\n", nMatchedMuons);
      fflush(stdout);
#endif

      // Compute corrected jet: remove muon contributions, add J/psi
      // jet_corrected = jet - muons + J/psi
      PseudoJet jpsiPJ = jpsi_particles[0].pseudojet();
      PseudoJet jetCorrected = jets[i] - muonSum + jpsiPJ;
      double correctedJetPt = jetCorrected.pt();
#ifdef JpsiDEBUG
      printf("  Original jet pT=%.2f, muon sum pT=%.2f, J/psi pT=%.2f -> "
             "corrected jet pT=%.2f\n",
             jets[i].pt(), muonSum.pt(), jpsiPJ.pt(), correctedJetPt);
      fflush(stdout);
#endif

      // Apply jet pT cuts on corrected jet
      if (correctedJetPt < 30)
        continue;
#ifdef JpsiDEBUG
      printf("  Pass corrected jet pT > 30\n");
      fflush(stdout);
#endif
      if (correctedJetPt > 40)
        continue;
#ifdef JpsiDEBUG
      printf("  Pass corrected jet pT < 40\n");
      fflush(stdout);
#endif

      // Jet eta cut
      if (fabs(jetCorrected.pseudorapidity()) > 2.0)
        continue;
#ifdef JpsiDEBUG
      printf("  Pass jet |eta| < 2.0\n");
      fflush(stdout);
#endif

      // Only take first matching jet
      if (nJets > 0)
        continue;
#ifdef JpsiDEBUG
      printf("  Selected as first jet!\n");
      fflush(stdout);
#endif

      nJets++;

      // Calculate z = pT(J/psi) / pT(corrected jet)
      double z = jpsi_particles[0].pt() / correctedJetPt;
      printf("z value: %f (muons removed: %d)\n", z, nMatchedMuons);
      fflush(stdout);
      assert(z > 0 && z <= 1.0);

      if (isFromGtoCC(jpsi_particles[0]))
        _hZgcc->fill(z);
      else
        _hZqqcc->fill(z);
      _hZ->fill(z);

      // Fill new histograms: Fixed J/psi pT (6.5-10 GeV), freed jet pT (>=10
      // GeV)
      double jpsiPt = jpsi_particles[0].pt();
      if (jpsiPt >= 6.5 && jpsiPt < 10.0 && correctedJetPt >= 10.0) {
        _hZ_lowPtJpsi->fill(z);
        _hJetpT_lowPtJpsi->fill(correctedJetPt);
        if (isFromGtoCC(jpsi_particles[0]))
          _hZgcc_lowPtJpsi->fill(z);
        else
          _hZqqcc_lowPtJpsi->fill(z);
      }

      _njets->fill(nJets);
    }
  }
  // Finalize method was retired since there is no need to renormalize the
  // histogramm related to z
private:
  Histo1DPtr _hnJpsi;
  Histo1DPtr _hZ;
  Histo1DPtr _hZgcc;
  Histo1DPtr _hZqqcc;
  Histo1DPtr _hJpsipT;
  Histo1DPtr _hJetpT;
  CounterPtr _njets;
  // New histograms: Fixed J/psi pT (6.5-10 GeV), freed jet pT (>=10 GeV)
  Histo1DPtr _hZ_lowPtJpsi;
  Histo1DPtr _hZqqcc_lowPtJpsi;
  Histo1DPtr _hZgcc_lowPtJpsi;
  Histo1DPtr _hJetpT_lowPtJpsi;
  bool isFromGtoCC(const Particle &p) const {
    // Start with the current particle
    std::vector<Particle> ancestors = {p};

    // Traverse the ancestry
    while (!ancestors.empty()) {
      const Particle current = ancestors.back();
      //    	printf("Starting with PID : %d", current.pid());
      ancestors.pop_back();

      // Check if the current particle is a gluon
      if (current.pid() == 21) { // PDG ID 21 = gluon
        int cCount = 0;
        for (const Particle &child : current.children()) {
          //    		printf(" : child PID : %d\n", child.pid());
          if (std::abs(child.pid()) == 4) { // PDG ID 4 = charm quark
            ++cCount;
          }
        }
        if (cCount >= 2) {
          return true;
        }
      }

      // Add the parents of the current particle to the list for further
      // checking
      for (const Particle &parent : current.parents()) {
        ancestors.push_back(parent);
      }
    }
    return false;
  }
};

RIVET_DECLARE_PLUGIN(JpsiJet_RivetAnalyzer);
} // namespace Rivet
