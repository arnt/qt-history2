#ifndef UTYPES_H
#define UTYPES_H

#include "ucom.h"

#define UTYPE_INIT( o, x, ok )     if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) { \
	if ( ok ) \
	    *ok = false; \
	o->type->clear( o ); \
	set( o, x ); \
    }

// {F1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_ptr;
struct UType_ptr : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, void* );
    void* &get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_ptr * pUType_ptr;

// {F2D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_iface;
struct UType_iface : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, UUnknownInterface* );
    UUnknownInterface* &get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_iface * pUType_iface;

// {F3D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_idisp;
struct UType_idisp : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, UDispatchInterface* );
    UDispatchInterface* &get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_idisp * pUType_idisp;

// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_int;
struct UType_int : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, int );
    int &get( UObject *, bool * = 0 );
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
    double &get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );
};
extern UType_double * pUType_double;


// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_charstar;
struct UType_charstar : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const char* );
    char* get( UObject *, bool * = 0 );
    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern UType_charstar * pUType_charstar;



//##### TODO: UType_Utf8 UType_Local8Bit


#endif //UTYPES_H
