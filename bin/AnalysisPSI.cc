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
#include "TGraphErrors.h"




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
  PLTEvent Event(DataFileName, GainCalFileName, true);
  Event.SetPlaneClustering(PLTPlane::kClustering_Seed_5x5, PLTPlane::kFiducialRegion_m2_m2);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);

  // Map for all ROC hists and canvas
  std::map<int, TH1F*>    hClusterSizeMap;
  std::map<int, TH1F*>    hClusterSizeMapB;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;
  std::map<int, std::vector<TH1F*> > hMapB;
  std::map<int, TCanvas*>            cMapB;
  std::map<int, TH2F* >              hMap2D;
  std::map<int, TH2F* >              hMapB2D;
  std::map<int, TH2F* >              hOccupancy;
  std::map<int, TH2F* >              hOccupancyB;

  double Avg2D[250][PLTU::NCOL][PLTU::NROW];
  int      N2D[250][PLTU::NCOL][PLTU::NROW];
  double AvgB2D[250][PLTU::NCOL][PLTU::NROW];
  int      NB2D[250][PLTU::NCOL][PLTU::NROW];

  // Bins and max for pulse height plots
  int   const NBins  =     60;
  float const XMin   =  -1000;
  float const XMax   =  50000;
  float const XMaxB  = 300000;

  float const CutValue = 40000;



  // Loop over all events in file
  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << std::endl;
    }


    //if (ientry == 300000) break;





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
          hMap[id].push_back( new TH1F(
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", Channel, ROC),
                TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ROC),
                NBins, XMin, XMax) );
            hMap2D[id] = new TH2F( 
              TString::Format("PixelCharge_Ch%02i_ROC%1i_All", Channel, ROC),
              TString::Format("Avg Cluster Charge Ch %02i ROC %1i", Channel, ROC),
              PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
          for (size_t ih = 1; ih != 4; ++ih) {
            hMap[id].push_back( new TH1F(
                  TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih),
                  TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                  NBins, XMin, XMax) );
          }

          hMapB[id].push_back( new TH1F(
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All_NoCut", Channel, ROC),
                TString::Format("Pulse Height for Ch %02i ROC %1i NoCut Pixels All", Channel, ROC),
                NBins, XMin, XMaxB) );
            hMapB2D[id] = new TH2F( 
              TString::Format("PixelCharge_Ch%02i_ROC%1i_All_NoCut", Channel, ROC),
              TString::Format("Avg Cluster Charge Ch %02i ROC %1i NoCut", Channel, ROC),
              PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
          for (size_t ih = 1; ih != 4; ++ih) {
            hMapB[id].push_back( new TH1F(
                  TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i_NoCut", Channel, ROC, (int) ih),
                  TString::Format("Pulse Height for Ch %02i ROC %1i NoCut Pixels %i", Channel, ROC, (int) ih),
                  NBins, XMin, XMaxB) );
          }

          // Occupancy
            hOccupancy[id] = new TH2F( 
              TString::Format("Occupancy_Ch%02i_ROC%1i", Channel, ROC),
              TString::Format("Occupancy Ch %02i ROC %1i", Channel, ROC),
              PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
            hOccupancyB[id] = new TH2F( 
              TString::Format("Occupancy_Ch%02i_ROC%1i_NoCut", Channel, ROC),
              TString::Format("Occupancy Ch %02i ROC %1i NoCut", Channel, ROC),
              PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);



          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(id)) {
            // Create canvas with given name
            cMap[id] = new TCanvas( TString::Format("Summary_Ch%02i", id), TString::Format("Summary_Ch%02i", id), 900, 900);
            cMap[id]->Divide(2, 2);
            cMapB[id] = new TCanvas( TString::Format("Summary_Ch%02i_NoCut", id), TString::Format("Summary_Ch%02i_NoCut", id), 900, 900);
            cMapB[id]->Divide(2, 2);
          }
        }



        // If this id doesn't exist in the cluster size map, make the hist and possibly canvas for this channel
        if (!hClusterSizeMap.count(id)) {
          hClusterSizeMap[id] = new TH1F( TString::Format("ClusterSize_Ch%02i_ROC%i", Channel, ROC), TString::Format("ClusterSize Ch%02i ROC%i", Channel, ROC), 10, 0, 10);
          hClusterSizeMap[id]->SetXTitle("Number of pixels in Cluster");

          hClusterSizeMapB[id] = new TH1F( TString::Format("ClusterSize_Ch%02i_ROC%i_NoCut", Channel, ROC), TString::Format("ClusterSize Ch%02i ROC%i NoCut", Channel, ROC), 10, 0, 10);
          hClusterSizeMapB[id]->SetXTitle("Number of pixels in Cluster");

        }


        // Loop over all clusters on this plane
        for (size_t iCluster = 0; iCluster != Plane->NClusters(); ++iCluster) {
          PLTCluster* Cluster = Plane->Cluster(iCluster);

          //if (Cluster->NHits() != 1) continue;
          //if (Cluster->Hit(0)->Column() != 31 || Cluster->Hit(0)->Row() != 55) continue;

          // Get number of hits in this cluster
          size_t NHits = Cluster->NHits();

          int const col = PLTGainCal::ColIndex(Cluster->SeedHit()->Column());
          int const row = PLTGainCal::RowIndex(Cluster->SeedHit()->Row());

          // Call it once.. it's faster.
          float const ThisClusterCharge = Cluster->Charge();

          if (ThisClusterCharge >= 0) {
            AvgB2D[id][col][row] = AvgB2D[id][col][row] * ((double) NB2D[id][col][row] / ((double) NB2D[id][col][row] + 1.)) + ThisClusterCharge / ((double) NB2D[id][col][row] + 1.);
            ++NB2D[id][col][row];
          }


          // Fill cluster size
          hClusterSizeMapB[id]->Fill(NHits);
          for (int ihit = 0; ihit != Cluster->NHits(); ++ihit) {
            hOccupancyB[id]->Fill(Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row());
          }

          hMapB[id][0]->Fill( ThisClusterCharge );
          if (NHits == 1) {
            hMapB[id][1]->Fill( ThisClusterCharge );
          } else if (NHits == 2) {
            hMapB[id][2]->Fill( ThisClusterCharge );
          } else if (NHits >= 3) {
            hMapB[id][3]->Fill( ThisClusterCharge );
          }

          if (ThisClusterCharge < CutValue && ThisClusterCharge >= 0) {
            Avg2D[id][col][row] = Avg2D[id][col][row] * ((double) N2D[id][col][row] / ((double) N2D[id][col][row] + 1.)) + ThisClusterCharge / ((double) N2D[id][col][row] + 1.);
            ++N2D[id][col][row];


            // Fill cluster size
            hClusterSizeMap[id]->Fill(NHits);
            for (int ihit = 0; ihit != Cluster->NHits(); ++ihit) {
              hOccupancy[id]->Fill(Cluster->Hit(ihit)->Column(), Cluster->Hit(ihit)->Row());
            }

            hMap[id][0]->Fill( ThisClusterCharge );
            if (NHits == 1) {
              hMap[id][1]->Fill( ThisClusterCharge );
            } else if (NHits == 2) {
              hMap[id][2]->Fill( ThisClusterCharge );
            } else if (NHits >= 3) {
              hMap[id][3]->Fill( ThisClusterCharge );
            }

          }
        }
      }
    }




  }
  std::cout << "Events read: " << ientry+1 << std::endl;



  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, std::vector<TH1F*> >::iterator it = hMap.begin(); it != hMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    // change to correct pad on canvas and draw the hist
    cMap[id]->cd(1);

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


    cMap[id]->cd(2);
    hClusterSizeMap[id]->Draw("hist");




    // change to correct pad on canvas and draw the hist
    cMap[id]->cd(3);
    for (int ja = 0; ja != PLTU::NROW; ++ja) {
      for (int ia = 0; ia != PLTU::NCOL; ++ia) {
        if (Avg2D[id][ia][ja] < 25000) {
          int const hwdAddy = Event.GetGainCal()->GetHardwareID(Channel);
          int const mf  = hwdAddy / 1000;
          int const mfc = (hwdAddy % 1000) / 100;
          int const hub = hwdAddy % 100;
          //fprintf(OutPix, "%1i %1i %2i %1i %2i %2i\n", mf, mfc, hub, ROC, PLTU::FIRSTCOL + ia, PLTU::FIRSTROW + ja);
        } else {
        }
        if (Avg2D[id][ia][ja] > 0) {
          hMap2D[id]->SetBinContent(ia+1, ja+1, Avg2D[id][ia][ja]);
        }
        //printf("%6.0f ", Avg2D[id][ia][ja]);
      }
      std::cout << std::endl;
    }
    hMap2D[id]->SetMaximum(60000);
    hMap2D[id]->SetStats(false);
    hMap2D[id]->SetXTitle("Column");
    hMap2D[id]->SetYTitle("Row");
    hMap2D[id]->SetZTitle("Electrons");
    hMap2D[id]->Draw("colz");


    cMap[id]->cd(4);
    hOccupancy[id]->Draw("colz");

  }


  for (std::map<int, std::vector<TH1F*> >::iterator it = hMapB.begin(); it != hMapB.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    // change to correct pad on canvas and draw the hist
    cMapB[id]->cd(1);

    TLegend* Leg = new TLegend(0.65, 0.7, 0.80, 0.88, "");
    Leg->SetFillColor(0);
    Leg->SetBorderSize(0);

    for (size_t ih = 0; ih != 4; ++ih) {
      TH1F* Hist = it->second[ih];
      Hist->SetStats(false);

      Hist->SetNdivisions(5);
      Hist->SetLineColor(HistColors[ih]);
      if (ih == 0) {
        Hist->SetTitle( TString::Format("PulseHeight Ch%02i ROC%1i NoCut", Channel, ROC) );
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


    cMapB[id]->cd(2);
    hClusterSizeMapB[id]->Draw("hist");




    // change to correct pad on canvas and draw the hist
    cMapB[id]->cd(3);
    for (int ja = 0; ja != PLTU::NROW; ++ja) {
      for (int ia = 0; ia != PLTU::NCOL; ++ia) {
        if (AvgB2D[id][ia][ja] > 0) {
          hMapB2D[id]->SetBinContent(ia+1, ja+1, AvgB2D[id][ia][ja]);
        }
        printf("%6.0f ", AvgB2D[id][ia][ja]);
      }
      std::cout << std::endl;
    }
    hMapB2D[id]->SetMaximum(60000);
    hMapB2D[id]->SetStats(false);
    hMapB2D[id]->SetXTitle("Column");
    hMapB2D[id]->SetYTitle("Row");
    hMapB2D[id]->SetZTitle("Electrons");
    hMapB2D[id]->Draw("colz");


    cMapB[id]->cd(4);
    hOccupancyB[id]->Draw("colz");

  }

  // Save Cluster Size canvases
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
    delete it->second;
  }
  for (std::map<int, TCanvas*>::iterator it = cMapB.begin(); it != cMapB.end(); ++it) {
    it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
    delete it->second;
  }



  // Loop over cluster size plots
  for (std::map<int, TH1F*>::iterator it = hClusterSizeMap.begin(); it != hClusterSizeMap.end(); ++it) {
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    //cClusterSizeMap[Channel]->cd(ROC+1)->SetLogy(1);
    it->second->Draw("hist");
  }

  // Save Cluster Size canvases
  //for (std::map<int, TCanvas*>::iterator it = cClusterSizeMap.begin(); it != cClusterSizeMap.end(); ++it) {
  //  it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
  //  delete it->second;
  ///}

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
