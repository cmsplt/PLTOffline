////////////////////////////////////////////////////////////////////
//
//  ParseCondDBData -- takes a text file produced from
//   MeasureAccidentalsTele and a CSV file obtained from the
//   conditions browser and produces an output file
//   with the PLTzero luminosity per telescope per timestamp
//    Paul Lujan, November 10 2015
//    Joseph Heideman, March 10 2016
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <vector>
#include <time.h>

struct stepInfo {
  uint32_t timeBegin;
  uint32_t timeEnd;
  int nEvents;
  int tracksAll;
  int tracksGood;
  int tracksAllTele1;
  int tracksAllTele2;
  int tracksAllTele4;
  int tracksAllTele5;
  int tracksAllTele7;
  int tracksAllTele8;
  int tracksAllTele10;
  int tracksAllTele11;
  int tracksAllTele13;
  int tracksAllTele14;
  int tracksAllTele16;
  int tracksAllTele17;
  int tracksAllTele19;
  int tracksAllTele20;
  int tracksAllTele22;
  int tracksAllTele23;

  int tracksGoodTele1;
  int tracksGoodTele2;
  int tracksGoodTele4;
  int tracksGoodTele5;
  int tracksGoodTele7;
  int tracksGoodTele8;
  int tracksGoodTele10;
  int tracksGoodTele11;
  int tracksGoodTele13;
  int tracksGoodTele14;
  int tracksGoodTele16;
  int tracksGoodTele17;
  int tracksGoodTele19;
  int tracksGoodTele20;
  int tracksGoodTele22;
  int tracksGoodTele23;
};

int ParseCondDBDataTele(const std::string accidentalFileName, const std::string csvFileName) {
  std::cout << "accidental file name: " << accidentalFileName << std::endl;
  std::cout << "csv file name:        " << csvFileName << std::endl;

  // First, read the accidental rates.

  std::vector<struct stepInfo> stepRates;
  FILE *afile;
  afile = fopen(accidentalFileName.c_str(), "r");
  if (afile == NULL) {
    std::cerr << "Couldn't open accidental file " << accidentalFileName << "!" << std::endl;
    return(1);
  }
  int nsteps;
  struct stepInfo thisStep;
  fscanf(afile, "%d", &nsteps);

//this does not read the the last two dead scopes. Need to add in the 4 more corresponding values for it to read all

  for (int i=0; i<nsteps; ++i) {
    fscanf(afile, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", &(thisStep.timeBegin), &(thisStep.timeEnd), &(thisStep.nEvents), &(thisStep.tracksAll), &(thisStep.tracksGood), &(thisStep.tracksAllTele1), &(thisStep.tracksGoodTele1), &(thisStep.tracksAllTele2), &(thisStep.tracksGoodTele2), &(thisStep.tracksAllTele4), &(thisStep.tracksGoodTele4), &(thisStep.tracksAllTele5), &(thisStep.tracksGoodTele5), &(thisStep.tracksAllTele7), &(thisStep.tracksGoodTele7), &(thisStep.tracksAllTele8), &(thisStep.tracksGoodTele8), &(thisStep.tracksAllTele10), &(thisStep.tracksGoodTele10), &(thisStep.tracksAllTele11), &(thisStep.tracksGoodTele11), &(thisStep.tracksAllTele13), &(thisStep.tracksGoodTele13), &(thisStep.tracksAllTele14), &(thisStep.tracksGoodTele14), &(thisStep.tracksAllTele16), &(thisStep.tracksGoodTele16), &(thisStep.tracksAllTele17), &(thisStep.tracksGoodTele17), &(thisStep.tracksAllTele19), &(thisStep.tracksGoodTele19), &(thisStep.tracksAllTele20), &(thisStep.tracksGoodTele20), &(thisStep.tracksAllTele22), &(thisStep.tracksGoodTele22), &(thisStep.tracksAllTele23), &(thisStep.tracksGoodTele23));
    stepRates.push_back(thisStep);
  }
  fclose(afile);
  std::cout << "Read " << nsteps << " rows in accidental rate file" << std::endl;
  // for (unsigned int i=0; i<stepRates.size(); ++i) {
  //   std::cout << "start: " << stepRates[i].timeBegin << " end: " << stepRates[i].timeEnd << " nevents: " << stepRates[i].nEvents
  //       << " all: " << stepRates[i].tracksAll << " good: " << stepRates[i].tracksGood << std::endl;
  // }

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
    if (convertedTime > stepRates[nsteps-1].timeEnd) break;
    int stepNumber = -1;
    for (int iStep = 0; iStep < nsteps; ++iStep) {
      if (convertedTime >= stepRates[iStep].timeBegin && convertedTime <= stepRates[iStep].timeEnd) {
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
  fprintf(outf, "%d\n", nsteps);
  for (int i=0; i<nsteps; ++i) {
    fprintf(outf, "%d %d %d %d %d %d %f\n", stepRates[i].timeBegin, stepRates[i].timeEnd,
	    stepRates[i].nEvents, stepRates[i].tracksAll, stepRates[i].tracksGood,
	    stepLumis[i].first, stepLumis[i].second);
  }
  fclose(outf);
  std::cout << "Output saved to CombinedRates.txt" << std::endl;

//create combined file with just Telescope information

FILE *outt = fopen("CombinedRatesTele.txt", "w");
  fprintf(outt, "%d\n", nsteps);
  for (int i=0; i<nsteps; ++i) {
    fprintf(outt, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f\n", stepRates[i].timeBegin, stepRates[i].timeEnd, stepRates[i].nEvents, stepRates[i].tracksAllTele1, stepRates[i].tracksGoodTele1, stepRates[i].tracksAllTele2, stepRates[i].tracksGoodTele2, stepRates[i].tracksAllTele4, stepRates[i].tracksGoodTele4, stepRates[i].tracksAllTele5, stepRates[i].tracksGoodTele5, stepRates[i].tracksAllTele7, stepRates[i].tracksGoodTele7, stepRates[i].tracksAllTele8, stepRates[i].tracksGoodTele8, stepRates[i].tracksAllTele10, stepRates[i].tracksGoodTele10, stepRates[i].tracksAllTele11, stepRates[i].tracksGoodTele11, stepRates[i].tracksAllTele13, stepRates[i].tracksGoodTele13, stepRates[i].tracksAllTele14, stepRates[i].tracksGoodTele14, stepRates[i].tracksAllTele16, stepRates[i].tracksGoodTele16, stepRates[i].tracksAllTele17, stepRates[i].tracksGoodTele17, stepRates[i].tracksAllTele19, stepRates[i].tracksGoodTele19, stepRates[i].tracksAllTele20, stepRates[i].tracksGoodTele20, stepRates[i].tracksAllTele22, stepRates[i].tracksGoodTele22, stepRates[i].tracksAllTele23, stepRates[i].tracksGoodTele23, stepLumis[i].first, stepLumis[i].second);
  }
  fclose(outt);
  std::cout << "Output saved to CombinedRatesTele.txt" << std::endl;

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

  ParseCondDBDataTele(accidentalFileName, csvFileName);

  return 0;
}
