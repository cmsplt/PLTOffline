#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <stdint.h>

int ReadEventHits (std::string const DataFileName)
{
  uint32_t n1;

  int wordcount = 0;
  bool bheader = true;
  std::ifstream fInFile(DataFileName.c_str(),  std::ios::in | std::ios::binary);
  while (bheader) {
    fInFile.read((char *) &n1, sizeof n1);
    if (fInFile.eof()) {
      bheader=false;
      return -1;
    }
    std::cout<<std::hex<<std::setw(8)<<n1<<std::endl;
    if ( ((n1 & 0xff000000) == 0x50000000)){
//      std::cout<<wordcount<<std::endl;
      wordcount = 0;
      //std::cout << "Found Event Header: " << Event << std::endl;
    }
    else{++wordcount;}
  }

  return 0; 
}
int main (int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [DataFile.dat]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  ReadEventHits(DataFileName);

  return 0;
}
  


