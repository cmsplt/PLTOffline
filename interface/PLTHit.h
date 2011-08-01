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
    float fCharge;

    // Local coordinates on plane as define from the center of the diamond
    float fLX;
    float fLY;

    // Telescope coordinates on plane as define from the center of the diamond
    // After being aligned in telescope (if any alignment is done)
    float fTX;
    float fTY;
    float fTZ;

    // Global coordinates on plane as define from the center of the diamond
    float fGX;
    float fGY;
    float fGZ;


  public:
    void  SetCharge (float const);
    void  SetLXY (float const, float const);
    void  SetTXYZ (float const, float const, float const);
    void  SetGXYZ (float const, float const, float const);
    bool  MatchesColumnRow (PLTHit*);
    int   Channel ();
    int   ROC ();
    int   Row ();
    int   Column ();
    int   ADC ();
    float Charge ();

    float LX ();
    float LY ();

    float TX ();
    float TY ();
    float TZ ();

    float GX ();
    float GY ();
    float GZ ();


};




#endif

