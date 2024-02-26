void legendVdMFit(void) {
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  TLegend *l = new TLegend(0.2, 0.2, 0.45, 0.35);

  gROOT->SetStyle("Plain");
  TH1F *h1 = new TH1F("h1", "h1", 50, 0, 50);
  h1->SetLineColor(kRed);
  h1->Draw();
  
  TLatex *t1 = new TLatex(12, 0.3, "#splitline{CMS Preliminary 2016}{VdM Scan: Fill 4945}");
  t1->Draw();
  statsbox = new TPaveStats(0.60, 0.755, 0.98, 0.98, "NDC");
  statsl = statsbox->GetListOfLines();
  dummy = new TLatex(0,0, "dummy");
  statsl->Add(dummy);
  chi2 = new TLatex(0, 0, "#chi^{2} / ndf = 20.26 / 19");
  chi2->SetTextSize(0.03);
  statsl->Add(chi2);
  width = new TLatex(0, 0, "Effective width = 0.1242 #pm 0.0004");
  width->SetTextSize(0.03);
  statsl->Add(width);
  constant = new TLatex(0, 0, "Constant = 5.8e-06 #pm 0.5e-06");
  constant->SetTextSize(0.03);
  statsl->Add(constant);
  statsbox->SetFillColor(0);
  statsbox->SetBorderSize(1);
  statsbox->Draw();
}
