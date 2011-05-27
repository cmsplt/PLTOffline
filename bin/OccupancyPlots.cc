////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Tue May 24 09:54:58 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"

// FUNCTION DEFINITIONS HERE
int OccupancyPlots (std::string const);








// CODE BELOW

int OccupancyPlots (std::string const DataFileName)
{
  TH2F h1("OccupancyR1", "OccupancyR1", 50, 0, 50, 60, 30, 90);
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  
  // Grab the plt event reader
  PLTEvent Event(DataFileName);

  // Map for all ROC hists and canvas
  std::map<int, TH2F*> hMap;
  std::map<int, TH1F*> phitsMap;
  std::map<int, TCanvas*> cMap;
  std::map<int, TCanvas*> cpMap;
  std::map<int, TCanvas*> cphitsMap;
  
  // char buffer for writing names
  char BUFF[200];
  
  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    
    // Loop over all planes with hits in event
    for (size_t ip = 0; ip != Event.NPlanes(); ++ip) {
      
      // THIS plane is
      PLTPlane* Plane = Event.Plane(ip);
      if (Plane->ROC() > 2) {
        std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
        continue;
      }
      if (Plane->Channel() > 99) {
        std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
        continue;
      }
      
      
      // Loop over all hits on this plane
      for (size_t ihit = 0; ihit != Plane->NHits(); ++ihit) {
	
        // THIS hit is
        PLTHit* Hit = Plane->Hit(ihit);
	
        // ID the plane and roc by 3 digit number
        int const id = 10*Plane->Channel() + Plane->ROC();
	
        // If the hist doesn't exist yet we have to make it
        if (hMap.count(id) == 0) {
	  
          // Create new hist with the given name
          sprintf(BUFF, "Occupancy_Ch%02i_ROC%1i", Plane->Channel(), Plane->ROC());
          std::cout << "Creating New Hist: " << BUFF << std::endl;
          hMap[id] = new TH2F(BUFF, BUFF, 50, 0, 50, 60, 30, 90);
          hMap[id]->SetXTitle("Column");
          hMap[id]->SetYTitle("Row");

	  
          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Plane->Channel())) {			  
	    
            // Create canvas with given name
            sprintf(BUFF, "Occupancy_Ch%02i", Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 900, 300);
            cMap[Plane->Channel()]->Divide(3,1);
	    
            sprintf(BUFF, "Occupancy_Projection_Ch%02i", Plane->Channel());
            cpMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 900, 900);
            cpMap[Plane->Channel()]->Divide(3,3);
	    
	    sprintf(BUFF, "Planes Hit in Ch%02i", Plane->Channel());
	    cphitsMap[Plane->Channel()]=new TCanvas(BUFF,BUFF, 900,900);
	    phitsMap[Plane->Channel()]=new TH1F(BUFF, BUFF, 7, 0, 7);
          }
        }
	
        // Fill this histogram with the given id
        hMap[id]->Fill(Hit->Column(), Hit->Row());
        // Just print some example info
        //if (Plane->ROC() > 3) {
        //  printf("Channel: %2i  ROC: %1i  col: %2i  row: %2i  adc: %3i\n", Plane->Channel(), Plane->ROC(), Hit->Column(), Hit->Row(), Hit->ADC());
        //}
      }
    }    
	  for (size_t ip = 0; ip != Event.NTelescopes(); ++ip)//loop over Telescopes
	  {
		  PLTTelescope* Tele = Event.Telescope(ip);
		  int phit=Tele->HitPlaneBits();
		  //here we fill the Plot of Planes in Coincidence
		  if(phit==0x4)phitsMap[Tele->Channel()]->Fill(1); //only first plane hit
		  if(phit==0x2)phitsMap[Tele->Channel()]->Fill(2);//only 2nd plane hit
		  if(phit==0x1)phitsMap[Tele->Channel()]->Fill(3);//only 3rd plane hit
		  if(phit==0x6)phitsMap[Tele->Channel()]->Fill(4); //Plane 0and1 in coincidence
		  if(phit==0x3)phitsMap[Tele->Channel()]->Fill(5);//Plane 1and2 in coincidence
		  if(phit==0x5)phitsMap[Tele->Channel()]->Fill(6);//Plane 1 and 3 in coincidence
		  if(phit==0x7)phitsMap[Tele->Channel()]->Fill(7);//All planes in coincidence
	  }
  }
  
  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, TH2F*>::iterator it = hMap.begin(); it != hMap.end(); ++it) {
    
    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;
    
    printf("Drawing hist for Channel %2i ROC %i\n", Channel, ROC);
    
    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+1);
    it->second->Draw("colz");

    cpMap[Channel]->cd(ROC+1);
    it->second->ProjectionX()->Draw("hist");
    cpMap[Channel]->cd(ROC+3+1);
	it->second->ProjectionY()->Draw("hist");
	cpMap[Channel]->cd(ROC+6+1);
	gStyle->SetPalette(1);
	it->second->Draw("colz");
  }
  for (std::map<int, TH1F*>::iterator it = phitsMap.begin(); it != phitsMap.end(); ++it) {
	  
	  cphitsMap[it->first]->cd();
	  it->second->Draw("");
  }
	  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) 
    {
      sprintf(BUFF, "Occupancy_Ch%02i.gif", it->first);
      it->second->SaveAs(BUFF);
      delete it->second;      
      sprintf(BUFF, "Occupancy_Projection_Ch%02i.gif", it->first);
      cpMap[it->first]->SaveAs(BUFF);
    }
   for (std::map<int, TCanvas*>::iterator it = cphitsMap.begin(); it != cphitsMap.end(); ++it)
   {
	   sprintf(BUFF, "Planes in Coincidence Ch%02i.gif", it->first);
	   it->second->SaveAs(BUFF);
   }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  OccupancyPlots(DataFileName);

  return 0;
}
