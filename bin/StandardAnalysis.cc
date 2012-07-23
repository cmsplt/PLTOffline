////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
// Allison Sachs     <asachs2@utk.edu>
//
// Created on: Tue Jun  5 16:41:13 CEST 2012
//
// Allison's dirty and ugly prototype of Standard Analysis!
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include <numeric>
#include <cstring>
#include <vector>

#include "PLTEvent.h"
#include "PLTU.h"
#include "PLTHistReader.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TRegexp.h"
#include "TStyle.h"
#include "TLine.h"
#include "TROOT.h"
#include "TSystem.h"


// FUNCTION DEFFINITIONS HERE



float Average (std::vector<float>& V)
{
  double Sum = 0;
  for (std::vector<float>::iterator it = V.begin(); it != V.end(); ++it) {
    Sum += *it;
  }

  return Sum / (float) V.size();
}


TH1F* FidHistFrom2D (TH2F* hIN, TString const NewName, int const NBins, PLTPlane::FiducialRegion FidRegion)
{
  // This function returns a TH1F* and YOU are then the owner of
  // that memory.  please delete it when you are done!!!

  int const NBinsX = hIN->GetNbinsX();
  int const NBinsY = hIN->GetNbinsY();
  float const ZMin = hIN->GetMinimum();
  float const ZMax = hIN->GetMaximum() * (1.0 + 1.0 / (float) NBins);
  int const MyNBins = NBins + 1;

  TString const hNAME = NewName == "" ? TString(hIN->GetName()) + "_1DZFid" : NewName;

  TH1F* h;
  h = new TH1F(hNAME, hNAME, MyNBins, ZMin, ZMax);
  h->SetXTitle("Number of Hits");
  h->SetYTitle("Number of Pixels");
  h->GetXaxis()->CenterTitle();
  h->GetYaxis()->CenterTitle();
  h->SetTitleOffset(1.4, "y");
  h->SetFillColor(40);

  for (int ix = 1; ix <= NBinsX; ++ix) {
    for (int iy = 1; iy <= NBinsY; ++iy) {
      int const px = (int) hIN->GetXaxis()->GetBinLowEdge(ix);
      int const py = (int) hIN->GetYaxis()->GetBinLowEdge(iy);
      if (PLTPlane::IsFiducial(FidRegion, px, py)) {
        if (hIN->GetBinContent(ix, iy) > ZMax) {
          h->Fill(ZMax - hIN->GetMaximum() / (float) NBins);
        } else {
          h->Fill( hIN->GetBinContent(ix, iy) );
        }
      }
    }
  }

  return h;
}

TH2F* Get3x3EfficiencyHist (TH2F& HistIn, int const FirstCol, int const LastCol, int const FirstRow, int const LastRow)
{

  TH2F* HistEff = (TH2F*) HistIn.Clone(HistIn.GetName() + TString("_3x3Efficiency"));
  HistEff->Reset();

  for (int icol = 1; icol <= HistIn.GetNbinsX(); ++icol) {

    // What pixel column is this?  If it's outside the range skip it
    int const PixCol = (int) HistEff->GetXaxis()->GetBinLowEdge(icol);
    if (PixCol < FirstCol || PixCol > LastCol) {
      continue;
    }

    for (int irow = 1; irow <= HistIn.GetNbinsY(); ++irow) {

      // What row is this?  If it's outside the range skip it
      int const PixRow = (int) HistEff->GetYaxis()->GetBinLowEdge(irow);
      if (PixRow < FirstRow || PixRow > LastRow) {
        continue;
      }


      // For *this* pixel get the 3x3 surrounding values..  If a neighbor is outside of bounds we skip it
      std::vector<int> HitsNearThisPixel;
      for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
          if (PixCol + i >= FirstCol && PixCol + i <= LastCol && PixRow + j >= FirstRow && PixRow + j <= LastRow) {
            if ( (i != 0 || j != 0)) {
              //std::cout << "here " << HistIn.GetBinContent(icol + i, irow + j) << std::endl;
            }
            if ( (i != 0 || j != 0) && HistIn.GetBinContent(icol + i, irow + j) > 0) {
              HitsNearThisPixel.push_back( (int) HistIn.GetBinContent(icol + i, irow + j));
            }
          }
        }
      }

      // Calcluate average neighbor occupancy
      float const NeighborsMean = std::accumulate(HitsNearThisPixel.begin(), HitsNearThisPixel.end(), 0) / (float) HitsNearThisPixel.size();

      // Set the content to itself divided by the average in neighbors
      if (NeighborsMean > 0) {
        HistEff->SetBinContent(icol, irow, HistIn.GetBinContent(icol, irow) / NeighborsMean);
        //printf("Efficiency: %2i %2i   %12.3f\n", PixCol, PixRow, HistIn.GetBinContent(icol, irow) / NeighborsMean);
      }

    }
  }

  return HistEff;
}






// CODE BELOW







