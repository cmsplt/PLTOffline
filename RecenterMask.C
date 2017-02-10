// This script will help realign the planes for the outer masks
// so that they are properly centered over the middle plane's shadow.
// To run, first run TrackOccupancy so that you get histo_track_occupancy.root.
// Then run this program. It will show you each of the outer plane
// histograms and propose a new mask. You can then adjust this new mask
// manually and accept it when you are satisfied.
//
// The program will output the new mask file to Mask_Recentered.txt.
// IMPORTANT!! This will only contain the mask for the active area for
// active scopes. If there are masked pixels, or masks for inactive
// scopes, you will have to add those to this file yourself.
//
//  -- Paul Lujan, May 5 2016

// #include <iostream>
// #include <fstream>
// #include "TH2F.h"
// #include "TFile.h"
// #include "TBox.h"
// #include "TCanvas.h"

const int centerPlaneSize[2] = {24, 36}; // cols and rows
const int outerPlaneSize[2] = {26, 38};  // cols and rows

const int defaultCenterPlaneActiveArea[2][2] = {{14, 37}, {22, 57}};
// This contains the offsets for center planes where the active area
// has been shifted from the default above. Necessary for working with
// the 2015 masks but since the 2016 masks are all now recentered, this
// can stay all zeroes.
int centerPlaneXShift[25] = {0};
int centerPlaneYShift[25] = {0};

const char *infileName = "histo_track_occupancy_4892_24x36.root";
const char *outfileName = "Mask_Recentered_Test.txt";

// Conversion of FED channel numbers to mFEC channel and hub. Crude
// but effective.
const char *chanName[25] = {"", "8 1 5", "8 1 13", "",  //0-3
			    "8 1 21", "8 1 29", "",     //4-6
			    "8 2 5", "8 2 13", "",      //7-9
			    "8 2 21", "8 2 29", "",     //10-12
			    "7 1 5", "7 1 13", "",      //13-15
			    "7 1 21", "7 1 29", "",     //16-18
			    "7 2 5", "7 2 13", "",      //19-21
			    "7 2 21", "7 2 29", ""};    //22-24

char buf[512];

void writeOutFile(std::ofstream* ofile, int iCh, int iRoc, int maskEdge[2][2]) {
  *ofile << chanName[iCh] << " " << iRoc << " " << "0-" << maskEdge[0][0]-1 << " 0-79 0" << std::endl;
  *ofile << chanName[iCh] << " " << iRoc << " " << maskEdge[0][1]+1 << "-51 0-79 0" << std::endl;
  *ofile << chanName[iCh] << " " << iRoc << " " << "0-51 0-" << maskEdge[1][0]-1 << " 0" << std::endl;
  *ofile << chanName[iCh] << " " << iRoc << " " << "0-51 " << maskEdge[1][1]+1 << "-79 0" << std::endl;
}

