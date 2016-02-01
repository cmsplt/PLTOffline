void normalize(TGraph *g) {
  // Since TGraph doesn't include a Scale() function,
  // we have to modify the data in-place. This solution
  // is from Rene Brun himself!

  float s = 1.0/g->GetHistogram()->GetMaximum();
  float m = g->GetHistogram()->GetMinimum()*s;
  for (int i=0; i<g->GetN(); ++i)
    g->GetY()[i] *= s;

  g->SetMaximum(1.0);
  g->SetMinimum(m);
  return;
}

void MakeClusterLumiPlots() {
  // style from PLTU
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

  TFile *f = new TFile("clusterLumiPlots.root");
  TGraph *gH0 = (TGraph*)f->Get("grH0");
  TGraph *gH10 = (TGraph*)f->Get("grH10");
  TGraph *gH21 = (TGraph*)f->Get("grH21");
  TGraph *gT0 = (TGraph*)f->Get("grT0");
  TGraph *gT10 = (TGraph*)f->Get("grT10");
  TGraph *gT21 = (TGraph*)f->Get("grT21");

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  c1->Divide(2,2);

  c1->cd(1);
  gH21->Draw("ALP");
  gH21->GetYaxis()->SetTitleOffset(1.25);
  gH10->Draw("same");
  gH10->SetLineColor(kRed);
  gH0->Draw("same");
  gH0->SetLineColor(kBlue);
  gH21->SetTitle("Lumi from PLT pixel data, single-plane clusters");
  TLegend *l1 = new TLegend(0.50, 0.65, 0.80, 0.88);
  l1->AddEntry(gH21, "#mu = 2f_{2}/f_{1}", "L");
  l1->AddEntry(gH10, "#mu = f_{1}/f_{0}", "L");
  l1->AddEntry(gH0, "#mu = -ln f_{0}", "L");
  l1->SetFillColor(0);
  l1->SetBorderSize(0);
  l1->Draw();

  c1->cd(2);
  gT21->Draw("ALP");
  gT21->GetYaxis()->SetTitleOffset(1.25);
  gT10->Draw("same");
  gT10->SetLineColor(kRed);
  gT0->Draw("same");
  gT0->SetLineColor(kBlue);
  gT21->SetTitle("Lumi from PLT pixel data, three-plane clusters");

  TLegend *l2 = new TLegend(0.50, 0.65, 0.80, 0.88);
  l2->AddEntry(gT21, "#mu = 2f_{2}/f_{1}", "L");
  l2->AddEntry(gT10, "#mu = f_{1}/f_{0}", "L");
  l2->AddEntry(gT0, "#mu = -ln f_{0}", "L");
  l2->SetFillColor(0);
  l2->SetBorderSize(0);
  l2->Draw();

  c1->cd(3);
  TGraph *gH21n = gH21->Clone("gH21n");
  gH21n->GetYaxis()->SetTitle("#mu (normalized to max=1)");
  TGraph *gH10n = gH10->Clone("gH10n");
  TGraph *gH0n = gH0->Clone("gH0n");

  normalize(gH21n);
  normalize(gH10n);
  normalize(gH0n);
  
  gH21n->Draw("ALP");
  gH10n->Draw("same");
  gH0n->Draw("same");
  l1->Draw();

  c1->cd(4);
  TGraph *gT21n = gT21->Clone("gT21n");
  gT21n->GetYaxis()->SetTitle("#mu (normalized to max=1)");
  TGraph *gT10n = gT10->Clone("gT10n");
  TGraph *gT0n = gT0->Clone("gT0n");

  normalize(gT21n);
  normalize(gT10n);
  normalize(gT0n);

  gT21n->Draw("ALP");
  gT10n->Draw("same");
  gT0n->Draw("same");
  l2->Draw();

  //c1->Print("LumiSlinkFill4246.png");
}
