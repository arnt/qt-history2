#include "utypes.h"

// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_int = { 0xe1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_int static_UType_int;
UType_int *pUType_int = &static_UType_int;
const UUid *UType_int::uuid() const  { return &TID_UType_int; }
const char *UType_int::desc() const { return "int"; }

void UType_int::set( UObject *o, int v )
{
    o->payload.l = v;
    o->type = this;
}

int UType_int::get( UObject *o, bool *ok )
{
    if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) {
	if ( ok )
	    *ok = false;
	return 0;
    }
    return o->payload.l;
}

bool UType_int::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_double ) ) {
	o->payload.l = (long)o->payload.d;
    } else {
	return t->convertTo( o, this );
    }

    o->type = this;
    return true;
}

bool UType_int::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_double ) ) {
	o->payload.d = (double)o->payload.l;
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

double UType_double::get( UObject *o, bool *ok )
{
    if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) {
	if ( ok )
	    *ok = false;
	return 0;
    }
    return o->payload.d;
}

bool UType_double::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_int ) ) {
	o->payload.d = (double)o->payload.l;
    } else {
	return t->convertTo( o, this );
    }

    o->type = this;
    return true;
}

bool UType_double::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_int ) ) {
	o->payload.l = (long)o->payload.d;
	o->type = pUType_int;
	return true;
    }

    return false;
}

// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_CharStar = { 0xc1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_CharStar static_UType_CharStar;
UType_CharStar *pUType_CharStar = &static_UType_CharStar;
const UUid *UType_CharStar::uuid() const { return &TID_UType_CharStar; }
const char *UType_CharStar::desc() const { return "char*"; }

void UType_CharStar::set( UObject *o, const char* v )
{
    if ( v ) {
	o->payload.ptr = new char[ strlen(v) + 1 ];
	strcpy( (char*)o->payload.ptr, v );
    } else {
	o->payload.ptr = 0;
    }
    o->type = this;
}

char* UType_CharStar::get( UObject *o, bool *ok )
{
    if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) {
	if ( ok )
	    *ok = false;
	return 0;
    }
    return (char*)o->payload.ptr;
}

bool UType_CharStar::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_CharStar::convertTo( UObject *, UType * )
{
    return false;
}

void UType_CharStar::clear( UObject *o )
{
    delete [](char*)o->payload.ptr;
    o->payload.ptr = 0;
}

