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
#include "TLatex.h"



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
  Event.SetPlaneClustering(PLTPlane::kClustering_Seed_5x5, PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  //  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);

  // Map for all ROC hists and canvas
  std::map<int, std::vector<TGraphErrors*> > gClEnTimeMap;
  std::map<int, TH1F*>    hClusterSizeMap;
  std::map<int, TCanvas*> cClusterSizeMap;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;
  std::map<int, TH2F* >              hMap2D;
  std::map<int, TCanvas*>            cMap2D;

  //double Avg2D[250][PLTU::NCOL][PLTU::NROW];
  std::map<int, std::vector< std::vector<double> > > Avg2D;
  std::map<int, std::vector< std::vector<int> > > N2D;
  //int      N2D[250][PLTU::NCOL][PLTU::NROW];

  // Bins and max for pulse height plots
  int   const NBins =     60;
  float const XMin  =  -1000;
  float const XMax  =  50000;

  // Time width in events for energy time dep plots
  // This is the time width in ms
  // const unsigned int TimeWidth = 1000 * (60 * 1);
  const unsigned int TimeWidth = 1000;
  std::map<int, std::vector< std::vector<float> > > ChargeHits;

  // Loop over all events in file
  int NGraphPoints = 0;
  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << std::endl;
    }



    // First event time
    //static uint32_t const StartTime = Event.Time();
    //uint32_t const ThisTime = Event.Time();
    static uint32_t const StartTime = 0;
    uint32_t static ThisTime = 0;
    ++ThisTime;

    if (ientry == 300000) {
      std::cout << "Reached target of 300000 events; stopping..." << std::endl;
      break;
    }

    while (ThisTime - (StartTime + NGraphPoints * TimeWidth) > TimeWidth) {
      // make point(s)
      for (std::map<int, std::vector<TGraphErrors*> >::iterator mit = gClEnTimeMap.begin(); mit != gClEnTimeMap.end(); ++mit) {
        int const id = mit->first;
        for (size_t ig = 0; ig != mit->second.size(); ++ig) {
          TGraphErrors* g = (mit->second)[ig];

          if (g->GetN() != NGraphPoints) {
            // Play some catchup
            g->Set(NGraphPoints);
            for (int i = 0; i > NGraphPoints; ++i) {
              g->SetPoint(i, i * TimeWidth, 0);
            }
          }

          g->Set( NGraphPoints + 1 );
          if (ChargeHits[id][ig].size() != 0) {
            float const Avg = PLTU::Average(ChargeHits[id][ig]);
            g->SetPoint(NGraphPoints, NGraphPoints * TimeWidth, Avg);
            g->SetPointError( NGraphPoints, 0, Avg/sqrt((float) ChargeHits[id][ig].size()));
            ChargeHits[id][ig].clear();
            ChargeHits[id][ig].reserve(10000);
          } else {
            g->SetPoint(NGraphPoints , NGraphPoints * TimeWidth, 0);
            g->SetPointError( NGraphPoints , 0, 0 );
          }
        }
      }
      ++NGraphPoints;

//      std::cout << NGraphPoints << std::endl;

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

        if (!Avg2D.count(id)) {
          Avg2D[id].resize(PLTU::NCOL);
          N2D[id].resize(PLTU::NCOL);
          for (int icol = 0; icol != PLTU::NCOL; ++icol) {
            Avg2D[id][icol].resize(PLTU::NROW);
            N2D[id][icol].resize(PLTU::NROW);
          }
        }

        if (!hMap.count(id)) {
          hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ROC),
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", Channel, ROC), NBins, XMin, XMax) );
            hMap2D[id] = new TH2F( TString::Format("Avg Charge Ch %02i ROC %1i Pixels All", Channel, ROC),
              TString::Format("PixelCharge_Ch%02i_ROC%1i_All", Channel, ROC), PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
          for (size_t ih = 1; ih != 4; ++ih) {
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                   TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih), NBins, XMin, XMax) );
          }

          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Channel)) {
            // Create canvas with given name
            cMap[Channel] = new TCanvas( TString::Format("PulseHeight_Ch%02i", Channel), TString::Format("PulseHeight_Ch%02i", Channel), 900, 900);
            cMap[Channel]->Divide(3, 3);
          }
        }

        if (!gClEnTimeMap.count(id)) {
          gClEnTimeMap[id].resize(4);
          for (int ig = 0; ig != 4; ++ig) {
            TString const Name = TString::Format("TimeAvgGraph_id%d_Cl%d", id, ig);
            gClEnTimeMap[id][ig] = new TGraphErrors();
            gClEnTimeMap[id][ig]->SetName(Name);
          }
        }


        if (!ChargeHits.count(id)) {
          ChargeHits[id].resize(4);
          ChargeHits[id][0].reserve(10000);
          ChargeHits[id][1].reserve(10000);
          ChargeHits[id][2].reserve(10000);
          ChargeHits[id][3].reserve(10000);
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

          //if (Cluster->NHits() != 1) continue;
          //if (Cluster->Hit(0)->Column() != 31 || Cluster->Hit(0)->Row() != 55) continue;

          // Get number of hits in this cluster
          size_t NHits = Cluster->NHits();

          int const col = PLTGainCal::ColIndex(Cluster->SeedHit()->Column());
          int const row = PLTGainCal::RowIndex(Cluster->SeedHit()->Row());

          // Call it once.. it's faster.
          float const ThisClusterCharge = Cluster->Charge();

          if (ThisClusterCharge < 100000 && ThisClusterCharge >= 0) {
            Avg2D[id][col][row] = Avg2D[id][col][row] * ((double) N2D[id][col][row] / ((double) N2D[id][col][row] + 1.)) + ThisClusterCharge / ((double) N2D[id][col][row] + 1.);
            ++N2D[id][col][row];
          }


          // Fill cluster size
          hClusterSizeMap[id]->Fill(NHits);

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
    cMap[Channel]->cd(ROC+1);

    TLegend* Leg = new TLegend(0.65, 0.7, 0.80, 0.88, "");
    Leg->SetFillColor(0);
    Leg->SetBorderSize(0);

    for (int ih = 0; ih != 4; ++ih) {
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
    char buf[512];
    sprintf(buf, "#splitline{Mean:}{%.0f}", it->second[0]->GetMean());
    TLatex *t1 = new TLatex(0, 0, buf);
    t1->SetNDC();
    t1->SetX(0.68);
    t1->SetY(0.6);
    t1->Draw();
    sprintf(buf, "#splitline{Peak:}{%.0f}", it->second[0]->GetXaxis()->GetBinCenter(it->second[0]->GetMaximumBin()));
    TLatex *t2 = new TLatex(0, 0, buf);
    t2->SetNDC();
    t2->SetX(0.68);
    t2->SetY(0.5);
    t2->Draw();
    
    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+3+1);
    for (int ig = 3; ig!= -1; --ig) {
      // Grab hist
      TGraphErrors* g = gClEnTimeMap[id][ig];

      g->SetMarkerColor(HistColors[ig]);
      g->SetLineColor(HistColors[ig]);
      if (ig == 3) {
        g->SetTitle( TString::Format("Average Pulse Height ROC %i", ROC) );
        g->SetMinimum(0);
        g->SetMaximum(60000);
        g->SetMinimum(0);
        g->Draw("Apl");
      } else {
        g->Draw("samep");
      }
    }
    cMap[Channel]->cd(ROC+6+1);
    for (int ja = 0; ja != PLTU::NROW; ++ja) {
      for (int ia = 0; ia != PLTU::NCOL; ++ia) {
        if (Avg2D[id][ia][ja] < 25000) {
          //int const hdwAddy = Event.GetGainCal()->GetHardwareID(Channel);
          //int const mf  = hdwAddy / 1000;
          //int const mfc = (hdwAddy % 1000) / 100;
          //int const hub = hdwAddy % 100;
          //fprintf(OutPix, "%1i %1i %2i %1i %2i %2i\n", mf, mfc, hub, ROC, PLTU::FIRSTCOL + ia, PLTU::FIRSTROW + ja);
        } else {
        }
        if (Avg2D[id][ia][ja] > 0) {
          hMap2D[id]->SetBinContent(ia+1, ja+1, Avg2D[id][ia][ja]);
        }
        //printf("%6.0f ", Avg2D[id][ia][ja]);
      }
      //      std::cout << std::endl;
    }
    hMap2D[id]->SetMaximum(60000);
    hMap2D[id]->SetStats(false);
    hMap2D[id]->SetXTitle("Column");
    hMap2D[id]->SetYTitle("Row");
    hMap2D[id]->SetZTitle("Electrons");
    hMap2D[id]->Draw("colz");

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
