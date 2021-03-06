/*
  Run from the command line with
  root -l -b RA2bZinvLoadClasses.C RA2bZinvDriver.C
  or
  root -l -b RA2bZinvLoadClasses.C RA2bZinvDriver.C\(\"2016B\"\)
*/

#include "TROOT.h"
#include "TEnv.h"

void RA2bZinvDriver(const std::string& runBlock = "") {
  cout << "runBlock passed to Driver = " << runBlock << endl;

  gEnv->SetValue("TFile.AsyncPrefetching", 1);

  bool doHzvv = false;
  bool doHttzvv = false;
  bool doHzmm = false;
  bool doHzee = false;
  bool doHphoton = false;
  bool doHdymm = false;
  bool doHdyee = false;
  bool doHttzmm = false;
  bool doHttzee = false;
  bool doHVVmm = false;
  bool doHVVee = false;
  bool doHttmm = false;
  bool doHttee = false;
  bool doHgjets = true;
  bool doHgjetsqcd = true;
  const std::string makeClassSample = "";  // Must be compatible with compiler directives
  bool doListTrigPrescales = false;
  const std::string dumpSelEvIDsample("");

  // RA2bZinvAnalysis analyzer("", runBlock);  // Default configuration, V12
  std::string cfgName("data");  cfgName += runBlock;  cfgName = regex_replace(cfgName, regex("HEM"), "");  cfgName += ".cfg";
  RA2bZinvAnalysis analyzer(cfgName, runBlock);
  // RA2bZinvAnalysis analyzer("data2016.cfg", runBlock);
  // RA2bZinvAnalysis analyzer("data2017.cfg", runBlock);
  // RA2bZinvAnalysis analyzer("data2018.cfg", runBlock);
  // RA2bZinvAnalysis analyzer("dataRun2.cfg", runBlock);
  // RA2bZinvAnalysis analyzer("lowDphi.cfg", runBlock);

  void combine(std::vector<TH1*> hl1, std::vector<TH1*> hl2);
  std::string fnstr;

  if (doHzvv || doHttzvv) {
    fnstr = "histsZjets";  fnstr += runBlock + ".root";
    char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
    TFile *histoOutFile = TFile::Open(outfn, "RECREATE");
    TH1F *hCCzvv = nullptr;
    if (doHzvv) {
      std::vector<TH1*> h_zinv = analyzer.makeHistograms("zinv");
      for (auto& theHist : h_zinv) {
	theHist->Print();
	theHist->Draw();
	TString hName(theHist->GetName());
	if (hName.Contains("hCC") && !hName.Contains("jb") && !hName.Contains("jk") && !hName.Contains("Jb") && !hName.Contains("spl")) {
	  hCCzvv = (TH1F*) theHist;
	}
      }
    }
    if (doHttzvv) {
      std::vector<TH1*> h_ttzvv = analyzer.makeHistograms("ttzvv");
      for (auto& theHist : h_ttzvv) {
	theHist->Print();
	theHist->Draw();
	TString hName(theHist->GetName());
	if (hName.Contains("hCC") && !hName.Contains("jb") && !hName.Contains("jk") && !hName.Contains("Jb") && !hName.Contains("spl") && hCCzvv != nullptr) {
	  TH1F* hCCzinvAll = (TH1F*) hCCzvv->Clone();  hCCzinvAll->SetName("hCCzinvAll");  hCCzinvAll->Sumw2();
	  hCCzinvAll->Add(theHist);
	  hCCzinvAll->SetName("hCCzinvAll");
	  hCCzinvAll->Print();
	  hCCzinvAll->Draw();
	}
      }
    }
    histoOutFile->Write();
  }

  if (doHzmm || doHzee) {
    std::vector<TH1*> h_zmm, h_zee;
    TFile *histoOutFile;
    fnstr = "histsZ";
    if (doHzmm && doHzee) {
      fnstr += "ll" + runBlock + ".root";
      char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
      histoOutFile = TFile::Open(outfn, "RECREATE");
    }
    if (doHzmm) {
      if (!doHzee) {
	fnstr += "mm" + runBlock + ".root";
	char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
	histoOutFile = TFile::Open(outfn, "RECREATE");
      }
      h_zmm = analyzer.makeHistograms("zmm");
      for (auto& theHist : h_zmm) {
	theHist->Print();
	theHist->Draw();
      }
    }
    if (doHzee) {
      if (!doHzmm) {
	fnstr += "ee" + runBlock + ".root";
	char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
	histoOutFile = TFile::Open(outfn, "RECREATE");
      }
      h_zee = analyzer.makeHistograms("zee");
      for (auto& theHist : h_zee) {
	theHist->Print();
	theHist->Draw();
      }
      if (doHzmm && doHzee) combine(h_zmm, h_zee);
    }
    histoOutFile->Write();
  }

  if (doHdymm || doHdyee || doHttzmm || doHttzee || doHVVmm || doHVVee || doHttmm || doHttee) {
    fnstr = "histsDYMC";  fnstr += runBlock + ".root";
    char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
    TFile *histoOutFile = TFile::Open(outfn, "RECREATE");
    std::vector<TH1*> h_dymm, h_dyee, h_ttzmm, h_ttzee, h_VVmm, h_VVee, h_ttmm, h_ttee;

    if (doHdymm) {
      h_dymm = analyzer.makeHistograms("dymm");
      for (auto& theHist : h_dymm) {theHist->Print();  theHist->Draw();}
    }
    if (doHdyee) {
      h_dyee = analyzer.makeHistograms("dyee");
      for (auto& theHist : h_dyee) {theHist->Print();  theHist->Draw();}
    }
    if (doHdymm && doHdyee) combine(h_dymm, h_dyee);

    if (doHttzmm) {
      h_ttzmm = analyzer.makeHistograms("ttzmm");
      for (auto& theHist : h_ttzmm) {theHist->Print();  theHist->Draw();}
    }
    if (doHttzee) {
      h_ttzee = analyzer.makeHistograms("ttzee");
      for (auto& theHist : h_ttzee) {theHist->Print();  theHist->Draw();}
    }
    if (doHttzmm && doHttzee) combine(h_ttzmm, h_ttzee);

    if (doHVVmm) {
      h_VVmm = analyzer.makeHistograms("VVmm");
      for (auto& theHist : h_VVmm) {theHist->Print();  theHist->Draw();}
    }
    if (doHVVee) {
      h_VVee = analyzer.makeHistograms("VVee");
      for (auto& theHist : h_VVee) {theHist->Print();  theHist->Draw();}
    }
    if (doHVVmm && doHVVee) combine(h_VVmm, h_VVee);

    if (doHttmm) {
      h_ttmm = analyzer.makeHistograms("ttmm");
      for (auto& theHist : h_ttmm) {theHist->Print();  theHist->Draw();}
    }
    if (doHttee) {
      h_ttee = analyzer.makeHistograms("ttee");
      for (auto& theHist : h_ttee) {theHist->Print();  theHist->Draw();}
    }
    if (doHttmm && doHttee) combine(h_ttmm, h_ttee);

    histoOutFile->Write();
  }

  if (doHphoton) {
    fnstr = "histsPhoton";  fnstr += runBlock + ".root";
    char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
    TFile *histoOutFile = TFile::Open(outfn, "RECREATE");
    std::vector<TH1*> h_photon = analyzer.makeHistograms("photon");
    for (auto& theHist : h_photon) {
      theHist->Print();
      theHist->Draw();
    }
    histoOutFile->Write();
  }

  if (doHgjets || doHgjetsqcd) {
    fnstr = "histsGjets";  fnstr += runBlock + ".root";
    char* outfn = new char[fnstr.length()+1];  std::strcpy (outfn, fnstr.c_str());
    TFile *histoOutFile = TFile::Open(outfn, "RECREATE");
    if (doHgjets) {
      std::vector<TH1*> h_gJets = analyzer.makeHistograms("gjets");
      for (auto& theHist : h_gJets) {theHist->Print();  theHist->Draw();}
    }
    if (doHgjetsqcd) {
      std::vector<TH1*> h_gjetsqcd = analyzer.makeHistograms("gjetsqcd");
      for (auto& theHist : h_gjetsqcd) {theHist->Print();  theHist->Draw();}
    }
    histoOutFile->Write();
  }

  if (doListTrigPrescales) {
    analyzer.checkTrigPrescales("zmm");
  }

  if (!makeClassSample.empty()) {
    analyzer.runMakeClass(makeClassSample);
  }

  if (!dumpSelEvIDsample.empty())
    analyzer.dumpSelEvIDs(dumpSelEvIDsample.data(), (std::string("evtIDs_") + runBlock + ".txt").data());

  gApplication->Terminate(0);

}

void combine(std::vector<TH1*> hl1, std::vector<TH1*> hl2) {
  for (auto& h1 : hl1) {
    TString h1Name(h1->GetName());
    if (!h1Name.Contains("hCC")) continue;
    if (h1Name.Contains("spl")) continue;
    TString heeName = h1Name;  heeName("mm") = "ee";
    TString hllName = h1Name;  hllName("mm") = "ll";
    // TString hcNameTS = h1Name;  hcNameTS("mm") = "ll";
    // const char* hcName = hcNameTS.Data();
    for (auto& h2 : hl2) {
      TString h2Name(h2->GetName());
      if (h2Name.EqualTo(heeName)) {
	TH1D* hll = (TH1D*) h1->Clone();  hll->SetName(hllName);
	hll->Add(h2);
	hll->Print();
	hll->Draw();
      }
    }
  }
}
