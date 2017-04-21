#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <cstring>





/////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{

  int ictdc = 0;
  unsigned long long eventbuff[1024*48];
  int eventbuflength = 0;
  unsigned long long TDCbuff[50*48];
  int TDCbuflength = 0;

  bool bheader = false;
  unsigned int lastheader = 0;
  bool lheader = false;
  int ifastorread = 0;
  bool fastorprint = false;
  int ifastorok = 0;
  int ifastorbad = 0;
  int offset = 0;

  char outputFilePixel[256];
  char outputFileTDC[256];
  char outputFileScaler[256];
  char outputFileFastor[256];
  char outputFileHisto[256];
  char outputFile[256];
  char temp[256];

  sscanf(argv[1], "%s", outputFile);


  if (argc<2) {
    std::cout << argc << " usage: splitintoev filename" << std::endl;
    return 1;
  }

  // Open the input file
  std::ifstream in((char*)argv[1], std::ios::in | std::ios::binary);
  if (!in) {
    std::cout << "Cannot open file. "<< std::endl;
    return 1;
  }
  std::cout<<"look for your split file output in:"<<std::endl;

  // Names for output files
  strcpy (temp,"_Pixel");
  strcpy (outputFilePixel,outputFile);
  strncat(outputFilePixel,temp,6); 

  strcpy (temp,"_TDC");
  strcpy (outputFileTDC,outputFile);
  strncat(outputFileTDC,temp,4); 

  strcpy (temp,"_Scaler");
  strcpy (outputFileScaler,outputFile);
  strncat(outputFileScaler,temp,7); 

  strcpy (temp,"_Fastor");
  strcpy (outputFileFastor,outputFile);
  strncat(outputFileFastor,temp,7); 

  strcpy (temp,"_Histo");
  strcpy (outputFileHisto,outputFile);
  strncat(outputFileHisto,temp,6); 

  // Print names of output files
  std::cout << "File: " << outputFilePixel << std::endl; 
  std::cout << "File: " << outputFileTDC << std::endl; 
  std::cout << "File: " << outputFileScaler << std::endl; 
  std::cout << "File: " << outputFileFastor << std::endl; 
  std::cout << "File: " << outputFileHisto << std::endl; 

  // Open the output files
  std::ofstream foutPixel(outputFilePixel, std::ios::binary);  
  std::ofstream foutTDC(outputFileTDC, std::ios::binary);
  std::ofstream foutScaler(outputFileScaler, std::ios::binary);
  std::ofstream foutFastor(outputFileFastor, std::ios::binary);
  std::ofstream foutHisto(outputFileHisto, std::ios::binary);


  unsigned int oldevent = 0;
  unsigned int header = 0;
  unsigned int n1, n2;
  unsigned long long n;
  unsigned int bigbuff[4096];
  int wrdcount = 0;
  int event = 0;


  // Loop over everything
  for (int i = 0; i < 2000000000; i++) {

    // Read 64-bit word
    in.read((char *) &n2, sizeof n2);
    in.read((char *) &n1, sizeof n1);

    // If that length doesn't match we're done
    if (in.gcount() != sizeof(n1)) {
      std::cout << "End of File!" << std::endl;
      in.close();
      foutPixel.close();
      foutTDC.close();
      foutScaler.close();
      foutFastor.close();
      foutHisto.close();
      return 0;
    }

    wrdcount++;

    if ((n1 == 0x53333333) && (n2 == 0x53333333)) {
      //tdc buffer, special handling
      if ((lastheader & 0xff000000) == 0x50000000) {
        lheader = true;
      } else {
        lheader = false;
      }
      lastheader = n1;
      n = (n1 & 0xf0000000) + oldevent;
      n = (n << 32);
      TDCbuff[TDCbuflength] = n;
      TDCbuff[TDCbuflength] += n2;
      TDCbuflength++;

      for (int ih = 0; ih < 100; ih++) {
        in.read((char *) &n1, sizeof n1);
        if ((ih % 2) == 0) {
          n=n1;
          n=(n << 32);
        } else {
          if ((n1 & 0xf0000000) != 0xa0000000) {
            TDCbuff[TDCbuflength] = n;
            TDCbuff[TDCbuflength] += n2;
            TDCbuflength++;
          }
        }

        if ((n1 & 0xf0000000) == 0xa0000000) {
          in.read((char *) &n2, sizeof n1);
          {
            TDCbuff[TDCbuflength] = n;
            TDCbuff[TDCbuflength] += 0x0;
            TDCbuflength++;
          }
          if ((ih % 2) != 0) {
            n = n1;
            n = (n << 32);
            TDCbuff[TDCbuflength] = n;
            TDCbuff[TDCbuflength] += 0x0;
            TDCbuflength++;
          }
          wrdcount = 0;
          ictdc++;
          break;
        }
      }

    } else if ((n2 & 0xffffffff) == 0x54444444) {
      //scaler record
      n = (n2 & 0xff000000) + oldevent;
      n = (n << 32) + n1;
      foutScaler.write((char*)&n, sizeof(unsigned long long));
      lastheader = n2;

      for (int ih = 0; ih < 26; ih++) {
        in.read((char *) &n1, sizeof n1);
        foutScaler.write((char*)&n1, sizeof n1);
      }

      in.read((char *) &n1, sizeof n1);
      in.read((char *) &n2, sizeof n1);
      n = (n1 & 0xff000000) + 15;
      n = (n << 32) + n2;
      foutScaler.write((char*)&n, sizeof(unsigned long long));
      wrdcount = 0;

    } else if ((n2 & 0xffffffff) == 0x56666666) {
      //fast or record
      ictdc = 0;
      lastheader = n2;
      if (n1 == 0xa) {
        ifastorread++;
        if ((ifastorread * 48 + offset) != (int) oldevent) {
          std::cout << "expected event " << std::dec << (ifastorread * 48 + offset) << " got " << oldevent << std::endl;
          offset = oldevent - (ifastorread * 48);
          fastorprint = false;
          ifastorbad++;
          std::cout << "FAST OR bad= " << std::dec << ifastorbad << std::endl;
        } else {
          ifastorok++;
          std::cout << "FAST OR ok= " << std::dec << ifastorok << " last event# " << oldevent << std::endl;
        }
        foutPixel.write((char*)eventbuff, eventbuflength * sizeof(unsigned long long));
        eventbuflength = 0;
        foutTDC.write((char*)TDCbuff, TDCbuflength * sizeof(unsigned long long));
        TDCbuflength = 0;
      }
      if (fastorprint) {
        n = (n2 & 0xff000000) + (oldevent - 47);
        n = (n << 32) + n1;
        foutFastor.write((char*)&n, sizeof(unsigned long long));
      }
      in.read((char *) bigbuff, 1024 * sizeof(unsigned int));
      if (fastorprint) {
        foutFastor.write((char*)bigbuff,1024*sizeof(unsigned int));
      }

      in.read((char *) &n1, sizeof n1);
      in.read((char *) &n2, sizeof n1);
      if (fastorprint) {
        n = (n1 & 0xffffffff);
        n = (n << 32) + oldevent;
        foutFastor.write((char*)&n, sizeof(unsigned long long));
      }
      wrdcount = 0;
    } else if ((n2 & 0xffffffff) == 0x55555555) {
      //histo record
      n = (n2 & 0xff000000) + oldevent;
      n = (n << 32) + n1;
      foutHisto.write((char*)&n, sizeof(unsigned long long));

      in.read((char *) bigbuff, 4096 * sizeof(unsigned int));
      foutHisto.write((char*)bigbuff, 4096 * sizeof(unsigned int));

      in.read((char *) &n1, sizeof n1);
      in.read((char *) &n2, sizeof n1);
      n = (n1 & 0xff000000) + 15;
      n = (n << 32) + n2;
      foutHisto.write((char*)&n, sizeof(unsigned long long));
      wrdcount = 0;
    } else {
      if (((n1 & 0xff000000) == 0x50000000) & (wrdcount == 1)) {
        wrdcount = 0;
        header = n1;
        bheader = true;
        lastheader = n1;

        if ((oldevent + 1) != (n1 & 0xffffff)) {
          std::cout << "event number sequence interruption expected: " << std::dec << (oldevent + 1) << " got: " << (n1 & 0xffffff) << std::endl;
        }
        oldevent = (n1 & 0xffffff);
        n = (n1 & 0xffffffff);
        n = (n << 32);
        eventbuff[eventbuflength] = n;
        eventbuff[eventbuflength] += n2;
        eventbuflength++;
        event = (n1 & 0xffffff);
        fastorprint = true;
      } else if ((n1 & 0xf0000000) == 0xa0000000) {
        bheader = false;
        if (( n1 & 0xffffff) != (unsigned) (wrdcount + 1)) {
          std::cout << "Event " << std::dec << event << " Trailer word count = " << std::dec << (n1 & 0xffffff) << " Real word count = " << std::dec << (wrdcount + 1) << std::endl;
          std::cout << "0x" << std::hex << n1 << " 0x" << n2 << std::endl;
        }
        wrdcount = 0;
        wrdcount = 0;
        n = (n1 & 0xffffffff);
        n = (n << 32);
        eventbuff[eventbuflength] = n;
        eventbuff[eventbuflength] += n2;
        eventbuflength++;
        if ((n2 & 4) != 0) {
          std::cout << "CRC error!" << std::endl;
          std::cout << "0x" << std::hex << n1 << " 0x" << n2 << std::endl;
        }
      } else if (bheader) {
        n = (n1 & 0xffffffff);
        n = (n << 32);
        eventbuff[eventbuflength] = n;
        eventbuff[eventbuflength] += n2;
        eventbuflength++;
      }
    }

  }


}
