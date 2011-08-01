////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu May 26 16:47:32 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"




// FUNCTION DEFINITIONS HERE
int PulseHeights (std::string const, std::string const);






// CODE BELOW




int PulseHeights (std::string const DataFileName, std::string const GainCalFileName)
{

  int const HistColors[4] = { 1, 4, 28, 2 };


  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);
  //Event.GetGainCal()->PrintGainCal5();
  //exit(0);

  // Map for all ROC hists and canvas
  std::map<int, TH1F*>    hClusterSizeMap;
  std::map<int, TCanvas*> cClusterSizeMap;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    for (size_t iTelescope = 0; iTelescope != Event.NTelescopes(); ++iTelescope) {
      PLTTelescope* Telescope = Event.Telescope(iTelescope);
      for (size_t iPlane = 0; iPlane != Telescope->NPlanes(); ++iPlane) {
        PLTPlane* Plane = Telescope->Plane(iPlane);

        int Channel = Plane->Channel();
        int ROC = Plane->ROC();

        if (ROC > 2) {
          std::cerr << "WARNING: ROC > 2 found: " << ROC << std::endl;
          continue;
        }
        if (Channel > 99) {
          std::cerr << "WARNING: Channel > 99 found: " << Channel << std::endl;
          continue;
        }

        // ID the plane and roc by 3 digit number
        int const id = 10 * Channel + ROC;

        if (!hMap.count(id)) {
          hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ROC),
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", Channel, ROC), 1000, 0, 65000) );
          for (size_t ih = 1; ih != 4; ++ih) {
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                   TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih), 1000, 0, 65000) );
          }

          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Channel)) {
            // Create canvas with given name
            TString BUFF;
            BUFF.Form("PulseHeight_Ch%02i", Channel);
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cMap[Channel] = new TCanvas(BUFF, BUFF, 900, 300);
            cMap[Channel]->Divide(3, 1);
          }


        }

        // If this id doesn't exist in the cluster size map, make the hist and possibly canvas for this channel
        if (!hClusterSizeMap.count(id)) {
          hClusterSizeMap[id] = new TH1F( TString::Format("ClusterSize_Ch%02i_ROC%i", Channel, ROC), TString::Format("ClusterSize_Ch%02i_ROC%i", Channel, ROC), 10, 0, 10);
          hClusterSizeMap[id]->SetXTitle("Number of pixels in Cluster");

          // One in three chance you'll need a new canvas for thnat =)
          if (!cClusterSizeMap.count(Channel)) {
            cClusterSizeMap[Channel] = new TCanvas( TString::Format("ClusterSize_Ch%02i", Channel), TString::Format("ClusterSize_Ch%02i", Channel), 900, 300);
            cClusterSizeMap[Channel]->Divide(3, 1);
          }
        }


        // Loop over all clusters on this plane
        for (size_t iCluster = 0; iCluster != Plane->NClusters(); ++iCluster) {
          PLTCluster* Cluster = Plane->Cluster(iCluster);

          // Get number of hits in this cluster
          size_t NHits = Cluster->NHits();

          // Fill cluster size
          hClusterSizeMap[id]->Fill(NHits);

          //printf("Ch %2i  ROC %1i  col %2i  row %2i  adc %3i charge %12.1f\n",
          //    Cluster->Channel(), Cluster->ROC(), Cluster->SeedHit()->Column(), Cluster->SeedHit()->Row(), Cluster->SeedHit()->ADC(),
          //    Cluster->Charge());

          hMap[id][0]->Fill( Cluster->Charge() );
          if (NHits == 1) {
            hMap[id][1]->Fill( Cluster->Charge() );
          } else if (NHits == 2) {
            hMap[id][2]->Fill( Cluster->Charge() );
          } else if (NHits == 3) {
            hMap[id][3]->Fill( Cluster->Charge() );
          }
        }



      }
    }
  }


  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, std::vector<TH1F*> >::iterator it = hMap.begin(); it != hMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+1);

    for (size_t ih = 0; ih != 4; ++ih) {
      TH1F* Hist = it->second[ih];

      Hist->SetLineColor(HistColors[ih]);
      if (ih == 0) {
        Hist->SetTitle( TString::Format("PulseHeight Ch%02i ROC%1i", Channel, ROC) );
        Hist->SetXTitle("Electrons");
        Hist->SetYTitle("Events");
        Hist->Draw("hist");
      } else {
        Hist->Draw("samehist");
      }
    }

  }

  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    std::cout << it->second->GetName() << std::endl;
    it->second->SaveAs( it->second->GetName()+TString(".gif") );
    delete it->second;
  }
  for (std::map<int, TH1F*>::iterator it = hClusterSizeMap.begin(); it != hClusterSizeMap.end(); ++it) {
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    cClusterSizeMap[Channel]->cd(ROC+1)->SetLogy(1);
    it->second->Draw("hist");
  }


  for (std::map<int, TCanvas*>::iterator it = cClusterSizeMap.begin(); it != cClusterSizeMap.end(); ++it) {
    it->second->SaveAs( it->second->GetName()+TString(".gif") );
    delete it->second;
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;

  PulseHeights(DataFileName, GainCalFileName);

  return 0;
}
