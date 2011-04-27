////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Apr  4 16:23:17 CDT 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TMath.h"

#include "PLTGainCal.h"


int const MAXCHNS =  48;
int const MAXROWS = 100;
int const MAXCOLS = 100;
int const MAXROCS =   3;

PLTGainCal* fGainCal;





class PLTHit
{
  public:
    PLTHit () {};
    PLTHit (TString const& Line) {
      std::istringstream ss;
      ss.str(Line.Data());
      ss >> fChannel
         >> fROC
         >> fColumn
         >> fRow
         >> fADC
         >> fEvent;
      fCharge = fGainCal->GetCharge(fChannel, fROC, fColumn, fRow, fADC);
    };
    ~PLTHit () {};

    int fChannel;
    int fROC;
    int fColumn;
    int fRow;
    int fADC;
    long unsigned fEvent;
    float fCharge;


    bool MatchesColumnRow (PLTHit* Hit)
    {
      if (fColumn == Hit->Column() && fRow == Hit->Row()) {
        return true;
      }
      return false;
    }

    int Channel ()
    {
      return fChannel;
    };

    int ROC ()
    {
      return fROC;
    };

    int Row ()
    {
      return fRow;
    }

    int Column ()
    {
      return fColumn;
    }

    float Charge ()
    {
      return fCharge;
    };

    float Event ()
    {
      return fEvent;
    };

};



class PLTCluster
{
  public:
    PLTCluster () {};
    ~PLTCluster () {
    };

    std::vector<PLTHit*> fHits;


    void AddHit (PLTHit* Hit)
    {
      fHits.push_back(Hit);
      return;
    }

    float Charge ()
    {
      float Sum = 0;
      for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
        Sum += (*it)->Charge();
      }
      return Sum;
    }
};




class PLTPlane
{
  public:
    PLTPlane () {};
    ~PLTPlane ()
    {
      // The Clusters belong to the Plane so we need to delete them
      for (size_t i = 0; i != fClusters.size(); ++i) {
        delete fClusters[i];
      }
    }

    int fChannel;
    int fROC;
    std::vector<PLTHit*> fHits;
    std::vector<PLTCluster*> fClusters;


    void AddHit (PLTHit* Hit)
    {
      fHits.push_back(Hit);
      fChannel = Hit->Channel();
      fROC = Hit->ROC();
      return;
    }

    float Charge ()
    {
      double Sum = 0;
      for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
        Sum += (*it)->Charge();
      }
      return Sum;
    }



    int Channel ()
    {
      return fChannel;
    }

    bool AddClusterFromSeed (PLTHit* Hit)
    {
      PLTCluster* Cluster = new PLTCluster();
      Cluster->AddHit(Hit);
      for (size_t i = 0; i != fHits.size(); ++i) {
        if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) == 1) {
          Cluster->AddHit(fHits[i]);
        }
      }

      fClusters.push_back(Cluster);

      return true;
    }

    bool IsBiggestHitIn3x3(PLTHit* Hit)
    {
      for (size_t i = 0; i != fHits.size(); ++i) {
        if (abs(fHits[i]->Row() - Hit->Row()) == 1 && abs(fHits[i]->Column() - Hit->Column()) <= 1 && fHits[i]->Charge() > Hit->Charge()) {
          return false;
        }
        if (abs(fHits[i]->Column() - Hit->Column()) == 1 && abs(fHits[i]->Row() - Hit->Row()) <= 1 && fHits[i]->Charge() > Hit->Charge()) {
          return false;
        }
      }

      return true;
    }

    void Clusterize ()
    {
      for (size_t i = 0; i != fHits.size(); ++i) {
        if (IsBiggestHitIn3x3(fHits[i])) {
          AddClusterFromSeed(fHits[i]);
        }
      }
      return;
    }

    TH2F* DrawHits2D ()
    {
      TString Name = "Plane_Channel_";
      Name += fChannel;
      Name += "_ROC_";
      Name += fROC;
      TH2F* h = new TH2F(Name.Data(), Name.Data(), 30, 10, 40, 40, 40, 80);
      for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
        h->Fill((*it)->Column(), (*it)->Row(), (*it)->Charge());
      }
      return h;
    }

    size_t NHits ()
    {
      return fHits.size();
    }

    int ROC ()
    {
      return fROC;
    }
};



class PLTTelescope
{
  public:
    PLTTelescope () {};
    ~PLTTelescope ()
    {
      for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
        //delete *it;
      }
    }

    std::vector<PLTPlane*> fPlanes;
    int fChannel;


    void AddPlane (PLTPlane* Plane)
    {
      fPlanes.push_back(Plane);
      fChannel = Plane->Channel();
      return;
    }

    int Channel ()
    {
      return fChannel;
    }

    PLTPlane* Plane(size_t i) {
      return fPlanes[i];
    }

    void Draw2D (int const np, TString const Name)
    {
      std::vector<TH2F*> h;
      TCanvas c("TelescopeHits", "Telescope Hits", 400, 900);
      c.Divide(1,3);
      for (size_t i = 0; i != fPlanes.size(); ++i) {
        c.cd(fPlanes[i]->ROC());
        h.push_back( fPlanes[i]->DrawHits2D() );
        //h[i]->SetMaximum(70000);
        //h[i]->SetMinimum(0);
        //h[i]->Draw("colz");
        h[i]->Draw("box");
      }

      c.SaveAs(Name);
      for (size_t i = 0; i != fPlanes.size(); ++i) {
        delete h[i];
      }

      return;
    }

    size_t NPlanes ()
    {
      return fPlanes.size();
    }
};



