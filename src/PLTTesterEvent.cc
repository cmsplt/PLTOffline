////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Mar  4 17:13:09 CET 2013
//
////////////////////////////////////////////////////////////////////

#include "PLTTesterEvent.h"


PLTTesterEvent::PLTTesterEvent ()
{
}


PLTTesterEvent::PLTTesterEvent (std::string const DataFileName, std::string const GainCalFileName, TString const OutDir, TFile* RootFile, int const NLinesForLevels)
{
  fEvent = 0;
  fOutRoot = RootFile;
  fPLTTesterReader.SetOutDir(OutDir);
  fPLTTesterReader.SetOutRootFile(RootFile);
  fPLTTesterReader.OpenFile(DataFileName);
  fGainCal.ReadTesterGainCalFile(GainCalFileName);
  fPLTTesterReader.CalculateLevels(DataFileName, NLinesForLevels);
  SetPlaneClustering(PLTPlane::kClustering_AllTouching, PLTPlane::kFiducialRegion_All);
}


PLTTesterEvent::~PLTTesterEvent ()
{
  fEvent = 0;
}



void PLTTesterEvent::SetPlaneClustering(PLTPlane::Clustering cl, PLTPlane::FiducialRegion fid)
{
  fClustering = cl;
  fFiducial = fid;
  fPLTTesterReader.SetPlaneFiducialRegion(fFiducial);
  return;
}



void PLTTesterEvent::MakePlots ()
{
  fPLTTesterReader.MakePlots();
  return;
}

int PLTTesterEvent::GetNextEvent ()
{
  // Clear out old info
  Clear();

  // inc the false event number (ok first event will be >= 1)
  ++fEvent;

  // The number we'll return.. number of hits, or -1 for end
  int ret = fPLTTesterReader.ReadEventHits(fHits, fTime, fVCal, fVCalMult);
  if (ret < 0) {
    //std::cout << "HERE" << std::endl;
    return ret;
  }

  static bool const DoGainCal = fGainCal.IsGood();

  // If the GC is good let's compute the charge
  if (DoGainCal) {
    for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
      fGainCal.SetCharge(**it);
    }
  }

  // Now make the event into some useful form
  //MakeEvent();
  Clusterize(fClustering, fFiducial);

  return fHits.size();
  
}



void PLTTesterEvent::SetEvent (int const in)
{
  fEvent = in;
  return;
}



int PLTTesterEvent::Event ()
{
  return fEvent;
}



void PLTTesterEvent::SetOutRootFile (TFile* f)
{
  fOutRoot = f;
  fPLTTesterReader.SetOutRootFile(f);
  return;
}


size_t PLTTesterEvent::NHits ()
{
  return fHits.size();
}




void PLTTesterEvent::WriteEventText (std::ofstream& f)
{
  // Be careful with this because things will be cut out by fiducial requirements!!

  for (std::vector<PLTHit*>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    f << (*it)->Channel() << " "
      << (*it)->ROC() << " "
      << (*it)->Column() << " "
      << (*it)->Row() << " "
      << (*it)->ADC() << " "
      << fEvent << "\n";
  }

  return;
}



PLTHit* PLTTesterEvent::Hit (size_t const i)
{
  if (i < fHits.size()) {
    return fHits[i];
  }

  std::cerr << "ERROR: PLTTesterEvent::Hit you are asking for a hit outside allowed range in vector" << std::endl;
  return (PLTHit*) 0x0;
}



void PLTTesterEvent::Clear ()
{
  // Clear vectors
  // You own the hits and need to delete them!!
  for (std::vector<PLTHit*>::iterator i = fHits.begin(); i != fHits.end(); ++i) {
    delete *i;
  }

  fHits.clear();
  fClusterizedHits.clear();
  fUnclusteredHits.clear();
  for (std::vector<PLTCluster*>::iterator i = fClusters.begin(); i != fClusters.end(); ++i) {
    delete *i;
  }
  fClusters.clear();


  return;
}
