#include "qucom.h"
#include "qcom.h"

void QUType::clear( QUObject* )
{
}

bool QUType::serializeTo( QUObject *, void * )
{
    return FALSE;
}

bool QUType::serializeFrom( QUObject *, void * )
{
    return FALSE;
}

// Standard types

// {DE56510E-4E9F-4b76-A3C2-D1E2EF42F1AC}
const QUuid TID_QUType_Null( 0xde56510e, 0x4e9f, 0x4b76, 0xa3, 0xc2, 0xd1, 0xe2, 0xef, 0x42, 0xf1, 0xac );
struct QUType_Null : public QUType
{
    const QUuid *uuid() const { return &TID_QUType_Null; }
    const char *desc() const { return "null"; }

    bool canConvertFrom( QUObject *, QUType * ) { return FALSE; }
    bool canConvertTo( QUObject *, QUType * ) { return FALSE; }
    bool convertFrom( QUObject *, QUType * ) { return FALSE; }
    bool convertTo( QUObject *, QUType * ) { return FALSE; }
};
static QUType_Null static_QUType_Null;
QUType *pQUType_Null = &static_QUType_Null;


// {7EE17B08-5419-47e2-9776-8EEA112DCAEC}
const QUuid TID_QUType_enum( 0x7ee17b08, 0x5419, 0x47e2, 0x97, 0x76, 0x8e, 0xea, 0x11, 0x2d, 0xca, 0xec );
static QUType_enum static_QUType_enum;
QUType_enum *pQUType_enum = &static_QUType_enum;
const QUuid *QUType_enum::uuid() const { return &TID_QUType_enum; }
const char *QUType_enum::desc() const { return "enum"; }
void QUType_enum::set( QUObject *o, int v )
{
    o->payload.i = v;
    o->type = this;
}

bool QUType_enum::canConvertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_int ) ) // ## todo unsigned int?
	return TRUE;

    return t->canConvertTo( o, this );
}

bool QUType_enum::canConvertTo( QUObject * /*o*/, QUType *t )
{
    return isEqual( t, pQUType_int );
}

bool QUType_enum::convertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_int ) ) // ## todo unsigned int?
	;
    else
	return t->convertTo( o, this );

    o->type = this;
    return TRUE;
}

bool QUType_enum::convertTo( QUObject *o, QUType *t )
{
    if ( isEqual( t,  pQUType_int ) ) {
	o->type = pQUType_int;
	return TRUE;
    }
    return FALSE;
}


// {8AC26448-5AB4-49eb-968C-8F30AB13D732}
const QUuid TID_QUType_ptr( 0x8ac26448, 0x5ab4, 0x49eb, 0x96, 0x8c, 0x8f, 0x30, 0xab, 0x13, 0xd7, 0x32 );
static QUType_ptr static_QUType_ptr;
QUType_ptr *pQUType_ptr = &static_QUType_ptr;
const QUuid *QUType_ptr::uuid() const  { return &TID_QUType_ptr; }
const char *QUType_ptr::desc() const { return "ptr"; }

void QUType_ptr::set( QUObject *o, const void* v )
{
    o->payload.ptr = (void*) v;
    o->type = this;
}

bool QUType_ptr::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_ptr::canConvertTo( QUObject *, QUType * )
{
    return FALSE;
}

bool QUType_ptr::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_ptr::convertTo( QUObject *, QUType * )
{
    return FALSE;
}

// {97A2594D-6496-4402-A11E-55AEF2D4D25C}
const QUuid TID_QUType_iface( 0x97a2594d, 0x6496, 0x4402, 0xa1, 0x1e, 0x55, 0xae, 0xf2, 0xd4, 0xd2, 0x5c );
static QUType_iface static_QUType_iface;
QUType_iface *pQUType_iface = &static_QUType_iface;
const QUuid *QUType_iface::uuid() const  { return &TID_QUType_iface; }
const char *QUType_iface::desc() const { return "UnknownInterface"; }

void QUType_iface::set( QUObject *o, QUnknownInterface* iface )
{
    o->payload.iface = iface;
    o->type = this;
}

bool QUType_iface::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_iface::canConvertTo( QUObject *, QUType * )
{
    return FALSE;
}

bool QUType_iface::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_iface::convertTo( QUObject *, QUType * )
{
    return FALSE;
}

// {2F358164-E28F-4bf4-9FA9-4E0CDCABA50B}
const QUuid TID_QUType_idisp( 0x2f358164, 0xe28f, 0x4bf4, 0x9f, 0xa9, 0x4e, 0xc, 0xdc, 0xab, 0xa5, 0xb );
static QUType_idisp static_QUType_idisp;
QUType_idisp *pQUType_idisp = &static_QUType_idisp;
const QUuid *QUType_idisp::uuid() const  { return &TID_QUType_idisp; }
const char *QUType_idisp::desc() const { return "DispatchInterface"; }

