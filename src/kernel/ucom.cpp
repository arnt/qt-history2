#ifndef UCOM_CPP
#define UCOM_CPP

#include "ucom.h"
#include <string.h>

void UType::clear( UObject* )
{
}

// {DE56510E-4E9F-4b76-A3C2-D1E2EF42F1AC}
const UUid TID_UType_Null =
{ 0xde56510e, 0x4e9f, 0x4b76, { 0xa3, 0xc2, 0xd1, 0xe2, 0xef, 0x42, 0xf1, 0xac } };
struct UType_Null : public UType
{
    const UUid *uuid() const { return &TID_UType_Null; }
    const char *desc() const { return "null"; }

    bool canConvertFrom( UObject *, UType * ) { return false; }
    bool canConvertTo( UObject *, UType * ) { return false; }
    bool convertFrom( UObject *, UType * ) { return false; }
    bool convertTo( UObject *, UType * ) { return false; }
};
static UType_Null static_UType_Null;
UType *pUType_Null = &static_UType_Null;


// {7EE17B08-5419-47e2-9776-8EEA112DCAEC}
const UUid TID_UType_enum =
{ 0x7ee17b08, 0x5419, 0x47e2, { 0x97, 0x76, 0x8e, 0xea, 0x11, 0x2d, 0xca, 0xec } };
static UType_enum static_UType_enum;
UType_enum *pUType_enum = &static_UType_enum;
const UUid *UType_enum::uuid() const { return &TID_UType_enum; }
const char *UType_enum::desc() const { return "enum"; }
void UType_enum::set( UObject *o, int v )
{
    o->payload.i = v;
    o->type = this;
}

bool UType_enum::canConvertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_int ) ) // ## todo unsigned int?
	return true;

    return t->canConvertTo( o, this );
}

bool UType_enum::canConvertTo( UObject *o, UType *t )
{
    return isEqual( t, pUType_int );
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


// {8AC26448-5AB4-49eb-968C-8F30AB13D732}
const UUid TID_UType_ptr =
{ 0x8ac26448, 0x5ab4, 0x49eb, { 0x96, 0x8c, 0x8f, 0x30, 0xab, 0x13, 0xd7, 0x32 } };
static UType_ptr static_UType_ptr;
UType_ptr *pUType_ptr = &static_UType_ptr;
const UUid *UType_ptr::uuid() const  { return &TID_UType_ptr; }
const char *UType_ptr::desc() const { return "ptr"; }

void UType_ptr::set( UObject *o, const void* v )
{
    o->payload.ptr = (void*) v;
    o->type = this;
}

bool UType_ptr::canConvertFrom( UObject *o, UType *t )
{
    return t->canConvertTo( o, this );
}

bool UType_ptr::canConvertTo( UObject *, UType * )
{
    return false;
}

bool UType_ptr::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_ptr::convertTo( UObject *, UType * )
{
    return false;
}

// {97A2594D-6496-4402-A11E-55AEF2D4D25C}
const UUid TID_UType_iface =
{ 0x97a2594d, 0x6496, 0x4402, { 0xa1, 0x1e, 0x55, 0xae, 0xf2, 0xd4, 0xd2, 0x5c } };
static UType_iface static_UType_iface;
UType_iface *pUType_iface = &static_UType_iface;
const UUid *UType_iface::uuid() const  { return &TID_UType_iface; }
const char *UType_iface::desc() const { return "UnknownInterface"; }

void UType_iface::set( UObject *o, UUnknownInterface* iface )
{
    o->payload.iface = iface;
    o->type = this;
}

bool UType_iface::canConvertFrom( UObject *o, UType *t )
{
    return t->canConvertTo( o, this );
}

bool UType_iface::canConvertTo( UObject *, UType * )
{
    return false;
}

bool UType_iface::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_iface::convertTo( UObject *, UType * )
{
    return false;
}

// {2F358164-E28F-4bf4-9FA9-4E0CDCABA50B}
const UUid TID_UType_idisp =
{ 0x2f358164, 0xe28f, 0x4bf4, { 0x9f, 0xa9, 0x4e, 0xc, 0xdc, 0xab, 0xa5, 0xb } };
static UType_idisp static_UType_idisp;
UType_idisp *pUType_idisp = &static_UType_idisp;
const UUid *UType_idisp::uuid() const  { return &TID_UType_idisp; }
const char *UType_idisp::desc() const { return "DispatchInterface"; }

void UType_idisp::set( UObject *o, UDispatchInterface* idisp )
{
    o->payload.idisp = idisp;
    o->type = this;
}

bool UType_idisp::canConvertFrom( UObject *o, UType *t )
{
    return t->canConvertTo( o, this );
}

bool UType_idisp::canConvertTo( UObject *o, UType *t )
{
    return isEqual( t, pUType_iface );
}

bool UType_idisp::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_idisp::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_iface ) ) {
	o->payload.iface = o->payload.idisp;
	o->type = pUType_iface;
	return true;
    }
    return false;
}

