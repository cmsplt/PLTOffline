#include <iostream>
#include <string>
#include <map>
#include "PLTEvent.h"
#include "PLTU.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TGraphErrors.h"
#include "TLatex.h"
#include "TFile.h"
#include "TSpectrum.h"
#include "THStack.h"
#include "TMultiGraph.h"
#include <string>
#include <typeinfo>


// FUNCTION DEFINITIONS HERE
int PulseHeights (std::string const, std::string const, std::string const);
std::vector<std::vector<int>> goodfitmap_vector(std::string const, int, int);

//test

float Average (std::vector<float>& V)
{
  double Sum = 0;
  for (std::vector<float>::iterator it = V.begin(); it != V.end(); ++it) {
    Sum += *it;
  }

  return Sum / (float) V.size();
}


// This function replaces .dat with .root (GainCalFile)
std::string ReplaceString(std::string x, const std::string& search, const std::string& replace) {
 size_t pos = 0;
 while ((pos = x.find(search, pos)) != std::string::npos) {
         x.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return x;
}


// CODE BELOW


 //Function to fill a vector with the pixel fit results, GoodFit=1 / BadFit=0
 std::vector<std::vector<int>> goodfitmap_vector(std::string const GainCalFileName, int Channel, int ROC)  
 {  
 
  
   const std::string path = "/home/damanakis/rootfiles_new/";
   const std::string name = ReplaceString(GainCalFileName, ".dat", ".root");
   const std::string full_path = (path+name);
   TFile* file = new TFile(full_path.c_str(), "READ");
 

   TH2F *h2= (TH2F*)file->FindObjectAny(TString::Format("GoodFitMap_Channel%i_ROC%i", Channel, ROC));
   int x_max = h2->GetNbinsX(); //colums=52
   int y_max = h2->GetNbinsY(); //rows = 80
   std::vector< std::vector<int>> fit;
 
   fit.resize(y_max);

   for (int m=0; m<y_max; m++){
       fit[m].resize(x_max);}
   for (int i=1; i<=y_max; i++){
      for (int j=0; j<x_max; j++){
         if (h2->GetBinContent(j,i) ==1){
             fit[i-1][j-1]=1;
      }
         else { 
             fit[i-1][j-1] = 0;
        }
     } 
   } 
   
  //You can plot the vector contains, create a TH2D and compare it with the GoodFitMap
    
  /* TCanvas *canvas = new TCanvas("canvas", "canvas", 800, 600);
   TH2F *f2 = new TH2F("f2", "f2", 52, 0, 51, 80, 0, 79);
    for (int ii=1; ii<=y_max; ii++){
      for (int ji=1; ji<=x_max; ji++){  
         f2->SetBinContent(ji,ii,fit[ii-1][ji-1]);
     }
   }
   f2->SetStats(false);
   f2->SetXTitle("Column");
   f2->SetYTitle("Row");
   f2->Draw("colz");
   canvas->SaveAs("goodfitmap.gif");
*/
   file->Close();
   return fit;

}
 

int PulseHeights (std::string const DataFileName, std::string const GainCalFileName, std::string const Fill)
{
  PLTU::SetStyle();
  gStyle->SetOptStat(111111);
   
  int const HistColors[4] = { 1, 4, 28, 2 };  


//Grab the plt event reader
   PLTEvent Event(DataFileName, GainCalFileName);
   Event.SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
   Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  // Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);


  // Map for all ROC hists and canvas
  std::map<int, std::vector<TGraph*> > gClEnTimeMap;
  std::map<int, TH1F*>    hClusterSizeMap;
  std::map<int, TCanvas*> cClusterSizeMap;
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*>            cMap;
  std::map<int, TH2F* >              hMap2D;
  std::map<int, TCanvas*>            cMap2D;
 
  //double Avg2D[250][PLTU::NCOL][PLTU::NROW];
  std::map<int, std::vector< std::vector<double> > > Avg2D;
  std::map<int, std::vector< std::vector<int> > > N2D;
  std::map<int, std::vector< std::vector<int> > > N12D;
  std::map<int, std::vector< std::vector<int> > > N22D;
  //int      N2D[250][PLTU::NCOL][PLTU::NROW];
  std::map<int, std::vector< std::vector<double> > >good_pixels2D;
  std::map<int, std::vector< std::vector<double> > >bad_pixels2D; 
  // Bins and max for pulse height plots
  int   const NBins =     60;
  float const XMin  =  -1000;
  float const XMax  =  50000;

  // Time width in events for energy time dep plots
  // This is the time width in ms
  // const unsigned int TimeWidth = 1000 * (60 * 1);
  const unsigned int TimeWidth = 1000;
  std::map<int, std::vector< std::vector<float> > > ChargeHits;
  std::vector<double> peaks;  
 
  float roc0=0;
  float roc1=0;
  float roc2=0;


  // Loop over all events in file
  int NGraphPoints = 0; 
  int ientry = 0;
  int limit = 600000 /*139000000*/;

  std::string fname = "hist_"; 
  TH1F *hist = new TH1F((fname+Fill).c_str(), (fname+Fill).c_str(), NBins, XMin, XMax);
  std::vector<std::vector<int>> pixfit = goodfitmap_vector(GainCalFileName, 19, 0); 


 for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 1000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << std::endl;
      }



    // First event time
    //static uint32_t const StartTime = Event.Time();
    //uint32_t const ThisTime = Event.Time();
    static uint32_t const StartTime = 0;
    uint32_t static ThisTime = 0;
    ++ThisTime;

    if (ientry == limit) {
      std::cout << "Reached target of 500000 events; stopping..." << std::endl;
      break;
    }
   
      

    while (ThisTime - (StartTime + NGraphPoints * TimeWidth) > TimeWidth) {
      // make point(s)
       for (std::map<int, std::vector<TGraph*> >::iterator mit = gClEnTimeMap.begin(); mit != gClEnTimeMap.end(); ++mit) {
        int const id = mit->first;
        for (size_t ig = 0; ig != mit->second.size(); ++ig) {
          TGraph* g = (mit->second)[ig];

          if (g->GetN() != NGraphPoints) {
            // Play some catchup
            g->Set(NGraphPoints);
            for (int i = 0; i > NGraphPoints; ++i) {
              g->SetPoint(i, i * TimeWidth, 0);
            }
          }

          g->Set( NGraphPoints + 1 );
          if (ig==1){
           
            if (ChargeHits[id][ig].size() != 0) {
         
            // float const Peak = PLTU::peak_finder(ChargeHits[id][ig]); //If you want to find the peak of the histogram instead
               float const Avg = PLTU::Average(ChargeHits[id][ig]);
               g->SetPoint(NGraphPoints, NGraphPoints * TimeWidth, Avg);
           //  g->SetPointError( NGraphPoints, 0, Avg/sqrt((float) ChargeHits[id][ig].size()));
            
             ChargeHits[id][ig].clear();
             ChargeHits[id][ig].reserve(10000); 
           
           
          } else {
            g->SetPoint(NGraphPoints , NGraphPoints * TimeWidth, 0);
            //g->SetPointError( NGraphPoints , 0, 0 );
          }
        }//ig=1;
      }
    }
      ++NGraphPoints;

    }
    

    for (size_t iTelescope = 0; iTelescope != Event.NTelescopes(); ++iTelescope) {
      PLTTelescope* Telescope = Event.Telescope(iTelescope);

      for (size_t iPlane = 0; iPlane != Telescope->NPlanes(); ++iPlane) {
        PLTPlane* Plane = Telescope->Plane(iPlane);

        int Channel = Plane->Channel(); //14 Channels
        int ROC = Plane->ROC(); //3 ROCs


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
       if (Channel ==19){
     
        if (!Avg2D.count(id)) {
          Avg2D[id].resize(PLTU::NCOL);
          N2D[id].resize(PLTU::NCOL);
          N12D[id].resize(PLTU::NCOL);
          N22D[id].resize(PLTU::NCOL);
          good_pixels2D[id].resize(PLTU::NCOL);
          bad_pixels2D[id].resize(PLTU::NCOL);
      
          for (int icol = 0; icol != PLTU::NCOL; ++icol) {
            Avg2D[id][icol].resize(PLTU::NROW);
            N2D[id][icol].resize(PLTU::NROW);
            N12D[id][icol].resize(PLTU::NROW);
            N22D[id][icol].resize(PLTU::NROW);
            good_pixels2D[id][icol].resize(PLTU::NROW);
            bad_pixels2D[id][icol].resize(PLTU::NROW);
          }
        }

        if (!hMap.count(id)) {
          hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", Channel, ROC),
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", Channel, ROC), NBins, XMin, XMax) );
            hMap2D[id] = new TH2F( TString::Format("Avg Charge Ch %02i ROC %1i Pixels All", Channel, ROC),
              TString::Format("PixelCharge_Ch%02i_ROC%1i_All", Channel, ROC), PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
          for (size_t ih = 1; ih != 4; ++ih) {
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", Channel, ROC, (int) ih),
                    TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", Channel, ROC, (int) ih), NBins, XMin, XMax) );
        
             }
 
          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Channel)) {
            // Create canvas with given name
            cMap[Channel] = new TCanvas( TString::Format("PulseHeight_Ch%02i", Channel), TString::Format("PulseHeight_Ch%02i", Channel), 900, 900);
            cMap[Channel]->Divide(3, 3);
          }
        }

        if (!gClEnTimeMap.count(id)) {
          gClEnTimeMap[id].resize(4);
          for (int ig = 0; ig != 4; ++ig) {
            TString const Name = TString::Format("TimeAvgGraph_id%d_Cl%d", id, ig);
            gClEnTimeMap[id][ig] = new TGraph();
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
          hClusterSizeMap[id] = new TH1F( TString::Format("ClusterSize_Ch%02i_ROC%i", Channel, ROC), TString::Format("ClusterSize_Ch%02i_ROC%i", Channel, ROC), 10, 0, 10);
          hClusterSizeMap[id]->SetXTitle("Number of pixels in Cluster");

          // One in three chance you'll need a new canvas for thnat =)
          if (!cClusterSizeMap.count(Channel)) {
            cClusterSizeMap[Channel] = new TCanvas( TString::Format("ClusterSize_Ch%02i", Channel), TString::Format("ClusterSize_Ch%02i", Channel), 900, 300);
            cClusterSizeMap[Channel]->Divide(3, 1);
          }
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
          //std::vector<std::vector<int>> pixfit = goodfitmap_vector(GainCalFileName, Channel, 0);
           
          if (ThisClusterCharge < 100000 && ThisClusterCharge >= 0) {
            Avg2D[id][col][row] = Avg2D[id][col][row] * ((double) N2D[id][col][row] / ((double) N2D[id][col][row] + 1.)) + ThisClusterCharge / ((double) N2D[id][col][row] + 1.);
            ++N2D[id][col][row];
             
          
          
          // Fill cluster size
          hClusterSizeMap[id]->Fill(NHits);
          hMap[id][0]->Fill( ThisClusterCharge );
          if (NHits == 1) {
           hMap[id][1]->Fill( ThisClusterCharge );
           if (id==190){
             if (col<52 && row<80){ //Sanity check 
         
                if(pixfit[row][col]==1){
                    roc0=ThisClusterCharge; 

   // Fill a 2D vector with the goodfit pixels in the fiducial region which recorded a charge. 
   // Calculate the avg charge that each pixel has recorded 
                    good_pixels2D[id][col][row] = good_pixels2D[id][col][row] * ((double) N12D[id][col][row] / ((double) N12D[id][col][row] + 1.)) + ThisClusterCharge / ((double) N12D[id][col][row] + 1.);                                                                                                                                              good_pixels2D[id][col][row] = good_pixels2D[id][col][row] + 1;                                                                                                     ++N12D[id][col][row]; }
                
                if (pixfit[row][col]==0) {    
                 // Fill a 2D vector with the badfit pixels in the fiducial region which recorded a charge.                                                     
                 // Calculate the avg charge that each pixel has recorded                                                                                          
                 bad_pixels2D[id][col][row] = bad_pixels2D[id][col][row] * ((double) N22D[id][col][row] / ((double) N22D[id][col][row] + 1.)) + ThisClusterCharge / ((double) N22D[id][col][row] + 1.);
                 bad_pixels2D[id][col][row] = bad_pixels2D[id][col][row] +1; 
                 ++N22D[id][col][row];}
               }//col<52 and row<80

               }else if(id==191){
                    roc1 = ThisClusterCharge;
               }else if (id==192){
                roc2 = ThisClusterCharge; }
      
         }else if (NHits == 2) {
              hMap[id][2]->Fill( ThisClusterCharge );
         }else if (NHits >= 3) {
              hMap[id][3]->Fill( ThisClusterCharge );
        }  
     
      
     }// if ThisCluster<100000

         if (ThisClusterCharge < 200000) {
            ChargeHits[id][0].push_back( ThisClusterCharge );
            if (NHits == 1) {
                 ChargeHits[id][1].push_back( ThisClusterCharge ); 
             }else if (NHits == 2) {
              ChargeHits[id][2].push_back( ThisClusterCharge );
             }else if (NHits >= 3) {
              ChargeHits[id][3].push_back( ThisClusterCharge );
            }
          }//thiscluster<200000

      }//icluster

    }//Channel=22 

  }//rocs

 }//channels
 

  if (roc0>0 && roc1>0 && roc2>0){ //Require hit to all 3 ROCs 
      hist->Fill(roc0); //it depends on which ROC you are interested in
      peaks.push_back(roc0);}
    roc0=0;
    roc1=0;
    roc2=0;  

}

 std::cout << "Events read: " << ientry+1 << std::endl;
  
 
  TFile *histogram_file = new TFile("PH.root", "UPDATE");
  hist->Scale(1/hist->GetEntries()); //Normalize Histogram

  //###If you want to replot the histogram with the overflow bin included##
 // int nx = histroc->GetNbinsX() +1; //number of bins plus the overflow bin
  //double x1 = histroc->GetBinLowEdge(1); //low limit
  //double bw = histroc->GetBinWidth(nx); //bin width
  //double x2 = histroc->GetBinLowEdge(nx) +bw; //upper limit
  //TH1F *hist_overflow = new TH1F("hist_overflow_<Fill>", "Histogram with overflow bin", nx, x1, x2);
  //for (int c=1; c<=nx; c++){
  // over_hist->Fill(over_hist->GetBinCenter(c), histroc->GetBinContent(c));
 // }
 // ####
 //
   hist->Draw();
   hist->Write();
   histogram_file->Close();
  

  // Loop over all histograms and draw them on the correct canvas in the correct pad

 //TFile *plot_root = new TFile("file.root", "UPDATE");
  TH2F *f2D = new TH2F("heatmap_goodfits", "heatmap_goodfits", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
  TH2F *f3D = new TH2F("heatmap_badfits", "heatmap_badfits", PLTU::NCOL, PLTU::FIRSTCOL, PLTU::LASTCOL, PLTU::NROW, PLTU::FIRSTROW, PLTU::LASTROW);
 
  for (std::map<int, std::vector<TH1F*> >::iterator it = hMap.begin(); it != hMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;
 
   if (id == 190){
    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+1);

    TLegend* Leg = new TLegend(0.65, 0.7, 0.80, 0.88, "");
    Leg->SetFillColor(0);
    Leg->SetBorderSize(0);
    Double_t norm=1;
    for (int ih = 0; ih != 4; ++ih) {
     if (ih ==1){
      TH1F* Hist = it->second[ih];
      Hist->SetStats(1);
      gStyle->SetOptStat("e");
      gStyle->SetTitleSize(0.05, "t");
      Hist->SetNdivisions(5);
      Hist->SetLineColor(HistColors[ih]);
      //if (ih == 0) {
      Hist->SetTitle( TString::Format("PulseHeight Ch%02i ROC%1i", Channel, ROC) );
      Hist->SetXTitle("Electrons");
      Hist->SetYTitle("Events");
      Hist->Scale(norm/Hist->GetEntries());
      Hist->Draw("hist");
      Hist->Write();
      
     // Leg->AddEntry(Hist, "Good and bad pixels", "l");
      /*} else {
        Hist->Draw("samehist");
        if (ih != 3) {
          Leg->AddEntry(Hist, TString::Format(" %i Pixel", ih), "l");
        } else {
          Leg->AddEntry(Hist, TString::Format("#geq%i Pixel", ih), "l");
        }
      }*/
    }
    Leg->Draw("same");
    char buf[512];
    sprintf(buf, "#splitline{Mean:}{%.0f}", it->second[0]->GetMean());
    TLatex *t1 = new TLatex(0, 0, buf);
    t1->SetNDC();
    t1->SetX(0.68);
    t1->SetY(0.6);
    t1->Draw();
    sprintf(buf, "#splitline{Peak:}{%.0f}", it->second[0]->GetXaxis()->GetBinCenter(it->second[0]->GetMaximumBin()));
    TLatex *t2 = new TLatex(0, 0, buf);
    t2->SetNDC();
    t2->SetX(0.68);
    t2->SetY(0.5);
    t2->Draw();
    }
    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+3+1);
    for (int ig = 1; ig!= 0; --ig) {
      // Grab hist
      TGraph* g = gClEnTimeMap[id][ig];

      g->SetMarkerColor(HistColors[ig]);
      g->SetLineColor(HistColors[ig]);
      if (ig == 1) {
        g->SetTitle( TString::Format(" Pulse Height ROC %i", ROC) );
        g->SetMinimum(0);
        g->SetMaximum(60000);
        g->SetMinimum(0);
       // g->SetMarkerStyle(4);
        g->Draw("AP");
       // g->Print("all");
        //char buf[512];
        //double maxy = g->GetPointY(1);
        //sprintf(buf, "#splitline{Peak:}{%.0f}", maxy);
        //TLatex *t3 = new TLatex(0, 0, buf);
        //t3->SetNDC();
        //t3->SetX(0.68);
        //t3->SetY(0.6);
        //t3->Draw();
      } else {
        g->Draw("samep");
      }
    }
  
    cMap[Channel]->cd(ROC+6+1);
    for (int ja = 0; ja != PLTU::NROW; ++ja) {
      for (int ia = 0; ia != PLTU::NCOL; ++ia) {
        if (Avg2D[id][ia][ja] < 25000) {
          //int const hdwAddy = Event.GetGainCal()->GetHardwareID(Channel);
          //int const mf  = hdwAddy / 1000;
          //int const mfc = (hdwAddy % 1000) / 100;
          //int const hub = hdwAddy % 100;
          //fprintf(OutPix, "%1i %1i %2i %1i %2i %2i\n", mf, mfc, hub, ROC, PLTU::FIRSTCOL + ia, PLTU::FIRSTROW + ja);
        } else {
        }
        if (Avg2D[id][ia][ja] > 0) {
          hMap2D[id]->SetBinContent(ia+1, ja+1, Avg2D[id][ia][ja]);
         
        }
       // if  (Avg2D[id][ia][ja] <= 0) {
        //printf("%6.0f ", Avg2D[id][ia][ja]);
       // bad_pixels +=1;
      //}
      //      std::cout << std::endl;
       if (good_pixels2D[id][ia][ja]>0){
         f2D->SetBinContent(ia+1, ja+1, good_pixels2D[id][ia][ja]);
    }
      if (bad_pixels2D[id][ia][ja]>0){
        f3D->SetBinContent(ia+1, ja+1, bad_pixels2D[id][ia][ja]);
    }
   }
    hMap2D[id]->SetMaximum(60000);
    hMap2D[id]->SetStats(false);
    hMap2D[id]->SetXTitle("Column");
    hMap2D[id]->SetYTitle("Row");
    hMap2D[id]->SetZTitle("Electrons");
    hMap2D[id]->Draw("colz");

  //}
  } 
 // f2D->Write();
 // f3D->Write();
 }
  
}
  
  // Save Cluster Size canvases
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
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
    it->second->SaveAs( TString("plots/") + it->second->GetName()+TString(".gif") );
    delete it->second;
  }


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName] [Fill]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const Fill = argv[3];
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout<<"Fill: " <<Fill << std::endl;
  PulseHeights(DataFileName, GainCalFileName, Fill);

  return 0;
}
