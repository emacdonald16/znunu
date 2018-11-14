/*
  Run from the command line with
  root -l -b RA2bZinvLoadClasses.C Nb0bExtrap.C
*/

#include "CCbinning.h"
#include <stdio.h>
#include "TROOT.h"
#include "TH1F.h"
#include "TFile.h"

#include "TMath.h"
using TMath::Sqrt; using TMath::Power;

void Nb0bExtrap(const std::string& era = "2016", const std::string& deltaPhi = "nominal") {

  CCbinning CCmaps(era, deltaPhi);
  std::vector< std::vector<double> > kinThresholds = CCmaps.kinThresholds();
  std::vector<int> nJet1Thresholds = CCmaps.nJet1Thresholds();
  std::vector<int> nJetThresholds = CCmaps.nJetThresholds();
  std::vector<int> nbThresholds = CCmaps.nbThresholds();
  unsigned kinSize = CCmaps.kinSize();
  std::vector< std::vector<int> > jetSubBins = CCmaps.jetSubBins();
  CCbinning::ivector_map toCCbin = CCmaps.toCCbin();
  CCbinning::ivector_map toCCbinjb = CCmaps.toCCbinjb();
  CCbinning::ivector_map toCCbinSpl = CCmaps.toCCbinSpl();
  CCbinning::ivector_map toCCbinJb = CCmaps.toCCbinJb();

  bool doJfromData = false;
  int extrapByMCthreshold = nJetThresholds.size() - 1;  // Use MC for the NJets 9+ bin
  // std::vector<int> extrapFromRange = {5, 6};
  float relErrXsec_ttz = 0.3;

  TFile ZllData("../outputs/histsDYspl_2016v15.root");
  TFile photonData("../outputs/histsPhoton_2016v15.root");
  TFile ZllXMC("../outputs/histsDYMCspl_2016v12.root");
  
  // Get the Z->ll data histograms
  TH1F* hCC_zmm = (TH1F*) ZllData.Get("hCC_zmm");
  TH1F* hCC_zee = (TH1F*) ZllData.Get("hCC_zee");
  TH1F* hCCjb_zmm = (TH1F*) ZllData.Get("hCCjb_zmm");
  TH1F* hCCjb_zee = (TH1F*) ZllData.Get("hCCjb_zee");
  TH1F* hCCjb_zll = (TH1F*) hCCjb_zmm->Clone();  hCCjb_zll->SetName("hCCjb_zll");
  hCCjb_zll->Add(hCCjb_zee);
  TH1F* hCCspl_photon = (TH1F*) photonData.Get("hCCspl_photon");
  TH1F* hCCJb_zmm = (TH1F*) ZllData.Get("hCCJb_zmm");
  TH1F* hCCJb_zee = (TH1F*) ZllData.Get("hCCJb_zee");
  TH1F* hCCJb_zll = (TH1F*) hCCJb_zmm->Clone();  hCCJb_zll->SetName("hCCJb_zll");
  hCCJb_zll->Add(hCCJb_zee);
  TH1F* hCCJb_dymm = (TH1F*) ZllXMC.Get("hCCJb_dymm");
  TH1F* hCCJb_dyee = (TH1F*) ZllXMC.Get("hCCJb_dyee");
  TH1F* hCCJb_ttzmm = (TH1F*) ZllXMC.Get("hCCJb_ttzmm");
  TH1F* hCCJb_ttzee = (TH1F*) ZllXMC.Get("hCCJb_ttzee");
  TH1F* hCCJb_VVmm = (TH1F*) ZllXMC.Get("hCCJb_VVmm");
  TH1F* hCCJb_VVee = (TH1F*) ZllXMC.Get("hCCJb_VVee");
  TH1F* hCCJb_ttmm = (TH1F*) ZllXMC.Get("hCCJb_ttmm");
  TH1F* hCCJb_ttee = (TH1F*) ZllXMC.Get("hCCJb_ttee");
  TH1F* hCCJb_MCall = (TH1F*) hCCJb_dymm->Clone();  hCCJb_MCall->SetName("hCCJb_MCall");
  hCCJb_MCall->Add(hCCJb_dyee);
  hCCJb_MCall->Add(hCCJb_ttzmm);
  hCCJb_MCall->Add(hCCJb_ttzee);
  hCCJb_MCall->Add(hCCJb_VVmm);
  hCCJb_MCall->Add(hCCJb_VVee);
  // hCCJb_MCall->Add(hCCJb_ttmm);  // Omit non-peaking, since data are purity corrected.
  // hCCJb_MCall->Add(hCCJb_ttee);

  // Calculate Z->ll data stat errors
  std::vector<float> DYstat;
  for (int j = 0; j < nJetThresholds.size(); ++j) {
    float b0 = 0, b0err = 0, bb = 0, bberr = 0;
    for (int b = 0; b < nbThresholds.size(); ++b) {
      int jj = j < extrapByMCthreshold ? j : j-1;
      std::vector<int> jb = {jj, b};
      int binCCjb;
      try {binCCjb = toCCbinjb.at(jb);}
      catch (const std::out_of_range& oor) {continue;}
      if (b == 0) {
	b0 = hCCjb_zll->GetBinContent(binCCjb);
	b0err = hCCjb_zll->GetBinError(binCCjb);
	DYstat.push_back(0);
      } else {
	bb = hCCjb_zll->GetBinContent(binCCjb);
	bberr = hCCjb_zll->GetBinError(binCCjb);
	if (b0 > 0 && bb > 0)
	  DYstat.push_back(Sqrt(Power((b0err/b0), 2) + Power((bberr/bb), 2)));
	else
	  DYstat.push_back(1);
      }
    }
  }

  // Compute the purity uncertainties
  // TFile effFile("../plots/histograms/effHists.root");
  TFile effFile("../python/effHists.root");
    
  TH1F* h_pur_m = (TH1F*) effFile.Get("h_pur_m");
  TH1F* h_pur_e = (TH1F*) effFile.Get("h_pur_e");
  std::vector<float> DYpurSys;
  // Declare the bin merging map for purities
  std::vector< std::vector<int> > pbins;
  pbins.push_back({1});
  pbins.push_back({2});
  pbins.push_back({3});
  pbins.push_back({4});
  pbins.push_back({5});
  pbins.push_back({6, 7});
  pbins.push_back({6, 7});
  pbins.push_back({8, 12, 16});
  pbins.push_back({9, 13, 17});
  pbins.push_back({10, 11, 14, 15, 18, 19});
  pbins.push_back({10, 11, 14, 15, 18, 19});
  pbins.push_back({8, 12, 16});
  pbins.push_back({9, 13, 17});
  pbins.push_back({10, 11, 14, 15, 18, 19});
  pbins.push_back({10, 11, 14, 15, 18, 19});
  pbins.push_back({8, 12, 16});
  pbins.push_back({9, 13, 17});
  pbins.push_back({10, 11, 14, 15, 18, 19});
  pbins.push_back({10, 11, 14, 15, 18, 19});
  // Compute the relative purity error averaged over Zmm, Zee, and bin groups
  for (int i = 0; i < pbins.size(); ++i) {
    float y = 0, ype = 0;
    for (int j = 0; j < pbins[i].size(); ++j) {
      int bin = pbins[i][j];
      y += hCCjb_zmm->GetBinContent(bin) + hCCjb_zee->GetBinContent(bin);
      ype += hCCjb_zmm->GetBinContent(bin)*h_pur_m->GetBinError(bin)/h_pur_m->GetBinContent(bin)
       	   + hCCjb_zee->GetBinContent(bin)*h_pur_e->GetBinError(bin)/h_pur_e->GetBinContent(bin);
    }
    DYpurSys.push_back(ype/y);
  }

  // Calculate the MC J factors from MC to extend the Nb/0b F factors
  // Take the Nphoton average of F over NJet sub-bins at each kin bin.
  std::vector<std::pair<float, float>> Jextrap, jb0;
  for (int j = 0; j <= extrapByMCthreshold; ++j) {
    for (int b = 0; b < nbThresholds.size(); ++b) {
      for (int k = 0; k < kinSize; ++k) {
	int binCC;
	try {binCC = toCCbin.at((std::vector<int>) {j, b, k});}  catch (const std::out_of_range& oor) {continue;}
	float Npho = 0;
	std::pair<float, float> jbb, NphoF;
	for (auto J : jetSubBins[j]) {
	  int binCCJb, binCCjb;
	  try {binCCJb = toCCbinJb.at((std::vector<int>) {J, b});}  catch (const std::out_of_range& oor) {continue;}
	  try {binCCjb = toCCbinjb.at((std::vector<int>) {j, b});}  catch (const std::out_of_range& oor) {continue;}
	  std::pair<float, float> nMC;
	  if (doJfromData) {
	    nMC.first = hCCJb_zll->GetBinContent(binCCJb);  // for Jextrap systematic
	    nMC.second = hCCJb_zll->GetBinError(binCCJb);  //  "
	  } else {
	    nMC.first = hCCJb_MCall->GetBinContent(binCCJb);
	    nMC.second = hCCJb_MCall->GetBinError(binCCJb);
	  }
	  if (J >= jb0.size()) {
	    jb0.push_back(nMC);
	    // cout << "j, J, b, k, jb0Size = " << j << ", " << J << ", " << b << ", " << k << ", " << jb0.size() << endl;
	  } else {
	    jbb.first = nMC.first / jb0.at(J).first;
	    jbb.second = nMC.first > 0 ? Sqrt(Power(nMC.second/nMC.first, 2) + Power(jb0.at(J).second/jb0.at(J).first, 2)) : 0;
	    int binCCJ0k;
	    try {binCCJ0k = toCCbinSpl.at((std::vector<int>) {J, 0, k});}  catch (const std::out_of_range& oor) {continue;}
	    float NphoJ = hCCspl_photon->GetBinContent(binCCJ0k);
	    Npho += NphoJ;
	    NphoF.first += NphoJ*jbb.first;
	    NphoF.second += NphoJ*jbb.second;
	    // cout << "j, J, b, k, jbb, NphoJ, Npho = " << j << ", " << J << ", " << b << ", " << k << ", "
	    // 	 << jbb.first << ", relErr = " << jbb.second << ", " << NphoJ << ", " << Npho << endl;
	  }
	}
	if (b == 0) {
	  Jextrap.push_back(std::pair<float, float>(1, 0));
	} else {
	  if (Npho > 0)
	    Jextrap.push_back(std::pair<float, float>(NphoF.first/Npho, NphoF.second/Npho));
	  else
	    Jextrap.push_back(std::pair<float, float>(jbb.first, jbb.second));
	}
	if (j >= extrapByMCthreshold) {
	  // Compute ratio of NJets = 9+ to NJets = 7-8
	  float relErrjbk = Jextrap[binCC - 1].second;
	  float relErrjm1bk = Jextrap[toCCbin.at((std::vector<int>) {j-1, b, k}) - 1].second;
	  Jextrap[binCC - 1].first /= Jextrap[toCCbin.at((std::vector<int>) {j-1, b, k}) - 1].first;
	  Jextrap[binCC - 1].second = Sqrt(Power(relErrjbk, 2) + Power(relErrjm1bk, 2));
	  if (k == k) cout << "j, b, k, Jextrap = " << j << ", " << b << ", " << k << ", " <<
			Jextrap[binCC - 1].first << ", rel err = " << Jextrap[binCC - 1].second << endl;
	}
      }
    }
  }

  // Jextrap systematics derived from analysis of the above for data, MC
  std::vector<float> systJ = {0, 0.10, 0.10, 0.20};

  if (extrapByMCthreshold != nJetThresholds.size() - 1) return;

  // Calculate the ttz cross section uncertainty from the ttz fraction.
  TH1F* hCCjb_dymm = (TH1F*) ZllXMC.Get("hCCjb_dymm");
  TH1F* hCCjb_dyee = (TH1F*) ZllXMC.Get("hCCjb_dyee");
  TH1F* hCCjb_ttzmm = (TH1F*) ZllXMC.Get("hCCjb_ttzmm");
  TH1F* hCCjb_ttzee = (TH1F*) ZllXMC.Get("hCCjb_ttzee");
  TH1F* hCCjb_VVmm = (TH1F*) ZllXMC.Get("hCCjb_VVmm");
  TH1F* hCCjb_VVee = (TH1F*) ZllXMC.Get("hCCjb_VVee");
  TH1F* hCCjb_ttmm = (TH1F*) ZllXMC.Get("hCCjb_ttmm");
  TH1F* hCCjb_ttee = (TH1F*) ZllXMC.Get("hCCjb_ttee");
  TH1F* hCCjb_MCall = (TH1F*) hCCjb_dymm->Clone();  hCCjb_MCall->SetName("hCCjb_MCall");
  hCCjb_MCall->Add(hCCjb_dyee);
  hCCjb_MCall->Add(hCCjb_ttzmm);
  hCCjb_MCall->Add(hCCjb_ttzee);
  hCCjb_MCall->Add(hCCjb_VVmm);
  hCCjb_MCall->Add(hCCjb_VVee);
  hCCjb_MCall->Add(hCCjb_ttmm);
  hCCjb_MCall->Add(hCCjb_ttee);
  TH1F* hCCjb_MCttzFrac = (TH1F*) hCCjb_ttzmm->Clone();  hCCjb_MCall->SetName("hCCjb_MCttzFrac");
  hCCjb_MCttzFrac->Add(hCCjb_ttzee);
  hCCjb_MCttzFrac->Divide(hCCjb_MCall);
  hCCjb_MCttzFrac->Print("all");

  // Calculate Nb/0b F factors
  // Take the Nphoton average of F over NJet sub-bins at each kin bin.
  std::vector<float> Fextrap, fb0;
  for (int j = 0; j < nJetThresholds.size(); ++j) {
    for (int b = 0; b < nbThresholds.size(); ++b) {
      for (int k = 0; k < kinSize; ++k) {
	int binCC = 0;
	try {binCC = toCCbin.at((std::vector<int>) {j, b, k});}  catch (const std::out_of_range& oor) {continue;}
	if (j < extrapByMCthreshold) {
	  float Npho = 0, NphoF = 0, fbb = 0;
	  for (auto J : jetSubBins[j]) {
	    int binCCJb, binCCjb;
	    try {binCCJb = toCCbinJb.at((std::vector<int>) {J, b});}  catch (const std::out_of_range& oor) {continue;}
	    try {binCCjb = toCCbinjb.at((std::vector<int>) {j, b});}  catch (const std::out_of_range& oor) {continue;}
	    float Nemuxp = hCCJb_zmm->GetBinContent(binCCJb) * h_pur_m->GetBinContent(binCCjb)
	                 + hCCJb_zee->GetBinContent(binCCJb) * h_pur_e->GetBinContent(binCCjb);
	    if (J >= fb0.size()) {
	      fb0.push_back(Nemuxp);
	      // cout << "j, J, b, k, fb0Size = " << j << ", " << J << ", " << b << ", " << k << ", " << fb0.size() << endl;
	    } else {
	      fbb = Nemuxp / fb0.at(J);
	      int binCCJ0k;
	      try {binCCJ0k = toCCbinSpl.at((std::vector<int>) {J, 0, k});}  catch (const std::out_of_range& oor) {continue;}
	      float NphoJ = hCCspl_photon->GetBinContent(binCCJ0k);
	      // cout << "j, J, b, k, fbb, NphoJ = " << j << ", " << J << ", " << b << ", " << k << ", " << fbb << ", " << NphoJ << endl;
	      Npho += NphoJ;
	      NphoF += NphoJ*fbb;
	    }
	  }
	  if (b == 0) {
	    Fextrap.push_back(1);
	  } else {
	    Fextrap.push_back(NphoF/Npho);
	  }
	} else {
	  // Set F(9+) = F(7,8) * J(7,8;9+)
	  // cout << "j, b, k, Jextrap (rel. err) = " << j << ", " << b << ", " << k << ", " <<
	  //   Jextrap[toCCbin.at((std::vector<int>) {j, b, k}) - 1].first << " (" <<
	  //   Jextrap[toCCbin.at((std::vector<int>) {j, b, k}) - 1].second << ")" << endl;
	  Fextrap.push_back(Fextrap[toCCbin.at((std::vector<int>) {j-1, b, k}) - 1] *
			    Jextrap[binCC - 1].first);
	}
      }
    }
  }

  // kin systematics derived from analysis of the closure plot
  std::vector<float> systKin = {0, 0.07, 0.10, 0.20};




  // Create and write the output dat file
  FILE* outFile;
  std::string outFileName("DY_");
  if (deltaPhi == "nominal") outFileName += "signal";
  else outFileName += deltaPhi;
  outFileName += ".dat";
  outFile = fopen(outFileName.data(), "w");
  fprintf(outFile, "%s\n",
	  " j b k|| Nmumu |  Nee  | Nb/0b | stat  |MC stat| ttz SF| syst+ | syst- | sysKin| sysPur"
	  );
  for (int j = 0; j < nJetThresholds.size(); ++j) {
    for (int b = 0; b < nbThresholds.size(); ++b) {
      for (int k = 0; k < kinSize; ++k) {
	std::vector<int> jbk = {j, b, k}, jb = {j, b};
	int binCC = 0, binCCjb = 0;
	try {binCC = toCCbin.at(jbk);}	catch (const std::out_of_range& oor) {continue;}
	try {binCCjb = toCCbinjb.at(jb);}  catch (const std::out_of_range& oor) {continue;}
	float JextrapErr, ttzErr;
	if (j < extrapByMCthreshold || b == 0) {
	  JextrapErr = 0;
	  ttzErr = 0;
	} else {
	  JextrapErr = Jextrap[binCC - 1].second;
	  // Factors in the Jextrap double ratio are fully correlated
	  ttzErr = relErrXsec_ttz * (hCCjb_MCttzFrac->GetBinContent(binCCjb) -
				     hCCjb_MCttzFrac->GetBinContent(toCCbinjb.at((std::vector<int>) {j-1, b})) -
				     hCCjb_MCttzFrac->GetBinContent(toCCbinjb.at((std::vector<int>) {j, 0})) +
				     hCCjb_MCttzFrac->GetBinContent(toCCbinjb.at((std::vector<int>) {j-1, 0}))
				     );
	}
  	fprintf(outFile, " %1d %1d %1d||%7d|%7d|%7.4f|%7.4f|%7.4f|%7.4f|%7.4f|%7.4f|%7.4f|%7.4f\n", j, b, k,
		(int) hCC_zmm->GetBinContent(binCC),
		(int) hCC_zee->GetBinContent(binCC),
		Fextrap[binCC - 1],
		DYstat[binCCjb - 1],
		JextrapErr,
		ttzErr,
		systJ[b],
		systJ[b],
		systKin[b],
		DYpurSys[binCCjb - 1]
		);
      }
    }
  }
  fclose(outFile);

  // Under construction ...

}