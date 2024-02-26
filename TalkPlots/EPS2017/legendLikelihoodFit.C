void legendLikelihoodFit(void) {
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  TLegend *l = new TLegend(0.2, 0.2, 0.45, 0.35);

  TH1F *h1 = new TH1F("h1", "h1", 50, 0, 50);
  h1->SetLineColor(kBlue);
  h1->Draw();
  
  TH1F *h2 = new TH1F("h2", "h2", 50, 0, 50);
  h2->SetLineColor(kGreen);

  TH1F *h3 = new TH1F("h3", "h3", 50, 0, 50);
  h3->SetLineColor(kMagenta);

  l->AddEntry(h1, "Overall fit to data", "L");
  l->AddEntry(h2, "Fit from VdM data", "L");
  l->AddEntry(h3, "Additional accidental component", "L");
  l->Draw();
}
