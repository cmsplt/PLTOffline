void plotEffVsTime(void) {
  std::vector<float> fill;
  std::vector<float> xerr;
  std::vector<float> eff;
  std::vector<float> efferr;
  
  FILE *f = fopen("eff_by_fill.csv", "r");
  float thisFill, thisEff, thisEffErr;
  for (int i=0; i<90; ++i) {
    fscanf(f, "%f,%f,%f\n", &thisFill, &thisEff, &thisEffErr);
    fill.push_back(thisFill);
    xerr.push_back(0);
    eff.push_back(thisEff);
    efferr.push_back(thisEffErr);
  }
  fclose(f);

  TGraph *g = new TGraphErrors(90, &(fill[0]), &(eff[0]),
			       &(xerr[0]), &(efferr[0]));

  gROOT->SetStyle("Plain");                  
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.17);
  gStyle->SetPadRightMargin(0.17);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(1.0);
  gStyle->SetTitleH(0.09);
  gStyle->SetTitleW(0.7);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetLegendBorderSize(0);

  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  g->Draw("AP");
  g->SetTitle("Measured efficiency vs. time for 2016");
  g->GetXaxis()->SetTitle("Fill number");
  g->GetYaxis()->SetTitle("Efficiency, channel 4, ROC 1");
  g->GetYaxis()->SetTitleOffset(1.2);
  g->SetMarkerStyle(kFullCircle);

  c1->Print("Efficiency2016.png");
}
