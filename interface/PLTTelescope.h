#ifndef GUARD_PLTTelescope_h
#define GUARD_PLTTelescope_h

#include "PLTPlane.h"

#include "TCanvas.h"

class PLTTelescope
{
  public:
    PLTTelescope ();
    ~PLTTelescope ();

    void   AddPlane (PLTPlane*);
    int    Channel ();
    PLTPlane* Plane(size_t i);
    void   Draw2D (int const, TString const);
    size_t NPlanes ();

  private:
    std::vector<PLTPlane*> fPlanes;
    int fChannel;

};






#endif
