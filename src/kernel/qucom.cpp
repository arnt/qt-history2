#include "qucom.h"

// {44C2A547-01E7-4e56-8559-35AF9D2F42B7}
const UUid TID_UType_QString =
{ 0x44c2a547, 0x1e7, 0x4e56, { 0x85, 0x59, 0x35, 0xaf, 0x9d, 0x2f, 0x42, 0xb7 } };
static UType_QString static_UType_QString;
UType_QString *pUType_QString = &static_UType_QString;
const UUid *UType_QString::uuid() const { return &TID_UType_QString; }
const char *UType_QString::desc() const { return "QString"; }

void UType_QString::set( UObject *o, const QString& v )
{
    o->payload.ptr = new QString( v );
    o->type = this;
}

bool UType_QString::canConvertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_charstar ) ||
	 isEqual( t, pUType_double ) ||
	 isEqual( t, pUType_int ) )
	return true;

    return t->canConvertTo( o, this );
}

bool UType_QString::canConvertTo( UObject * /*o*/, UType *t )
{
    return isEqual( t, pUType_charstar ) ||
	isEqual( t,  pUType_int ) ||
	isEqual( t,  pUType_double );
}

bool UType_QString::convertFrom( UObject *o, UType *t )
{
    QString *str = 0;
    if ( isEqual( t, pUType_charstar ) )
	str = new QString( o->payload.charstar.ptr );
    else if ( isEqual( t, pUType_double ) )
	str = new QString( QString::number( o->payload.d ) );
    else if ( isEqual( t, pUType_int ) )
	str = new QString( QString::number( o->payload.i ) );
    else
	return t->convertTo( o, this );

    o->type->clear( o );
    o->payload.ptr = str;
    o->type = this;
    return true;
}

bool UType_QString::convertTo( UObject *o, UType *t )
{
    QString *str = (QString *)o->payload.ptr;
    if ( isEqual( t, pUType_charstar ) ) {
	o->payload.charstar.ptr = qstrdup( str->local8Bit().data() );
	o->payload.charstar.owner = true;
	o->type = pUType_charstar;
    } else if ( isEqual( t,  pUType_int ) ) {
	o->payload.l = str->toLong();
	o->type = pUType_int;
    } else if ( isEqual( t,  pUType_double ) ) {
	o->payload.d = str->toDouble();
	o->type = pUType_double;
    } else {
        return false;
    }
    delete str;
    return true;
}

void UType_QString::clear( UObject *o )
{
    delete (QString*)o->payload.ptr;
    o->payload.ptr = 0;
}


// 6dc75d58-a1d9-4417-b591-d45c63a3a4ea
const UUid TID_UType_QVariant =
{ 0x6dc75d58, 0xa1d9, 0x4417, { 0xb5, 0x91, 0xd4, 0x5c, 0x63, 0xa3, 0xa4, 0xea } };
static UType_QVariant static_UType_QVariant;
UType_QVariant *pUType_QVariant = &static_UType_QVariant;
const UUid *UType_QVariant::uuid() const { return &TID_UType_QVariant; }
const char *UType_QVariant::desc() const { return "QVariant"; }

void UType_QVariant::set( UObject *o, const QVariant& v )
{
    o->payload.ptr = new QVariant( v );
    o->type = this;
}

bool UType_QVariant::canConvertFrom( UObject *o, UType *t )
{
    return t->canConvertTo( o, this );
}

bool UType_QVariant::canConvertTo( UObject * /*o*/, UType *t )
{
    return false;
}

bool UType_QVariant::convertFrom( UObject *o, UType *t )
{
    return t->convertTo( o, this );
}

bool UType_QVariant::convertTo( UObject *o, UType *t )
{
    return false;
}

void UType_QVariant::clear( UObject *o )
{
    delete (QVariant*)o->payload.ptr;
    o->payload.ptr = 0;
}


#include "ucom.cpp"
