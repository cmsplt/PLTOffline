////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Apr 10 17:35:17 CEST 2013
//
////////////////////////////////////////////////////////////////////


#include <iostream>

#include "PLTTesterEvent.h"

#include "TFile.h"
#include "TSystem.h"
#include "TGraphErrors.h"


int TestStandTest (std::string const DataFileName, std::string const GainCalFileName, std::string const OutDir)
{
  std::cout << "Output will be sent to: " << OutDir << std::endl;
  TString const OutRootFileName = OutDir + "/TestOut.root";
  gSystem->mkdir(OutDir.c_str(), true);

  TFile fOutRoot(OutRootFileName, "recreate");
  std::ofstream OutFile("TestOut.txt");


  PLTTesterEvent Event(DataFileName, GainCalFileName, OutDir, &fOutRoot);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_m2_m2);


  // Bins and max for pulse height plots
  int   const NBins =     60;
  float const XMin  =  -1000;
  float const XMax  =  50000;



  TH2I hOccupancy("Occupancy", "Occupancy", 26, 13, 39, 40, 40, 80);
  hOccupancy.SetDirectory(&fOutRoot);


  int const HistColors[4] = { 1, 4, 28, 2 };
  TGraphErrors gPHT[4];
  for (int ig = 0; ig < 4; ++ig) {
    gPHT[ig].Set(0);
    gPHT[ig].SetName( TString::Format("TimeAvgGraph_Cl%i", ig) );
    gPHT[ig].SetMinimum(0);
    gPHT[ig].SetMaximum(30000);
    gPHT[ig].SetMarkerColor(HistColors[ig]);
    gPHT[ig].SetLineColor(HistColors[ig]);
  }
  gPHT[0].SetTitle("Average Pulse Height");
  gPHT[1].SetTitle("Avg PH for 1 Pixel Clusters");
  gPHT[2].SetTitle("Avg PH for 2 Pixel Clusters");
  gPHT[3].SetTitle("Avg PH for #ge 3 Pixel Clusters");

  int const TimeWidth = 2;
  int EndTimeWindow = TimeWidth;

  std::vector<float> vClPH[4];
  vClPH[0].reserve(10000);
  vClPH[1].reserve(10000);
  vClPH[2].reserve(10000);
  vClPH[3].reserve(10000);


  TH1F* hPulseHeight[4];
  hPulseHeight[0] = new TH1F("PulseHeight_All", "Pulse Height for All Clusters", NBins, XMin, XMax);
  hPulseHeight[0]->SetLineColor(HistColors[0]);
  hPulseHeight[0]->SetDirectory(&fOutRoot);
  for (int ih = 1; ih != 4; ++ih) {
    hPulseHeight[ih] = new TH1F( TString::Format("PulseHeight_Pixels%i", ih), TString::Format("Pulse Height for %i Pixel Clusters", ih), NBins, XMin, XMax);
    hPulseHeight[ih]->SetLineColor(HistColors[ih]);
    hPulseHeight[ih]->SetDirectory(&fOutRoot);
  }




  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << " time: " << Event.Time() << std::endl;
    }

    static int const StartTime = Event.Time();
    int const ThisTime = Event.Time() - StartTime;

    if (ThisTime < EndTimeWindow) {
      for (size_t icluster = 0; icluster != Event.NClusters(); ++icluster) {
        PLTCluster* Cluster = Event.Cluster(icluster);
        float const Charge = Cluster->Charge();
        if (Charge > 900000 || Charge < -900000) {
          continue;
        }

        if (Charge == 0) {
          printf("%i %i %i\n", (int) Cluster->NHits(), Cluster->Hit(0)->Column(), Cluster->Hit(0)->Row());
          continue;
        }



        vClPH[0].push_back(Charge);
        hPulseHeight[0]->Fill(Charge);
        Cluster->NHits();
        if (Cluster->NHits() == 1) {
          vClPH[1].push_back(Charge);
          hPulseHeight[1]->Fill(Charge);
        } else if (Cluster->NHits() == 2) {
          vClPH[2].push_back(Charge);
          hPulseHeight[2]->Fill(Charge);
        } else if (Cluster->NHits() >= 3) {
          vClPH[3].push_back(Charge);
          hPulseHeight[3]->Fill(Charge);
        }

      }

    } else {
      for (int ig = 0; ig < 4; ++ig) {
        if (vClPH[ig].size() > 0) {
          int const N = gPHT[ig].GetN();
          gPHT[ig].Set(N+1);
          float const Average = PLTU::Average(vClPH[ig]);
          gPHT[ig].SetPoint(N , EndTimeWindow - TimeWidth / 2, Average);
          gPHT[ig].SetPointError(N, TimeWidth / 2, Average / sqrt((float) vClPH[ig].size()));
          //std::cout << vClPH[ig].size() << std::endl;
          printf("Adding point to graph %i: Time: %7i to %7i seconds.  Agv %9.1f +/- %9.1f\n", ig, EndTimeWindow - TimeWidth, EndTimeWindow, Average,  Average/sqrt((float) vClPH[ig].size()));
          vClPH[ig].clear();
          vClPH[ig].reserve(10000);
        }
      }

      EndTimeWindow += TimeWidth;

    }

    //Event.WriteEventText(OutFile);

    for (size_t ihit = 0; ihit != Event.NHits(); ++ihit) {
      PLTHit* Hit = Event.Hit(ihit);
      hOccupancy.Fill(Hit->Column(), Hit->Row());
    }
  }

  Event.MakePlots();


  for (int ig = 0; ig < 4; ++ig) {
    TString const Name = TString::Format("PHvsTime_%i", ig);
    TCanvas c(Name, Name);
    c.cd();
    gPHT[ig].Draw("Ap");
    gPHT[ig].GetXaxis()->SetTitle("Time (s)");
    gPHT[ig].GetYaxis()->SetTitle("PH (electrons)");
    c.Write();
  }

  TCanvas cPHT("PHVsTime", "PHVsTime");
  gPHT[0].Draw("Ap");
  gPHT[1].Draw("p");
  gPHT[2].Draw("p");
  gPHT[3].Draw("p");
  gPHT[0].GetXaxis()->SetTitle("Time (s)");
  gPHT[0].GetYaxis()->SetTitle("PH (electrons)");
  cPHT.Write();
  cPHT.SaveAs(TString(OutDir) + "/PHVsTime.gif");

  TCanvas cPH("PH", "PH");
  hPulseHeight[0]->Draw("Hist");
  hPulseHeight[1]->Draw("SameHist");
  hPulseHeight[2]->Draw("SameHist");
  hPulseHeight[3]->Draw("SameHist");
  hPulseHeight[0]->GetXaxis()->SetTitle("PH (electrons)");
  cPH.Write();
  cPH.SaveAs(TString(OutDir) + "/PulseHeight.gif");


  fOutRoot.Write();
  fOutRoot.Close();
  OutFile.close();

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName] [OutDir]" << std::endl;
    return 1;
  }

  TestStandTest(argv[1], argv[2], argv[3]);

  return 0;
}