void QUType_idisp::set( QUObject *o, QDispatchInterface* idisp )
{
    o->payload.idisp = idisp;
    o->type = this;
}

bool QUType_idisp::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_idisp::canConvertTo( QUObject * /*o*/, QUType *t )
{
    return isEqual( t, pQUType_iface );
}

bool QUType_idisp::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_idisp::convertTo( QUObject *o, QUType *t )
{
#ifndef QT_NO_COMPONENT
    if ( isEqual( t, pQUType_iface ) ) {
	o->payload.iface = (QUnknownInterface*)o->payload.idisp;
	o->type = pQUType_iface;
	return TRUE;
    }
#endif
    return FALSE;
}

// {CA42115D-13D0-456c-82B5-FC10187F313E}
const QUuid TID_QUType_bool( 0xca42115d, 0x13d0, 0x456c, 0x82, 0xb5, 0xfc, 0x10, 0x18, 0x7f, 0x31, 0x3e );
static QUType_bool static_QUType_bool;
QUType_bool *pQUType_bool = &static_QUType_bool;
const QUuid *QUType_bool::uuid() const  { return &TID_QUType_bool; }
const char *QUType_bool::desc() const { return "bool"; }

void QUType_bool::set( QUObject *o, bool v )
{
    o->payload.b = v;
    o->type = this;
}

bool QUType_bool::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_bool::canConvertTo( QUObject *, QUType * )
{
    return FALSE;
}

bool QUType_bool::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_bool::convertTo( QUObject *, QUType * )
{
    return FALSE;
}


// {53C1F3BE-73C3-4c7d-9E05-CCF09EB676B5}
const QUuid TID_QUType_int( 0x53c1f3be, 0x73c3, 0x4c7d, 0x9e, 0x5, 0xcc, 0xf0, 0x9e, 0xb6, 0x76, 0xb5 );
static QUType_int static_QUType_int;
QUType_int *pQUType_int = &static_QUType_int;
const QUuid *QUType_int::uuid() const  { return &TID_QUType_int; }
const char *QUType_int::desc() const { return "int"; }

void QUType_int::set( QUObject *o, int v )
{
    o->payload.i = v;
    o->type = this;
}

bool QUType_int::canConvertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_double ) )
	return TRUE;

    return t->canConvertTo( o, this );
}

bool QUType_int::canConvertTo( QUObject * /*o*/, QUType *t )
{
    return isEqual( t,  pQUType_double );
}

bool QUType_int::convertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_double ) )
	o->payload.i = (long)o->payload.d;
    else
	return t->convertTo( o, this );

    o->type = this;
    return TRUE;
}

bool QUType_int::convertTo( QUObject *o, QUType *t )
{
    if ( isEqual( t,  pQUType_double ) ) {
	o->payload.d = (double)o->payload.i;
	o->type = pQUType_double;
	return TRUE;
    }
    return FALSE;
}

// {2D0974E5-0BA6-4ec2-8837-C198972CB48C}
const QUuid TID_QUType_double( 0x2d0974e5, 0xba6, 0x4ec2, 0x88, 0x37, 0xc1, 0x98, 0x97, 0x2c, 0xb4, 0x8c );
static QUType_double static_QUType_double;
QUType_double *pQUType_double = &static_QUType_double;
const QUuid *QUType_double::uuid() const { return &TID_QUType_double; }
const char *QUType_double::desc() const {return "double"; }

void QUType_double::set( QUObject *o, double v )
{
    o->payload.d = v;
    o->type = this;
}

bool QUType_double::canConvertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_int ) )
	return TRUE;

    return t->canConvertTo( o, this );
}

bool QUType_double::canConvertTo( QUObject * /*o*/, QUType *t )
{
    return isEqual( t,  pQUType_int );
}

bool QUType_double::convertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_int ) )
	o->payload.d = (double)o->payload.i;
    else
	return t->convertTo( o, this );

    o->type = this;
    return TRUE;
}

bool QUType_double::convertTo( QUObject *o, QUType *t )
{
    if ( isEqual( t,  pQUType_int ) ) {
	o->payload.i = (int) o->payload.d;
	o->type = pQUType_int;
	return TRUE;
    }
    return FALSE;
}

// {EFCDD1D4-77A3-4b8e-8D46-DC14B8D393E9}
const QUuid TID_QUType_charstar( 0xefcdd1d4, 0x77a3, 0x4b8e, 0x8d, 0x46, 0xdc, 0x14, 0xb8, 0xd3, 0x93, 0xe9 );
static QUType_charstar static_QUType_charstar;
QUType_charstar *pQUType_charstar = &static_QUType_charstar;
const QUuid *QUType_charstar::uuid() const { return &TID_QUType_charstar; }
const char *QUType_charstar::desc() const { return "char*"; }

