#include "qutypes.h"

// {B1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
const UUid TID_UType_QString = { 0xb1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };
static UType_QString static_UType_QString;
UType_QString *pUType_QString = &static_UType_QString;
const UUid *UType_QString::uuid() const { return &TID_UType_QString; }
const char *UType_QString::desc() const { return "QString"; }

void UType_QString::set( UObject *o, const QString& v )
{
    o->payload.ptr = new QString( v );
    o->type = this;
}

QString &UType_QString::get( UObject *o, bool *ok )
{
    UTYPE_INIT( o, QString::null, ok )
    return *(QString*)o->payload.ptr;
}

bool UType_QString::convertFrom( UObject *o, UType *t )
{
    QString *str = 0;
    if ( isEqual( t, pUType_charstar ) )
	str = new QString( o->payload.charstar );
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
	o->payload.ptr = qstrdup( str->local8Bit().data() );
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

