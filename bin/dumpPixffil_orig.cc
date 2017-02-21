#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

int convPXL(int ROC){
  int iconx[200];

  iconx[0]=-80;
  iconx[1]=80;
  iconx[2]=-79;
  iconx[3]=79;
  iconx[4]=-78;
  iconx[5]=78;
  iconx[6]=-77;
  iconx[7]=77;
  iconx[8]=-76;
  iconx[9]=76;
  iconx[10]=-75;
  iconx[11]=75;
  iconx[12]=-74;
  iconx[13]=74;
  iconx[14]=-73;
  iconx[15]=73;
  iconx[16]=-72;
  iconx[17]=72;
  iconx[18]=-71;
  iconx[19]=71;
  iconx[20]=-70;
  iconx[21]=70;
  iconx[22]=-69;
  iconx[23]=69;
  iconx[24]=-68;
  iconx[25]=68;
  iconx[26]=-67;
  iconx[27]=67;
  iconx[28]=-66;
  iconx[29]=66;
  iconx[30]=-65;
  iconx[31]=65;
  iconx[32]=-64;
  iconx[33]=64;
  iconx[34]=-63;
  iconx[35]=63;
  iconx[36]=-62;
  iconx[37]=62;
  iconx[38]=-61;
  iconx[39]=61;
  iconx[40]=-60;
  iconx[41]=60;
  iconx[42]=-59;
  iconx[43]=59;
  iconx[44]=-58;
  iconx[45]=58;
  iconx[46]=-57;
  iconx[47]=57;
  iconx[48]=-56;
  iconx[49]=56;
  iconx[50]=-55;
  iconx[51]=55;
  iconx[52]=-54;
  iconx[53]=54;
  iconx[54]=-53;
  iconx[55]=53;
  iconx[56]=-52;
  iconx[57]=52;
  iconx[58]=-51;
  iconx[59]=51;
  iconx[60]=-50;
  iconx[61]=50;
  iconx[62]=-49;
  iconx[63]=49;
  iconx[64]=-48;
  iconx[65]=48;
  iconx[66]=-47;
  iconx[67]=47;
  iconx[68]=-46;
  iconx[69]=46;
  iconx[70]=-45;
  iconx[71]=45;
  iconx[72]=-44;
  iconx[73]=44;
  iconx[74]=-43;
  iconx[75]=43;
  iconx[76]=-42;
  iconx[77]=42;
  iconx[78]=-41;
  iconx[79]=41;
  iconx[80]=-40;
  iconx[81]=40;
  iconx[82]=-39;
  iconx[83]=39;
  iconx[84]=-38;
  iconx[85]=38;
  iconx[86]=-37;
  iconx[87]=37;
  iconx[88]=-36;
  iconx[89]=36;
  iconx[90]=-35;
  iconx[91]=35;
  iconx[92]=-34;
  iconx[93]=34;
  iconx[94]=-33;
  iconx[95]=33;
  iconx[96]=-32;
  iconx[97]=32;
  iconx[98]=-31;
  iconx[99]=31;
  iconx[100]=-30;
  iconx[101]=30;
  iconx[102]=-29;
  iconx[103]=29;
  iconx[104]=-28;
  iconx[105]=28;
  iconx[106]=-27;
  iconx[107]=27;
  iconx[108]=-26;
  iconx[109]=26;
  iconx[110]=-25;
  iconx[111]=25;
  iconx[112]=-24;
  iconx[113]=24;
  iconx[114]=-23;
  iconx[115]=23;
  iconx[116]=-22;
  iconx[117]=22;
  iconx[118]=-21;
  iconx[119]=21;
  iconx[120]=-20;
  iconx[121]=20;
  iconx[122]=-19;
  iconx[123]=19;
  iconx[124]=-18;
  iconx[125]=18;
  iconx[126]=-17;
  iconx[127]=17;
  iconx[128]=-16;
  iconx[129]=16;
  iconx[130]=-15;
  iconx[131]=15;
  iconx[132]=-14;
  iconx[133]=14;
  iconx[134]=-13;
  iconx[135]=13;
  iconx[136]=-12;
  iconx[137]=12;
  iconx[138]=-11;
  iconx[139]=11;
  iconx[140]=-10;
  iconx[141]=10;
  iconx[142]=-9;
  iconx[143]=9;
  iconx[144]=-8;
  iconx[145]=8;
  iconx[146]=-7;
  iconx[147]=7;
  iconx[148]=-6;
  iconx[149]=6;
  iconx[150]=-5;
  iconx[151]=5;
  iconx[152]=-4;
  iconx[153]=4;
  iconx[154]=-3;
  iconx[155]=3;
  iconx[156]=-2;
  iconx[157]=2;
  iconx[158]=-1;
  iconx[159]=1;


  return iconx[ROC];
}

