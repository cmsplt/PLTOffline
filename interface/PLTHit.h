#ifndef GUARD_PLTHit_h
#define GUARD_PLTHit_h

#include <string>
#include <sstream>



class PLTHit
{
  public:
    PLTHit ();
    PLTHit (std::string&);
    PLTHit (int, int, int, int, int);
    ~PLTHit ();

  private:
    int fChannel;
    int fROC;
    int fColumn;
    int fRow;
    int fADC;
    long unsigned fEvent;
    float fCharge;

  public:
    void  SetCharge (float const);
    bool  MatchesColumnRow (PLTHit*);
    int   Channel ();
    int   ROC ();
    int   Row ();
    int   Column ();
    int   ADC ();
    float Charge ();
    float Event ();


};




#endif

