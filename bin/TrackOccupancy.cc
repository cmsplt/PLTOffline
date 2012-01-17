////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue Nov  1 10:26:12 CET 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"
#include "PLTU.h"


int TrackOccupancy (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching,PLTPlane::kFiducialRegion_m2_m2);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);

  // Map for all ROC hists and canvas
  std::map<int, TH2F*> hMap;
  std::map<int, std::vector<TH1F*> > hPulseHeightMap;

  // Bins and max for pulse height plots
  int   const NBins =    50;
  float const XMax  = 50000;
  int   const HistColors[4] = { 1, 4, 28, 2 };


  // Loop over all events
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing entry: " << ientry << std::endl;
    }


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      // Look for three planes in a telescope each having 1 cluster
      if (Telescope->HitPlaneBits() != 0x7 || Telescope->NClusters() != 3) {
        continue;
      }
      if (Telescope->Plane(0)->NHits() > 2 || Telescope->Plane(1)->NHits() > 2 || Telescope->Plane(2)->NHits() > 2) {
        continue;
      }

      // If this hists for this telescope don't exist create them
      int const Channel = Telescope->Channel();
      if (hMap.count(Channel * 10) == 0) {
        TString Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 0);
        hMap[Channel * 10 + 0] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 1);
        hMap[Channel * 10 + 1] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);
        Name = TString::Format("TrackOccupancy_Ch%02i_ROC%1i", Channel, 2);
        hMap[Channel * 10 + 2] = new TH2F(Name, Name, PLTU::NCOL-1, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW-1,PLTU::FIRSTROW, PLTU::LASTROW);

        // Pulse heights
        for (int ir = 0; ir != 3; ++ir) {
          int const id = Channel * 10 + ir;
          hPulseHeightMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ir),
                TString::Format("TrackPulseHeight_Ch%02i_ROC%1i_All", Channel, ir), NBins, 0, XMax) );
          for (size_t ih = 1; ih != 4; ++ih) {
            hPulseHeightMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ir, (int) ih),
                  TString::Format("TrackPulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ir, (int) ih), NBins, 0, XMax) );
          }
        }
      }

      // Make 4 different tracks.  The index corresponds to which plane is *not* used to make the track.
      // 3 = all planes used, otherwise only two planes used
      PLTTrack Tracks[4];
      Tracks[3].AddCluster(Telescope->Plane(0)->Cluster(0));
      Tracks[3].AddCluster(Telescope->Plane(1)->Cluster(0));
      Tracks[3].AddCluster(Telescope->Plane(2)->Cluster(0));

      Tracks[2].AddCluster(Telescope->Plane(0)->Cluster(0));
      Tracks[2].AddCluster(Telescope->Plane(1)->Cluster(0));

      Tracks[1].AddCluster(Telescope->Plane(0)->Cluster(0));
      Tracks[1].AddCluster(Telescope->Plane(2)->Cluster(0));

      Tracks[0].AddCluster(Telescope->Plane(1)->Cluster(0));
      Tracks[0].AddCluster(Telescope->Plane(2)->Cluster(0));

      // Loop over all tracks and do the reconstruction
      for (int i = 0; i != 4; ++i) {
        Tracks[i].MakeTrack(Alignment);

        // For each track compute a residual on each plane
        for (int j = 0; j != 3; ++j) {
          PLTCluster* Cluster = Telescope->Plane(j)->Cluster(0);
          int const id = 10 * Channel + Telescope->Plane(j)->ROC();

          std::pair<float, float> const RLXY = Tracks[i].LResiduals(*Cluster, Alignment);
          std::pair<float, float> const RPXY = Alignment.PXYDistFromLXYDist(RLXY);
          //printf("Residual L ch ROC i ResXY: %2i %i %i %9.3f %9.3f\n", Telescope->Channel(), Cluster->ROC(), i, RLXY.first, RLXY.second);
          //printf("Residual P ch ROC i ResXY: %2i %i %i %9.3f %9.3f\n", Telescope->Channel(), Cluster->ROC(), i, RPXY.first, RPXY.second);

          // If this is the plane we're extrapolating to plot it..
          if (i == j && i <= 2) {
            if (RPXY.first < 1 || RPXY.second < 1) {
              int const NHits = Cluster->NHits();

              // Fill pulse height for cluster
              float const ThisClusterCharge = Cluster->Charge();
              hPulseHeightMap[id][0]->Fill( ThisClusterCharge );
              if (NHits == 1) {
                hPulseHeightMap[id][1]->Fill( ThisClusterCharge );
              } else if (NHits == 2) {
                hPulseHeightMap[id][2]->Fill( ThisClusterCharge );
              } else if (NHits >= 3) {
                hPulseHeightMap[id][3]->Fill( ThisClusterCharge );
              }

              // Fill for each hit in cluster
              for (size_t ih = 0; ih != Cluster->NHits(); ++ih) {
                PLTHit* Hit = Cluster->Hit(ih);
                hMap[id]->Fill(Hit->Column(), Hit->Row());
              }
            }
          }
        }
      }
    }
  }


  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TH2F*>::iterator it = hMap.begin(); it != hMap.end(); ++it) {
    //    std::cout << "Here again!" << std::endl;
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    TString const Name = it->second->GetName();

    TCanvas Can(Name, Name, 400, 400);
    Can.cd();
    it->second->Draw("zcol");
    Can.SaveAs(Name + ".gif");
    delete it->second;
  }


  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, std::vector<TH1F*> >::iterator it = hPulseHeightMap.begin(); it != hPulseHeightMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    TString const Name = TString::Format("TrackPulseHeight_Ch%i_ROC%i", Channel, ROC);
    TCanvas Can(Name, Name, 400, 400);

    for (size_t ih = 0; ih != 4; ++ih) {
      TH1F* Hist = it->second[ih];
      Hist->SetStats(false);

      Hist->SetNdivisions(5);
      Hist->SetLineColor(HistColors[ih]);
      if (ih == 0) {
        Hist->SetTitle( TString::Format("TrackPulseHeight Ch%02i ROC%1i", Channel, ROC) );
        Hist->SetXTitle("Electrons");
        Hist->SetYTitle("Events");
        Hist->Draw("hist");
      } else {
        Hist->Draw("samehist");
      }
    }

    Can.SaveAs(Name + ".gif");


    // change to correct pad on canvas and draw the hist
    //cMap[Channel]->cd(ROC+3+1);
    //for (size_t ih = 1; ih != 4; ++ih) {
    //  // change to correct pad on canvas and draw the hist

    //  // Grab hist
    //  TH1F* Hist = hClEnTimeMap[id][ih];

    //  Hist->SetNdivisions(5);
    //  Hist->SetMaximum(60000);
    //  Hist->SetMinimum(0);
    //  Hist->SetLineColor(HistColors[ih]);
    //  if (ih == 1) {
    //    Hist->SetTitle( TString::Format("Average Pulse Height ROC %i", ROC) );
    //    Hist->Draw("hist");
    //  } else {
    //    Hist->Draw("histsame");
    //  }
    //}
  }

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

  TrackOccupancy(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
