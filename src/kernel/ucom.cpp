#include "ucom.h"
#include <string.h>

void UType::clear( UObject* )
{
}

// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF}
const UUid TID_UType_Null = { 0x261d70eb, 0x047d, 0x40b1, { 0xbe, 0xde, 0xda, 0xf1, 0xee, 0xc2, 0x73, 0xbf } };
struct UType_Null : public UType
{
    const UUid *uuid() const { return &TID_UType_Null; }
    const char *desc() const { return "null"; }

    bool convertFrom( UObject *, UType * ) { return false; }
    bool convertTo( UObject *, UType * ) { return false; }
};
static UType_Null static_UType_Null;
UType *pUType_Null = &static_UType_Null;


// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF}
const UUid TID_UType_enum = { 0x261d70eb, 0x047d, 0x40b1, {0xbe, 0xde, 0xda, 0xf1, 0xee, 0xc2, 0x73, 0xbf } };
static UType_enum static_UType_enum;
UType_enum *pUType_enum = &static_UType_enum;
const UUid *UType_enum::uuid() const { return &TID_UType_enum; }
const char *UType_enum::desc() const { return "enum"; }
void UType_enum::set( UObject *o, int v )
{
    o->payload.i = v;
    o->type = this;
}

bool UType_enum::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_int ) ) // ## todo unsigned int?
	;
    else
	return t->convertTo( o, this );

    o->type = this;
    return true;
}

bool UType_enum::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_int ) ) {
	o->type = pUType_int;
	return true;
    }
    return false;
}


// {F1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_ptr = { 0xf1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_ptr static_UType_ptr;
UType_ptr *pUType_ptr = &static_UType_ptr;
const UUid *UType_ptr::uuid() const  { return &TID_UType_ptr; }
const char *UType_ptr::desc() const { return "ptr"; }

void UType_ptr::set( UObject *o, const void* v )
{
    o->payload.ptr = (void*) v;
    o->type = this;
}

bool UType_ptr::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_ptr::convertTo( UObject *, UType * )
{
    return false;
}

// {F2D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_iface = { 0xf2d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_iface static_UType_iface;
UType_iface *pUType_iface = &static_UType_iface;
const UUid *UType_iface::uuid() const  { return &TID_UType_iface; }
const char *UType_iface::desc() const { return "UnknownInterface"; }

void UType_iface::set( UObject *o, UUnknownInterface* iface )
{
    o->payload.iface = iface;
    o->type = this;
}

bool UType_iface::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_iface::convertTo( UObject *, UType * )
{
    return false;
}

// {F3D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_idisp = { 0xf3d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_idisp static_UType_idisp;
UType_idisp *pUType_idisp = &static_UType_idisp;
const UUid *UType_idisp::uuid() const  { return &TID_UType_idisp; }
const char *UType_idisp::desc() const { return "DispatchInterface"; }

void UType_idisp::set( UObject *o, UDispatchInterface* idisp )
{
    o->payload.idisp = idisp;
    o->type = this;
}

bool UType_idisp::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_idisp::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_iface ) ) {
	o->payload.iface = o->payload.idisp;
	o->type = pUType_iface;
	return true;
    }
    return false;
}

// {E1D3BE80-2F2F-45F7-AB11-E8A0CEC84B82}
const UUid TID_UType_bool = { 0xe1d3be80, 0x2f2f, 0x45f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_bool static_UType_bool;
UType_bool *pUType_bool = &static_UType_bool;
const UUid *UType_bool::uuid() const  { return &TID_UType_bool; }
const char *UType_bool::desc() const { return "bool"; }

void UType_bool::set( UObject *o, bool v )
{
    o->payload.b = v;
    o->type = this;
}

bool UType_bool::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_bool::convertTo( UObject *o, UType *t )
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

void UType_charstar::set( UObject *o, const char* v, bool take )
{
    if ( take ) {
	if ( v ) {
	    o->payload.charstar.ptr = new char[ strlen(v) + 1 ];
	    strcpy( o->payload.charstar.ptr, v );
	} else {
	    o->payload.charstar.ptr = 0;
	}
	o->payload.charstar.owner = true;
    } else {
	o->payload.charstar.ptr = (char*) v;
	o->payload.charstar.owner = false;
    }
    o->type = this;
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
    if ( o->payload.charstar.owner )
	delete [] o->payload.charstar.ptr;
    o->payload.charstar.ptr = 0;
}
