#include "PLTTelescope.h"

PLTTelescope::PLTTelescope ()
{
}


PLTTelescope::~PLTTelescope ()
{
  for (std::vector<PLTPlane*>::iterator it = fPlanes.begin(); it != fPlanes.end(); ++it) {
    //delete *it;
  }
}


void PLTTelescope::AddPlane (PLTPlane* Plane)
{
  fPlanes.push_back(Plane);
  fChannel = Plane->Channel();
  return;
}


int PLTTelescope::Channel ()
{
  return fChannel;
}


PLTPlane* PLTTelescope::Plane(size_t i) {
  return fPlanes[i];
}


void PLTTelescope::Draw2D (int const np, TString const Name)
{
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
  return fPlanes.size();
}


