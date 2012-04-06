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
#include "PLTU.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"




// FUNCTION DEFINITIONS HERE
int PulseHeights (std::string const, std::string const);



float Average (std::vector<float>& V)
{
  double Sum = 0;
  for (std::vector<float>::iterator it = V.begin(); it != V.end(); ++it) {
    Sum += *it;
  }

  return Sum / (float) V.size();
}



// CODE BELOW




int PulseHeights (std::string const DataFileName, std::string const GainCalFileName)
{
  PLTU::SetStyle();
  gStyle->SetOptStat(111111);

  int const HistColors[4] = { 1, 4, 28, 2 };


  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_Seed_5x5, PLTPlane::kFiducialRegion_m5_m5);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  //  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);

  // Map for all ROC hists and canvas
  std::map<int, std::vector< std::vector<float> > > vClEnTimeMap;
  std::map<int, std::vector<TH1F*> > hClEnTimeMap;
  std::map<int, TCanvas*> cClEnTimeMap;
  std::map<int, TH1F*>    hClusterSizeMap;
  std::map<int, TCanvas*> cClusterSizeMap;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;

  // Bins and max for pulse height plots
  int   const NBins =    50;
  float const XMax  = 50000;

  // Time width in events for energy time dep plots
  int const TimeWidth = 100;
  std::map<int, std::vector< std::vector<float> > > ChargeHits;


  // Loop over all events in file
  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }

    int const TimeBinNumber = ientry / TimeWidth;
    if (ientry % TimeWidth == 0) {
      for (std::map<int, std::vector< std::vector<float> > >::iterator it = vClEnTimeMap.begin(); it != vClEnTimeMap.end(); ++it) {
        int const id = it->first;

        // extend each hist
        for (int iTimeHist = 0; iTimeHist < 4; ++iTimeHist) {
          it->second[iTimeHist].push_back(0);
        }
      }
    }

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
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", Channel, ROC), NBins, 0, XMax) );
          for (size_t ih = 1; ih != 4; ++ih) {
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                   TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih), NBins, 0, XMax) );
          }

          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Channel)) {
            // Create canvas with given name
            cMap[Channel] = new TCanvas( TString::Format("PulseHeight_Ch%02i", Channel), TString::Format("PulseHeight_Ch%02i", Channel), 900, 600);
            cMap[Channel]->Divide(3, 2);
          }
        }


        if (!vClEnTimeMap.count(id)) {
          vClEnTimeMap[id].resize(4);
          vClEnTimeMap[id][0].resize(ientry / TimeWidth + 1, 0);
          vClEnTimeMap[id][1].resize(ientry / TimeWidth + 1, 0);
          vClEnTimeMap[id][2].resize(ientry / TimeWidth + 1, 0);
          vClEnTimeMap[id][3].resize(ientry / TimeWidth + 1, 0);
          ChargeHits[id].resize(4);


          // One in three chance you'll need a new canvas for thnat =)
          if (!cClEnTimeMap.count(id)) {
            cClEnTimeMap[id] = new TCanvas( TString::Format("PulseHeightTime_Ch%02i_ROC%i", Channel, ROC), TString::Format("PulseHeightTime_Ch%02i_ROC%i", Channel, ROC), 900, 300);
            cClEnTimeMap[id]->Divide(3, 1);
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
          //if (iCluster == 1) break;

          // Get number of hits in this cluster
          size_t NHits = Cluster->NHits();

          // Fill cluster size
          hClusterSizeMap[id]->Fill(NHits);

          // Call it once.. it's faster.
          float const ThisClusterCharge = Cluster->Charge();

          hMap[id][0]->Fill( ThisClusterCharge );
          if (NHits == 1) {
            hMap[id][1]->Fill( ThisClusterCharge );
          } else if (NHits == 2) {
            hMap[id][2]->Fill( ThisClusterCharge );
          } else if (NHits >= 3) {
            hMap[id][3]->Fill( ThisClusterCharge );
          }

          if (ThisClusterCharge < 200000) {
            ChargeHits[id][0].push_back( ThisClusterCharge );
            if (NHits == 1) {
              ChargeHits[id][1].push_back( ThisClusterCharge );
            } else if (NHits == 2) {
              ChargeHits[id][2].push_back( ThisClusterCharge );
            } else if (NHits >= 3) {
              ChargeHits[id][3].push_back( ThisClusterCharge );
            }
          }


          // Extend the plots if we need to (less careful but I assume 0123 all scale together)
          if (ientry % TimeWidth == 0) {

            // Loop over all ids
            for (std::map<int, std::vector< std::vector<float> > >::iterator it = vClEnTimeMap.begin(); it != vClEnTimeMap.end(); ++it) {
              int const id = it->first;

              // extend each hist
              for (int iTimeHist = 0; iTimeHist < 4; ++iTimeHist) {
                std::vector<float>* Vec = &it->second[iTimeHist];
                if (ChargeHits[id][iTimeHist].size() > 0) {
                  (*Vec)[TimeBinNumber] = Average(ChargeHits[id][iTimeHist]);
                }
                ChargeHits[id][iTimeHist].clear();
              }
            }
          }
        }
      }
    }


  }
  std::cout << "Events read: " << ientry+1 << std::endl;

  // Loop over all histograms and draw them on the correct canvas in the correct pad for ClEnTime
  for (std::map<int, std::vector< std::vector<float> > >::iterator it = vClEnTimeMap.begin(); it != vClEnTimeMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    int const Remainder = vClEnTimeMap[id][0].size() % 2;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);
    hClEnTimeMap[id].push_back( new TH1F( TString::Format("PulseHeightTime_Ch%02i_ROC%1i_All", Channel, ROC),
          TString::Format("Pulse Height Time Dep for Ch %02i ROC %1i Pixels All", Channel, ROC), vClEnTimeMap[id][0].size() + Remainder, 0, (vClEnTimeMap[id][0].size() + Remainder) * TimeWidth ) );
    for (size_t ih = 1; ih != 3; ++ih) {
      hClEnTimeMap[id].push_back( new TH1F( TString::Format("PulseHeightTime_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih),
            TString::Format("Pulse Height Time Dep for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih), vClEnTimeMap[id][ih].size() + Remainder, 0, (vClEnTimeMap[id][ih].size() + Remainder) * TimeWidth) );
    }
    hClEnTimeMap[id].push_back( new TH1F( TString::Format("PulseHeightTime_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) 3),
          TString::Format("Pulse Height Time Dep for Ch %02i ROC %1i Pixels %i+", Channel, ROC, (int) 3), vClEnTimeMap[id][3].size() + Remainder, 0, (vClEnTimeMap[id][3].size() + Remainder) * TimeWidth) );
    for (size_t ih = 0; ih != 4; ++ih) {
      hClEnTimeMap[id][ih]->SetNdivisions(5);
      hClEnTimeMap[id][ih]->SetStats(false);
      hClEnTimeMap[id][ih]->SetMinimum(0);
      hClEnTimeMap[id][ih]->SetXTitle("Event Number");
      //hClEnTimeMap[id][ih]->SetYTitle("Average Charge (electrons)");
      for (size_t ib = 0; ib != vClEnTimeMap[id][ih].size(); ++ib) {
        hClEnTimeMap[id][ih]->SetBinContent(ib, vClEnTimeMap[id][ih][ib]);
      }
      while (hClEnTimeMap[id][ih]->GetNbinsX() > 25) {
        if (hClEnTimeMap[id][ih]->GetNbinsX() % 2 == 1) {
          hClEnTimeMap[id][ih]->SetBins(hClEnTimeMap[id][ih]->GetNbinsX()+1, 0, hClEnTimeMap[id][ih]->GetXaxis()->GetXmax());
        }
        hClEnTimeMap[id][ih]->Rebin();
        hClEnTimeMap[id][ih]->Scale(0.5);
      }
    }

    for (size_t ih = 1; ih != 4; ++ih) {
      // Grab hist
      TH1F* Hist = hClEnTimeMap[id][ih];

      Hist->SetLineColor(HistColors[ih]);
      //if (Hist->GetNbinsX() % 2 == 0) {
      //  Hist->Rebin(2);
      //  Hist->Scale(1./2.);
      //}
    }

  }

  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cClEnTimeMap.begin(); it != cClEnTimeMap.end(); ++it) {
    //it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
    //delete it->second;
  }

  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, std::vector<TH1F*> >::iterator it = hMap.begin(); it != hMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+1);

    TLegend* Leg = new TLegend(0.65, 0.7, 0.80, 0.88, "");
    Leg->SetFillColor(0);
    Leg->SetBorderSize(0);

    for (size_t ih = 0; ih != 4; ++ih) {
      TH1F* Hist = it->second[ih];
      Hist->SetStats(false);

      Hist->SetNdivisions(5);
      Hist->SetLineColor(HistColors[ih]);
      if (ih == 0) {
        Hist->SetTitle( TString::Format("PulseHeight Ch%02i ROC%1i", Channel, ROC) );
        Hist->SetXTitle("Electrons");
        Hist->SetYTitle("Events");
        Hist->Draw("hist");
        Leg->AddEntry(Hist, "All", "l");
      } else {
        Hist->Draw("samehist");
        if (ih != 3) {
          Leg->AddEntry(Hist, TString::Format(" %i Pixel", ih), "l");
        } else {
          Leg->AddEntry(Hist, TString::Format("#geq%i Pixel", ih), "l");
        }
      }
    }
    Leg->Draw("same");
    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+3+1);
    for (size_t ih = 1; ih != 4; ++ih) {

      // Grab hist
      TH1F* Hist = hClEnTimeMap[id][ih];

      Hist->SetNdivisions(5);
      Hist->SetMaximum(60000);
      Hist->SetMinimum(0);
      Hist->SetLineColor(HistColors[ih]);
      if (ih == 1) {
        Hist->SetTitle( TString::Format("Average Pulse Height ROC %i", ROC) );
        Hist->Draw("hist");
      } else {
        Hist->Draw("histsame");
      }
    }
  }

  // Save Cluster Size canvases
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
    delete it->second;
  }


  // Loop over cluster size plots
  for (std::map<int, TH1F*>::iterator it = hClusterSizeMap.begin(); it != hClusterSizeMap.end(); ++it) {
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    cClusterSizeMap[Channel]->cd(ROC+1)->SetLogy(1);
    it->second->Draw("hist");
  }

  // Save Cluster Size canvases
  for (std::map<int, TCanvas*>::iterator it = cClusterSizeMap.begin(); it != cClusterSizeMap.end(); ++it) {
    it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
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