// {CA42115D-13D0-456c-82B5-FC10187F313E}
const UUid TID_UType_bool =
{ 0xca42115d, 0x13d0, 0x456c, { 0x82, 0xb5, 0xfc, 0x10, 0x18, 0x7f, 0x31, 0x3e } };
static UType_bool static_UType_bool;
UType_bool *pUType_bool = &static_UType_bool;
const UUid *UType_bool::uuid() const  { return &TID_UType_bool; }
const char *UType_bool::desc() const { return "bool"; }

void UType_bool::set( UObject *o, bool v )
{
    o->payload.b = v;
    o->type = this;
}

bool UType_bool::canConvertFrom( UObject *o, UType *t )
{
    return t->canConvertTo( o, this );
}

bool UType_bool::canConvertTo( UObject *, UType * )
{
    return false;
}

bool UType_bool::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_bool::convertTo( UObject *, UType * )
{
    return false;
}


// {53C1F3BE-73C3-4c7d-9E05-CCF09EB676B5}
const UUid TID_UType_int =
{ 0x53c1f3be, 0x73c3, 0x4c7d, { 0x9e, 0x5, 0xcc, 0xf0, 0x9e, 0xb6, 0x76, 0xb5 } };
static UType_int static_UType_int;
UType_int *pUType_int = &static_UType_int;
const UUid *UType_int::uuid() const  { return &TID_UType_int; }
const char *UType_int::desc() const { return "int"; }

void UType_int::set( UObject *o, int v )
{
    o->payload.i = v;
    o->type = this;
}

bool UType_int::canConvertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_double ) )
	return true;

    return t->canConvertTo( o, this );
}

bool UType_int::canConvertTo( UObject *o, UType *t )
{
    return isEqual( t,  pUType_double );
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

// {2D0974E5-0BA6-4ec2-8837-C198972CB48C}
const UUid TID_UType_double =
{ 0x2d0974e5, 0xba6, 0x4ec2, { 0x88, 0x37, 0xc1, 0x98, 0x97, 0x2c, 0xb4, 0x8c } };
static UType_double static_UType_double;
UType_double *pUType_double = &static_UType_double;
const UUid *UType_double::uuid() const { return &TID_UType_double; }
const char *UType_double::desc() const {return "double"; }

void UType_double::set( UObject *o, double v )
{
    o->payload.d = v;
    o->type = this;
}

bool UType_double::canConvertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_int ) )
	return true;

    return t->canConvertTo( o, this );
}

bool UType_double::canConvertTo( UObject *o, UType *t )
{
    return isEqual( t,  pUType_int );
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

// {EFCDD1D4-77A3-4b8e-8D46-DC14B8D393E9}
const UUid TID_UType_charstar =
{ 0xefcdd1d4, 0x77a3, 0x4b8e, { 0x8d, 0x46, 0xdc, 0x14, 0xb8, 0xd3, 0x93, 0xe9 } };
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

bool UType_charstar::canConvertFrom( UObject *o, UType *t )
{
    return t->canConvertTo( o, this );
}

bool UType_charstar::canConvertTo( UObject *, UType * )
{
    return false;
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

#endif //UCOM_CPP
