#include "ucom.h"

// {261D70EB-047D-40B1-BEDE-DAF1EEC273BF} 
const UUid TID_UType_Null = { 0x261d70eb, 0x047d, 0x40b1, { 0xbe, 0xde, 0xda, 0xf1, 0xee, 0xc2, 0x73, 0xbf } };

static UType_Null static_UType_Null;
UType_Null *pUType_Null = &static_UType_Null;

const char *UType_Null::desc() const
{
    return "Null";
}

void UType::copy( UObject *dst, const UObject *src )
{
    //ASSERT( isEqual( src->type, this ) );
    memcpy( dst, src, sizeof(UObject) );
}

// {E1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
const UUid TID_UType_Int = { 0xe1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };

static UType_Int static_UType_Int;
UType_Int *pUType_Int = &static_UType_Int;

const char *UType_Int::desc() const
{
    return "int";
}

void UType_Int::set( UObject *o, int v )
{
    o->payload.l = v;
    o->type = this;
}

int UType_Int::get( UObject *o, bool *ok )
{
    if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) {
	if ( ok )
	    *ok = false;
	return 0;
    }
    return o->payload.l;
}

bool UType_Int::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_Double ) ) {
	o->payload.l = (long)o->payload.d;
    } else {
	return t->convertTo( o, this );
    }

    o->type = this;
    return true;
}

bool UType_Int::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_Double ) ) {
	o->payload.d = (double)o->payload.l;
	o->type = pUType_Double;
	return true;
    }

    return false;
}

// {A1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
const UUid TID_UType_Double = { 0xa1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };

static UType_Double static_UType_Double;
UType_Double *pUType_Double = &static_UType_Double;

const char *UType_Double::desc() const
{
    return "double";
}

void UType_Double::set( UObject *o, double v )
{
    o->payload.d = v;
    o->type = this;
}

double UType_Double::get( UObject *o, bool *ok )
{
    if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) {
	if ( ok )
	    *ok = false;
	return 0;
    }
    return o->payload.d;
}

bool UType_Double::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_Int ) ) {
	o->payload.d = (double)o->payload.l;
    } else {
	return t->convertTo( o, this );
    }

    o->type = this;
    return true;
}

bool UType_Double::convertTo( UObject *o, UType *t )
{
    if ( isEqual( t,  pUType_Int ) ) {
	o->payload.l = (long)o->payload.d;
	o->type = pUType_Int;
	return true;
    }

    return false;
}

// {C1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
const UUid TID_UType_CharStar = { 0xc1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };

static UType_CharStar static_UType_CharStar;
UType_CharStar *pUType_CharStar = &static_UType_CharStar;

const char *UType_CharStar::desc() const
{
    return "char*";
}

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

// {B1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82} 
const UUid TID_UType_QString = { 0xb1d3be80, 0x2f2f, 0x44f7, { 0xab, 0x11, 0xe8, 0xa0, 0xce, 0xc8, 0x4b, 0x82 } };

static UType_QString static_UType_QString;
UType_QString *pUType_QString = &static_UType_QString;

const char *UType_QString::desc() const
{
    return "QString";
}

void UType_QString::set( UObject *o, const QString& v )
{
    o->payload.ptr = new QString( v );
    o->type = this;
}

QString UType_QString::get( UObject *o, bool *ok )
{
    if ( !isEqual( o->type, this ) && !convertFrom( o, o->type ) ) {
	if ( ok )
	    *ok = false;
	return QString::null;
    }
    return *(QString*)o->payload.ptr;
}

bool UType_QString::convertFrom( UObject *o, UType *t )
{
    if ( isEqual( t, pUType_CharStar ) ) {
	QString *tmp = new QString( (char*)o->payload.ptr );
	o->payload.ptr = tmp;
    } else if ( isEqual( t, pUType_Double ) ) {
	QString *tmp = new QString;
	*tmp = QString::number( o->payload.d );
	o->payload.ptr = tmp;
    } else if ( isEqual( t, pUType_Int ) ) {
	QString *tmp = new QString;
	*tmp = QString::number( o->payload.l );
	o->payload.ptr = tmp;
    } else {
	return t->convertTo( o, this );
    }
    o->type = this;
    return true;
}

bool UType_QString::convertTo( UObject *o, UType *t )
{
    QString *str = (QString *)o->payload.ptr;
    if ( isEqual( t, pUType_CharStar ) ) {
	o->payload.ptr = qstrdup( str->local8Bit().data() );
	o->type = pUType_CharStar;
    } else if ( isEqual( t,  pUType_Int ) ) {
	o->payload.l = str->toLong();
	o->type = pUType_Int;
    } else if ( isEqual( t,  pUType_Double ) ) {
	o->payload.d = str->toDouble();
	o->type = pUType_Double;
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
