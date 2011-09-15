////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Sep 13 17:08:10 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TPaveLabel.h"

int PlotEvent (std::string const DataFileName, int const Channel, int const ROC, std::string const GainCalFileName = "")
{
  // Set some basic style
  PLTU::SetStyle();
  gStyle->SetOptStat(0);

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    // Loop over all planes with hits in event
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {

      // THIS plane is
      PLTPlane* Plane = Event.Plane(ip);
      if (Plane->ROC() == ROC) {
        TString const Name = TString::Format("Event_%i_Ch%i_ROC%i", ientry, Channel, ROC);
        TCanvas Can;
        Can.cd();
        TH2F h2D(Name, Name+";Column;Row", PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL);
        // Loop over all hits on this plane
        for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {

          // THIS hit is
          PLTHit* Hit = Plane->Hit(ihit);
          h2D.Fill(Hit->Row(), Hit->Column());
          std::cout << Hit->ADC() << std::endl;
        }
        h2D.Draw("colz");
        TPaveLabel Label;
        Label.SetLabel(TString::Format("Hits: %i  Clusters: %i", (int) Plane->NHits(), (int) Plane->NClusters()));
        Label.SetX1NDC(0.51);
        Label.SetX2NDC(0.70);
        Label.SetY1NDC(0.918);
        Label.SetY2NDC(0.956);
        Label.SetFillColor(0);
        //Label.SetTextSize();
        Label.SetTextSize(1.31);
        Label.Draw("same");


        Can.SaveAs(Name+".eps");
        //std::cout << Name << "  " << Plane->NHits() << " " << Plane->NClusters() << std::endl;

      }
    }

  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [Channel] [ROC]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  int const Channel = atoi(argv[2]);
  int const ROC = atoi(argv[3]);
  std::string const GainCalFileName = argc == 5 ? argv[4] : "";

  PlotEvent(DataFileName, Channel, ROC, GainCalFileName);

  return 0;
}
