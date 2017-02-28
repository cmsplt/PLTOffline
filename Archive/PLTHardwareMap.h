#ifndef GUARD_PLTHardwareMap_h
#define GUARD_PLTHardwareMap_h

#include <map>

class PLTHardwareMap
{
  public:
    PLTHardwareMap ();
    ~PLTHardwareMap ();

    int GetFEDChannel (int const, int const, int const);

  private:
    std::map<int, int> fFEDFECMap;

};






#endif
