////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 10:38:26 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"

// FUNCTION DEFINITIONS HERE
int TrackTest (std::string const, std::string const, std::string const);


// CODE BELOW


int TrackTest (std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);

  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);


  // 3 2-tracks, 1 3-track per chan
  // 0 is for 3-track, 01 == 1, 02 == 2, 12 ==3
  // This map is [ROC][track]
  std::map<int, std::vector< std::vector<TH2F*> > > ResidualMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {


    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      if (ResidualMap.count(Telescope->Channel()) == 0) {
        TString Name = TString::Format("Residual_Ch%i", Telescope->Channel());
          for (int ir = 0; ir != 3; ++ir) {
            std::vector<TH2F*> Vec;
            TString R = TString::Format("%i", ir);
            Vec.push_back( new TH2F( Name+"_3Plane_ROC"+R,  Name+"_3Plane_ROC"+R,  30, -15, 15, 30, -15, 15 ));
            Vec.push_back( new TH2F( Name+"_01Plane_ROC"+R, Name+"_01Plane_ROC"+R, 30, -15, 15, 30, -15, 15 ));
            Vec.push_back( new TH2F( Name+"_02Plane_ROC"+R, Name+"_02Plane_ROC"+R, 30, -15, 15, 30, -15, 15 ));
            Vec.push_back( new TH2F( Name+"_12Plane_ROC"+R, Name+"_12Plane_ROC"+R, 30, -15, 15, 30, -15, 15 ));
            ResidualMap[Telescope->Channel()].push_back(Vec);
        }
      }

      // Look for three planes in a telescope each having 1 cluster
      if (Telescope->NPlanes() == 3 && Telescope->NClusters() == Telescope->NPlanes()) {

        PLTTrack Tracks[4];
        Tracks[0].AddCluster(Telescope->Plane(0)->Cluster(0));
        Tracks[0].AddCluster(Telescope->Plane(1)->Cluster(0));
        Tracks[0].AddCluster(Telescope->Plane(2)->Cluster(0));

        Tracks[1].AddCluster(Telescope->Plane(0)->Cluster(0));
        Tracks[1].AddCluster(Telescope->Plane(1)->Cluster(0));

        Tracks[2].AddCluster(Telescope->Plane(0)->Cluster(0));
        Tracks[2].AddCluster(Telescope->Plane(2)->Cluster(0));

        Tracks[3].AddCluster(Telescope->Plane(1)->Cluster(0));
        Tracks[3].AddCluster(Telescope->Plane(2)->Cluster(0));

        for (int i = 0; i != 4; ++i) {
          Tracks[i].MakeTrack(Alignment);

          for (int j = 0; j != 3; ++j) {
            PLTCluster* Cluster = Telescope->Plane(j)->Cluster(0);
            std::pair<float, float> RXY = Tracks[i].LResiduals(*Cluster, Alignment);
            printf("Filling ResidualMap ch ROC i ResXY: %2i %i %i %9.3f %9.3f\n", Telescope->Channel(), Cluster->ROC(), i, RXY.first, RXY.second);
            ResidualMap[Telescope->Channel()][Cluster->ROC()][i]->Fill(RXY.first, RXY.second);
          }
        }

        Telescope->AddTrack(&Tracks[0]);
        static int ievent = 0;
        //comment
	//Telescope->DrawTracksAndHits( TString::Format("plots/Tracks_Ch%i_Ev%i.eps", Telescope->Channel(), ++ievent).Data() );

        // Let's see if tracks are fiducial to planes..
        for (size_t ip = 0; ip != Telescope->NPlanes(); ++ip) {
          if (!Tracks[3].IsFiducial(Telescope->Plane(ip), Alignment)) {
            printf("IsFiducial: NOT FIDUCIAL: ROC: %1i\n", Telescope->Plane(ip)->ROC());
          }
        }

      }

      if (false) {
        for (size_t ip = 0; ip != Telescope->NPlanes(); ++ip) {

          // THIS plane is
          PLTPlane* Plane = Telescope->Plane(ip);

          // Loop over all hits on this plane
          for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {

            // THIS hit is
            PLTHit* Hit = Plane->Hit(ihit);

            // Just print some example info
            printf("Channel: %2i  ROC: %1i  col: %2i  row: %2i  LX: %12.3E LY: %12.3E  TX: %12.3E TY: %12.3E TZ: %12.3E  GX: %12.3E GY: %12.3E GZ: %12.3E  adc: %3i  Charge: %9.3E\n",
                Telescope->Channel(), Plane->ROC(),
                Hit->Column(), Hit->Row(),
                Hit->LX(), Hit->LY(),
                Hit->TX(), Hit->TY(), Hit->TZ(),
                Hit->GX(), Hit->GY(), Hit->GZ(),
                Hit->ADC(), Hit->Charge());
            printf("         %2i  ROC: %1i  col: %2i  row: %2i  LX: %12.3E LY: %12.3E  TX: %12.3E TY: %12.3E TZ: %12.3E  LX: %12.3E LY: %12.3E GZ: %12.3E  adc: %3i  Charge: %9.3E\n",
                Telescope->Channel(), Plane->ROC(),
                Hit->Column(), Hit->Row(),
                Hit->LX(), Hit->LY(),
                Hit->TX(), Hit->TY(), Hit->TZ(),
                Alignment.TtoLX(Hit->TX(), Hit->TY(), Telescope->Channel(), Plane->ROC()),
                Alignment.TtoLY(Hit->TX(), Hit->TY(), Telescope->Channel(), Plane->ROC()),
                Hit->GZ(),
                Hit->ADC(), Hit->Charge());
          }
        }
      }
    }
  }



  for (std::map<int, std::vector< std::vector<TH2F*> > >::iterator It = ResidualMap.begin(); It != ResidualMap.end(); ++It) {
    TString CanNameAll  = TString::Format("CanResidual_All_Ch%i", It->first);
    TString SaveNameAll = TString::Format("plots/CanResidual_All_Ch%i.eps", It->first);
    TString CanName  = TString::Format("CanResidual_Ch%i", It->first);
    TString SaveName = TString::Format("plots/CanResidual_Ch%i.eps", It->first);
    TString CanNameXY  = TString::Format("CanResidual_Ch%i_XY", It->first);
    TString SaveNameXY = TString::Format("plots/CanResidual_Ch%i_XY.eps", It->first);
    TString CanName3XY  = TString::Format("CanResidual_Ch%i_3XY", It->first);
    TString SaveName3XY = TString::Format("plots/CanResidual_Ch%i_3XY.eps", It->first);

    TCanvas CanResidualAll(CanNameAll, CanNameAll, 1600, 1200);
    TCanvas CanResidual(CanName, CanName, 900, 600);
    TCanvas CanResidualXY(CanNameXY, CanNameXY, 900, 900);
    TCanvas CanResidual3XY(CanName3XY, CanName3XY, 900, 900);
    CanResidual.Divide(3, 2);
    CanResidualXY.Divide(3, 3);
    CanResidual3XY.Divide(3, 3);
    CanResidualAll.Divide(3, 4);
    int iPad = 0;
    int iPadAll = 0;
    for (size_t itrack = 0; itrack != 4; ++itrack) {
      for (size_t iroc = 0; iroc != 3; ++iroc) {
        CanResidualAll.cd(++iPadAll);
        It->second[iroc][itrack]->SetXTitle("X");
        It->second[iroc][itrack]->SetYTitle("Y");
        It->second[iroc][itrack]->Draw("colz");
        if (itrack == 0 || (itrack == 1 && iroc == 2) || (itrack == 2 && iroc == 1) || (itrack == 3 && iroc == 0)) {
          CanResidual.cd(++iPad);
          It->second[iroc][itrack]->Draw("colz");
        }
        if ((itrack == 1 && iroc == 2) || (itrack == 2 && iroc == 1) || (itrack == 3 && iroc == 0)) {
          CanResidualXY.cd(iroc+1);
          It->second[iroc][itrack]->Draw("colz");
          CanResidualXY.cd(iroc+1+3);
          It->second[iroc][itrack]->ProjectionX()->Draw("hist");
          CanResidualXY.cd(iroc+1+6);
          It->second[iroc][itrack]->ProjectionY()->Draw("hist");
        }
        if (itrack == 0) {
          CanResidual3XY.cd(iroc+1);
          It->second[iroc][itrack]->Draw("colz");
          CanResidual3XY.cd(iroc+1+3);
          It->second[iroc][itrack]->ProjectionX()->Draw("hist");
          CanResidual3XY.cd(iroc+1+6);
          It->second[iroc][itrack]->ProjectionY()->Draw("hist");
        }
      }
    }
    CanResidual.SaveAs(SaveName);
    CanResidualXY.SaveAs(SaveNameXY);
    CanResidual3XY.SaveAs(SaveName3XY);
    CanResidualAll.SaveAs(SaveNameAll);
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

  TrackTest(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
