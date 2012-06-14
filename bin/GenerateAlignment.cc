////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Mar 16 12:43:08 CET 2012
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH1F.h"
#include "TF1.h"



int GenerateAlignment (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_Diamond);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_Diamond);

  // Additional Input alignment Obj
  PLTAlignment InAlignment;
  InAlignment.ReadAlignmentFile(AlignmentFileName);

  PLTAlignment NewAlignment;
  NewAlignment.ReadAlignmentFile(AlignmentFileName);


  // Track.. the find for this track

  std::map<int, TH1F*> MapCol;
  std::map<int, TH1F*> MapRow;

  std::map<int, TH2F*> Map2DCol;
  std::map<int, TH2F*> Map2DRow;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      if (Telescope->NTracks() == 1 && Telescope->NClusters() == 3) {

        // Grab the track
        PLTTrack* Track = Telescope->Track(0);

        // Cluster from plane0
        PLTCluster* Cluster0 = Telescope->Plane(0)->Cluster(0);

        for (size_t ip = 1; ip != Telescope->NPlanes(); ++ip) {
          PLTPlane*   Plane   = Telescope->Plane(ip);
          PLTCluster* Cluster = Plane->Cluster(0);

          int const Channel = Telescope->Channel();
          int const ROC     = ip;
          int const ID = 10 * Channel + ROC;

          if (!Map2DCol.count(ID)) {
            TString const Name = TString::Format("DeltaYforX_Channel%i_ROC%i", Channel, ROC);
            Map2DCol[ID] = new TH2F(Name, Name, 100, -0.2, 0.2, 100, -0.05, 0.05);
            Map2DCol[ID]->SetXTitle("Cluster Position in X");
            Map2DCol[ID]->SetYTitle("Track Residual #DeltaY");
          }
          if (!Map2DRow.count(ID)) {
            TString const Name = TString::Format("DeltaXforrY_Channel%i_ROC%i", Channel, ROC);
            Map2DRow[ID] = new TH2F(Name, Name, 100, -0.2, 0.2, 100, -0.05, 0.05);
            Map2DRow[ID]->SetXTitle("Cluster Position in Y");
            Map2DRow[ID]->SetYTitle("Track Residual #DeltaX");
          }


          Map2DCol[ID]->Fill(Cluster0->LX(), Cluster0->LY() - Cluster->LY());
          Map2DRow[ID]->Fill(Cluster0->LY(), Cluster0->LX() - Cluster->LX());

        }


      }


    }
  }



  std::map<int, float> FitAngleCol;
  for (std::map<int, TH2F*>::iterator it = Map2DCol.begin(); it != Map2DCol.end(); ++it) {
    int ID      = it->first;
    int Channel = it->first / 10;
    int ROC     = it->first % 10;

    TH2F* Hist = it->second;


    TCanvas Can;
    Can.cd();
    Hist->Draw("colz");
    Can.SaveAs(Hist->GetName() + TString(".gif"));

    // Graph the averages
    TGraph Graph(Hist->GetNbinsX());

    // Loop over the xbins and grab projection and mean in Y
    for (int ibin = 1; ibin <= Hist->GetNbinsX(); ++ibin) {
      if (Hist->ProjectionY("_py", ibin, ibin)->GetEntries() != 0) {
        std::cout << "Entries: " << Hist->ProjectionY("_py", ibin, ibin)->GetEntries() << "   " << Hist->GetXaxis()->GetBinCenter(ibin) << "  "<< Hist->ProjectionY("_py", ibin, ibin)->GetMean()<< std::endl;
        Graph.SetPoint(ibin - 1, Hist->GetXaxis()->GetBinCenter(ibin), Hist->ProjectionY("_py", ibin, ibin)->GetMean());
      }
    }

    Can.Clear();
    Can.cd();
    Graph.Fit("pol1");
    Graph.Draw("A*");
    Can.SaveAs(Hist->GetName() + TString("_Fit.gif"));

    TF1* Func = Graph.GetFunction("pol1");
    float const FitOffset = Func->GetParameter(0);
    float const FitAngle  = atan(Func->GetParameter(1));
    printf("Fit parameters Offset: %15E  Angle: %15E\n", FitOffset, FitAngle);

    PLTAlignment::CP* C = NewAlignment.GetCP(Channel, ROC);
    std::cout << Channel << "  " << ROC << std::endl;
    C->LR += FitAngle;
    //C->LY -= FitOffset;

    FitAngleCol[ID] = FitAngle;

  }

  for (std::map<int, TH2F*>::iterator it = Map2DRow.begin(); it != Map2DRow.end(); ++it) {
    int ID      = it->first;
    int Channel = it->first / 10;
    int ROC     = it->first % 10;

    TH2F* Hist = it->second;


    TCanvas Can;
    Can.cd();
    Hist->Draw("colz");
    Can.SaveAs(Hist->GetName() + TString(".gif"));

    // Graph the averages
    TGraph Graph(Hist->GetNbinsY());

    // Loop over the xbins and grab projection and mean in Y
    for (int ibin = 1; ibin <= Hist->GetNbinsX(); ++ibin) {
      if (Hist->ProjectionY("_py", ibin, ibin)->GetEntries() != 0) {
        Graph.SetPoint(ibin - 1, Hist->GetXaxis()->GetBinCenter(ibin), Hist->ProjectionY("_py", ibin, ibin)->GetMean());
      }
    }

    Can.Clear();
    Can.cd();
    Graph.Fit("pol1");
    Graph.Draw("A*");
    Can.SaveAs(Hist->GetName() + TString("_Fit.gif"));

    TF1* Func = Graph.GetFunction("pol1");
    float const FitOffset = Func->GetParameter(0);
    float const FitAngle  = atan(Func->GetParameter(1));
    printf("Fit parameters Offset: %15E  Angle: %15E\n", FitOffset, FitAngle);

    PLTAlignment::CP* C = NewAlignment.GetCP(Channel, ROC);
    //C->LR += FitAngle;
    //C->LX -= FitOffset;

    std::cout << "Diff: " << FitAngle - FitAngleCol[ID] << std::endl;

  }


  NewAlignment.WriteAlignmentFile("TestAlign.dat");

  exit(0);

  TH1F hist("hist", "hist", 26, 13, 39);
  TGraph Graph(PLTU::LASTCOL - PLTU::FIRSTCOL);
  for (std::map<int, TH1F*>::iterator It = MapCol.begin(); It != MapCol.end(); ++It) {

    int const Channel = It->first / 100;
    int const Column  = It->first % 100;
    TH1F* Hist = It->second;

    printf("MapCol: %4i %12.3E\n", It->first, Hist->GetMean());


    //Hist->Fit("gaus");
    //float const GausMean = Hist->GetFunction("gaus")->GetParameter(1);

    Graph.SetPoint(Column - PLTU::FIRSTCOL, InAlignment.PXtoLX(Column), Hist->GetMean());
    //Graph.SetPoint(Column - PLTU::FIRSTCOL, InAlignment.PXtoLX(Column), GausMean);
    hist.SetBinContent(hist.FindBin(Column), Hist->GetMean());
    //hist.SetBinContent(hist.FindBin(Column), GausMean);


    TCanvas Can;
    Can.cd();
    Hist->Draw();
    Can.SaveAs( TString::Format("ByCol%i.gif", It->first) );
  }
  TCanvas Can;
  Can.cd();
  //hist.Draw("hist");
  //hist.Fit("pol1");
  Graph.Fit("pol1");
  Graph.Draw("A*");
  Can.SaveAs("ByColAvg.gif");

  float Angle = atan( Graph.GetFunction("pol1")->GetParameter(1) );
  printf("Correction angle(rad): %15.6E\n", Angle);


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat] [AlignmentFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];

  GenerateAlignment(DataFileName, GainCalFileName, AlignmentFileName);


  return 0;
}