/////////////////////////////////////////////////////////////////////////////
// Decode error FIFO 
void decodeErrorFifo(unsigned long word) {   
  // Works for both, the error FIFO and the SLink error words. d.k. 25/04/07

  const unsigned long  errorMask      = 0x3e00000;
  const unsigned long  timeOut        = 0x3a00000;
  const unsigned long  eventNumError  = 0x3e00000;
  const unsigned long  trailError     = 0x3c00000;
  const unsigned long  fifoError      = 0x3800000;

  //  const unsigned long  timeOutChannelMask = 0x1f;  // channel mask for timeouts
  const unsigned long  eventNumMask = 0x1fe000; // event number mask
  const unsigned long  channelMask = 0xfc000000; // channel num mask
  const unsigned long  tbmEventMask = 0xff;    // tbm event num mask
  const unsigned long  overflowMask = 0x100;   // data overflow
  const unsigned long  tbmStatusMask = 0xff;   //TBM trailer info
  const unsigned long  BlkNumMask = 0x700;   //pointer to error fifo #
  const unsigned long  FsmErrMask = 0x600;   //pointer to FSM errors
  const unsigned long  RocErrMask = 0x800;   //pointer to #Roc errors 
  const unsigned long  ChnFifMask = 0x1f;   //channel mask for fifo error 
  const unsigned long  Fif2NFMask = 0x40;   //mask for fifo2 NF 
  const unsigned long  TrigNFMask = 0x80;   //mask for trigger fifo NF 

  const int offsets[8] = {0,4,9,13,18,22,27,31};


  if(word&0xffffffff){

    cout<<"error word "<<hex<<word<<dec<<endl;

    if( (word&errorMask)==timeOut ) { // TIMEOUT
      // More than 1 channel within a group can have a timeout error
      unsigned long index = (word & 0x1F);  // index within a group of 4/5
      unsigned long chip = (word& BlkNumMask)>>8;
      int offset = offsets[chip];
      cout<<"Timeout Error- channels: ";
      for(int i=0;i<5;i++) {
        if( (index & 0x1) != 0) {
          int channel = offset + i + 1;
          cout<<channel<<" ";
        }
        index = index >> 1;
      }
      //end of timeout  chip and channel decoding

    } else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
      unsigned long channel =  (word & channelMask) >>26;
      unsigned long tbm_event   =  (word & tbmEventMask);

      cout<<"Event Number Error- channel: "<<channel<<" tbm event nr. "
        <<tbm_event;

    } else if( ((word&errorMask) == trailError)) {
      unsigned long channel =  (word & channelMask) >>26;
      unsigned long tbm_status   =  (word & tbmStatusMask);
      if(word & RocErrMask)cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" ";
      if(word & FsmErrMask)cout<<"Finite State Machine Error- "<<"channel: "<<channel<<
        " Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" ";;
      if(word & overflowMask)cout<<"Overflow Error- "<<"channel: "<<channel<<" ";
      if(!((word & RocErrMask)|(word & FsmErrMask)|(word & overflowMask))) cout<<"Trailer Error- ";
      cout<<"channel: "<<channel<<" TBM status:0x"<<hex<<tbm_status<<dec<<" ";

    } else if((word&errorMask)==fifoError) {
      if(word & Fif2NFMask)cout<<"A fifo 2 is Nearly full- ";
      if(word & TrigNFMask)cout<<"The trigger fifo is nearly Full - ";
      if(word & ChnFifMask)cout<<"fifo-1 is nearly full for channel"<<(word & ChnFifMask);

    }
    unsigned long event   =  (word & eventNumMask) >>13;
    //unsigned long tbm_status   =  (word & tbmStatusMask);

    if(event>0)cout<<":event: "<<event;
    cout<<endl;


  }

}
//////////////////////////////////////////////////////////////////////////////
//
// Decode FIFO 2 spy data 
// The headers are still not treated correctly.
// 
void decodeSpyDataFifo(unsigned long word,int event) {  
  const bool ignoreInvalidData=false;
  if(word&0xfffffff){ 
    const unsigned long int plsmsk = 0xff;
    const unsigned long int pxlmsk = 0xff00;
    const unsigned long int dclmsk = 0x1f0000;
    const unsigned long int rocmsk = 0x3e00000;
    const unsigned long int chnlmsk = 0xfc000000;
    unsigned long int chan= ((word&chnlmsk)>>26);
    unsigned long int roc= ((word&rocmsk)>>21);

    // Check for embeded special words
    if(roc>25){

      if((word&0xffffffff)==0xffffffff) {cout<<" fifo-2 End of Event word"<<endl;
      } else if (roc==26) {cout<<"Gap word, Private Data = 0x"<<hex<<(word&0xff)<<dec<<endl;
      } else if (roc==27) {cout<<"Dummy Data Word, Private Data = 0x"<<hex<<(word&0xff)<<dec<<endl;
      } else {decodeErrorFifo(word);} 

    } else if(chan>0 && chan<37) {
      //cout<<hex<<word<<dec;
      int mycol=0;
      if(convPXL((word&pxlmsk)>>8)>0){mycol=((word&dclmsk)>>16)*2+1;} else {mycol=((word&dclmsk)>>16)*2;} 
      cout<<dec<<chan<<" "<<((word&rocmsk)>>21)<<" "<<mycol<<" "<<abs(convPXL((word&pxlmsk)>>8))<<" "<<(word&plsmsk)<<" "<<event;
      cout<<"                    ";
      cout<<" Chnl- "<<dec<<chan;
      cout<<" ROC- "<<((word&rocmsk)>>21);
      cout<<" COL- "<<mycol;
      cout<<" Pixel- "<<abs(convPXL((word&pxlmsk)>>8));
      cout<<" ADC- "<<(word&plsmsk)<<" event "<<event<<endl;

    } else {
      if(!ignoreInvalidData) cout<<" Invalid channel, possible Fifo-2 event count "<<chan<<" "<<hex<<(word&0xffffffff)<<dec<<endl;
    }
  } else {
    if(!ignoreInvalidData)cout<<" Possible Fifo-2 Begin of Event, data = "<<hex<<word<<dec<<endl;       
  }
} // end
int main(int argc, char *argv[])
{
  bool bheader=false;
  if(argc<2) {cout<<argc<< " usage: decodefil filename"<<endl; return 1;}
  //std::ifstream in("/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs/Run_43533/PhysicsDataFED_20_43533.dmp", std::ios::in | std::ios::binary); 
  //cout<<(char*)argv[1]<<endl;
  std::ifstream in((char*)argv[1], std::ios::in | std::ios::binary); 
  if(!in) { 
    std::cout << "Cannot open file. "<<endl; 
    return 1; 
  } 
  unsigned long long totevent=0;
  unsigned long header=0;
  unsigned long n1,n2;
  //unsigned long bxold=0;
  unsigned int wrdcount = 0;int event=0;int stopit=0;
  in.read((char *) &n2, sizeof n2);in.read((char *) &n1, sizeof n1); //begin of file word
  int hitcount=0;

  //for(int i=0;i<160;i++)cout<<convPXL(i)<<" "<<i<<endl;

  for(int i=0;i<2000000000;i++){

    in.read((char *) &n2, sizeof n2);in.read((char *) &n1, sizeof n1);

    //decodeSpyDataFifo(n1);//decodeSpyDataFifo(n2);

    if(in.gcount()!= sizeof(n1)){cout<<"End of File!"<<endl; return 0;}

    wrdcount++;
    cout<<"0x"<<hex<<n1<<" 0x"<<n2<<"  cnt "<<wrdcount<<endl;
    if((n1==0x53333333)&&(n1==0x53333333)){//tdc buffer, special handling

      for(int ih=0;ih<100;ih++){
        in.read((char *) &n1, sizeof n1);//cout<<"0x"<<hex<<n1<<
        if((n1&0xf0000000)==0xa0000000){
          in.read((char *) &n2, sizeof n1);
          cout<<"TTTTTTTTTTT0x"<<hex<<n1<<" 0x"<<n2<<"  cnt "<<dec<<ih<<" "<<(n1&0xffffff)<<endl;
          wrdcount=0;
          break;
        }
      }


    } else {
      if(((n1&0xff000000)==0x50000000)&(wrdcount==1))
      {wrdcount=0; header=n1;bheader=true;
        //cout<<"Header for event = "<<dec<<(n1&0xffffff)<<" fedID "<<((n2&0xff00)>>8)<<" BX = "<<((n2&0xffff0000)>>16)<<endl;
        cout<<dec<<" BX = "<<((n2&0xfff00000)>>20)<<endl;
        stopit=0;
        event=(n1&0xffffff);totevent++;
        //cout<<"event "<<event<<" "<<totevent<<endl;
        //cout<<"0x"<<hex<<n1<<" 0x"<<n2<<endl;
        //cout<<"myheadev "<<dec<<(n1&0xffffff)<<" "<<((n2&0xfff00000)>>20)<<" "<<((n2&0x000fff00)>>8)<<endl;
        //cout<<event<<"  "<<(((n2&0xfff00000)>>20))<<endl;//bx
        //bxold=((n2&0xfff00000)>>20);
        //<<dec<<"   0x"<<hex<<n2<<dec<<endl; 
      }

      else if((n1&0xf0000000)==0xa0000000){bheader=false;
        if(stopit>0){string input;getline( cin, input );}
        //out<<"Trailer word count = "<<dec<<(n1&0xffffff)<<"0x"<<hex<<n1<<" 0x"<<n2<<endl;
        if(event>16500000&&event<16600000)cout<<" ";
        if(event>16600000)cout<<"event "<<event<<" wordcount "<<dec<<(n1&0xffffff)<<" number of hits "<<dec<<hitcount<<endl;
        if(hitcount>2)cout<<hitcount<<endl;
        if((n1&0xffffff)!=(wrdcount+1)){
          //cout<<"Event "<<dec<<event<<" Trailer word count = "<<dec<<(n1&0xffffff)<<" Real word count = "<<dec<<(wrdcount+1)<<endl;
          //cout<<"0x"<<hex<<n1<<" 0x"<<n2<<endl;
        }wrdcount=0;
        //if(event!=1956)event=0;
        wrdcount=0;hitcount=0;
        if((n2&4)!=0){cout<<"CRC error!"<<endl;cout<<"0x"<<hex<<n1<<" 0x"<<n2<<endl;}}

      else if(((n1&0xfff00000)==0x3600000)&&((n2&0xfff00000)==0x3600000)){
        if((n1&0xff)!=(n2&0xff)) bheader=false;
      }
      else if(((n1&0x3e00000)>0x3700000)) {
        if((n1&0x3e00000)==0x3800000)cout<<"******************** NF Floagged 0x"<<hex<<n1<<" 0x"<<n2<<dec<<endl;
        if(bheader)decodeSpyDataFifo(n1,event);
        if(((n2&0x3e00000)>0x3700000)) {if(bheader)decodeSpyDataFifo(n2,event);}
        if(((n2&0x3e00000)<0x3200000)) {if(bheader)decodeSpyDataFifo(n2,event);}
      }

      else if(((n2&0x3e00000)>0x3700000)) {
        if((n2&0x3e00000)==0x3800000)cout<<"******************** NF Floagged "<<hex<<n1<<" 0x"<<n2<<dec<<endl;
        if(bheader)decodeSpyDataFifo(n2,event);
        if(((n1&0x3e00000)>0x3700000)) {if(bheader)decodeSpyDataFifo(n1,event);}
        if(((n1&0x3e00000)<0x3200000)) {if(bheader)decodeSpyDataFifo(n1,event);}
      }

      else if(((n1&0x3e00000)<0x3200000)) {
        //if((n2&0x3e00000)!=0x3c00000)
        hitcount++;
        //cout<<"Hit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
        if(bheader)decodeSpyDataFifo(n1,event);
        if(((n2&0x3e00000)>0x3700000)) {if(bheader)decodeSpyDataFifo(n2,event);}
        if(((n2&0x3e00000)<0x3200000)) {if(bheader)decodeSpyDataFifo(n2,event);}
        //if(((n2&0xfc000000)>>26)==17)stopit++;
      }

      else if(((n2&0x3e00000)<0x3200000)) {
        //if((n2&0x3e00000)!=0x3c00000)
        hitcount++;
        //cout<<"Hit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
        if(bheader)decodeSpyDataFifo(n2,event);
        if(((n1&0x3e00000)>0x3700000)) {if(bheader)decodeSpyDataFifo(n1,event);}
        if(((n1&0x3e00000)<0x3200000)) {if(bheader)decodeSpyDataFifo(n1,event);}
        //if(((n2&0xfc000000)>>26)==17)stopit++;

      }




      else {



        //if(event==1956){
        //cout<<"0x"<<hex<<n1<<" 0x"<<n2<<endl;
        //decodeSpyDataFifo(n1);
        //decodeSpyDataFifo(n2);


      }
      }
    }  
    in.close();


  }

