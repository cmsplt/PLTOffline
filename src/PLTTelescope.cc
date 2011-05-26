#include "PLTTelescope.h"

PLTTelescope::PLTTelescope ()
{
  // Con me
}


PLTTelescope::~PLTTelescope ()
{
  // Byebye
}


void PLTTelescope::AddPlane (PLTPlane* Plane)
{
  // Add a plane
  fPlanes.push_back(Plane);
  fChannel = Plane->Channel();
  return;
}


int PLTTelescope::Channel ()
{
  // Get the channel
  return fChannel;
}


PLTPlane* PLTTelescope::Plane(size_t i)
{
  // Get a specific plane
  return fPlanes[i];
}


void PLTTelescope::Draw2D (int const np, TString const Name)
{
  // This is to draw a telescope.  I'll get back to this later on
  std::vector<TH2F*> h;
  TCanvas c("TelescopeHits", "Telescope Hits", 400, 900);
  c.Divide(1,3);
  for (size_t i = 0; i != fPlanes.size(); ++i) {
    c.cd(fPlanes[i]->ROC());
    h.push_back( fPlanes[i]->DrawHits2D() );
    //h[i]->SetMaximum(70000);
    //h[i]->SetMinimum(0);
    //h[i]->Draw("colz");
    h[i]->Draw("box");
  }

  c.SaveAs(Name);
  for (size_t i = 0; i != fPlanes.size(); ++i) {
    delete h[i];
  }

  return;
}



size_t PLTTelescope::NPlanes ()
{
  // Number of planes in this telescope with at least one hit
  return fPlanes.size();
}


int PLTTelescope::HitPlaneBits ()
{
  // This function return a binary representation of hit planes
  int HitPlanes = 0x0;

  for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
    HitPlanes |= (0x1 << (*it)->ROC());
  }

  return HitPlanes;
}