int StandardAnalysis(std::string const DataFileName, std::string const GainCalFileName, std::string const AlignmentFileName, std::string const OutDir,  std::string const RunNumber)
{
  std::cout << "DataFileName:      " << DataFileName << std::endl;
  std::cout << "GainCalFileName:   " << GainCalFileName << std::endl;
  std::cout << "AlignmentFileName: " << AlignmentFileName << std::endl;
  std::cout << "OutDir:            " << OutDir << std::endl;
  std::cout << "RunNumber:         " << RunNumber << std::endl;


  // Make output dir
  if (gSystem->mkdir(OutDir.c_str(), true) != 0) {
    std::cerr << "WARNIGN: either OutDir exists or it is un-mkdir-able: " << OutDir << std::endl;
  }

  // Set some basic style
  PLTU::SetStyle();
  gStyle->SetOptStat(111111);

  // Hist colors for pulse heights
  int const HistColors[4] = { 1, 4, 28, 2 };


  // Output name for root file, just tack it on the end..
  TString const NI = DataFileName;
  TString const BaseInName = NI.Contains('/') ? NI(NI.Last('/')+1, NI.Length()-NI.Last('/')-1) : NI;
  TString const OutFileName =  BaseInName + ".PHT.root";
  std::cout << "PulseHeights saved to: " << OutFileName << std::endl;

  // Output root file
  TFile OUTFILE(TString(OutDir.c_str()) + "/" + OutFileName, "recreate");
  if (!OUTFILE.IsOpen()) {
    std::cerr << "ERROR: cannot open output file" << std::endl;
    exit(1);
  }



  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName, AlignmentFileName);
  PLTPlane::FiducialRegion MyFiducialRegion = PLTPlane::kFiducialRegion_Diamond;
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_Diamond);
  Event.SetPlaneFiducialRegion(MyFiducialRegion);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_01to2_All);

  //Searching for treasure!!
  std::map<int, TH2F*>               hAllMap;
  std::map<int, TCanvas*>            cAllMap;
  std::map<int, TH2F*>               hOccupancyMap;
  std::map<int, TCanvas*>            cOccupancyMap;
  std::map<int, TH2F*>               hQuantileMap;
  std::map<int, TCanvas*>            cQuantileMap;
  std::map<int, TCanvas*>            cProjectionMap;
  std::map<int, TCanvas*>            cEfficiencyMap;
  std::map<int, TH2F*>               hEfficiencyMap;
  std::map<int, TH1F*>               hEfficiency1DMap;
  std::map<int, TCanvas*>            cCoincidenceMap;
  std::map<int, TH1F*>               hCoincidenceMap;
  std::map<int, TH2F*>               hMeanMap;
  std::map<int, TCanvas*>            cMeanMap;
  std::map<int, std::vector<TGraphErrors*> > gClEnTimeMap;
  std::map<int, TH1F*>               hClusterSizeMap;
  std::map<int, TCanvas*>            cClusterSizeMap;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;
  std::map<int, TH2F* >              hMap2D;
  std::map<int, TCanvas*>            cMap2D;

  double Avg2D[250][PLTU::NCOL][PLTU::NROW];
  int      N2D[250][PLTU::NCOL][PLTU::NROW];


  // Bins and max for pulse height plots
  int   const NBins =    60;
  float const XMin  =  -1000;
  float const XMax  =  50000;

  // Time width in events for energy time dep plots
  uint32_t const TimeWidth = 1000 * 3;
  std::map<int, std::vector< std::vector<float> > > ChargeHits;

  TH1F HistNTracks("NTracksPerEvent", "NTracksPerEvent", 50, 0, 50);

  //Character buffer for writing names
  char BUFF[200];


  std::vector<int> unique;
  std::vector<double> firstMean;
  std::vector<double> secondMean;
  std::vector<double> thirdMean;
  std::vector<double> fourthMean;

  std::vector<double> firstRMS;
  std::vector<double> secondRMS;
  std::vector<double> thirdRMS;
  std::vector<double> fourthRMS;






  ///////////////////////////////////////
  //                                   //
  //         BEGIN EVENT LOOP          //
  //                                   //
  //   Loop over all events in file    //
  ///////////////////////////////////////

  int ie = 0;
  int NGraphPoints = 0;

  TH1F hBX("Bunch Crossing", "Bunch Crossing", PLTHistReader::NBUCKETS, 0, PLTHistReader::NBUCKETS);


  for ( ; Event.GetNextEvent() >= 0 ; ++ie) 
  {

    if (ie % 10000 == 0) 
    {
      std::cout << "Processing Event:" << ie << std::endl;
    }

    //BunchCrossing??
    int BX = Event.BX();
    hBX.Fill(BX);

    if (ie == 100000) break;


    ///////////////////////////////////////
    //                                   //
    //         PHT Timing Stuff          //
    //                                   //
    //  Dean's crazy crazy crazy stuff   //
    ///////////////////////////////////////

    int NTracksPerEvent = 0;

    // First event time
    //static uint32_t const StartTime = Event.Time();
    //uint32_t const ThisTime = Event.Time();
    static uint32_t const StartTime = 0;
    uint32_t static ThisTime = 0;
    ++ThisTime;

    while (ThisTime - (StartTime + NGraphPoints * TimeWidth) > TimeWidth) 
    {
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
    }

    ///////////////////////////////////////
    //        End of timing stuff        //
    ///////////////////////////////////////
    //                                   //
    //         BEGIN TELES LOOP          //
    //                                   //
    // Loop over all telescopes in file  //
    ///////////////////////////////////////

    for (size_t ip = 0; ip != Event.NTelescopes(); ++ip) 
    {
      PLTTelescope* Tele = Event.Telescope(ip);

      NTracksPerEvent += Tele->NTracks();
      int const Channel = Tele->Channel();



      ///////////////////////////////////////
      //                                   //
      //         BEGIN TRACK LOOP          //
      //                                   //
      // Loop over all tracks in telescope //
      ///////////////////////////////////////

      for (size_t itrack = 0; itrack < Tele->NTracks(); ++itrack) 
      {
        PLTTrack* Track = Tele->Track(itrack);



        ///////////////////////////////////////
        //                                   //
        //         BEGIN CLUST LOOP          //
        //                                   //
        // Loop over all clusters in tracks  //
        ///////////////////////////////////////

        //std::cout << Track->NClusters() << std::endl;
        for (size_t icluster = 0; icluster < Track->NClusters(); ++icluster)
        {

          PLTCluster* Cluster = Track->Cluster(icluster);

          int const ROC = Cluster->ROC();

          // ID the plane and roc by 3 digit number
          int const id = 10 * Channel + ROC;


          //std::cout << hMap.count(id) << std::endl;
          if (!hMap.count(id)) {
            hMap[id].push_back( new TH1F( TString::Format("Track Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ROC),
                  TString::Format("PulseHeightTrack_Ch%02i_ROC%1i_All", Channel, ROC), NBins, XMin, XMax) );
            hMap2D[id] = new TH2F( TString::Format("Avg Charge Ch %02i ROC %1i Pixels All", Channel, ROC),
                TString::Format("PixelCharge_Ch%02i_ROC%1i_All", Channel, ROC), PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, 
                PLTU::FIRSTROW, PLTU::LASTROW);
            for (size_t ih = 1; ih != 4; ++ih) {

              hMap[id].push_back( new TH1F( TString::Format("Track Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                    TString::Format("PulseHeightTrack_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih), NBins, XMin, XMax) );
            }

            // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
            if (!cMap.count(Channel)) {
              // Create canvas with given name
              cMap[Channel] = new TCanvas( TString::Format("PulseHeightTrack_Ch%02i", Channel), TString::Format("%s/PulseHeightTrack_Ch%02i", OutDir.c_str(), Channel), 900, 900);
              cMap[Channel]->Divide(3, 3);
            }
          }

          if (!gClEnTimeMap.count(id)) {
            gClEnTimeMap[id].resize(4);
            for (size_t ig = 0; ig != 4; ++ig) {
              TString const Name = TString::Format("TimeAvgGraph_id%i_Cl%i", (int) id, (int) ig);
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
            hClusterSizeMap[id] = new TH1F( TString::Format("ClusterSizeTrack_Ch%02i_ROC%i", Channel, ROC), TString::Format("ClusterSizeTrack_Ch%02i_ROC%i", Channel, ROC), 10, 0, 10);
            hClusterSizeMap[id]->SetXTitle("Number of pixels in Cluster");

            // One in three chance you'll need a new canvas for thnat =)
            if (!cClusterSizeMap.count(Channel)) {
              cClusterSizeMap[Channel] = new TCanvas( TString::Format("ClusterSizeTrack_Ch%02i", Channel), TString::Format("%s/ClusterSizeTrack_Ch%02i", OutDir.c_str(), Channel), 900, 300);
              cClusterSizeMap[Channel]->Divide(3, 1);
            }
          }

          // Get number of hits in this cluster
          size_t NHits = Cluster->NHits();

          // Fill cluster size
          hClusterSizeMap[id]->Fill(NHits);

          // Call it once.. it's faster.
          float const ThisClusterCharge = Cluster->Charge();

          //if (ThisClusterCharge <= 0) {
          //  printf("%12.0f\n", ThisClusterCharge);
          //}

          int const col = PLTGainCal::ColIndex(Cluster->SeedHit()->Column());
          int const row = PLTGainCal::RowIndex(Cluster->SeedHit()->Row());

          if (ThisClusterCharge < 100000 && ThisClusterCharge >= 0) {
            Avg2D[id][col][row] = Avg2D[id][col][row] * ((double) N2D[id][col][row] / ((double) N2D[id][col][row] + 1.)) + ThisClusterCharge / ((double) N2D[id][col][row] + 1.);
            ++N2D[id][col][row];


          }

          hMap[id][0]->Fill( ThisClusterCharge );

          if (NHits == 1) {

            hMap[id][1]->Fill( ThisClusterCharge );
          } else if (NHits == 2) {

            hMap[id][2]->Fill( ThisClusterCharge );
          } else if (NHits >= 3) {

            hMap[id][3]->Fill( ThisClusterCharge );
          }

          if (ThisClusterCharge < 200000) 
          {
            ChargeHits[id][0].push_back( ThisClusterCharge );
            if (NHits == 1) 
            {
              ChargeHits[id][1].push_back( ThisClusterCharge );
            } 
            else if (NHits == 2) 
            {
              ChargeHits[id][2].push_back( ThisClusterCharge );
            }
            else if (NHits >= 3) 
            {
              ChargeHits[id][3].push_back( ThisClusterCharge );
            }
          } 
          else 
          {
          }
          double rmsa = 0;
          double rmsb = 0;
          double rmsc = 0;
          double meaa = 0;
          double meab = 0;
          double meac = 0;

          rmsa = hMap[id][1]->GetRMS();
          rmsb = hMap[id][2]->GetRMS();
          rmsc = hMap[id][3]->GetRMS();
          meaa = hMap[id][1]->GetMean();
          meab = hMap[id][2]->GetMean();
          meac = hMap[id][3]->GetMean();

        }
        ///////////////////////////////////////
        //          END CLUST LOOP           //
        ///////////////////////////////////////
      }
      ///////////////////////////////////////
      //          END TRACK LOOP           //
      ///////////////////////////////////////



      ///////////////////////////////////////
      //                                   //
      //         BEGIN PLANE LOOP          //
      //                                   //
      // Loop over all planes in telescope //
      ///////////////////////////////////////

      for (size_t iPlane = 0; iPlane != Tele->NPlanes(); ++iPlane) 
      {

        PLTPlane* Plane = Tele->Plane(iPlane);

        // Check Roc# 0, 1, 2
        if (Plane->ROC() > 2) {
          std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
          continue;
        }
        if (Plane->ROC() < 0) {
          std::cerr << "WARNING: ROC < 0 found: " << Plane->ROC() << std::endl;
          continue;
        }
        if (Plane->Channel() > 99) {
          std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
          continue;
        }

        ///////////////////////////////////////
        //                                   //
        //          BEGIN HITS LOOP          //
        //                                   //
        //    Loop over all hits in plane    //
        ///////////////////////////////////////

        for(size_t ihit = 0; ihit != Plane->NHits(); ++ihit)
        {


          int const id = Tele->Channel() * 10 + Plane->ROC();

          PLTHit* Hit = Plane->Hit(ihit);

          //printf("Channel ROC Row Col ADC: %2i %1i %2i %2i %4i %12i\n", Hit->Channel(), Hit->ROC(), Hit->Row(), Hit->Column(), Hit->ADC(), Event.EventNumber());


          // If the hist doesn't exist yet we have to make it
          if (hOccupancyMap.count(id) == 0) 
          {

            // Create new hist with the given name
            sprintf(BUFF, "Occupancy Ch%02i ROC%1i", Plane->Channel(), Plane->ROC());

            hOccupancyMap[id] = new TH2F(BUFF, BUFF, PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL+1, PLTU::NROW,PLTU::FIRSTROW, PLTU::LASTROW+1);
            hOccupancyMap[id]->SetXTitle("Column");
            hOccupancyMap[id]->SetYTitle("Row");
            hOccupancyMap[id]->SetZTitle("Number of Hits");
            hOccupancyMap[id]->GetXaxis()->CenterTitle();
            hOccupancyMap[id]->GetYaxis()->CenterTitle();
            hOccupancyMap[id]->GetZaxis()->CenterTitle();
            hOccupancyMap[id]->SetTitleOffset(1.2, "y");
            hOccupancyMap[id]->SetTitleOffset(1.4, "z");
            hOccupancyMap[id]->SetFillColor(40); // We need this for projections later
            hOccupancyMap[id]->SetStats(false);



            // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
            if (!cOccupancyMap.count(Plane->Channel())) 
            {

              // Create canvas with given name
              sprintf(BUFF, "Occupancy All Ch%02i", Plane->Channel());
              cAllMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
              cAllMap[Plane->Channel()]->Divide(3,3);

              sprintf(BUFF, "Occupancy Ch%02i", Plane->Channel());
              cOccupancyMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
              cOccupancyMap[Plane->Channel()]->Divide(3,3);

              sprintf(BUFF, "Occupancy w/ QuantilesCh%02i", Plane->Channel());
              cQuantileMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 800);
              cQuantileMap[Plane->Channel()]->Divide(3,2);

              sprintf(BUFF, "Occupancy Projection Ch%02i", Plane->Channel());
              cProjectionMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
              cProjectionMap[Plane->Channel()]->Divide(3,3);

              sprintf(BUFF, "Occupancy Efficiency Ch%02i", Plane->Channel());
              cEfficiencyMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 1200);
              cEfficiencyMap[Plane->Channel()]->Divide(3,3);

              sprintf(BUFF, "Planes hit in Ch%02i", Plane->Channel());
              cCoincidenceMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 400, 400);
              hCoincidenceMap[Plane->Channel()] = new TH1F(BUFF, BUFF, 7, 0, 7);

              sprintf(BUFF, "Occupancy by Mean Ch%02i", Plane->Channel());
              cMeanMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 1200, 800);
              cMeanMap[Plane->Channel()]->Divide(3,2);
            }
          }

          // Fill this histogram with the given id
          hOccupancyMap[id]->Fill(Hit->Column(), Hit->Row());

        }
        ///////////////////////////////////////
        //           END HITS LOOP           //
        ///////////////////////////////////////

      }
      ///////////////////////////////////////
      //          END PLANE LOOP           //
      ///////////////////////////////////////
      // Binary number for planes hit
      int phit = Tele->HitPlaneBits();

      // here we fill the plot of Planes in Coincidence
      if(phit==0x1) hCoincidenceMap[Tele->Channel()]->Fill(0); //only first plane hit
      if(phit==0x2) hCoincidenceMap[Tele->Channel()]->Fill(1); //only 2nd plane hit
      if(phit==0x4) hCoincidenceMap[Tele->Channel()]->Fill(2); //only 3rd plane hit

      if(phit==0x3) hCoincidenceMap[Tele->Channel()]->Fill(3); //Plane 0 and 1 in coincidence
      if(phit==0x6) hCoincidenceMap[Tele->Channel()]->Fill(4); //Plane 1 and 2 in coincidence
      if(phit==0x5) hCoincidenceMap[Tele->Channel()]->Fill(5); //Plane 0 and 2 in coincidence
      if(phit==0x7) hCoincidenceMap[Tele->Channel()]->Fill(6); //All planes in coincidence

    }
    ///////////////////////////////////////
    //          END TELE LOOP            //
    ///////////////////////////////////////

    if (NTracksPerEvent != 0) 
    {
      HistNTracks.Fill(NTracksPerEvent);
    }

  }
  ///////////////////////////////////////
  //          END EVENT LOOP           //
  ///////////////////////////////////////

  std::cout << "Done reading events. Will make some plots now..." << std::endl;
  std::cout << "Events read: " << ie << std::endl;


  ///////////////////////////////////////
  //                                   //
  //       BEGIN OCCUPANCY HISTO       //
  //                                   //
  //  Loop over all histograms & draw  //
  ///////////////////////////////////////
  for (std::map<int, TH2F*>::iterator it = hOccupancyMap.begin(); it != hOccupancyMap.end(); ++it) 
  {
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hist for Channel %2i ROC %i\n", Channel, ROC);

    // Grab a 1-D hist from the 2D occupancy
    TH1F* hOccupancy1D = PLTU::HistFrom2D(it->second, "", 50);

    // Draw the 2D and 1D distribution on occupancy canvas
    cOccupancyMap[Channel]->cd(ROC+1);
    hOccupancyMap[id]->Draw("colz");
    cOccupancyMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hOccupancy1D->SetMinimum(0.5);
    hOccupancy1D->Clone()->Draw("hist");
    cOccupancyMap[Channel]->cd(ROC+6+1);
    hOccupancy1D->SetMinimum(0);
    hOccupancy1D->Draw("hist");



    // Grab the quantile you're interested in here
    Double_t QProbability[1] = { 0.95 }; // Quantile positions in [0, 1]
    Double_t QValue[1];                  // Quantile values
    hOccupancy1D->GetQuantiles(1, QValue, QProbability);



    // Plot the occupancy z-scale determined by quantile
    cQuantileMap[Channel]->cd(ROC+1);
    hQuantileMap[it->first] = (TH2F*) it->second->Clone();
    hQuantileMap[it->first]->SetTitle(TString::Format("Occupancy by Quantiles Ch%02i ROC%1i", Channel, ROC));
    hQuantileMap[it->first]->Draw("colz");
    if(QValue[0] > 1 && it->second->GetMaximum() > QValue[0]) {
      hQuantileMap[it->first]->SetMaximum(QValue[0]);
    }
    cQuantileMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hOccupancy1D->Draw("hist");

    // Grab a line and draw it on the plot
    TLine* LineQuantile = new TLine(QValue[0], hOccupancy1D->GetMaximum(), QValue[0], .5);
    LineQuantile->SetLineColor(2);
    LineQuantile->SetLineWidth(2);
    LineQuantile->Draw("same");



    // Draw on projection canvas
    cProjectionMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");

    // Column projection
    cProjectionMap[Channel]->cd(ROC+3+1);
    TH1D* hProjectionX = it->second->ProjectionX();
    hProjectionX->SetYTitle("Number of Hits");
    hProjectionX->GetYaxis()->CenterTitle();
    hProjectionX->SetTitleOffset(2, "Y");
    hProjectionX->Draw("hist");

    // Row projection
    cProjectionMap[Channel]->cd(ROC+6+1);
    TH1D* hProjectionY = it->second->ProjectionY();
    hProjectionY->SetYTitle("Number of Hits");
    hProjectionY->GetYaxis()->CenterTitle();
    hProjectionY->SetTitleOffset(2, "Y");
    hProjectionY->Draw("hist");





    // 2D Occupancy efficiency plots wrt 3x3
    hEfficiencyMap[it->first] = Get3x3EfficiencyHist(*hQuantileMap[it->first], PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::FIRSTROW, PLTU::LASTROW);
    hEfficiencyMap[it->first]->SetTitle( TString::Format("Occupancy 3x3Eff Ch%i ROC%i", Channel, ROC));
    hEfficiencyMap[it->first]->SetXTitle("Column");
    hEfficiencyMap[it->first]->SetYTitle("Row");
    hEfficiencyMap[it->first]->SetZTitle("Efficiency (relative to neighbors)");
    hEfficiencyMap[it->first]->GetXaxis()->CenterTitle();
    hEfficiencyMap[it->first]->GetYaxis()->CenterTitle();
    hEfficiencyMap[it->first]->GetZaxis()->CenterTitle();
    hEfficiencyMap[it->first]->SetTitleOffset(1.2, "z");
    hEfficiencyMap[it->first]->GetZaxis()->SetRangeUser(0, 3);
    hEfficiencyMap[it->first]->SetStats(false);

    // Make 1d plot
    hEfficiency1DMap[id] = FidHistFrom2D(hEfficiencyMap[it->first], "", 50, MyFiducialRegion);
    hEfficiency1DMap[id]->SetXTitle("3x3 relative efficiency");
    hEfficiency1DMap[id]->SetYTitle("Number of pixels");
    hEfficiency1DMap[id]->GetXaxis()->CenterTitle();
    hEfficiency1DMap[id]->GetYaxis()->CenterTitle();
    hEfficiency1DMap[id]->SetFillColor(40);


    cEfficiencyMap[Channel]->cd(ROC+1);
    hEfficiencyMap[it->first]->Draw("colz");

    cEfficiencyMap[Channel]->cd(ROC+3+1)->SetLogy(1);
    hEfficiency1DMap[it->first]->Clone()->Draw("");


    cEfficiencyMap[Channel]->cd(ROC+6+1);
    hEfficiency1DMap[it->first]->Draw("");


    // Occupancy normalized by mean
    sprintf(BUFF, "Occupancy by Mean Ch%02i ROC%1i", Channel, ROC);
    hMeanMap[id] = (TH2F*) it->second->Clone(BUFF);
    hMeanMap[id]->SetTitle(BUFF);
    TH1F* hMean = PLTU::HistFrom2D(hMeanMap[it->first], "", 50);
    hMeanMap[id]->Scale(1.0/hMean->GetMean());
    delete hMean;
    hMean = PLTU::HistFrom2D(hMeanMap[it->first], "", 50);
    hMeanMap[id]->SetZTitle("Relative number of hits");
    hMeanMap[id]->GetXaxis()->CenterTitle();
    hMeanMap[id]->GetYaxis()->CenterTitle();
    hMeanMap[id]->GetZaxis()->CenterTitle();
    hMeanMap[id]->SetTitleOffset(1.2, "z");
    hMeanMap[id]->SetStats(false);
    cMeanMap[Channel]->cd(ROC+1);
    hMeanMap[it->first]->Draw("colz");
    cMeanMap[Channel]->cd(ROC+3+1);
    hMean->Draw("hist");


    // Summary canvas of your three favorite plots
    cAllMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");
    cAllMap[Channel]->cd(ROC+3+1);
    hQuantileMap[id]->Draw("colz");
    cAllMap[Channel]->cd(ROC+6+1);
    hEfficiencyMap[id]->Draw("colz");

  }


  for (std::map<int, TH1F*>::iterator it = hCoincidenceMap.begin(); it != hCoincidenceMap.end(); ++it) 
  {
    cCoincidenceMap[it->first]->cd();

    // Naming for coincidence map
    char *bin[7] = { (char*)"ROC0"
      , (char*)"ROC1"
        , (char*)"ROC2"
        , (char*)"ROC0&1"
        , (char*)"ROC1&2"
        , (char*)"ROC0&2"
        , (char*)"All ROC Hit"
    };

    it->second->SetBit(TH1::kCanRebin);
    for (int r = 0; r <= 6; r++) 
    {
      it->second->Fill(bin[r], 0);
    }

    it->second->LabelsDeflate();
    it->second->SetFillColor(40);
    it->second->SetYTitle("Number of Hits");
    it->second->GetYaxis()->SetTitleOffset(1.9);
    it->second->GetYaxis()->CenterTitle();
    it->second->Draw("");
  }

  std::ofstream fHTML((OutDir + "/index.html").c_str());
  if (!fHTML.is_open()) {
    std::cerr << "ERROR: cannot open index.html in dir: " << OutDir << std::endl;
    return 2;
  }
  fHTML << "<html><body>\n";
  fHTML << "<h1>Run Summary: </h1>\n";
  fHTML << "DataFileName: " << DataFileName << "<br />\n";
  fHTML << "GainCalFileName: " << GainCalFileName << "<br />\n";
  fHTML << "AlignmentFileName: " << AlignmentFileName << "<br />\n";
  fHTML << "Number of events: " << ie << "<br />\n";
  fHTML << "<br />\n<a href=\"" << OutFileName << "\">" << OutFileName << "</a><br />\n";
  fHTML << "<hr />\n";



  // Draw and save bunch crossing histogram
  hBX.SetXTitle("Bunnch Crossing");
  hBX.SetYTitle("Events");
  hBX.SetZTitle("Bunch Crossing");
  hBX.SetTitleOffset(1.2, "y");
  hBX.SetTitleOffset(1.4, "z");
  hBX.SetFillColor(40); 
  hBX.SetStats(false);
  TCanvas cBX("cname", "ctitle", 1200,300);
  cBX.SetRightMargin(0.01);
  cBX.SetLeftMargin(0.01);
  cBX.cd();
  hBX.Draw("hist");
  cBX.SaveAs(OutDir + TString("/BunchCrossing.gif"));
  fHTML << "<hr /><h2>BX</h2>\n";
  fHTML << "<a href=\"BunchCrossing.gif\"><img src=\"BunchCrossing.gif\" width=\"600\" /></a>\n";


  // Loop over all canvas, save them, and delete them
  fHTML << "<hr /><h2>Occupancy</h2>\n";
  for (std::map<int, TCanvas*>::iterator it = cAllMap.begin(); it != cAllMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_All_Ch%02i.gif", it->first);
    fHTML << "<a href=\"" << FileName << "\"><img src=\"" << FileName << "\" width=\"300\" /></a>\n";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "<br />\n";

  fHTML << "<table><tr>\n";
  fHTML << "<td>Occupancy:</td><td>";
  for (std::map<int, TCanvas*>::iterator it = cOccupancyMap.begin(); it != cOccupancyMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_Ch%02i.gif", it->first);
    fHTML << " <a href=\"" << FileName << "\">" << it->first << "</a>";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "</td></tr>\n";
  fHTML << "<tr><td>Occupancy by Quantile:</td><td>";
  for (std::map<int, TCanvas*>::iterator it = cQuantileMap.begin(); it != cQuantileMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_Quantile_Ch%02i.gif", it->first);
    fHTML << " <a href=\"" << FileName << "\">" << it->first << "</a>";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "</td></tr>\n";
  fHTML << "<tr><td>Occupancy Projection:</td><td>";
  for (std::map<int, TCanvas*>::iterator it = cProjectionMap.begin(); it != cProjectionMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_Projection_Ch%02i.gif", it->first);
    fHTML << " <a href=\"" << FileName << "\">" << it->first << "</a>";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "</td></tr>\n";
  fHTML << "<tr><td>Occupancy Efficiency:</td><td>";
  for (std::map<int, TCanvas*>::iterator it = cEfficiencyMap.begin(); it != cEfficiencyMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_Efficiency_Ch%02i.gif", it->first);
    fHTML << " <a href=\"" << FileName << "\">" << it->first << "</a>";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "</td></tr>\n";
  fHTML << "<tr><td>Occupancy Coincidence:</td><td>";
  for (std::map<int, TCanvas*>::iterator it = cCoincidenceMap.begin(); it != cCoincidenceMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_Coincidence_Ch%02i.gif", it->first);
    fHTML << " <a href=\"" << FileName << "\">" << it->first << "</a>";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "</td></tr>\n";
  fHTML << "<tr><td>Occupancy by Mean:</td><td>";
  for (std::map<int, TCanvas*>::iterator it = cMeanMap.begin(); it != cMeanMap.end(); ++it) {
    TString const FileName = TString::Format("Occupancy_Mean_Ch%02i.gif", it->first);
    fHTML << " <a href=\"" << FileName << "\">" << it->first << "</a>";
    it->second->SaveAs(OutDir + "/" + FileName);
    delete it->second;
  }
  fHTML << "</td></tr></table>\n";


  ///////////////////////////////////////
  //      END OCUPANCY HISTO           //
  ///////////////////////////////////////

  TCanvas CanNTracks;
  CanNTracks.cd();
  HistNTracks.Draw("hist");
  CanNTracks.cd(1)->SetLogy(1);
  CanNTracks.SaveAs(OutDir + TString("/NTracksPerEvent.gif"));
  CanNTracks.SetCanvasSize(400, 250);
  CanNTracks.Modified();
  CanNTracks.Update();
  CanNTracks.SaveAs(OutDir + TString("/thm_NTracksPerEvent.gif"));
  fHTML << "<h2>Tracks</h2>\n";
  fHTML << "<a href=\"NTracksPerEvent.gif\"><img src=\"thm_NTracksPerEvent.gif\"></a>\n";
  fHTML << "<hr />\n";



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


    unique.push_back (id);

    firstMean.push_back (it->second[0]->GetMean());  
    secondMean.push_back (it->second[1]->GetMean());  
    thirdMean.push_back (it->second[2]->GetMean());  
    fourthMean.push_back (it->second[3]->GetMean());  

    firstRMS.push_back (it->second[0]->GetRMS());
    secondRMS.push_back (it->second[1]->GetRMS());
    thirdRMS.push_back (it->second[2]->GetRMS());
    fourthRMS.push_back (it->second[3]->GetRMS());



    for (size_t ih = 0; ih != 4; ++ih) {
      TH1F* Hist = it->second[ih];
      Hist->SetStats(false);

      Hist->SetNdivisions(5);
      Hist->SetLineColor(HistColors[ih]);
      if (ih == 0) {
        Hist->SetTitle( TString::Format("PulseHeightTrack Ch%02i ROC%1i", Channel, ROC) );
        Hist->SetXTitle("Electrons");
        Hist->SetYTitle("Events");
        Hist->Draw("hist");
        Leg->AddEntry(Hist, "All", "l");
      } else {
        Hist->Draw("samehist");
        if (ih != 3) {
          Leg->AddEntry(Hist, TString::Format(" %i Pixel", (int) ih), "l");
        } else {
          Leg->AddEntry(Hist, TString::Format("#geq%i Pixel", (int) ih), "l");
        }
      }
    }
    Leg->Draw("same");

    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+3+1);
    for (size_t ig = 3; ig != 0; --ig) {

      // Grab hist
      TGraphErrors* g = gClEnTimeMap[id][ig];
      if (g == 0x0) {
        std::cerr << "ERROR: no g for ig == " << ig << std::endl;
        continue;
      }

      g->SetMarkerColor(HistColors[ig]);
      g->SetLineColor(HistColors[ig]);
      if (ig == 3) {
        g->SetTitle( TString::Format("Average Pulse Height ROC %i", ROC) );
        g->SetMinimum(0);
        g->SetMaximum(60000);
        g->Draw("Ap");
      } else {
        g->Draw("samep");

      }
      OUTFILE.cd();
      g->Write();
    }


    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+6+1);

    for (int ja = 0; ja != PLTU::NROW; ++ja) {
      for (int ia = 0; ia != PLTU::NCOL; ++ia) {
        if (Avg2D[id][ia][ja] > 0) {
          hMap2D[id]->SetBinContent(ia+1, ja+1, Avg2D[id][ia][ja]);
        }
        printf("%6.0f ", Avg2D[id][ia][ja]);
      }
    }
    hMap2D[id]->SetMaximum(60000);
    hMap2D[id]->SetStats(false);
    hMap2D[id]->SetXTitle("Column");
    hMap2D[id]->SetYTitle("Row");
    hMap2D[id]->SetZTitle("Electrons");
    hMap2D[id]->Draw("colz");
    OUTFILE.cd();
    hMap2D[id]->Write();


  }

  // Save Cluster Size canvases
  fHTML << "<hr /><h2>Pulse Heights Track</h2>\n";
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    TString const Name = it->second->GetName()+TString(".gif");
    fHTML << "<a href=\"" << Name << "\"><img src=\"" << Name << "\" width=\"300\"></a>\n";
    it->second->SaveAs( OutDir + TString("/") + Name );
    it->second->Write();
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
    it->second->SaveAs( OutDir + TString("/") + it->second->GetName()+TString(".gif") );
    delete it->second;
  }

  OUTFILE.Close();


  for(size_t asd = 0; asd < firstMean.size(); ++asd) {
    std::cout << "\nFirst Mean & RMS:  " << firstMean[asd] << " " << firstRMS[asd] <<""<< std::endl;
    std::cout <<"\nSecond Mean & RMS: " <<secondMean[asd] << " " << secondRMS[asd] <<""<< std::endl;
    std::cout <<"\nThird Mean & RMS:  " <<thirdMean[asd] << " " << thirdRMS[asd] <<""<< std::endl;
    std::cout <<"\nFourth Mean & RMS: " <<fourthMean[asd]<< " " << fourthRMS[asd] <<""<< std::endl;
  }






  FILE* ofile = fopen( (OutDir + "/output.txt").c_str(), "w");
  fprintf(ofile, "$n%s $d%s $ie%d ", RunNumber.c_str(), OutDir.c_str(), ie);
  for(size_t q = 0; q <firstMean.size(); ++q) {
    fprintf(ofile, "$id%d ", unique[q]);
    fprintf(ofile, "$m1%lf $m2%lf $m3%lf $m4%lf ", firstMean[q], secondMean[q], thirdMean[q], fourthMean[q]);
    fprintf(ofile, "$r1%lf $r2%lf $r3%lf $r4%lf ", firstRMS[q], secondRMS[q], thirdRMS[q], fourthRMS[q]);
  }
  fprintf(ofile, "!!\n");
  fclose(ofile);

  fHTML << "</body></html>";
  fHTML.close();




  return 0;
}






int main (int argc, char* argv[])
{
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName] [AlignmentFileName] [OutDir] [RunNumber]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const AlignmentFileName = argv[3];
  std::string const OutDir = argv[4];
  std::string const RunNumber = argv[5];

  return StandardAnalysis(DataFileName, GainCalFileName, AlignmentFileName, OutDir, RunNumber);

}
