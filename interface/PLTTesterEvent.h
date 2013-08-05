////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Mon Mar  4 17:13:09 CET 2013
//
////////////////////////////////////////////////////////////////////
#ifndef GUARD_PLTTESTEREVENT_H
#define GUARD_PLTTESTEREVENT_H

#include "PLTTesterReader.h"
#include "PLTPlane.h"
#include "PLTGainCal.h"


class PLTTesterEvent : public PLTPlane
{
  public:
    PLTTesterEvent ();
    PLTTesterEvent (std::string const, std::string const, TString const, TFile*, int const NLinesForLevels = 10000);
    ~PLTTesterEvent ();

    void SetPlaneClustering(PLTPlane::Clustering, PLTPlane::FiducialRegion);
    void MakePlots ();
    PLTGainCal* GetGainCal ()
    {
      return &fGainCal;
    }

    int  GetNextEvent ();
    void SetEvent (int const);
    int  Event ();
    void SetOutRootFile (TFile*);
    void WriteEventText (std::ofstream&);
    size_t NHits ();
    PLTHit* Hit (size_t const);
    void Clear ();

    int Time () {
      return fTime;
    }

    int VCal ()
    {
      return fVCal * fVCalMult;
    }



  private:
    int fTime;
    int fVCal;
    int fVCalMult;
    int fEvent;  // Here this is a false event number, be careful

    PLTPlane::Clustering fClustering;
    PLTPlane::FiducialRegion fFiducial;


    PLTGainCal fGainCal;

    TFile* fOutRoot;

    PLTTesterReader fPLTTesterReader;


};
















#endif
