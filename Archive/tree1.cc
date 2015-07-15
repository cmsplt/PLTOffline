#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cstdlib>

// This example is a variant of hsimple.C but using a TTree instead
// of a TNtuple. It shows :
//   -how to fill a Tree with a few simple variables.
//   -how to read this Tree
//   -how to browse and analyze the Tree via the TBrowser and TTreeViewer
// This example can be run in many different ways:
//  way1:  .x tree1.C    using the CINT interpreter
//  way2:  .x tree1.C++  using the automatic compiler interface
//  way3:  .L tree1.C  or .L tree1.C++
//          tree1()
// One can also run the write and read parts in two separate sessions.
// For example following one of the sessions above, one can start the session:
//   .L tree1.C
//   tree1r();
//
//  Author: Rene Brun

void tree1(TString const InFileName, TString const OutFileName)
{
  //create a Tree file tree1.root
  std::ifstream FileHist(InFileName.Data(), std::ios::in | std::ios::binary); 
  if(!FileHist) { 
    std::cout << "Cannot open file. "<< std::endl; 
    return; 
  } 

  //create the file, the Tree and a few branches
  TFile f(OutFileName,"create");
  if (!f.IsOpen()) {
    std::cerr << "ERROR: file already exists.  move it first.." << std::endl;
    exit(1);
  }
  TTree t1("t1","a simple Tree with simple variables");
  Int_t ch;
  Int_t htot;

  Int_t bigbuff[3564];
  Int_t time_orbit;
  Int_t orbit;


  t1.Branch("ch",&ch,"ch/I");
  t1.Branch("time_orbit",&time_orbit,"time_orbit/I");
  t1.Branch("orbit",&orbit,"evn/I");
  t1.Branch("htot",&htot,"htot/I");




  for(int i=0;i<20000000;i++){
    ///////////////////////////////HISTOGRAMS/////////////////////////////////

    FileHist.read((char*)&time_orbit,sizeof(uint32_t));
    if(FileHist.gcount()!= sizeof(time_orbit)){std::cout<<"End of File!"<<std::endl; t1.Write(); return;}
    FileHist.read((char*)&orbit,sizeof(uint32_t));
    FileHist.read((char*)&ch,sizeof(uint32_t));
    FileHist.read((char*)bigbuff,3564*sizeof(uint32_t));
    //cout<<"orbit "<<dec<<orbit<<" time "<<time_orbit<<" ch ij "<<ij<<" Coinc ";
    htot=0;for(int ij=0;ij<3564;ij++) {htot+=(bigbuff[ij]&0xfff);}//cout<<tot<<endl;
    t1.Fill();

  }

  //save the Tree header. The file will be automatically closed
  //when going out of the function scope
  t1.Write();
}



int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [InFile.dat] [OutFile.root]" << std::endl;
    return 1;
  }

  tree1(argv[1], argv[2]);

  return 0;
}
