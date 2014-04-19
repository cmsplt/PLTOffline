////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Wed Jan 16 16:32:54 CET 2013
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include <stdint.h>


int ScalarTree (TString const InFileName, TString const OutFileName)
{
  TFile OutFile(OutFileName, "create");
  if (!OutFile.IsOpen()) {
    std::cerr << "ERROR: cannot open outfile.  If it exists delete it first.  This will not overwrite a file." << std::endl;
    return 1;
  }

  uint32_t time_orbit=0;
  uint32_t orbit=0;
  uint32_t ij=0;
  uint32_t scaldat[27];
  int fedid;
  int fpga;


  std::ifstream FileScaler(InFileName.Data(), std::ios::in | std::ios::binary); 
  if(!FileScaler) { 
    std::cout << "ERROR: Cannot open input file."<< std::endl; 
    return 1; 
  }

  TTree t1("ScalarTree","Pasta Sauce Boss");
  t1.SetDirectory(&OutFile);
  t1.Branch("time_orbit",&time_orbit,"time_orbit/I");
  t1.Branch("fpga",&fpga,"fpga/I");
  t1.Branch("fedid",&fedid,"fedid/I");
  t1.Branch("scaldat",&scaldat,"scaldat[6]/I");

  for(int i=0;i<2000000000;i++){
    FileScaler.read((char*)&time_orbit,sizeof(uint32_t));
    if(FileScaler.gcount()!= sizeof(time_orbit)){std::cout<<"End of File!"<<std::endl; break;}
    FileScaler.read((char*)&orbit,sizeof(uint32_t));
    FileScaler.read((char*)&ij,sizeof(uint32_t));
    FileScaler.read((char*)scaldat,27*sizeof(uint32_t));
    fedid = (ij&0x8)>>3;
    fpga  = (ij&0x3);

    //printf("%12lu %12lu %12lu  %12lu %12lu %12lu\n", scaldat[0], scaldat[1], scaldat[2], scaldat[3], scaldat[4], scaldat[5]);


    t1.Fill();

  } 

  t1.Write();
  OutFile.Close();


  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFile.dat] [OutFile.root]" << std::endl;
    return 1;
  }

  return ScalarTree(argv[1], argv[2]);
}
