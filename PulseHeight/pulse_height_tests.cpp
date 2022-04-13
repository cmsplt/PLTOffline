#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include "PLTEvent.h"
#include "PLTU.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TGraphErrors.h"
#include "TLatex.h"
#include "TFile.h"
#include "THStack.h"
#include "TMultiGraph.h"


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
  PLTEvent Event(DataFileName, GainCalFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  //  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);

  // Map for all ROC hists and canvas
  //std::map<int, std::vector<TGraphErrors*> > gClEnTimeMap;
  //std::map<int, TH1F*>    hClusterSizeMap;
  //std::map<int, TCanvas*> cClusterSizeMap;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;
  //std::map<int, TH2F* >              hMap2D;
 // std::map<int, TCanvas*>            cMap2D;
  std::map<int, std::map<int, std::vector<TH1F*> >> five_elem; 
  
  //double Avg2D[250][PLTU::NCOL][PLTU::NROW];
  //std::map<int, std::vector< std::vector<double> > > Avg2D;
  //std::map<int, std::vector< std::vector<int> > > N2D;
  //int      N2D[250][PLTU::NCOL][PLTU::NROW];
  // Bins and max for pulse height plots
  int   const NBins =     60;
  float const XMin  =  -1000;
  float const XMax  =  50000;

  // Time width in events for energy time dep plots
  // This is the time width in ms
  // const unsigned int TimeWidth = 1000 * (60 * 1);
  //const unsigned int TimeWidth = 1000;
  std::map<int, std::vector< std::vector<float> > > ChargeHits;
  std::map<int, std::vector<float>> charge_interval;
  // Loop over all events in file
  int ientry = 0;
  int limit=120000000;
  int count =0;
  int n=1;
  
  std::vector<int> leadbx;
  std::vector<int> noncol;
  std::vector<float> charge;

  std::ifstream csv_1; 
  csv_1.open("leadingBX.csv");
 
  while (csv_1.good()){ 
       std::string line;
       std::getline(csv_1, line, '\n'); 
       std::istringstream ss(line);
       int x;
       ss >> x;
       leadbx.push_back(x);
     } 

  csv_1.close();
    
  std::ifstream csv_2;
  csv_2.open("non_coll.csv");
    
  while (csv_2.good()){
      std::string line2;
      std::getline(csv_2, line2, '\n');
      std::istringstream ss(line2);
      int y;
      ss >> y;
      noncol.push_back(y);
     }
    
  for (int i=0; i<leadbx.size(); i++){
    std::cout<<leadbx[i]<<std::endl; 
     }
 
  int index=0;
  double roc0charge_leadbx;
  double roc1charge_leadbx;
  double roc2charge_leadbx;
  double roc0charge_noncol; 
  double roc1charge_noncol;
  double roc2charge_noncol;

  double roc0charge;
  double roc1charge;
  double roc2charge;


  TFile *fout = new TFile("BX.root", "UPDATE");

  TH1F *hist_allbx = new TH1F("hist_allbx", "hist_allbx", NBins, XMin, XMax);
  TH1F *hist_leadbx = new TH1F("hist_leadingbx", "hist_leadingbx", NBins, XMin, XMax);
  TH1F *hist_noncoll = new TH1F("hist_noncoll", "hist_noncoll", NBins, XMin, XMax);



  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << std::endl;
    }

    // First event time
    //static uint32_t const StartTime = Event.Time();
    //uint32_t const ThisTime = Event.Time();
   // static uint32_t const StartTime = 0;
   // uint32_t static ThisTime = 0;
   // ++ThisTime;
   
    if (ientry == limit +1) {
      std::cout << "Reached target of 5000000 events; stopping..." << std::endl;
      break;
    } 


    if(ientry>0 && (ientry%12000000)==0){
        index +=1;
        for (int el=0; el!=charge.size(); el++){
           charge_interval[index].push_back(charge[el]);
  
         } 
       charge.clear();
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
          int const roc = ROC; 

          
          if (Channel == 19){
           if (!hMap.count(id)) {
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ROC),
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", Channel, ROC), NBins, XMin, XMax) );
           // hMap2D[id] = new TH2F( TString::Format("Avg Charge Ch %02i ROC %1i Pixels All", Channel, ROC),
             // TString::Format("PixelCharge_Ch%02i_ROC%1i_All", Channel, ROC), PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
            for (size_t ih = 1; ih != 4; ++ih){
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                   TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih), NBins, XMin, XMax) );
          }
            if (!cMap.count(Channel)) {
            // Create canvas with given name
              cMap[Channel] = new TCanvas( TString::Format("PulseHeight_Ch%02i", Channel), TString::Format("PulseHeight_Ch%02i", Channel), 900, 900);
              cMap[Channel]->Divide(1, 1);
          }
       }
       

         if (!ChargeHits.count(id)) {
           ChargeHits[id].resize(4);
           ChargeHits[id][0].reserve(10000);
           ChargeHits[id][1].reserve(10000);
           ChargeHits[id][2].reserve(10000);
           ChargeHits[id][3].reserve(10000);
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
           

           // Fill cluster size
         // hClusterSizeMap[id]->Fill(NHits);
        
          hMap[id][0]->Fill( ThisClusterCharge );
          if (NHits == 1) {
            hMap[id][1]->Fill( ThisClusterCharge );
            if(id==190){
              charge.push_back(ThisClusterCharge);
              roc0charge=ThisClusterCharge;}
            if(id==191){
              roc1charge=ThisClusterCharge;} 
            if(id==192){roc2charge=ThisClusterCharge;}
            
            for (int l=0; l !=leadbx.size(); l++){    
              if (Event.BX() == leadbx[l]){      
                 if(id==190){ roc0charge_leadbx= ThisClusterCharge;}
                 else if(id==191){roc1charge_leadbx= ThisClusterCharge;}
                 else if(id==192){roc2charge_leadbx = ThisClusterCharge;}
               
           }           
         }   
            for (int j=0; j !=noncol.size(); j++){
               if (Event.BX()+1 == noncol[j]){
                  if(id==190){ roc0charge_noncol = ThisClusterCharge;}
                  else if(id==191){roc1charge_noncol = ThisClusterCharge;}
                  else if(id==192){roc2charge_noncol = ThisClusterCharge;} 
                }
             }

          } else if (NHits == 2) {
            hMap[id][2]->Fill( ThisClusterCharge );
          } else if (NHits >= 3) {
            hMap[id][3]->Fill( ThisClusterCharge );
          } 
       


       }//cluster
      
      
      }//roc=0 and channel=22
     }//roc
  
    }//channel
    
    if (roc0charge>0 && roc1charge>0 && roc2charge>0){
            hist_allbx->Fill(roc0charge);}

    if (roc0charge_leadbx>0 && roc1charge_leadbx>0 && roc2charge_leadbx>0){
            hist_leadbx->Fill(roc0charge_leadbx);}
    if (roc0charge_noncol>0 && roc1charge_noncol>0 && roc2charge_noncol>0){ 
            hist_noncoll->Fill(roc0charge_noncol);}         
    
     
    roc0charge_leadbx=0;
    roc1charge_leadbx=0;
    roc2charge_leadbx=0;
    roc0charge_noncol=0;
    roc1charge_noncol=0;
    roc2charge_noncol=0;
    roc0charge=0;
    roc1charge=0;
    roc2charge=0;      
   
  }//ientry

  
   hist_allbx->Draw();
   hist_allbx->Write();
   hist_leadbx->Draw();
   hist_leadbx->Write();
   hist_noncoll->Draw();
   hist_noncoll->Write();
   
   TLegend *leg = new TLegend(0.45, 0.7, 0.7, 0.85);  
   THStack *stack = new THStack("stack","Pulse Height for Ch 19 ROC0" );
   TGraph *gr = new TGraph(); 
   for (auto iter = charge_interval.begin(); iter != charge_interval.end(); ++iter){
     int ind = iter->first; 
     std::cout<<ind<<std::endl; 
     TH1F *hist = new TH1F(TString::Format("hist_%i", ind), TString::Format("hist_%i", ind), NBins, XMin, XMax);
     for (auto b = iter->second.begin(); b !=iter->second.end(); ++b){
           
            hist->Fill(*b);
            }
            hist->Scale(1/hist->GetEntries());
            hist->Write(); 

            hist->SetLineColor(ind);
            hist->SetLineWidth(3);
            leg->AddEntry(hist, TString::Format("%i - %i hour", ind-1, ind));
            stack->Add(hist);

            float peak = PLTU::peak_fromHisto(hist); 
            gr->SetPoint(ind-1, ind, peak);
            gr->SetMarkerStyle(20);
            gr->SetMarkerColor(1);
            
}

 
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  TLegend *Leg = new TLegend(0.45, 0.7, 0.7, 0.85);
  THStack *hs = new THStack("hs","Pulse Height for Ch 22 ROC0" );


  gStyle->SetTitleFontSize(0.05);
  gStyle->SetLegendBorderSize(1);
  gStyle->SetLegendFillColor(0);
   
  hist_allbx->SetLineColor(2);
  hist_allbx->SetLineWidth(3);
  Leg->AddEntry(hist_allbx, TString::Format("Events from all BXs", "l"));

  hist_leadbx->SetLineColor(4);
  hist_leadbx->SetLineWidth(3);
  Leg->AddEntry(hist_leadbx, TString::Format("Events from Leading BXs", "l"));

  hist_noncoll->SetLineColor(1);
  hist_noncoll->SetLineColor(3);
  Leg->AddEntry(hist_noncoll, TString::Format("Events from non colliding BX after each train", "l"));
 
  
  hs->Add(hist_allbx);
  hs->Add(hist_leadbx);
  hs->Add(hist_noncoll);

  Leg->SetTextSize(0.03);
  Leg->SetFillColor(0);

  c1->SetLogy();

  hs->SetTitle("PulseHeight for Ch 19 ROC 0; Electrons");
  gStyle->SetTitleFontSize(0.05);
  gStyle->SetTitleH(0.1);
  gStyle->SetTitleW(0.5);
  gStyle->SetTitleX(0.25);
  hs->Draw("nostack");
  Leg->Draw("same");
  c1->SaveAs("timewalk.gif");
  
  TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);  
  stack->Draw("nostack");
  leg->Draw("same");
  c2->SaveAs("stability.gif");

  TCanvas *c3 = new TCanvas("c3", "c3", 800, 600);
  gr->SetTitle("PulseHeight Peak over time");
  gr->GetXaxis()->SetTitle("Time [hour]");
  gr->GetYaxis()->SetTitle("PulseHeight Peak");
  gr->GetYaxis()->SetTitleOffset(1.6);
  gr->Draw("APL");

  c3->SaveAs("peakovertime.gif");
  
  fout->Close();

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
