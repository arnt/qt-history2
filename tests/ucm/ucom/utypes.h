#ifndef UTYPES_H
#define UTYPES_H

#include "ucom.h"


// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_int;
struct UType_int : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_int * pUType_int;


// {A1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_double;
struct UType_double : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, double );
    double get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_double * pUType_double;


// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_CharStar;
struct UType_CharStar : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const char* );
    char* get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern UType_CharStar * pUType_CharStar;



//##### TODO: UType_Utf8 UType_Local8Bit


#endif //UTYPES_H
