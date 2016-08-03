////////////////////////////////////////////////////////////////////
//
//  ParseCondDBData -- takes a text file produced from
//   TrackLumiZeroCounting and a CSV file obtained from the
//   conditions browser and produces an output file
//   with the PLTzero luminosity per timestamp. Works regardless of 
//   whether the channel info is in the output file or not.
//   output as well.
//    Paul Lujan, NOvember 2015, adapted by Daniel Gift, July 14 2016
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <time.h>

int ParseCondDBData(const std::string accidentalFileName, const std::string csvFileName) {
  std::cout << "accidental file name: " << accidentalFileName << std::endl;
  std::cout << "csv file name:        " << csvFileName << std::endl;

  // First, read the accidental rates.

  std::vector<uint32_t> timeBegin;
  std::vector<uint32_t> timeEnd;
  std::vector<int> otherData1;
  std::vector<double> otherData2;
  std::vector<double> otherData3;
  std::vector<int> otherData4;
  std::vector<double> otherData5;
  std::vector<double> otherData6;

  std::ifstream afile(accidentalFileName.c_str());
  if (!afile.is_open()){ 
    std::cerr << "Couldn't open accidental file " << accidentalFileName << "!" << std::endl;
    return(1);
  }
  int nsteps, nFilledBunches;
  afile >> nsteps;
  afile >> nFilledBunches;

  for (int i=0; i<nsteps; ++i) {
    uint32_t thisTimeBegin, thisTimeEnd;
    std::string thisOtherData;
    afile >> thisTimeBegin >> thisTimeEnd;
    int t1, t4;
    double t2, t3, t5, t6;
    afile >> t1 >> t2 >> t3 >> t4 >>t5 >> t6; 
    std::getline(afile, thisOtherData);
    timeBegin.push_back(thisTimeBegin);
    timeEnd.push_back(thisTimeEnd);
    otherData1.push_back(t1);
    otherData2.push_back(t2);
    otherData3.push_back(t3);
    otherData4.push_back(t4);
    otherData5.push_back(t5);
    otherData6.push_back(t6);
  }
  afile.close();
  std::cout << "Read " << nsteps << " rows in accidental rate file" << std::endl;

  // Next, read the ConditionsBrowser file.
  
  std::vector<std::pair<int, double> > stepLumis;
  for (int i=0; i<nsteps; ++i)
    stepLumis.push_back(std::make_pair(0, 0.0));
  char dummy[1024];

  FILE *cfile = fopen(csvFileName.c_str(), "r");
  if (cfile == NULL) {
    std::cerr << "Couldn't open csv file " << csvFileName << "!" << std::endl;
    return(1);
  }

  int nrows;
  int lastDay = -1;
  int dayCounter = 0;
  fgets(dummy, sizeof(dummy), cfile);
  fscanf(cfile, "%d,,,\n", &nrows);
  fgets(dummy, sizeof(dummy), cfile);
  int row, weight, time;
  double lumi;
  for (int i=0; i<nrows; ++i) {
    fscanf(cfile, "%d,%d,%d,%lf\n", &row, &weight, &time, &lumi);
    //if (i<10) printf("%d %d %d %lf\n", row, weight, time, lumi);

    // Convert the timestamp.
    struct tm splitTime;
    const time_t timeSec = time;
    gmtime_r(&timeSec, &splitTime);
    if (lastDay == -1) lastDay = splitTime.tm_yday;
    uint32_t convertedTime = (splitTime.tm_hour*3600 + splitTime.tm_min*60 + splitTime.tm_sec)*1000;
    if (splitTime.tm_yday > lastDay) {
      lastDay = splitTime.tm_yday;
      dayCounter++;
    }
    convertedTime += 86400000*dayCounter;

    // Now see if this is a lumi we actually want.
    if (convertedTime > timeEnd[nsteps-1]) break;
    int stepNumber = -1;
    for (int iStep = 0; iStep < nsteps; ++iStep) {
      if (convertedTime >= timeBegin[iStep] && convertedTime <= timeEnd[iStep]) {
	stepNumber = iStep;
	break;
      }
    }
    if (stepNumber == -1) continue;
    
    stepLumis[stepNumber].first++;
    stepLumis[stepNumber].second += lumi;
    
  }
  fclose(cfile);

  FILE *outf = fopen("CombinedRates.txt", "w");
  fprintf(outf, "%d %d\n", nsteps, nFilledBunches);
  for (int i=0; i<nsteps; ++i) {
    fprintf(outf, "%d %d %d %f %f %d %f %f %d %f\n", timeBegin[i], timeEnd[i], 
	    otherData1[i], otherData2[i], otherData3[i], otherData4[i], 
	    otherData5[i], otherData6[i], stepLumis[i].first, stepLumis[i].second);
  }
  fclose(outf);
  std::cout << "Output saved to CombinedRates.txt" << std::endl;

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " accidentalFile.txt csvFile.txt" << std::endl;
    return 1;
  }
  
  const std::string accidentalFileName = argv[1];
  const std::string csvFileName = argv[2];

  ParseCondDBData(accidentalFileName, csvFileName);

  return 0;
}