void RecenterMask(void) {
  TFile *f = new TFile(infileName);
  if (!f->IsOpen()) {
    std::cout << "Failed to open file " << infileName << std::endl;
    return;   
  }
  std::ofstream ofile(outfileName);
  
  for (int iCh=1; iCh<=24; iCh++) {
    for (int iRoc=0; iRoc<3; iRoc++) {
      sprintf(buf, "TrackOccupancy_Ch%02d_ROC%d", iCh, iRoc);
      TH2F* occ = (TH2F*)f->Get(buf);
      if (occ == NULL) continue; // this channel doesn't exist

      if (iRoc==1) {
	// Just write out the default area, shifted as appropriate.

	int activeArea[2][2];
	for (int i=0; i<2; ++i) {
	  for (int j=0; j<2; ++j) {
	    activeArea[i][j] = defaultCenterPlaneActiveArea[i][j];
	  }
	}
	for (int j=0; j<2; ++j) {
	  activeArea[0][j] += centerPlaneXShift[iCh];   // x shift
	  activeArea[1][j] += centerPlaneYShift[iCh];  // y shift
	}

	writeOutFile(&ofile, iCh, iRoc, activeArea);
	continue;
      }

      std::cout << "Channel " << iCh << " (" << chanName[iCh] << ") ROC " << iRoc << std::endl;

      // Find the maximum bin.
      // WARNING: All of this work is in bin numbers which start with 1,
      // while the pixel numbers start with 0. So we have to correct them
      // as necessary.
      int maxX, maxY, maxZ;
      occ->GetMaximumBin(maxX, maxY, maxZ);
      double maxVal = occ->GetBinContent(maxX, maxY);
      
      // Now proceed in all four directions from this point to find the edges.
      int thisPoint[2] = {maxX, maxY};
      const char *axisName[2] = {"x", "y"};
      int inc[2] = {-1, 1};
      int edgeLoc[2][2];
      bool edgeIsHard[2][2];

      for (int iAxis = 0; iAxis < 2; iAxis++) {
	for (int iDir = 0; iDir < 2; iDir++) {
	  thisPoint[0] = maxX;
	  thisPoint[1] = maxY;

	  // A "hard" edge is where the counts go directly to zero. This indicates that the center plane
	  // shadow extends outside the edge of the outer plane mask, which is bad. A "soft" edge is
	  // where the counts go to some non-zero value. This indicates the correct transition from
	  // the shadow area to the fringe area.

	  while (1) {
	    thisPoint[iAxis] += inc[iDir];
	    // Sometime you'll get a single cold (or masked) pixel so check the next pixel too to make
	    // sure it's not just a single pixel.
	    int nextPoint[2] = {thisPoint[0], thisPoint[1]};
	    nextPoint[iAxis] += inc[iDir];
	    double thisVal = occ->GetBinContent(thisPoint[0], thisPoint[1]);
	    double nextVal = occ->GetBinContent(nextPoint[0], nextPoint[1]);
	    if (thisVal == 0 && nextVal == 0) {
	      //std::cout << "Found hard edge at " << axisName[iAxis] << "=" << thisPoint[iAxis]-inc[iDir] << std::endl;
	      edgeLoc[iAxis][iDir] = thisPoint[iAxis]-inc[iDir]-1;
	      edgeIsHard[iAxis][iDir] = true;
	      break;
	    }
	    if (thisVal < maxVal/4 && nextVal < maxVal/4) {
	      //std::cout << "Found soft edge at " << axisName[iAxis] << "=" << thisPoint[iAxis]-inc[iDir] << std::endl;
	      edgeLoc[iAxis][iDir] = thisPoint[iAxis]-inc[iDir]-1;
	      edgeIsHard[iAxis][iDir] = false;
	      break;
	    }
	  }
	}
      }

      // Found all four edges (hopefully). Now make the mask.
      int maskEdge[2][2]; // this is the farthest row/column that is within the active area
      for (int iAxis = 0; iAxis < 2; iAxis++) {
	int sizeSeen = edgeLoc[iAxis][1]-edgeLoc[iAxis][0]+1;

	if (edgeIsHard[iAxis][0] == edgeIsHard[iAxis][1]) {
	  // Both edges are hard (which shouldn't happen) or both are soft. In either case we just want to center
	  // the mask around the shadow area.
	  if (edgeIsHard[iAxis][0])
	    std::cout << "Alert: hard edges in both directions along " << axisName[iAxis] << " axis, help!" << std::endl;

	  int margin = outerPlaneSize[iAxis] - sizeSeen;

	  // If the margin is odd, somewhat arbitrarily put the extra pixel
	  // on the left (or bottom) side.
	  int leftMargin, rightMargin;
	  if (margin % 2 == 0) {
	    leftMargin = margin/2;
	    rightMargin = margin/2;
	  } else {
	    leftMargin = margin/2 + 1;
	    rightMargin = margin/2;
	  }
	    
	  maskEdge[iAxis][0] = edgeLoc[iAxis][0] - leftMargin;
	  maskEdge[iAxis][1] = edgeLoc[iAxis][1] + rightMargin;
	} else {
	  // One edge is soft and one is hard, so we're just going to have to make our best guess here.
	  int margin = outerPlaneSize[iAxis] - centerPlaneSize[iAxis];

	  // If the margin is odd, this will put the extra pixel on the side with
	  // the hard edge, to give us a little more margin for error.
	  if (edgeIsHard[iAxis][0]) {
	    maskEdge[iAxis][1] = edgeLoc[iAxis][1] + margin/2;
	    maskEdge[iAxis][0] = maskEdge[iAxis][1] - outerPlaneSize[iAxis] + 1;
	  } else {
	    maskEdge[iAxis][0] = edgeLoc[iAxis][0] - margin/2;
	    maskEdge[iAxis][1] = maskEdge[iAxis][0] + outerPlaneSize[iAxis] - 1;
	  }
	}

	// Sanity check: is the active area actually the right size?
	if ((maskEdge[iAxis][1] - maskEdge[iAxis][0] + 1) != outerPlaneSize[iAxis]) {
	  std::cout << "Oops, our active area seems to have ended up the wrong size in the " << axisName[iAxis]
		    << " direction" << std::endl;
	}
      } // loop over axes
      // Present the results.
      TCanvas *c1 = new TCanvas("c1", "c1", 800, 800);
      occ->Draw("colz");

      while (1) {
	std::cout << "Proposed active area: X " << maskEdge[0][0] << "-" << maskEdge[0][1] << "; Y "
		  << maskEdge[1][0] << "-" << maskEdge[1][1] << std::endl;
	if (maskEdge[0][0] < 2 || maskEdge[1][0] < 2 || maskEdge[0][1] > 49 || maskEdge[1][1] > 77) {
	  std::cout << "WARNING: mask extends into border pixel area" << std::endl;
	}
	TBox b(maskEdge[0][0], maskEdge[1][0], maskEdge[0][1]+1, maskEdge[1][1]+1);
	b.SetFillStyle(0);
	b.SetLineColor(kBlack);
	b.SetLineWidth(2);
	b.Draw();
	c1->Update();
	std::cout << "L to move left, R to move right, U to move up, D to move down, anything else to continue" << std::endl;
	std::cin.getline(buf, sizeof(buf));
	if (buf[0] == 'L' || buf[0] == 'l') {
	  maskEdge[0][0]--;
	  maskEdge[0][1]--;
	} else if (buf[0] == 'R' || buf[0] == 'r') {
	  maskEdge[0][0]++;
	  maskEdge[0][1]++;
	} else if (buf[0] == 'D' || buf[0] == 'd') {
	  maskEdge[1][0]--;
	  maskEdge[1][1]--;
	} else if (buf[0] == 'U' || buf[0] == 'u') {
	  maskEdge[1][0]++;
	  maskEdge[1][1]++;
	} else {
	  // we're done!
	  break;
	}
      } // manual adjustment loop 
      delete c1;
      
      // Now finally write out the results.
      writeOutFile(&ofile, iCh, iRoc, maskEdge);
    } // roc loop
  } // channel loop
} // function
