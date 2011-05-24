#include "PLTEvent.h"


PLTEvent::PLTEvent ()
{
  fRun = 0;
}


PLTEvent::PLTEvent (std::string const DataFileName)
{
  fBinFile.Open(DataFileName);
  fRun = 0;
}


PLTEvent::PLTEvent (std::string const DataFileName, std::string const GainCalFileName)
{
  fBinFile.Open(DataFileName);
  fGainCal.ReadGainCalFile(GainCalFileName);
  
  fRun = 0;
}


PLTEvent::~PLTEvent ()
{
  Clear();
};





size_t PLTEvent::NPlanes ()
{
  return fPlanes.size();
}



PLTPlane* PLTEvent::Plane(size_t i)
{
  return fPlanes[i];
}



size_t PLTEvent::NTelescopes ()
{
  return fTelescopes.size();
}



PLTTelescope* PLTEvent::Telescope (size_t i)
{
  return fTelescopes[i];
}



void PLTEvent::Clear ()
{
  // First delete anything we need to free memory
  //for (size_t i = 0; i != fHits.size(); ++i) {
  //  delete fHits[i];
  //}
  fTelescopeMap.clear();
  fPlaneMap.clear();

  // Clear vectors
  fHits.clear();
  //fClusters.clear();
  fPlanes.clear();
  fTelescopes.clear();
}



void PLTEvent::AddHit (PLTHit Hit)
{
  if (fGainCal.IsGood()) {
    fGainCal.SetCharge(Hit);
  }
  fHits.push_back(Hit);
  fEvent = Hit.Event();
  return;
}



void PLTEvent::MakeEvent ()
{
  // This function organizes the "hits" into Planes, clusters, and telescopes

  // Add hits to planes according to their channel-roc
  for (std::vector<PLTHit>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
    std::pair<int, int> ChannelRoc = std::make_pair<int, int>(it->Channel(), it->ROC());
    fPlaneMap[ChannelRoc].AddHit( &(*it) );
  }

  // Loop over all planes and clusterize each one, then add each plane to the correct telescope (by channel number
  for (std::map< std::pair<int, int>, PLTPlane>::iterator it = fPlaneMap.begin(); it != fPlaneMap.end(); ++it) {
    it->second.Clusterize();
    fTelescopeMap[it->second.Channel()].AddPlane( &(it->second) );
  }

  // Just to make it easier.. put them in a vector..
  for (std::map<int, PLTTelescope>::iterator it = fTelescopeMap.begin(); it != fTelescopeMap.end(); ++it) {
    for (size_t i = 0; i != it->second.NPlanes(); ++i) {
      fPlanes.push_back( it->second.Plane(i));
    }
    fTelescopes.push_back( &(it->second) );
  }

  return;
}



int PLTEvent::GetNextEvent ()
{
  Clear();
  int ret = fBinFile.ReadEventHits(fHits, fEvent);
  if (ret < 0) {
    return ret;
  }

  if (fGainCal.IsGood()) {
    for (std::vector<PLTHit>::iterator it = fHits.begin(); it != fHits.end(); ++it) {
      fGainCal.SetCharge(*it);
    }
  }

  MakeEvent();

  return ret;
}
  

