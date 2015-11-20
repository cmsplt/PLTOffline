////////////////////////////////////////////////////////////////////
//
// Krishna Thapa <kthapa@cern.ch>
//
// Created on: Thu November 05 2015
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include <iomanip>

#include "PLTEvent.h"
#include "PLTU.h"
#include "PLTTrack.h"
#include "PLTAlignment.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"
#include "TFile.h"

/*
  Generate per channel Coinc_Boolean, NCoinc, Double_Column, NEvents, NEmptyEvents

  Event->Telescope->Plane

  Double_Columns, DC = Number of double columns in a Plane  

  Coinc_Boolean[chNo] == Every plane has  a hit
  ==> For every channel, every 5 mins, get Coinc_Boolean count, DC count
  
  Coinc_BoolP == Within 3HitPlaneBits, if DC > 3, count++;p->prime
  
  NCoinc[chNo] = min(DC0,DC1,DC2)
  ==> For every channel, every 5 mins, get NCoinc count
  NCoincP = Within 3HitPlaneBits, if either of DC >=3, Then get min.
  
  NTracks[chNo] = Telescope->NTracks() st. Tracks->NTracks() ==3, which is satisfied by the 3HitPlaneBits condition.
  NTracksP = ntracks when max(DC0,DC1,DC3) >=3
  

*/


inline int min(int a, int b){
  return a > b ? b : a;
}

inline int max(int a, int b){
  return a > b ? a : b;
}


