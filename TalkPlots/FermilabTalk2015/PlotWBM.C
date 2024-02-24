//
// File: ConditionBrowser_1435569109207.C
//
// run this script
// root -b -q ConditionBrowser_1435569109207.C
//

char NAME[200];
Int_t NROW;
Double_t WEIGHT[20000];
Int_t TIME[20000];
Double_t TIMEd[20000];
Double_t VALUE[20000];
Double_t VALUEs[20000];

//void ConditionBrowser_1435569109207()
void PlotWBM()
{

  gROOT->LoadMacro("tdrstyle.C");
  setTDRStyle();
  gStyle->SetPadTopMargin(0.10);
  gStyle->SetPadRightMargin(0.08);

  TFile f("ConditionBrowser_1435569109207.root");
  TGraph *gr[3];
  TTree *t = (TTree*)f.Get("tree");
  t->SetBranchAddress("NAME", &NAME);
  t->SetBranchAddress("NROW", &NROW);
  t->SetBranchAddress("WEIGHT", WEIGHT);
  t->SetBranchAddress("TIME", TIME);
  t->SetBranchAddress("VALUE", VALUE);
  Int_t nentries = (Int_t)t->GetEntries();
  for (Int_t i=0; i<nentries; i++)
    {
      t->GetEntry(i);
      cout << NAME << endl;
      cout << NROW << endl;
      //      cout << "ROW \tWEIGHT \tTIME \tVALUE" << endl;
      Double_t maxVal = -1;
      for (Int_t j=0; j<NROW; ++j) {
	TIMEd[j] = TIME[j];
	if (VALUE[j] > maxVal) maxVal = VALUE[j];
      }
      for (Int_t j=0; j<NROW; ++j) {
	VALUEs[j] = VALUE[j]/maxVal;
      }
      // for (Int_t j=0; j<NROW; j++)
      // 	{
      // 	  cout << (j+1) << " \t"
      // 	       << WEIGHT[j] << " \t"
      // 	    //<< SecUTC(TIME[j]) << " \t"
      // 	       << TIME[j] << " \t"
      // 	       << VALUE[j] << endl;
      // 	}
      gr[i] = new TGraph(NROW, TIMEd, VALUEs);
    }
  f.Close();
  TCanvas *c = new TCanvas("c1", "Beam optimization", 600, 600);
  gr[0]->Draw("ALP");
  gr[0]->GetXaxis()->SetTitle("Time (GMT)");
  gr[0]->GetYaxis()->SetTitle("Rate (a.u.)");
  gr[0]->SetTitle("");
  gr[0]->GetXaxis()->SetTimeDisplay(1);
  gr[0]->GetXaxis()->SetNdivisions(-503);
  gr[0]->GetXaxis()->SetTimeFormat("%H:%M");
  gr[0]->GetXaxis()->SetTimeOffset(0, "gmt");
  gr[0]->SetLineColor(kRed);
  gr[0]->GetXaxis()->SetTitleSize(0.06);
  gr[0]->GetYaxis()->SetTitleSize(0.06);
  gr[0]->GetXaxis()->SetLabelOffset(0.015);
  gr[0]->GetXaxis()->SetTitleOffset(1.02);
  gr[0]->SetMarkerStyle(0);
  gr[0]->SetMarkerColor(kRed);
  gr[0]->SetFillColor(0);
  gr[0]->SetLineWidth(2);
  
  gr[1]->Draw("LP same");
  gr[1]->SetLineColor(kGreen);
  gr[1]->SetMarkerStyle(0);
  gr[1]->SetMarkerColor(kGreen);
  gr[1]->SetFillColor(0);
  gr[1]->SetLineWidth(2);

  gr[2]->Draw("LP same");
  gr[2]->SetLineColor(kBlue);
  gr[2]->SetMarkerStyle(0);
  gr[2]->SetMarkerColor(kBlue);
  gr[2]->SetFillColor(0);
  gr[2]->SetLineWidth(2);
  
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

  TText *t3 = new TText(0.83, 0.92, "Fill 3805");
  t3->SetNDC(true);
  t3->SetX(0.83);
  t3->SetY(0.92);
  t3->SetTextAlign(31);
  t3->SetTextSize(0.06*0.9);
  t3->SetTextFont(42);
  t3->Draw();

  TLegend *l = new TLegend(0.8, 0.47, 0.99, 0.7);
  l->AddEntry(gr[0], "BCM1F");
  l->AddEntry(gr[1], "HF");
  l->AddEntry(gr[2], "PLT");
  l->SetFillColor(0);
  l->SetBorderSize(1);
  l->Draw();
  c->Print("BeamOptimization.png");
  c->Print("BeamOptimization.pdf");
  c->Print("BeamOptimization.eps");
}

TString SecUTC(Int_t sec)
{
  TTimeStamp ts(sec, 0);
  TString s = ts.AsString("c");
  return s(0, 4) + "." +
    s(5, 2) + "." +
    s(8, 2) + " " +
    s(11, 2) + ":" +
    s(14, 2) + ":" +
    s(17, 2);
}
