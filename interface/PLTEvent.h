#ifndef GUARD_PLTEvent_h
#define GUARD_PLTEvent_h

#include "PLTBinaryFileReader.h"
#include "PLTTelescope.h"
#include "PLTGainCal.h"
#include "PLTAlignment.h"
#include "PLTTracking.h"
#include "PLTError.h"

#include <map>

class PLTEvent : public PLTTracking
{
  public:
    PLTEvent ();
    PLTEvent (std::string const, InputType inputType = kBinaryFile);
    PLTEvent (std::string const, std::string const, InputType inputType = kBinaryFile);
    PLTEvent (std::string const, std::string const, std::string const, InputType inputType = kBinaryFile);
    ~PLTEvent ();

    std::vector<PLTPlane*> fPlanes;
    std::vector<PLTTelescope*> fTelescopes;
    std::vector<PLTHit*> fHits;

    void SetDefaults ();

    size_t NPlanes ();
    PLTPlane* Plane(size_t);

    size_t NTelescopes ();
    PLTTelescope* Telescope (size_t);

    void Clear ();
    void AddHit (PLTHit&);
    void AddHit (PLTHit*);
    void MakeEvent ();
    void WriteEventText (std::ofstream&);
    void SetPlaneFiducialRegion (PLTPlane::FiducialRegion);
    void SetPlaneClustering (PLTPlane::Clustering, PLTPlane::FiducialRegion);

    PLTAlignment* GetAlignment ();

    unsigned long EventNumber ()
    { 
      return fEvent;
    }
    size_t NHits ()
    {
      return fHits.size();
    }

    bool FileIsOpen(void) { return fIsOpen; }
    int GetNextEvent(void);
    int GetNextEvent(uint32_t* buf, uint32_t bufSize);

    PLTGainCal* GetGainCal ()
    {
      return &fGainCal;
    }

    uint32_t Time ()
    {
      return fTime;
    }

    void SetTime (uint32_t in)
    {
      fTime = in;
      return;
    }

    uint32_t BX ()
    {
      return fBX;
    }

    void SetBX (uint32_t in)
    {
      fBX = in;
      return;
    }

    void ReadPixelMask (std::string const in) 
    {
      fBinFile.ReadPixelMask(in);
      return;
    }

    void ReadOnlinePixelMask(std::string const in) 
    {
      fBinFile.ReadOnlinePixelMask(in, fGainCal);
      return;
    }
    
    const std::set<int>& PixelMask ()
    {
      return fBinFile.PixelMask();
    } 

    int GetHardwareID (int const ch)
    {
      return fGainCal.GetHardwareID(ch);
    }

    int GetFEDChannel(int mFec, int mFecCh, int hubId) { return fGainCal.GetFEDChannel(mFec, mFecCh, hubId); }

    const std::vector<PLTError>& GetErrors(void) { return fErrors; }

    std::vector<int>& getDesyncChannels(void) { return fDesyncChannels; }

    std::string ReadableTime();

  private:
    unsigned long fRun;
    unsigned long fRunSection;
    unsigned long fEvent;
    uint32_t fTime;
    uint32_t fBX;
    std::vector<PLTError> fErrors;
    std::vector<int> fDesyncChannels;

    InputType fInputType;
    PLTGainCal fGainCal;
    bool fIsOpen; // check if the input file actually was successfully opened
    PLTBinaryFileReader fBinFile;
    PLTAlignment fAlignment;

    PLTPlane::Clustering fClustering;
    PLTPlane::FiducialRegion fFiducial;

    std::map<int, PLTTelescope> fTelescopeMap;
    std::map<std::pair<int, int>, PLTPlane> fPlaneMap;

};



#endif
