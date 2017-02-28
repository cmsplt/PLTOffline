////////////////////////////////////////////////////////////////////
//
// RawADCCounts
//
//  A very crude utility to plot the raw ADC counts for the pulse
//  height values given an input file. Inelegantly adapted from
//  OccupancyPlots.
//
//  Paul Lujan, May 2016
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <numeric>

#include "PLTEvent.h"
#include "PLTU.h"


#include "TFile.h"
#include "TProfile2D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"

// FUNCTION DEFINITIONS HERE

// CONSTANTS HERE

// CODE BELOW

int RawADCValues (std::string const DataFileName) {
  // Set some basic style
  PLTU::SetStyle();

  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName);
  //Event.SetPlaneClustering(PLTPlane::kClustering_NoClustering);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_All;
  //  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);

  // Map for all ROC hists and canvases
  std::map<int, TCanvas*>    cADCMap;
  std::map<int, TProfile2D*> hADCMap;

  // char buffer for writing names
  char buff[200];
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 500000 == 0) {
      int nsec = Event.Time()/1000;
      int hr = nsec/3600;
      int min = (nsec-(hr*3600))/60;
      int sec = nsec % 60;
      std::cout << "Processing event: " << ientry << " at " << std::setfill('0') << std::setw(2)
		<< hr << ":" << std::setw(2) << min << ":" << std::setw(2) << sec << "."
		<< std::setw(3) << Event.Time()%1000 << std::endl;
    }

    if (ientry > 3000000) {
      std::cout << "Reached maximum number of events; exiting" << std::endl;
      break;
    }

    // Loop over all planes with hits in event
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {

      // THIS plane is
      PLTPlane* Plane = Event.Plane(ip);
      if (Plane->ROC() > 2) {
        std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->ROC() < 0) {
        std::cerr << "WARNING: ROC < 0 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->Channel() > 99) {
        std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
        continue;
      }


      for (size_t icl = 0; icl != Plane->NClusters(); ++icl) {
        PLTCluster* Cluster = Plane->Cluster(icl);

        // Loop over all hits on this plane
        for (size_t ihit = 0; ihit != Cluster->NHits(); ++ihit) {

          // THIS hit is
          PLTHit* Hit = Cluster->Hit(ihit);
          //printf("Channel ROC Row Col ADC: %2i %1i %2i %2i %4i %12i\n", Hit->Channel(), Hit->ROC(), Hit->Row(), Hit->Column(), Hit->ADC(), Event.EventNumber());

          // ID the plane and roc by 3 digit number
          int const id = 10 * Plane->Channel() + Plane->ROC();

          // If the hist doesn't exist yet we have to make it
          if (hADCMap.count(id) == 0) {

            // Create new hist with the given name
            sprintf(buff, "Average ADC Counts Ch%02i ROC%1i", Plane->Channel(), Plane->ROC());
            hADCMap[id] = new TProfile2D(buff, buff, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL+1, PLTU::NROW,PLTU::FIRSTROW, PLTU::LASTROW+1);
            hADCMap[id]->SetXTitle("Column");
            hADCMap[id]->SetYTitle("Row");
            hADCMap[id]->SetZTitle("Average ADC Counts");
            hADCMap[id]->GetXaxis()->CenterTitle();
            hADCMap[id]->GetYaxis()->CenterTitle();
            hADCMap[id]->GetZaxis()->CenterTitle();
            hADCMap[id]->SetTitleOffset(1.2, "y");
            hADCMap[id]->SetTitleOffset(1.4, "z");
            hADCMap[id]->SetFillColor(40); // We need this for projections later
            hADCMap[id]->SetStats(false);

	    // Ditto for the canvas
            if (!cADCMap.count(Plane->Channel())) {
              sprintf(buff, "ADC Counts Ch%02i", Plane->Channel());
//               cADCMap[Plane->Channel()] = new TCanvas(buff, buff, 1200, 1200);
//               cADCMap[Plane->Channel()]->Divide(3,3);
              cADCMap[Plane->Channel()] = new TCanvas(buff, buff, 1200, 400);
              cADCMap[Plane->Channel()]->Divide(3,1);
	    } // new canvas
          } // new histo

          // Fill this histogram with the given id
          hADCMap[id]->Fill(Hit->Column(), Hit->Row(), Hit->ADC());
        }
      }
    }

  }

  std::cout << "Done reading events.  Will make some plots now" << std::endl;

  // Loop over all histograms and draw them on the correct canvas in the correct pad
  TFile *f = new TFile("histo_rawadc.root","RECREATE");

  for (std::map<int, TProfile2D*>::iterator it = hADCMap.begin(); it != hADCMap.end(); ++it) {
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hist for Channel %2i ROC %i\n", Channel, ROC);

    // Draw the 2D and 1D distribution on occupancy canvas
    cADCMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");
    it->second->SetMinimum(it->second->GetMaximum()-20);

    hADCMap[id]->Write();
    
  }
  f->Write();

  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cADCMap.begin(); it != cADCMap.end(); ++it) {
    it->second->SaveAs(TString::Format("plots/ADCCounts_Ch%02i.gif", it->first));
    delete it->second;
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  RawADCValues(DataFileName);

  return 0;
}
