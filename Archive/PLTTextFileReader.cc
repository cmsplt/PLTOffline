#include "PLTTextFileReader.h"



PLTTextFileReader::PLTTextFileReader ()
{
}


PLTTextFileReader::PLTTextFileReader (std::string const in)
{
  Open(in);
}


PLTTextFileReader::~PLTTextFileReader ()
{
}



bool PLTTextFileReader::Open (std::string const DataFileName)
{
  fFileName = DataFileName;
  fInfile.open(fFileName.c_str());
  if (!fInfile) {
    std::cerr << "ERROR: cannot open input file: " << fFileName << std::endl;
    return false;
  }

  return true;
}





int PLTTextFileReader::ReadEventHits (std::vector<PLTHit>& Hits, unsigned long& Event)
{
  return ReadEventHits(fInfile, Hits, Event);
}


int PLTTextFileReader::ReadEventHits (std::ifstream& InFile, std::vector<PLTHit>& Hits, unsigned long& Event)
{
  int LastEventNumber = -1;
  int EventNumber = -1;

  int Channel, ROC, Row, Col, ADC;
  std::string Line;
  std::istringstream LineStr;
  while (true) {
    std::getline(InFile, Line);
    LineStr.clear();
    LineStr.str(Line);

    LineStr >> Channel >> ROC >> Col >> Row >> ADC >> EventNumber;


    if (EventNumber != LastEventNumber && LastEventNumber != -1) {
      InFile.putback('\n');
      for (std::string::reverse_iterator It = Line.rbegin(); It != Line.rend(); ++It) {
        InFile.putback(*It);
      }
      break;
    }

    PLTHit Hit(Channel, ROC, Col, Row, ADC);

    Hits.push_back(Hit);


    LastEventNumber = EventNumber;
    Event = EventNumber;

  }
  

  return Hits.size();
}

