////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Sun Jun 19 17:39:24 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cmath>
#include <cstdlib>


#include "TString.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFitResult.h"
#include "TCanvas.h"

#include "PLTU.h"


int const MINCOL = 13;
int const MAXCOL = 38;
int const MINROW = 39;
int const MAXROW = 77;

int const NCOL = MAXCOL - MINCOL + 1;
int const NROW = MAXROW - MINROW + 1;

struct B
{
  float Eff[NCOL][NROW];
  int   Trim[NCOL][NROW];
  bool  Checked[NCOL][NROW];
};


int FindTrims (std::string const InFileName)
{
  // Open file
  std::ifstream InFile(InFileName.c_str());
  if (!InFile.is_open()) {
    std::cerr << "ERROR: cannot open file: " << InFileName << std::endl;
    throw;
  }


  std::map<int, B> STrim;

  int mFec, mFecChannel, hubAddress, Col, Row, ROC, VCal, NFired, itrim;
  float Efficiency;
  int ROCID;

  while (!InFile.eof()) {
    InFile >> mFec
           >> mFecChannel
           >> hubAddress
           >> Col
           >> Row
           >> ROC
           >> VCal
           >> itrim
           >> NFired
           >> Efficiency;
    //printf("%i %i %i %i %i %i %i %i %f\n" , mFec, mFecChannel, hubAddress, Col, Row, ROC, VCal, itrim, Efficiency);
    ROCID = 10000 * mFec + 1000 * mFecChannel + 10 * hubAddress + ROC;
    STrim[ROCID].Checked[Col - MINCOL][Row - MINROW] = true;
    if (fabs(0.5 - Efficiency)*100 < fabs(50 - STrim[ROCID].Eff[Col - MINCOL][Row - MINROW])) {
      if (Efficiency < 0.5){std::cout << "Efficiency = " << 100*Efficiency << " and STrim.Eff " << STrim[ROCID].Eff[Col - MINCOL][Row - MINROW]<< std::endl;}
      if (Col - MINCOL < 0 || Row - MINROW < 0) {
        std::cout << Col << "  " << Row << std::endl;
        std::cerr << "ERROR!!" << std::endl;
        exit(1);
      }
      STrim[ROCID].Eff[Col - MINCOL][Row - MINROW]  = 100*Efficiency;
      STrim[ROCID].Trim[Col - MINCOL][Row - MINROW] = itrim;
    }
  }



  //  std::cout << "STrim.begin() = " << STrim.begin() << " and STrim.end() = " << STrim.end() << std::endl;
  //  exit(0);
  for (std::map<int, B>::iterator It = STrim.begin(); It != STrim.end(); ++It) {
    char BUFF_EFFIC[400];
    char BUFF[400];
    mFec        = It->first / 10000;
    mFecChannel = It->first % 10000 / 1000;
    hubAddress  = It->first % 1000 / 10;
    ROC         = It->first % 10;


    //    printf("%i  mFec = %i  mFecChannel = %i  hubAddress = %i  ROC = %i\n", It->first, mFec, mFecChannel, hubAddress, ROC);

    sprintf(BUFF, "fasttrim_values_mFec%i_mFecChannel%i_hubAddress%i_roc%i.pix1", mFec, mFecChannel, hubAddress, ROC);
    sprintf(BUFF_EFFIC, "fasttrim_effic_mFec%i_mFecChannel%i_hubAddress%i_roc%i.txt", mFec, mFecChannel, hubAddress, ROC);


    FILE* ftrim = fopen(BUFF, "w");
    if (!ftrim) {
      std::cerr << "ERROR: cannot open output file: " << BUFF << std::endl;
      throw;
    }
    FILE* feff = fopen(BUFF_EFFIC, "w");
    if (!feff) {
      std::cerr << "ERROR: cannot open output file: " << BUFF_EFFIC << std::endl;
      throw;
    }

    TString Name;
    Name = TString::Format("Trims_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, ROC);
    TH1F * htrim = new TH1F (Name, Name,16,0,16);

    Name = TString::Format("Trims_2x2_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, ROC);
    TH2F * htrim_2x2 = new TH2F (Name, Name,40,5,45,60,30,90);

    Name = TString::Format("Effic_2x2_mF%1i_mFC%1i_hub%02i_ROC%1i", mFec, mFecChannel, hubAddress, ROC);
    TH2F * heff_2x2 = new TH2F (Name, Name,40,5,45,60,30,90);

    fprintf(ftrim, "5 0 0 0 0 0 0 0 0 0\n");
    for (int icol = 0; icol != 26; ++icol) {
      fprintf(ftrim, "%i\n", icol <= 5 || icol >= 20 ? 0 : 1);
    }
    for (int icol = 0; icol != NCOL; ++icol) {
      for (int irow = 0; irow != NROW; ++irow) {
        fprintf(ftrim, "%2i %2i 1 0 %2i\n", MINCOL + icol, MINROW + irow,
                It->second.Checked[icol][irow] ? It->second.Trim[icol][irow] : 8);
	htrim->Fill(It->second.Trim[icol][irow]);
	htrim_2x2->Fill(MINCOL+icol,MINROW +irow,It->second.Trim[icol][irow]+0.1);
        fprintf(feff,"%2i %2i %2i %9.5f 50.0 %2i\n", ROC, MINCOL + icol, MINROW + irow, It->second.Eff[icol][irow], It->second.Trim[icol][irow]);
	heff_2x2->Fill(MINCOL+icol,MINROW +irow,It->second.Eff[icol][irow]+0.1);
      }
    }

    fclose(ftrim);
    fclose(feff);

    int min = 0;
    int emax = 100;
    int tmax = 16;

    htrim->GetXaxis()->SetTitle("Trim Value");
    htrim->GetYaxis()->SetTitle("# Hits");

    htrim_2x2->GetXaxis()->SetTitle("Col");
    htrim_2x2->GetYaxis()->SetTitle("Row");
    htrim_2x2->GetZaxis()->SetTitle("Trim Value");
    htrim_2x2->SetMinimum(min);
    htrim_2x2->SetMaximum(tmax);

    heff_2x2->GetXaxis()->SetTitle("Col");
    heff_2x2->GetYaxis()->SetTitle("Row");
    heff_2x2->GetZaxis()->SetTitle("Efficiency");
    heff_2x2->SetMinimum(min);
    heff_2x2->SetMaximum(emax);


    TCanvas can;
    can.SetWindowSize(990,400);
    can.SetCanvasSize(990,400);
    can.Divide(3);
    can.cd(1);
    htrim->Draw();
    htrim->Fit("gaus");

    can.cd(2);
    htrim_2x2->Draw("colz");

    can.cd(3);
    heff_2x2->Draw("colz");
    TString Title;
    Title =  TString::Format("fasttrim_effic_mFec%1i_mFecChannel%1i_hubAddress%02i_roc%1i.png", mFec, mFecChannel, hubAddress, ROC);
    can.Print(Title);

  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [InFile]" << std::endl;
    return 1;
  }

  std::string const InFileName = argv[1];


  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetLabelFont(42,"XYZ");
  gStyle->SetLabelSize(0.03,"XYZ");
  gStyle->SetTextFont(42);
  gStyle->SetTextSize(0.07);
  //gStyle->SetTitleFont(42,"t");
  gStyle->SetTitleFontSize(0.06);
  gStyle->SetTitleX(0.15f);
  gStyle->SetTitleY(0.97f);
  //gStyle->SetTitleFont(42,"XYZ");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetCanvasDefW(990);
  gStyle->SetCanvasDefW(400);
  gStyle->SetPadGridX(1);
  gStyle->SetPadGridY(1);



  FindTrims(InFileName);

  return 0;
}