class PLTEvent
{
  public:
    PLTEvent ()
    {
      fRun = 0;
      char BUFF[200];
      char BUGG[200];
      for (int iROC = 1; iROC <= 3; ++iROC) {
        sprintf(BUFF, "ChargeHist_Ch22_ROC%i", iROC);
        sprintf(BUGG, "vcal=190 Ch22 ROC %i", iROC);
        fChargeHistMap[ std::make_pair<int, int>(22, iROC) ] = new TH1F(BUFF, BUGG, 100, 0, 50000);
        fChargeHistMap[ std::make_pair<int, int>(22, iROC) ]->GetXaxis()->SetTitle("Electrons");
        fChargeHistMap[ std::make_pair<int, int>(22, iROC) ]->GetYaxis()->SetTitle("Events");
      }
    }
    ~PLTEvent () {
      TFile f("ChargeHists.root", "recreate");
      f.cd();
      for (int iROC = 1; iROC <= 3; ++iROC) {
        fChargeHistMap[ std::make_pair<int, int>(22, iROC) ]->Write();
      }
      f.Close();
      Clear();
    };

    std::map< std::pair<int, int>, TH1F*> fChargeHistMap;
    void MakeChargePlots ()
    {
      for (size_t iTele = 0; iTele != this->NTelescopes(); ++iTele) {
        for (size_t iPlane = 0; iPlane != this->Telescope(iTele)->NPlanes(); ++iPlane) {
          PLTPlane* P = this->Telescope(iTele)->Plane(iPlane);
          if (P->NHits() == 1) {
            fChargeHistMap[ std::make_pair<int, int>(P->Channel(), P->ROC()) ]->Fill( P->Charge() );
          }
          //std::cout << P->Charge() << std::endl;
        }
      }
    }

    std::vector<PLTPlane*> fPlanes;
    std::vector<PLTTelescope*> fTelescopes;
    std::vector<PLTHit*> fHits;

    unsigned long fRun;
    unsigned long fRunSection;
    unsigned long fEvent;

    std::map< int, PLTTelescope> fTelescopeMap;
    std::map< std::pair<int, int>, PLTPlane> fPlaneMap;


    size_t NTelescopes ()
    {
      return fTelescopes.size();
    }

    PLTTelescope* Telescope (size_t i)
    {
      return fTelescopes[i];
    }

    void Clear ()
    {
      // Forst delete anything we need to free memory
      for (size_t i = 0; i != fHits.size(); ++i) {
        delete fHits[i];
      }
      fTelescopeMap.clear();
      fPlaneMap.clear();

      // Clear vectors
      fHits.clear();
      //fClusters.clear();
      fPlanes.clear();
      fTelescopes.clear();
    };

    void AddHit (PLTHit* Hit) {
      fHits.push_back(Hit);
      fEvent = Hit->Event();
      return;
    };

    void Analyze ()
    {
      if (false) {
        printf("Number of hits in event %12lu is: %12lu\n", fEvent, fHits.size());
      }
      MakeEvent();

      if (true) {
        static int nprinted = 0;
        if (nprinted > 10) {
          char BUFF[150];
          for (size_t i = 0; i != fTelescopes.size(); ++i) {
            if (fTelescopes[0]->NPlanes() == 3) {
              sprintf(BUFF, "Telescope_Run%lu_Event%lu_Ch%i.eps", fRun, fEvent, fTelescopes[i]->Channel());
              fTelescopes[0]->Draw2D(1, BUFF);
            }
          }
          ++nprinted;
        }
      }

      return;
    }


    void MakeEvent ()
    {
      for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
        std::pair<int, int> ChannelRoc = std::make_pair<int, int>((*it)->Channel(), (*it)->ROC());
        fPlaneMap[ChannelRoc].AddHit(*it);
      }

      for (std::map< std::pair<int, int>, PLTPlane>::iterator it = fPlaneMap.begin(); it != fPlaneMap.end(); ++it) {
        it->second.Clusterize();
        fTelescopeMap[it->second.Channel()].AddPlane( &(it->second) );
      }

      for (std::map<int, PLTTelescope>::iterator it = fTelescopeMap.begin(); it != fTelescopeMap.end(); ++it) {
        for (size_t i = 0; i != it->second.NPlanes(); ++i) {
          fPlanes.push_back(it->second.Plane(i));
        }
        fTelescopes.push_back( &(it->second) );
      }

      return;
    }
};









int NaiveROCCharge (TString const GainCalFileName, TString const DataFileName)
{
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  PLTGainCal GainCal(GainCalFileName);
  fGainCal = &GainCal;


  std::ifstream f(DataFileName.Data());
  if (!f) {
    std::cerr << "ERROR: cannot open file: " << DataFileName << std::endl;
    exit(1);
  }


  unsigned long LastEvent = -999;
  unsigned long ievent = 0;
  PLTEvent Event;
  for (TString Line; Line.ReadLine(f); ++ievent) {
    //std::cout << ievent << "  " << Line << std::endl;
    PLTHit* ThisHit = new PLTHit(Line);
    if (ThisHit->Event() == LastEvent || ievent == 0) {
      Event.AddHit(ThisHit);
      LastEvent = ThisHit->Event();
    } else {
      Event.Analyze();
      Event.MakeChargePlots();
      Event.Clear();
      Event.AddHit(ThisHit);
      LastEvent = ThisHit->Event();
    }

  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [GainCalFileName] [DataFileName]" << std::endl;
    return 1;
  }

  TString const GainCalFileName = argv[1];
  TString const DataFileName = argv[2];
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout << "DataFileName:    " << DataFileName << std::endl;

  NaiveROCCharge(GainCalFileName, DataFileName);

  return 0;
}
