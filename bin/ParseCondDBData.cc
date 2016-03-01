////////////////////////////////////////////////////////////////////
//
//  ParseCondDBData -- takes a text file produced from
//   MeasureAccidentals and a CSV file obtained from the
//   conditions browser and produces an output file
//   with the PLTzero luminosity per timestamp
//    Paul Lujan, November 10 2015
//
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
};

int ParseCondDBData(const std::string accidentalFileName, const std::string csvFileName) {
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
  for (int i=0; i<nsteps; ++i) {
    fscanf(afile, "%d %d %d %d %d", &(thisStep.timeBegin), &(thisStep.timeEnd), &(thisStep.nEvents),
	   &(thisStep.tracksAll), &(thisStep.tracksGood));
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
