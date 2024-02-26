void legendVdMScans(void) {
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  TLegend *l = new TLegend(0.2, 0.2, 0.5, 0.4);

  TH1F *h1 = new TH1F("h1", "h1", 50, 0, 50);
  h1->SetLineColor(kBlack);
  h1->SetMarkerColor(kRed);
  h1->SetMarkerStyle(kFullCircle);
  h1->Draw();
  
  TH1F *h2 = new TH1F("h2", "h2", 50, 0, 50);
  h2->SetLineColor(kBlack);
  h2->SetMarkerColor(kBlue);
  h2->SetMarkerStyle(kFullCircle);

  TH1F *h3 = new TH1F("h3", "h3", 50, 0, 50);
  h3->SetLineColor(kBlack);
  h3->SetMarkerColor(kGreen);
  h3->SetMarkerStyle(kFullCircle);

  TH1F *h4 = new TH1F("h4", "h4", 50, 0, 50);
  h4->SetLineColor(kBlack);
  h4->SetMarkerColor(kBlack);
  h4->SetMarkerStyle(kFullCircle);

  TH1F *h5 = new TH1F("h5", "h5", 50, 0, 50);
  h5->SetLineColor(kBlack);
  h5->SetMarkerColor(kYellow);
  h5->SetMarkerStyle(kFullCircle);

  l->AddEntry(h1, "PLT scan pair 1", "PE");
  l->AddEntry(h2, "PLT scan pair 2", "PE");
  l->AddEntry(h3, "PLT scan pair 3", "PE");
  l->AddEntry(h4, "PLT scan pair 4", "PE");
  l->AddEntry(h5, "PLT scan pair 5", "PE");
  l->Draw();
}
