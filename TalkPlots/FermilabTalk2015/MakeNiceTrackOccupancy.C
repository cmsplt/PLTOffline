void DrawHeader(void) {
  TText *t1 = new TText(0.17, 0.92, "CMS");
  t1->SetNDC(true);
  t1->SetX(0.17);
  t1->SetY(0.92);
  t1->SetTextAlign(11);
  t1->SetTextSize(0.075*0.9);
  t1->SetTextFont(61);
  t1->Draw();

  TText *t2 = new TText(0.32, 0.92, "Preliminary");
  t2->SetNDC(true);
  t2->SetX(0.32);
  t2->SetY(0.92);
  t2->SetTextAlign(11);
  t2->SetTextSize(0.057*0.9);
  t2->SetTextFont(52);
  t2->Draw();

  TText *t3 = new TText(0.83, 0.92, "Fill 3850");
  t3->SetNDC(true);
  t3->SetX(0.83);
  t3->SetY(0.92);
  t3->SetTextAlign(31);
  t3->SetTextSize(0.06*0.9);
  t3->SetTextFont(42);
  t3->Draw();
}

void MakeNiceTrackOccupancy(void) {
  gROOT->LoadMacro("tdrstyle.C");
  setTDRStyle();

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
  gStyle->SetOptStat(0);

  TFile *f = new TFile("histo_track_occupancy_new.root");
  TH2F *h1 = (TH2F*)f->Get("TrackOccupancy_Ch11_ROC0"); // was 5
  TH2F *h2 = (TH2F*)f->Get("TrackOccupancy_Ch11_ROC1");
  TH2F *h3 = (TH2F*)f->Get("TrackOccupancy_Ch11_ROC2");

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 400);
  c1->Divide(3,1);

  c1->cd(1);
  h1->Draw("colz");
  h1->SetTitle("");
  h1->GetXaxis()->SetTitleSize(0.06);
  h1->GetYaxis()->SetTitleSize(0.06);
  h1->GetZaxis()->SetTitleSize(0.06);
  h1->GetXaxis()->SetTitleOffset(0.8);
  h1->GetZaxis()->SetTitleOffset(0.95);
  h1->GetXaxis()->SetTitle("Column (ROC 0)");
  h1->GetYaxis()->SetTitle("Row (ROC 0)");
  h1->GetZaxis()->SetTitle("Number of Hits");
  h1->GetXaxis()->CenterTitle();
  h1->GetYaxis()->CenterTitle();
  h1->GetZaxis()->CenterTitle();
  DrawHeader();

  c1->cd(2);
  h2->Draw("colz");
  h2->SetTitle("");
  h2->GetXaxis()->SetTitleSize(0.06);
  h2->GetYaxis()->SetTitleSize(0.06);
  h2->GetZaxis()->SetTitleSize(0.06);
  h2->GetXaxis()->SetTitleOffset(0.8);
  h2->GetZaxis()->SetTitleOffset(0.95);
  h2->GetXaxis()->SetTitle("Column (ROC 1)");
  h2->GetYaxis()->SetTitle("Row (ROC 1)");
  h2->GetZaxis()->SetTitle("Number of Hits");
  h2->GetXaxis()->CenterTitle();
  h2->GetYaxis()->CenterTitle();
  h2->GetZaxis()->CenterTitle();
  DrawHeader();

  c1->cd(3);
  h3->Draw("colz");
  h3->SetTitle("");
  h3->GetXaxis()->SetTitleSize(0.06);
  h3->GetYaxis()->SetTitleSize(0.06);
  h3->GetZaxis()->SetTitleSize(0.06);
  h3->GetXaxis()->SetTitleOffset(0.8);
  h3->GetZaxis()->SetTitleOffset(0.95);
  h3->GetXaxis()->SetTitle("Column (ROC 2)");
  h3->GetYaxis()->SetTitle("Row (ROC 2)");
  h3->GetZaxis()->SetTitle("Number of Hits");
  h3->GetXaxis()->CenterTitle();
  h3->GetYaxis()->CenterTitle();
  h3->GetZaxis()->CenterTitle();
  DrawHeader();

  c1->Print("trackoccupancies3850.png");
  c1->Print("trackoccupancies3850.pdf");
  c1->Print("trackoccupancies3850.eps");
}