void QUType_charstar::set( QUObject *o, const char* v, bool take )
{
    if ( take ) {
	if ( v ) {
	    o->payload.charstar.ptr = new char[ strlen(v) + 1 ];
	    strcpy( o->payload.charstar.ptr, v );
	} else {
	    o->payload.charstar.ptr = 0;
	}
	o->payload.charstar.owner = TRUE;
    } else {
	o->payload.charstar.ptr = (char*) v;
	o->payload.charstar.owner = FALSE;
    }
    o->type = this;
}

bool QUType_charstar::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_charstar::canConvertTo( QUObject *, QUType * )
{
    return FALSE;
}

bool QUType_charstar::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_charstar::convertTo( QUObject *, QUType * )
{
    return FALSE;
}

void QUType_charstar::clear( QUObject *o )
{
    if ( o->payload.charstar.owner )
	delete [] o->payload.charstar.ptr;
    o->payload.charstar.ptr = 0;
}


// Qt specific types

// {44C2A547-01E7-4e56-8559-35AF9D2F42B7}
const QUuid TID_QUType_QString( 0x44c2a547, 0x1e7, 0x4e56, 0x85, 0x59, 0x35, 0xaf, 0x9d, 0x2f, 0x42, 0xb7 );
static QUType_QString static_QUType_QString;
QUType_QString *pQUType_QString = &static_QUType_QString;
const QUuid *QUType_QString::uuid() const { return &TID_QUType_QString; }
const char *QUType_QString::desc() const { return "QString"; }

void QUType_QString::set( QUObject *o, const QString& v )
{
    o->payload.ptr = new QString( v );
    o->type = this;
}

bool QUType_QString::canConvertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( t, pQUType_charstar ) ||
	 isEqual( t, pQUType_double ) ||
	 isEqual( t, pQUType_int ) )
	return TRUE;

    return t->canConvertTo( o, this );
}

bool QUType_QString::canConvertTo( QUObject * /*o*/, QUType *t )
{
    return isEqual( t, pQUType_charstar ) ||
	isEqual( t,  pQUType_int ) ||
	isEqual( t,  pQUType_double );
}

bool QUType_QString::convertFrom( QUObject *o, QUType *t )
{
    QString *str = 0;
    if ( isEqual( t, pQUType_charstar ) )
	str = new QString( o->payload.charstar.ptr );
    else if ( isEqual( t, pQUType_double ) )
	str = new QString( QString::number( o->payload.d ) );
    else if ( isEqual( t, pQUType_int ) )
	str = new QString( QString::number( o->payload.i ) );
    else
	return t->convertTo( o, this );

    o->type->clear( o );
    o->payload.ptr = str;
    o->type = this;
    return TRUE;
}

bool QUType_QString::convertTo( QUObject *o, QUType *t )
{
    QString *str = (QString *)o->payload.ptr;
    if ( isEqual( t, pQUType_charstar ) ) {
	o->payload.charstar.ptr = qstrdup( str->local8Bit().data() );
	o->payload.charstar.owner = TRUE;
	o->type = pQUType_charstar;
    } else if ( isEqual( t,  pQUType_int ) ) {
	o->payload.l = str->toLong();
	o->type = pQUType_int;
    } else if ( isEqual( t,  pQUType_double ) ) {
	o->payload.d = str->toDouble();
	o->type = pQUType_double;
    } else {
        return FALSE;
    }
    delete str;
    return TRUE;
}

void QUType_QString::clear( QUObject *o )
{
    delete (QString*)o->payload.ptr;
    o->payload.ptr = 0;
}


// 6dc75d58-a1d9-4417-b591-d45c63a3a4ea
const QUuid TID_QUType_QVariant( 0x6dc75d58, 0xa1d9, 0x4417, 0xb5, 0x91, 0xd4, 0x5c, 0x63, 0xa3, 0xa4, 0xea );
static QUType_QVariant static_QUType_QVariant;
QUType_QVariant *pQUType_QVariant = &static_QUType_QVariant;
const QUuid *QUType_QVariant::uuid() const { return &TID_QUType_QVariant; }
const char *QUType_QVariant::desc() const { return "QVariant"; }

void QUType_QVariant::set( QUObject *o, const QVariant& v )
{
    o->payload.ptr = new QVariant( v );
    o->type = this;
}

bool QUType_QVariant::canConvertFrom( QUObject *o, QUType *t )
{
    return t->canConvertTo( o, this );
}

bool QUType_QVariant::canConvertTo( QUObject * /*o*/, QUType * /*t*/ )
{
    return FALSE;
}

bool QUType_QVariant::convertFrom( QUObject *o, QUType *t )
{
    return t->convertTo( o, this );
}

bool QUType_QVariant::convertTo( QUObject * /*o*/, QUType * /*t*/ )
{
    return FALSE;
}

void QUType_QVariant::clear( QUObject *o )
{
    delete (QVariant*)o->payload.ptr;
    o->payload.ptr = 0;
}


