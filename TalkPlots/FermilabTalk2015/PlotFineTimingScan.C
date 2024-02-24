void PlotFineTimingScan(void) {
  FILE *infile = fopen("FineTimingScanAug23.txt", "r");

  const int nSettings = 8;  // was 14
  const int nChannels = 14; // was 15
  double settingVal[nSettings];
  double settingErr[nSettings];
  double finalRates[nChannels][nSettings];
  double finalErrors[nChannels][nSettings];

  double maxRate = -1;
  double minRate = 9999999;

  int fedSetting, delay;
  char dummy[255];

  int iSetting = -1;
  while (!feof(infile)) {
    fscanf(infile, "%d (%dns):\n\n", &fedSetting, &delay);
    iSetting++;

    settingVal[iSetting] = delay;
    settingErr[iSetting] = 0;

    int orbit;
    ULong64_t data;
    int nOrbits = 0;
    ULong64_t totals[nChannels];
    ULong64_t sum2[nChannels];
    for (int i=0; i<nChannels; ++i) {
      totals[i] = 0;
      sum2[i] = 0;
    }
    int nibble, ls, run;

    // Get the data
    while (1) {
      //int n=fscanf(infile, "Orbit: %d Totals:", &orbit);
      int n=fscanf(infile, "Orbit: %d Nibble: %d LS: %d Run: %d Totals:", &orbit, &nibble, &ls, &run);      
      if (feof(infile)) break; // end of file, let's get out of here!
      if (n==0) break; // end of data for this scan point
      nOrbits++;
      for (int i=0; i<nChannels; ++i) {
	fscanf(infile, "%d", &data);
	totals[i] += data;
	sum2[i] += data*data;
      }
      fscanf(infile, "\n", dummy);
    }

    for (int iCh=0; iCh<nChannels; ++iCh) {
      finalRates[iCh][iSetting] = (double)totals[iCh]/nOrbits;
      finalErrors[iCh][iSetting] = sqrt((double)sum2[iCh]/nOrbits - finalRates[iCh][iSetting]*finalRates[iCh][iSetting]);
      if (finalRates[iCh][iSetting] > maxRate) maxRate = finalRates[iCh][iSetting];
      if (finalRates[iCh][iSetting] < minRate) minRate = finalRates[iCh][iSetting];
    }
    std::cout << "Read " << nOrbits << " data points for scan point " << iSetting << " (delay " << delay << " ns, setting " << fedSetting << ")" << std::endl;

    if (feof(infile)) break;
    fscanf(infile, "\n", dummy);
  }
  fclose(infile);
  
  // Woot, all data read. Now plot it.
  TGraphErrors *gr[nChannels];

  for (int iCh=0; iCh<nChannels; ++iCh) {
    // We really should not need this step, but apparently we do
    double yvals[nSettings];
    double yerrs[nSettings];
    for (int iSet=0; iSet<nSettings; ++iSet) {
      yvals[iSet] = finalRates[iCh][iSet];
      yerrs[iSet] = finalErrors[iCh][iSet];
    }
      
    gr[iCh] = new TGraphErrors(nSettings, settingVal, yvals,
			     settingErr, yerrs);
  }

  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 500);
  c1->Divide(2,1);
  
  c1->cd(1);
  gPad->SetRightMargin(0.32);
  gr[0]->Draw("ALP");
  gr[0]->SetTitle("Timing scan, -z side");
  gr[0]->GetXaxis()->SetTitle("TTCFED delay (ns)");
  gr[0]->GetYaxis()->SetTitle("Triple coincidence rates");
  gr[0]->GetYaxis()->SetTitleOffset(1.55);
  gr[0]->SetMaximum(maxRate*1.05);
  gr[0]->SetMinimum(minRate*0.95);
  for (iCh = 1; iCh<8; ++iCh) {
    gr[iCh]->Draw("LP same");
    gr[iCh]->SetMarkerColor(iCh+1);
    gr[iCh]->SetLineColor(iCh+1);
  }
  TLegend *l1 = new TLegend(0.7, 0.2, 0.98, 0.7);
  for (int i=0; i<8; ++i) {
    sprintf(dummy, "Channel %d", i);
    l1->AddEntry(gr[i], dummy, "LP");
  }
  l1->SetFillColor(0);
  l1->SetBorderSize(0);
  l1->Draw();

  c1->cd(2);
  gPad->SetRightMargin(0.32);
  gr[8]->Draw("ALP");
  gr[8]->SetTitle("Timing scan, +z side");
  gr[8]->GetXaxis()->SetTitle("TTCFED delay (ns)");
  gr[8]->GetYaxis()->SetTitle("Triple coincidence rates");
  gr[8]->GetYaxis()->SetTitleOffset(1.55);
  gr[8]->SetMaximum(maxRate*1.05);
  gr[8]->SetMinimum(minRate*0.95);
  for (iCh = 9; iCh<nChannels; ++iCh) {
    gr[iCh]->Draw("LP same");
    gr[iCh]->SetMarkerColor(iCh-7);
    gr[iCh]->SetLineColor(iCh-7);
  }
  TLegend *l2 = new TLegend(0.7, 0.3, 0.98, 0.7);
  for (int i=8; i<nChannels; ++i) {
    sprintf(dummy, "Channel %d", i);
    l2->AddEntry(gr[i], dummy, "LP");
  }
  l2->SetFillColor(0);
  l2->SetBorderSize(0);
  l2->Draw();
  
  c1->Print("FineTimingScanAug23.png");
}

