void legendBackground(void) {
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  TLegend *l = new TLegend(0.2, 0.2, 0.45, 0.35);

  TH1F *h1 = new TH1F("h1", "h1", 50, 0, 50);
  h1->SetLineColor(kRed);
  h1->Draw();
  
  TH1F *h2 = new TH1F("h2", "h2", 50, 0, 50);
  h2->SetLineColor(kBlue);

  TH1F *h3 = new TH1F("h3", "h3", 50, 0, 50);
  h3->SetLineColor(kMagenta);

  TH1F *h4 = new TH1F("h4", "h4", 50, 0, 50);
  h4->SetLineColor(kCyan);

  l->AddEntry(h1, "PLT beam 1 background", "L");
  l->AddEntry(h2, "PLT beam 2 background", "L");
  l->AddEntry(h3, "BCM1F beam 1 background", "L");
  l->AddEntry(h4, "BCM1F beam 2 background", "L");
  l->Draw();
}
