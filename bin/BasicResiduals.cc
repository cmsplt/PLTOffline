////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Jun  1 10:42:40 CEST 2011
//
////////////////////////////////////////////////////////////////////

#include <iostream>

#include "PLTEvent.h"

#include "TString.h"
#include "TH1F.h"


int BasicResiduals (std::string const DataFileName, std::string const GainCalFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);

  // Maps for histograms
  std::map<int, TH2F*> hMap1;
  std::map<int, TH2F*> hMap2;
  std::map<int, TCanvas*> cMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {

    // Loop over all planes with hits in event
    for (size_t it = 0; it != Event.NTelescopes(); ++it) {

      // THIS telescope is
      PLTTelescope* Telescope = Event.Telescope(it);

      // Only look at telescopes with three plane and a total of 3 hits
      // That's one hit per telescope
      if (Telescope->NPlanes() != 3 || Telescope->NClusters() != 3) {
        continue;
      }

      PLTHit* Seed[3] = {
        Telescope->Plane(0)->Cluster(0)->SeedHit(),
        Telescope->Plane(1)->Cluster(0)->SeedHit(),
        Telescope->Plane(2)->Cluster(0)->SeedHit()
      };

      if (Seed[0]->ROC() != 0 || Seed[1]->ROC() != 1 || Seed[2]->ROC() != 2) {
        std::cerr << "ERROR: rocs don't match telescope plane order.." << std::endl;
        throw;
      }

      int Channel = (int) Telescope->Channel();
      if (!hMap1.count(Channel)) {
        hMap1[Channel] = new TH2F( TString::Format("Residual_ch%02i_1", Channel), TString::Format("Residual_ch%02i_1", Channel), 80, -40, 40, 80, -40, 40);
        hMap2[Channel] = new TH2F( TString::Format("Residual_ch%02i_2", Channel), TString::Format("Residual_ch%02i_2", Channel), 80, -40, 40, 80, -40, 40);
        cMap[Channel] = new TCanvas( TString::Format("Residual_ch%02i", Channel), TString::Format("Residual_ch%02i", Channel), 600, 900);
        hMap1[Channel]->SetXTitle("Row");
        hMap1[Channel]->SetYTitle("Column");
        hMap2[Channel]->SetXTitle("Row");
        hMap2[Channel]->SetYTitle("Column");
        cMap[Channel]->Divide(2, 3);
      }

      hMap1[Channel]->Fill( Seed[1]->Column() - Seed[0]->Column(), Seed[1]->Row() - Seed[0]->Row());
      hMap2[Channel]->Fill( Seed[2]->Column() - Seed[0]->Column(), Seed[2]->Row() - Seed[0]->Row());

      printf("DeltaX(1-0) DeltaX(2-0) DeltaY(1-0) DeltaY(2-0)  %4i %4i  %4i %4i\n" ,
          Seed[1]->Column() - Seed[0]->Column(), Seed[2]->Column() - Seed[0]->Column(),
          Seed[1]->Row() - Seed[0]->Row(), Seed[2]->Row() - Seed[0]->Row());


    }
  }


  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    int Channel = it->first;

    cMap[Channel]->cd(1);
    hMap1[Channel]->Draw("colz");
    cMap[Channel]->cd(2);
    hMap2[Channel]->Draw("colz");
    cMap[Channel]->cd(3);
    hMap1[Channel]->ProjectionX()->Draw("hist");
    cMap[Channel]->cd(4);
    hMap2[Channel]->ProjectionX()->Draw("hist");
    cMap[Channel]->cd(5);
    hMap1[Channel]->ProjectionY()->Draw("hist");
    cMap[Channel]->cd(6);
    hMap2[Channel]->ProjectionY()->Draw("hist");


    cMap[Channel]->SaveAs( TString::Format("plots/Residuals_Ch%02i.gif", Channel) );
    delete hMap1[Channel];
    delete hMap2[Channel];
    delete cMap[Channel];
  }




  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat] [GainCal.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];

  BasicResiduals(DataFileName, GainCalFileName);

  return 0;
}
