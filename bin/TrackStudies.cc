////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Wed September 10 2015
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"
#include "TFile.h"


int TrackStudies(std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;  
  TFile f("TrackPlots.root","RECREATE");
  
  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);
       
  std::map<int, TH1F*> hNTrackMap;  
  std::map<int, TH1F*> hNClusterMap;
  std::map<int, TH1F*> hexNClusterMap; //NCluster > 3
  std::map<int, double> emptyEventsMap;  
  std::map<int,std::pair<int,int> > timeStampWindow;
  std::set<int> snapshotCounter;

  
  int emptyEvents = 0; int totalEvents = 1;      
  int tLast = 0; int tNow = 0;
  int NEvents = 0; int snapshotC = 0;
    
  //  int to = 50000000;
  int div = 300000; //5 minute snapshot, 60 * 5*10^3 milliseconds
  //  int to = 100000;
  //  int div = 3000;

  bool aggregateFlag = false;  
  int t0 = 0; int ientry = 0;


  std::ofstream outFile ("OutFile.txt");
  if(outFile.is_open()){

    outFile << "#"<< "Time(5mins)" << " " << "chNo" << " " << "extraCl" <<" "<< "Ncluster" << " " << "Ntracks" <<" " << "AvgNClusters" << " " << "AvgNTracks" <<" " << "tFrom" << " " << "tTo"<<"\n";
    // Loop over all events in file
    for ( ; Event.GetNextEvent() >= 0; ++ientry) {

      //    if (ientry >= to){break;} 

      totalEvents +=1;        
    
      if(ientry == 0 ) {
        t0 = Event.Time();
      }
      int ID = Event.Time();
      int timeDiff = ID - t0;
      int tmp = timeDiff / div;
    
      if (snapshotCounter.count(tmp) == 0){
        snapshotC = tmp;
        snapshotCounter.insert(tmp);
        std::cout << " Max element: " <<  snapshotCounter.size() << " snapshot: " << snapshotC<< std::endl;
        
        std::cout << " Making time pairs for snapshot: " << snapshotC<< std::endl;
        int tNow = Event.Time();
        timeStampWindow[snapshotC] = std::make_pair(tLast, tNow);
        tLast = tNow;           
        NEvents = totalEvents;      // can also subtract emptyevents 
        aggregateFlag = true;
      }


      // to take care of the rollover of time, assuming t0 = 0
      // size of snapshotCounter gives the latest time chunk
      // eg. if tmp = 1 when snapshotCounter.size() == 100, snapshotC = tmp + snapshotCounter.size() = 101 which is what we want
      if (tmp + 1 != snapshotCounter.size()) {
        std::cout << " Caught time rollover: " << Event.Time()<< " " << ientry << std::endl;
        snapshotC = tmp + snapshotCounter.size();
        snapshotCounter.insert(tmp);
      
        int tNow = Event.Time();
        timeStampWindow[snapshotC] = std::make_pair(tLast, tNow);
        tLast = tNow;      
        NEvents = totalEvents;      
        aggregateFlag = true;
      
      }

    
      if (ientry % div == 0) {            
        std::cout << "Current time index:: "  << snapshotC << " ientry: " << ientry << " timeDiff(ms): " << timeDiff<< std::endl;
      } 

    
      if(Event.NHits() == 0){
        emptyEvents += 1;
      }

      // Loop over all planes with hits in event
      for (size_t tel = 0; tel != Event.NTelescopes(); tel++) {

        // THIS telescope is
        PLTTelescope* Telescope = Event.Telescope(tel);
        int chNo = Telescope->Channel();
       
        if (!hNTrackMap.count(chNo)) {
          TString Name = TString::Format("NTracks_Ch%02i",  chNo);
          hNTrackMap[chNo] = new TH1F(Name, Name, 100, 0, 5);

          Name = TString::Format("NClusters_Ch%02i",  chNo);
          hNClusterMap[chNo] = new TH1F(Name, Name, 100, 0, 40);

          Name = TString::Format("exNCluster_Ch%02i",  chNo);
          hexNClusterMap[chNo] = new TH1F(Name, Name, 100, 0, 40);            
        }


        int nclusters = Telescope->NClusters();
        int ntracks = Telescope->NTracks();

        if(nclusters !=0) {
          hNClusterMap[chNo]->Fill(nclusters);
        }

        if(nclusters > 3) {
          hexNClusterMap[chNo]->Fill(ntracks);
        }
          
        if(ntracks !=0) {
          hNTrackMap[chNo]->Fill(ntracks); 
        }
      }

    
      if (aggregateFlag) {
      
        std::cout << "Aggregating histograms... " << " ientry: " << ientry << std::endl;
        std::cout << "Number of events in this timeWindow: " << NEvents << std::endl;
        std::cout << "snapshotC: " << snapshotC << " timeWindow: (" << timeStampWindow[snapshotC].first << " , " << timeStampWindow[snapshotC].second << ")" << std::endl;      

        emptyEventsMap[snapshotC] = (100.0 * emptyEvents) / (totalEvents  -1 ) ;


        printf("Collecting counts from histograms for all Channels\n" );          
          
        // aggregate cluster count and track count for every telescope
        for (std::map<int, TH1F*>::iterator it = hNClusterMap.begin(); it != hNClusterMap.end(); ++it) {
          int clCount = hNClusterMap[it->first]->GetEntries();
          int trCount = hNTrackMap[it->first]->GetEntries();
          int exClcount = hexNClusterMap[it->first]->GetEntries();
          double AvgNCluster = 1.0 * hNClusterMap[it->first]->GetEntries() / NEvents;
          double AvgNTrack = 1.0 * hNTrackMap[it->first]->GetEntries() / NEvents;

          outFile << snapshotC << " " << it->first << " " << exClcount <<" "<< clCount << " " << trCount <<" " << AvgNCluster << " " << AvgNTrack<<" " << timeStampWindow[snapshotC].first<< " " << timeStampWindow[snapshotC].second<<"\n";


          // reset histograms            
          hNClusterMap[it->first]->Reset();
          hexNClusterMap[it->first]->Reset();                
          hNTrackMap[it->first]->Reset();            
        }       
          
            
      totalEvents = 1;
      emptyEvents = 0;
      aggregateFlag = false;
    }
 
    }
  outFile.close();
    
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
  
  TrackStudies(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}
