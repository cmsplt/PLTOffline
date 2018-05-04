#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "PLTU.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TGraphErrors.h"
#include "TLatex.h"
#include "TFile.h"

///////////////////////////////////////////////////////////////////////////////
/*
This cpp code is called by the EndOfFill Analyzer and is intended to create ROOT 
files that contain a cluster size distribution for the BRIL Webmonitor.
Currently the clusters are not constrained to be "on track".



The corresponding EndOfFill Analyzer section should be something along the lines of:

processClusterSizeDistribution(){
  # 
  printf '%s\n' "CLUSTER SIZE DISTRIBUTION"
  \mkdir -p                                                                         ${localPLTOffline}/plots/${fill}/
  ./MakeClusterSizeDistribution ${slinkFile} ${gaincalFile} ${fill} >               ${outputDir}/logs/MakeClusterSizeDistribution.txt 2>&1
  \mkdir -p                                                                         ${outputDir}/MakeClusterSizeDistribution/
  \cp    -f  ${localPLTOffline}/plots/${fill}/cluster_size_distr_output_*           ${outputDir}/MakeClusterSizeDistribution/
}
*/
///////////////////////////////////////////////////////////////////////////////

// FUNCTION DEFINITIONS HERE
int PulseHeights (std::string const, std::string const);




std::string split_last_part(std::string s)
{
  std::vector<char> ret_string_v;

  for (int i = s.size()-5; i>-1; i--)
  {
    if (s[i]=='/')
    {
      break;
    }
    else
    {
      ret_string_v.push_back(s[i]);
    }
  }

  std::string ret_string;
  for (int i=ret_string_v.size()-1; i>-1; i--)
  {
    if ( ret_string_v[i] != '.' )
    {
      ret_string += ret_string_v[i];
    }
    else
    {
      ret_string += '_';
    
    }
  }
  

  return ret_string;

}

// CODE BELOW




int PulseHeights (std::string const DataFileName, std::string const GainCalFileName, std::string const  fill_number)
{
  PLTU::SetStyle();
  gStyle->SetOptStat(111111);

  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);
  Event.SetPlaneClustering(PLTPlane::kClustering_Seed_5x5, PLTPlane::kFiducialRegion_All);
  Event.SetTrackingAlgorithm(PLTTracking::kTrackingAlgorithm_NoTracking);
  //  Event.SetPlaneFiducialRegion(PLTPlane::kFiducialRegion_m2_m2);

  // Map for all ROC hists and canvas
 
  std::map<int, TH1F*>    hClusterSizeMap;
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Loop over all events in file
  
  int ientry = 0;
  for ( ; Event.GetNextEvent() >= 0; ++ientry) {
    if (ientry % 10000 == 0) {
      std::cout << "Processing event: " << ientry << " at " << Event.ReadableTime() << std::endl;
    }

    if (ientry == 300000) {
      std::cout << "Reached target of 300000 events; stopping..." << std::endl;
      break;
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


        // If this id doesn't exist in the cluster size map, make the hist for this channel
        if (!hClusterSizeMap.count(id)) {
          hClusterSizeMap[id] = new TH1F( TString::Format("ClusterSize_Ch%02i_ROC%i %s", Channel, ROC, fill_number.c_str()), TString::Format("ClusterSize_Ch%02i_ROC%i", Channel, ROC), 10, 0, 10);
          hClusterSizeMap[id]->SetXTitle("Number of pixels in Cluster");
        }


        // Loop over all clusters on this plane
        for (size_t iCluster = 0; iCluster != Plane->NClusters(); ++iCluster) {
          PLTCluster* Cluster = Plane->Cluster(iCluster);
          size_t NHits = Cluster->NHits();
          // Fill cluster size
          hClusterSizeMap[id]->Fill(NHits);
        }
      }
    }




  }
  std::cout << "Events read: " << ientry+1 << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string f_name  = split_last_part(DataFileName);
  std::string of_name = "plots/" + std::string( fill_number ) + "/cluster_size_distr_output_" + f_name + "_" + fill_number + ".root";
  //std::string of_name = "plots/cluster_size_distr_output_" + f_name + "_" + fill_number + ".root";

  TFile* output_root = TFile::Open( of_name.c_str(),"RECREATE");
  std::cout << "File created:    " << of_name.c_str() << std::endl;
  
  std::vector<TDirectory*>	plottingFolders;
  std::vector<TH1F*> h_clusterSize;

  // Loop over all histograms and draw them in the root file
  for (std::map<int, TH1F* >::iterator it =  hClusterSizeMap.begin(); it !=  hClusterSizeMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    int const id      = it->first;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    //form output folder, insert relevant histograms
    output_root->cd();
    std::string temp_name = "Ch" + std::to_string(Channel) + "_ROC" + std::to_string(ROC);
    plottingFolders.push_back(output_root->mkdir(temp_name.c_str()));

    plottingFolders[plottingFolders.size()-1]->cd();

    //fill cluster size histo
    TH1F* h_tempTH1F;
    h_tempTH1F = (TH1F*)hClusterSizeMap[id]->Clone();
    h_tempTH1F->SetName("h_clusterSize");
    h_clusterSize.push_back(h_tempTH1F);
  }


  gROOT->SetBatch(kTRUE);
  output_root->Write();
  gROOT->GetListOfFiles()->Remove(output_root);

  output_root->Close();

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
  
  return 0;
}
  


int main (int argc, char* argv[])
{


  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName] [fill_ID]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::string const fill_ID = argv[3];
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;
  std::cout << "fill_ID:         " << fill_ID << std::endl;

  PulseHeights(DataFileName, GainCalFileName, fill_ID);


    

   return 0;
}
