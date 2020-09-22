#include <iostream>
#include <string>                                                                                                                                                  
#include "PLTEvent.h"                                                                                                                                              
#include "PLTU.h"                                                                                                                                                 
#include "TGraph.h"
#include "TMarker.h"
#include "TH2F.h"
#include "TCanvas.h"                                                                                                    
#include "TLegend.h"
#include "TGraphErrors.h"
#include "TLatex.h"                                                                                                  
#include "TFile.h"    
#include "TMultiGraph.h"                                                                                                  
#include "THStack.h"                                                                                                    
#include <string>

int PlotHisto();

//asasa
int PlotHisto(){

// Plot PH peaks over Integrated Luminosity


TFile *f = new TFile("PH.root", "READ");
TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);

//List with 2015-2018 selected fills
int list2015[7] = {4246, 4349, 4410, 4467, 4518, 4540, 4569};
int list2016[12] = {4879, 4924, 5024, 5085, 5111, 5161, 5198, 5211, 5279, 5340, 5401, 5451};
int list2017[12] = {5722, 5834, 5950, 6035, 6097, 6136, 6161, 6241, 6283, 6312, 6337, 6398};
int list2018[12] = {6584, 6617, 6654, 6693, 6762, 6912, 6953, 7024, 7063, 7118, 7236, 7328};

//Corresponding Integrated Luminosity
double lumi2015[7] = {0.2036, 0.333, 0.856, 2.175, 2.603, 3.390, 4.209};
double lumi2016[12] = {4.211, 4.411, 8.765, 11.942, 16.240, 21.957, 23.209, 25.738, 27.196, 32.105, 34.496, 38.515};
double lumi2017[12] = {44.346, 44.753, 45.272, 51.106, 55.672, 61.261, 65.238, 69.411, 73.527, 84.070, 88.273, 94.905};
double lumi2018[12] = {95.226, 95.865, 100.717, 105.418, 114.98, 120.183, 124.049, 127.791, 136.653, 143.247, 151.533, 160.406};


TGraph *gr2015 = new TGraph();
TGraph *gr2016 = new TGraph();
TGraph *gr2017 = new TGraph();
TGraph *gr2018 = new TGraph();
TMultiGraph *mg = new TMultiGraph();
TLegend *leg = new TLegend(0.65, 0.65, 0.85, 0.8);


std::vector<double> peaks;
for (int l=0; l<7; l++){

  TFile *f = new TFile("PH.root", "READ");
  TH1F *hist2015_l = (TH1F*)f->FindObjectAny(TString::Format("hist_%i", list2015[l]));
  const int peak1 = (PLTU::peak_fromHisto(hist2015_l));
  gr2015->SetPoint(l, lumi2015[l], peak1);
  gr2015->SetMarkerStyle(20);
  gr2015->SetMarkerColor(1);
}

for (int k=0; k<12; k++){
 
  TH1F *hist2016_k = (TH1F*)f->FindObjectAny(TString::Format("hist_%i", list2016[k]));
  const int peak2 = (PLTU::peak_fromHisto(hist2016_k));
  gr2016->SetPoint(k, lumi2016[k], peak2);
  gr2016->SetMarkerStyle(20);
  gr2016->SetMarkerColor(2); 

  TH1F *hist2017_k = (TH1F*)f->FindObjectAny(TString::Format("hist_%i", list2017[k]));
  const int peak3 = (PLTU::peak_fromHisto(hist2017_k));
  gr2017->SetPoint(k, lumi2017[k], peak3);
  gr2017->SetMarkerStyle(20);
  gr2017->SetMarkerColor(4);

  TH1F *hist2018_k = (TH1F*)f->FindObjectAny(TString::Format("hist_%i", list2018[k]));
  const int peak4 = (PLTU::peak_fromHisto(hist2018_k));
  gr2018->SetPoint(k, lumi2018[k], peak4);
  gr2018->SetMarkerStyle(20);
  gr2018->SetMarkerColor(8);

}

leg->AddEntry(gr2015, "2015", "p");
leg->AddEntry(gr2016, "2016", "p");
leg->AddEntry(gr2017, "2017", "p");
leg->AddEntry(gr2018, "2018", "p");


mg->Add(gr2015);
mg->Add(gr2016);
mg->Add(gr2017);
mg->Add(gr2018);

int y_min = 8000;
int y_max = 40000;
double lumi_hvchange[5] = {34.496, 55.672, 61.261, 84.070, 95.226}; //line x-coord
double text_pos[5] = {23.000, 46.200, 62.400, 85.000, 96.400}; //text position.... unfortunately it has to be hardcoded
int HV_setpoint[5] = {150, 200, 300, 400, 800}; //HV setpoints

mg->Draw("APL");
mg->SetTitle("Pulse Height Peak over Integrated Luminosity Ch19 ROC0");
mg->GetXaxis()->SetTitle("Integrated Luminosity [fb^{-1}]");
mg->GetYaxis()->SetTitle("Electrons");
mg->GetYaxis()->SetTitleOffset(1.5);
mg->GetYaxis()->SetRangeUser(y_min, y_max); 
leg->Draw();


for (int j=1; j<5; j++){
  TLine *line_j = new TLine(lumi_hvchange[j], y_min, lumi_hvchange[j], y_max);
  line_j->SetLineStyle(2);
  line_j->Draw();
  
  TLatex *t_j = new TLatex(text_pos[j], y_max-2000, TString::Format("%i V", HV_setpoint[j]));
  t_j->SetTextSize(0.025);
  t_j->Draw();

}


c2->SaveAs("PHoverLumi.gif");
f->Close();

return 0;

}

int main(){
PlotHisto();
return 0;
}                                                     
