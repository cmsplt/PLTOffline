//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jun 27 11:33:09 2016 by ROOT version 5.34/28
// from TTree Tracks/Tracks
// found on file: data/VdMAnalysis5013_5_to_15tmil.root
//////////////////////////////////////////////////////////

#ifndef trackClass_h
#define trackClass_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class trackClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           TrackID;
   Int_t           EvntBX;
   Int_t           EventN;
   Int_t           EventT;
   Int_t           Channel;
   Int_t           NTracks;
   Int_t           NClusters;
   Int_t           BestTrackI;
   Float_t         SlopeX[19];   //[NTracks]
   Float_t         SlopeY[19];   //[NTracks]
   Float_t         BeamspotX[19];   //[NTracks]
   Float_t         BeamspotY[19];   //[NTracks]
   Float_t         ResidualX[19];   //[NTracks]
   Float_t         ResidualY[19];   //[NTracks]
   Float_t         Distance[19];   //[NTracks]

   // List of branches
   TBranch        *b_TrackID;   //!
   TBranch        *b_EvntBX;   //!
   TBranch        *b_EventN;   //!
   TBranch        *b_EventT;   //!
   TBranch        *b_Channel;   //!
   TBranch        *b_NTracks;   //!
   TBranch        *b_NClusters;   //!
   TBranch        *b_BestTrackI;   //!
   TBranch        *b_SlopeX;   //!
   TBranch        *b_SlopeY;   //!
   TBranch        *b_BeamspotX;   //!
   TBranch        *b_BeamspotY;   //!
   TBranch        *b_ResidualX;   //!
   TBranch        *b_ResidualY;   //!
   TBranch        *b_Distance;   //!

   trackClass(TTree *tree=0);
   virtual ~trackClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(std::string const DipFile);//modified to get DipFile file as an input
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef trackClass_cxx
trackClass::trackClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
     //      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("data/VdMAnalysis5013_5_to_15tmil.root");
     //      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("data/VdMAnalysis5013_15_to_40.root");
     //      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("data/VdMAnalysis5013_160_to_190.root");
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("data/VdMAnalysis5013_210plus.root");
      if (!f || !f->IsOpen()) {
        //         f = new TFile("data/VdMAnalysis5013_5_to_15tmil.root");
        //         f = new TFile("data/VdMAnalysis5013_15_to_40.root");
        //         f = new TFile("data/VdMAnalysis5013_160_to_190.root");
         f = new TFile("data/VdMAnalysis5013_210plus.root");
      }
      f->GetObject("Tracks",tree);

   }
   Init(tree);
}

trackClass::~trackClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t trackClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t trackClass::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void trackClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("TrackID", &TrackID, &b_TrackID);
   fChain->SetBranchAddress("EvntBX", &EvntBX, &b_EvntBX);
   fChain->SetBranchAddress("EventN", &EventN, &b_EventN);
   fChain->SetBranchAddress("EventT", &EventT, &b_EventT);
   fChain->SetBranchAddress("Channel", &Channel, &b_Channel);
   fChain->SetBranchAddress("NTracks", &NTracks, &b_NTracks);
   fChain->SetBranchAddress("NClusters", &NClusters, &b_NClusters);
   fChain->SetBranchAddress("BestTrackI", &BestTrackI, &b_BestTrackI);
   fChain->SetBranchAddress("SlopeX", SlopeX, &b_SlopeX);
   fChain->SetBranchAddress("SlopeY", SlopeY, &b_SlopeY);
   fChain->SetBranchAddress("BeamspotX", BeamspotX, &b_BeamspotX);
   fChain->SetBranchAddress("BeamspotY", BeamspotY, &b_BeamspotY);
   fChain->SetBranchAddress("ResidualX", ResidualX, &b_ResidualX);
   fChain->SetBranchAddress("ResidualY", ResidualY, &b_ResidualY);
   fChain->SetBranchAddress("Distance", Distance, &b_Distance);
   Notify();
}

Bool_t trackClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void trackClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t trackClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef trackClass_cxx
