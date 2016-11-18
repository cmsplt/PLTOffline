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

/*
Generate per channel NTracks, NClusters, extra-clusters(ncluster>3), N_empty_events,N_events.
 */

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
       


  std::map<int, int> hNTrackMap;  
  std::map<int, int> hNClusterMap;  
  std::map<int, int> hexNClusterMap;  


  std::map<int, double> emptyEventsMap;  
  std::map<int,std::pair<int,int> > timeStampWindow;
  std::set<int> snapshotCounter;

  
  int emptyEvents = 0; int totalEvents = 1;      
  int tLast = 0; int tNow = 0;
  int NEvents = 0; int snapshotC = 0;
    
  //  int to = 50000000;
        int div = 300000; //5 minute snapshot, 60 * 5*10^3 milliseconds
	//  	int to = 100000;
	//  	int div = 10000;

  bool aggregateFlag = false;  
  int t0 = 0; int ientry = 0;


  std::ofstream outFile ("OutFile.txt");
  if(outFile.is_open()){
    
    std::ofstream eTFile ("emptyEvents.txt");
    if(eTFile.is_open()){
    
    outFile << "#"<< "Time(5mins)" << " " << "chNo" << " " << "extraCl" <<" "<< "Ncluster" << " " << "Ntracks" <<" " << "AvgNClusters" << " " << "AvgNTracks" <<" " << "tFrom" << " " << "tTo"<<"\n";
    // Loop over all events in file
    for ( ; Event.GetNextEvent() >= 0; ++ientry) {

      //      if (ientry >= to){break;} 

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
	  hNTrackMap[chNo] = 0;
	  hNClusterMap[chNo] = 0;
	  hexNClusterMap[chNo] = 0;
	}


        int nclusters = Telescope->NClusters();
	//        int ntracks = Telescope->NTracks();
	int nhitplanes = Telescope->NHitPlanes();
	
	
	if (nhitplanes == 3) {

	for (size_t itrack = 0; itrack != Telescope->NTracks(); ++itrack){
	    PLTTrack* Track = Telescope->Track(itrack);

	    if (Track->NClusters() >= 3){
	      hNTrackMap[chNo] += 1;
	    }
	  }


	  if(nclusters !=0) {
	    hNClusterMap[chNo] += nclusters;
	  }

	  if(nclusters > 3) {
	    hexNClusterMap[chNo] += nclusters;
	  }

	  //	  if(ntracks !=0) {
	  //	    hNTrackMap[chNo] += ntracks;
	  //	  }
	}
      }
      
      if (aggregateFlag) {
      
        std::cout << "Aggregating histograms... " << " ientry: " << ientry << std::endl;
        std::cout << "Number of events in this timeWindow: " << NEvents << std::endl;
        std::cout << "snapshotC: " << snapshotC << " timeWindow: (" << timeStampWindow[snapshotC].first << " , " << timeStampWindow[snapshotC].second << ")" << std::endl;      


	eTFile << snapshotC<< " " <<  1.0* emptyEvents/ (totalEvents -1)<<"\n";

        printf("Collecting counts for all Channels\n" );          
          
        // aggregate cluster count and track count for every telescope
	//        for (std::map<int, TH1F*>::iterator it = hNClusterMap.begin(); it != hNClusterMap.end(); ++it) {

        for (std::map<int, int>::iterator it = hNClusterMap.begin(); it != hNClusterMap.end(); ++it) {

          int clCount = hNClusterMap[it->first];
          int trCount = hNTrackMap[it->first];
          int exClcount = hexNClusterMap[it->first];

          double AvgNCluster = 1.0 * hNClusterMap[it->first] / NEvents;
          double AvgNTrack = 1.0 * hNTrackMap[it->first]/ NEvents;

          outFile << snapshotC << " " << it->first << " " << exClcount <<" "<< clCount << " " << trCount <<" " << AvgNCluster << " " << AvgNTrack<<" " << timeStampWindow[snapshotC].first<< " " << timeStampWindow[snapshotC].second<<"\n";
          // reset counts           

	  hNTrackMap[it->first] = 0;
	  hNClusterMap[it->first] = 0;
	  hexNClusterMap[it->first] = 0;

        }       
          
            
      totalEvents = 1;
      emptyEvents = 0;
      aggregateFlag = false;
      }
 
    }

    }
    eTFile.close();
  }
  outFile.close();

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
