#include "utypes.h"
// {F1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_ptr = { 0xf1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_ptr static_UType_ptr;
UType_ptr *pUType_ptr = &static_UType_ptr;
const UUid *UType_ptr::uuid() const  { return &TID_UType_ptr; }
const char *UType_ptr::desc() const { return "ptr"; }

void UType_ptr::set( UObject *o, void* v )
{
    o->payload.ptr = v;
    o->type = this;
}

void* &UType_ptr::get( UObject *o, bool *ok )
{
    UTYPE_INIT( o, 0, ok )
    return o->payload.ptr;
}

bool UType_ptr::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_ptr::convertTo( UObject *, UType * )
{
    return false;
}

// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_int = { 0xe1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_int static_UType_int;
UType_int *pUType_int = &static_UType_int;
const UUid *UType_int::uuid() const  { return &TID_UType_int; }
const char *UType_int::desc() const { return "int"; }

void UType_int::set( UObject *o, int v )
{
    o->payload.i = v;
    o->type = this;
}

int &UType_int::get( UObject *o, bool *ok )
{
    UTYPE_INIT( o, 0, ok )
    return o->payload.i;
}

bool UType_int::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_double ) )
	o->payload.i = (long)o->payload.d;
    else
	return t->convertTo( o, this );

    o->type = this;
    return true;
}

bool UType_int::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_double ) ) {
	o->payload.d = (double)o->payload.i;
	o->type = pUType_double;
	return true;
    }

    return false;
}

// {A1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_double = { 0xa1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_double static_UType_double;
UType_double *pUType_double = &static_UType_double;
const UUid *UType_double::uuid() const { return &TID_UType_double; }
const char *UType_double::desc() const {return "double"; }

void UType_double::set( UObject *o, double v )
{
    o->payload.d = v;
    o->type = this;
}

double &UType_double::get( UObject *o, bool *ok )
{
    UTYPE_INIT( o, 0, ok )
    return o->payload.d;
}

bool UType_double::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_int ) )
	o->payload.d = (double)o->payload.i;
    else
	return t->convertTo( o, this );

    o->type = this;
    return true;
}

bool UType_double::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_int ) ) {
	o->payload.i = (int) o->payload.d;
	o->type = pUType_int;
	return true;
    }

    return false;
}

// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_charstar = { 0xc1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_charstar static_UType_charstar;
UType_charstar *pUType_charstar = &static_UType_charstar;
const UUid *UType_charstar::uuid() const { return &TID_UType_charstar; }
const char *UType_charstar::desc() const { return "char*"; }

void UType_charstar::set( UObject *o, const char* v )
{
    if ( v ) {
	o->payload.charstar = new char[ strlen(v) + 1 ];
	strcpy( o->payload.charstar, v );
    } else {
	o->payload.charstar = 0;
    }
    o->type = this;
}

char* UType_charstar::get( UObject *o, bool *ok )
{
    UTYPE_INIT( o, 0, ok )
    return o->payload.charstar;
}

bool UType_charstar::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_charstar::convertTo( UObject *, UType * )
{
    return false;
}

void UType_charstar::clear( UObject *o )
{
    delete [] o->payload.charstar;
    o->payload.ptr = 0;
}


static UParameter param = {
    "nase",
    pUType_ptr,
    pUType_ptr->desc(),
    UParameter::In
} ;
