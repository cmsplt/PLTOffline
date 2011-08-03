////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Fri Jul 15 15:28:52 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>

#include "PLTEvent.h"

int ShowEvent (std::string const FileName, int const EventNumber)
{
  // Setup PLTEvent object
  PLTEvent Event(FileName);

  while (Event.GetNextEvent() >= 0) {
    if (Event.EventNumber() != EventNumber) {
      continue;
    }

    std::cout << "Found event: " << EventNumber << std::endl;

    printf("NTelescopes: %2i\n", (int) Event.NTelescopes());

    for (size_t it = 0; it != Event.NTelescopes(); ++it) {
      PLTTelescope* Telescope = Event.Telescope(it);

      TString const Name = TString::Format("Event%i_Ch%02i", EventNumber, Telescope->Channel());

      TCanvas C(Name, Name, 900, 300);
      C.Divide(3,1);

      for (size_t ip = 0; ip != Telescope->NPlanes(); ++ip) {
        PLTPlane* Plane = Telescope->Plane(ip);
        printf("Channel: %2i  ROC: %1i  Hits: %4i  Clusters: %4i\n", Telescope->Channel(), Plane->ROC(), Plane->NHits(), Plane->NClusters());

        C.cd(Plane->ROC() + 1);
        TString const ROCName = TString::Format("Ch%02i_ROC%1i", Telescope->Channel(), Plane->ROC());
        TH2F* h = new TH2F(ROCName, ROCName, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);

        for (size_t ih = 0; ih != Plane->NHits(); ++ih) {
          h->Fill( Plane->Hit(ih)->Column(), Plane->Hit(ih)->Row() );
        }
        h->Draw("colz");
      }
      //C.SaveAs(Name+".eps");
      C.SaveAs("plots/Event.eps");
    }

    break;
  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [FileName] [EventNumber]" << std::endl;
    return 1;
  }

  std::string const FileName = argv[1];
  int const EventNumber = atoi(argv[2]);

  ShowEvent(FileName, EventNumber);

  return 0;
}
