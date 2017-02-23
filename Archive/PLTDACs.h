#ifndef GUARD_PLTDACs_h
#define GUARD_PLTDACs_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

class PLTDAC
{
  public:
    PLTDAC (std::string const, std::string const);
    ~PLTDAC ();

    int Val(int const);
    void Set (int const, int const);
    void Write();

    static int MakeID (int const mF, int const mFC, int const hub, int const roc)
    {
      return 10000 * mF + 1000 * mFC + 10 * hub + roc;
    }
    static void BreakID (int const id, int &mF, int& mFC, int& hub, int& roc)
    {
      mF = id / 10000;
      mFC = id % 10000 / 1000;
      hub = id % 1000 / 10;
      roc = id % 10;
      return;
    }
    static int IdFromFileName (std::string const InFileName)
    {
      int mF, mFC, hub, roc;
      sscanf(InFileName.c_str(), "mFec%i_mFecChannel%i_hubAddress%i_roc%i.dacs1", &mF, &mFC, &hub, &roc);
      return MakeID(mF, mFC, hub, roc);
    }

  private:

    std::map<int, int> fMap;
    int fmFec;
    int fmFecChannel;
    int fhubAddress;
    int fROC;
};


class PLTDACs
{
  public:
    PLTDACs ();
    ~PLTDACs ();

    PLTDAC* GetDACS (int const, int const, int const, int const);

    void LoadAllDACSFiles ()
    {
      fMap[82210] = new PLTDAC("DACS", "mFec8_mFecChannel2_hubAddress21_roc0.dacs1");
      fMap[82211] = new PLTDAC("DACS", "mFec8_mFecChannel2_hubAddress21_roc1.dacs1");
      fMap[82212] = new PLTDAC("DACS", "mFec8_mFecChannel2_hubAddress21_roc2.dacs1");
      fMap[82910] = new PLTDAC("DACS", "mFec8_mFecChannel2_hubAddress29_roc0.dacs1");
      fMap[82911] = new PLTDAC("DACS", "mFec8_mFecChannel2_hubAddress29_roc1.dacs1");
      fMap[82912] = new PLTDAC("DACS", "mFec8_mFecChannel2_hubAddress29_roc2.dacs1");
      return;
    }

  private:

    std::map<int, PLTDAC*> fMap;

};





#endif