int CoincStudies(std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName)
{
  std::cout << "DataFileName:    " << DataFileName << std::endl;  
  TFile f("CoincStudies.root","RECREATE");
  
  // Set some basic style
  PLTU::SetStyle();

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_All;
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  
  PLTAlignment Alignment;
  Alignment.ReadAlignmentFile(AlignmentFileName);



  // ******************************** Initialize **********************************//

  
  std::map<int, std::pair<int,int> > cBoolRocMap;   //(rocId, (dCoinc, dc)) , dCoinc = #triple Coincidences
  std::map<int, std::pair<int,int> > cAllChMap;  // (ch, (all min_dc, min_dc @ bug))

  std::map<int, std::pair<int,int> > cTrackChMap;  // tracks affected by DC, (ch, (all ntracks, ntracks @ bug))
  
  std::map<int,int> cdTrackChMap; // Accidentals, digital good track, with square/box cut 
  std::map<int,int> cdTrackChMapC; // Accidentals, digital good track, with circular/oval cut



  TH1F *RxH = new TH1F("Rx","Rx",100,-0.5,0.5);
  TH1F *RyH = new TH1F("Ry","Ry",100,-0.5,0.5);  

  TH1F *SxH = new TH1F("Sx","Sx",100,-1.0,1.0);
  TH1F *SyH = new TH1F("Sy","Sy",100,-1.0,1.0);


  

  std::map<int,std::pair<int,int> > chBugMap;  // (ch, (dc,dc)), redundant. count of dc per channel
  std::map<int,std::pair<int,int> > timeStampWindow; // (tm, (tFrom, tTo))
  std::set<int> snapshotCounter; // Tm, unique set
  
  std::map< int, int > col_count;

  int emptyEvents = 0; int totalEvents = 1;      
  int tLast = 0; 
  int NEvents = 0; int snapshotC = 0;

  //  int to = 700000;
  int div = 300000; //5 minute snapshot, 60 * 5*10^3 milliseconds
  //  int to = 70000;
  //  int div = 5000;

  bool aggregateFlag = false;
  
  int t0 = 0; int ientry = 0;


  // ******************************** Output Files **********************************//

  std::ofstream perChannel ("coincCH_DC.txt");
  if(perChannel.is_open()){
  
    std::ofstream perROC ("coincROC_DC.txt");
    if(perROC.is_open()){

      std::ofstream meanRMS ("meanRMS_DC.txt");
      if(meanRMS.is_open()){
      
        meanRMS<< "#Tm" << std::setw(5)<< "RxMean" <<std::setw(10) << "RxRMS"<< std::setw(10) << "RyMean" << std::setw(10) << "RyRMS" << std::setw(10)<< "SxMean" <<std::setw(10) << "SxRMS"<< std::setw(10) << "SyMean" << std::setw(10) << "SyRMS" << std::endl;
            

        perROC << "#Tm" <<  std::setw(5) << "chNo"<<  std::setw(10) << "rocID" << std::setw(10) << "coinc" <<  std::setw(10) << "dcROC" << std::setw(10) << "dcCount" << std::setw(10)<< "ntgr" << std::setw(10) << "nempty" <<"\n";

        perChannel << "#Tm" << std::setw(5) << "chNo" << std::setw(10) << "dTrack" << std::setw(10) << "dTrackC" << std::setw(10) <<"NTracks" <<std::setw(10)<<"TrDC" << std::setw(10)<< "coinc" <<std::setw(10)<< "tcDC"<<std::setw(10)<<"NCoinc"<<std::setw(10) <<"nDC" <<std::setw(10) <<"NdcCount" <<std::setw(10) << "ntgr" << std::setw(10) << "nempty" <<std::setw(12) << "tFrom" <<  std::setw(12) << "tTo"<<"\n";

    
        // Loop over all events in file
        for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    
          //        if (ientry >= to){break;} 
    
          totalEvents +=1;           

          if(Event.NHits() == 0){
            emptyEvents += 1;
          }
    
          if(ientry == 0 ) {
            t0 = Event.Time();
          }

          int ID = Event.Time();
          int timeDiff = ID - t0;
          int tmp = timeDiff / div;

          // new timestamp/snapshot
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


          //Loop through telescopes for this event    
          for (size_t tel = 0; tel != Event.NTelescopes(); tel++) {

            // THIS telescope is
            PLTTelescope* Telescope = Event.Telescope(tel);
            int chNo = Telescope->Channel();

            // binary number for planes hit
            int phit = Telescope->HitPlaneBits();


            // if all 3 planes have a hit
            if(phit==0x7){


              std::vector<int> planes_DC(3);
              std::vector<int> planes_Hits(3);

            
              int nAccTracks = 0;
              int nAccTracksC = 0;
              int ntracks = 0;

              Float_t Sx, Sy, Rx, Ry;
              
              for (uint i=0;i != Telescope->NTracks();++i){
                PLTTrack* Track = Telescope->Track(i);
              
                if (Track->NClusters() == 3) {
                  ntracks++;


                  Sx = (Track->Cluster(2)->TX()-Track->Cluster(0)->TX())/7.54;
                  Sy = (Track->Cluster(2)->TY()-Track->Cluster(0)->TY())/7.54;
                  Rx= (pow(Track->LResidualX(0),2)+pow(Track->LResidualX(1),2)+pow(Track->LResidualX(2),2));
                  Ry = (pow(Track->LResidualY(0),2)+pow(Track->LResidualY(1),2)+pow(Track->LResidualY(2),2));

                  RxH->Fill(Rx);
                  RyH->Fill(Ry);
                  SxH->Fill(Sx);
                  SyH->Fill(Sy);                
                
                  // Accidentals
                                                  
                  // Fill 4444 cut
                  if (sqrt(Rx) < 0.03 && sqrt(Ry)< 0.03 && sqrt(pow(0.027-Sy,2) + pow((0.007/0.02)*Sx,2)) < 0.007) {
                    nAccTracksC++;                  
                  }
                
                  if (sqrt(Rx) < 0.03 && sqrt(Ry)< 0.03 && abs(0.027-Sy)<0.007 && abs(Sx) < 0.02) {
                  
                    nAccTracks++;                  
                  }
                                                               
                }

              }


              if(nAccTracks) {
                cdTrackChMap[chNo] += 1;
              }


            
              if(nAccTracksC) {
                cdTrackChMapC[chNo] += 1;
              }
                    

              // Loop through ROCs in this telescope
              for (size_t ip = 0; ip != 3; ++ ip){

                int rocID = chNo * 10 + ip;
            
                int DC = 0;    // Number of double columns
                PLTPlane* Plane = Telescope->Plane(ip);


                planes_Hits[ip] = Plane->NHits();              
                //              planes_Clusters[ip] = Plane->NClusters();
              
                // get 0-51 columns for hits and store cols / 2 ==> 0-26 "pre-double columns"
                std::set <int> hit_cols; 
            
                for (size_t hitN = 0; hitN != Plane->NHits(); ++ hitN){
                  PLTHit* Hit = Plane->Hit(hitN);
                  hit_cols.insert(Hit->Column()/2);
                }

                int set_sz = hit_cols.size();
                std::map<int, int> hit_cols_uniq;	                  

                // loop only if more than 1 double column
                if (set_sz > 1) {
                  // save it in a vector to access index later
                  
                  int index = 0;
                  for (std::set<int>::iterator it = hit_cols.begin(); it !=hit_cols.end(); ++it){
                    hit_cols_uniq[index] = *it;
                    index++;
                  }
                } else if (set_sz == 1){
                  DC = 1;
                } else {
                  DC = 0;
                }

                // if there are more than 1 double columns, check for DC "Bug"
                for (std::map<int, int>::iterator it = hit_cols_uniq.begin(); it !=hit_cols_uniq.end(); ++it){
                  if(it->first == 0) {
                    DC = 1;
                  } else if (hit_cols_uniq[it->first] == hit_cols_uniq[it->first -1]+1) {
                    continue;
                  } else {
                    DC++;
                  }
                
                }
            
                planes_DC[ip] = DC;
              
                // For this particular ROC, keep track of N_coincidence and N_>=3DC
                if (DC >=3) {
                  cBoolRocMap[rocID].first += 1;
                  cBoolRocMap[rocID].second += 1;
                } else {
                  cBoolRocMap[rocID].first += 1;
                }                      
            
              } // rOC


            

              // Correct for the ROC's Ch2-ROC0, and CH10-ROC1
              // works only for digital count
              if (chNo == 2){
                if (planes_DC[0] >=3 && planes_DC[1] < 3 && planes_DC[2] < 3) {
                  planes_DC[0] = 1;                
                }

              }

              if (chNo == 10){
                if (planes_DC[1] >=3 && planes_DC[0] < 3 && planes_DC[2] < 3) {
                  planes_DC[1] = 1;                               
                }
              }
              
                          
              int min_DC = min(planes_DC[0],min(planes_DC[1],planes_DC[2]));
              int max_DC = max(planes_DC[0],max(planes_DC[1],planes_DC[2]));            
            

              // if max_DC >=3, increment DC counter
              if (max_DC >=3){
                
                cAllChMap[chNo].first += min_DC;
                cAllChMap[chNo].second += min_DC;
              

                chBugMap[chNo].first += (planes_DC[0] || planes_DC[1] || planes_DC[2]);
                chBugMap[chNo].second += 1;

                cTrackChMap[chNo].first += ntracks;
                cTrackChMap[chNo].second += ntracks;            
              } else {
                cAllChMap[chNo].first += min_DC;
                cTrackChMap[chNo].first += ntracks;
              }
              


            } // coincidence


          } // telescope loop ends here
         

          // *************************** Write to File after 5 mins************************//
        
          if (aggregateFlag){

            std::cout << snapshotC<<" vars: " <<RxH->GetRMS() << " " << RxH->GetMean()<<" " <<SxH->GetRMS() << " " << SxH->GetMean()<<" "<<RyH->GetRMS() << " " << RyH->GetMean()<<" " <<SyH->GetRMS() << " " << SyH->GetMean()<<std::endl; 

            
            meanRMS<< snapshotC << std::setw(5)<< RxH->GetMean() <<std::setw(10) << RxH->GetRMS()<< std::setw(10) << RyH->GetMean() << std::setw(10) << RyH->GetRMS() << std::setw(10)<< SxH->GetMean() <<std::setw(10) << SxH->GetRMS()<< std::setw(10) << SyH->GetMean() << std::setw(10) << SyH->GetRMS() << std::endl;
            
            
            
            RxH->Reset();
            RyH->Reset();
            SxH->Reset();
            SyH->Reset();

          
            std::cout << "Number of events in this timeWindow: " << NEvents << std::endl;
            std::cout << "snapshotC: " << snapshotC << " timeWindow: (" << timeStampWindow[snapshotC].first << " , " << timeStampWindow[snapshotC].second << ")" << std::endl;


            int tDC = 0;    // total number of double columns for a particular channel
          
            for (std::map<int, std::pair<int,int> >::iterator it = cBoolRocMap.begin(); it != cBoolRocMap.end(); ++it) {

              int ch = it->first / 10;
              int roc = it->first % 10;



              tDC += cBoolRocMap[ch].second; // total DC cases

              // write to file: per ROC
              perROC<< snapshotC << std::setw(5) << ch <<std::setw(10) << roc << std::setw(10) << cBoolRocMap[ch].first << std::setw(10) << cBoolRocMap[ch].second << std::setw(10)<<chBugMap[ch].first << std::setw(10) << NEvents << std::setw(10) << emptyEvents<<std::endl;
            
              // write to file: per Channel
              if(roc == 2) {
                perChannel << snapshotC << std::setw(5) << ch << std::setw(10) << cdTrackChMap[ch]<<std::setw(10)<< cdTrackChMapC[ch]<<std::setw(10) << cTrackChMap[ch].first << std::setw(10) << cTrackChMap[ch].second<<std::setw(10) << cBoolRocMap[ch].first << std::setw(10) << tDC << std::setw(10)<< cAllChMap[ch].first << std::setw(10) <<cAllChMap[ch].second << std::setw(10)<<chBugMap[ch].second<< std::setw(10) << NEvents << std::setw(10) << emptyEvents<<  std::setw(12) << timeStampWindow[snapshotC].first <<  std::setw(12) << timeStampWindow[snapshotC].second <<std::endl;    

                tDC = 0;       // reset to 0 for another channel
                cdTrackChMap[ch] = 0;
                cdTrackChMapC[ch] = 0;
              }
            
            
              cBoolRocMap[ch].first = 0;
              cBoolRocMap[ch].second = 0;

            
            };

            for (std::map<int, std::pair<int,int> >::iterator it = cAllChMap.begin(); it != cAllChMap.end(); ++it) {
              cAllChMap[it->first].first = 0;
              cAllChMap[it->first].second = 0;                          
            }

            for (std::map<int, std::pair<int,int> >::iterator it = chBugMap.begin(); it != chBugMap.end(); ++it) {
              chBugMap[it->first].first = 0;
              chBugMap[it->first].second = 0;              

            }

            for (std::map<int, std::pair<int,int> >::iterator it = cTrackChMap.begin(); it != cTrackChMap.end(); ++it) {
              cTrackChMap[it->first].first = 0;
              cTrackChMap[it->first].second = 0;            
            }
                  
            totalEvents = 1;
            emptyEvents = 0;
            aggregateFlag = false;

          } // aggregateFlag
        } // NEvents
    
      }
      meanRMS.close();
    }
    perROC.close();      
  }
  perChannel.close();
    
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
  
  CoincStudies(DataFileName, GainCalFileName, AlignmentFileName);

  return 0;
}

