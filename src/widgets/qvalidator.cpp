/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.cpp#1 $
**
** C++ file skeleton
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qvalidator.h"

#include "qregexp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qvalidator.cpp#1 $");


struct QValidatorPrivate {
    bool useInt;
    bool useDouble;
    bool cs;

    int iBottom, iTop;
    double dBottom, dTop;
    int decimals;

    QString prefix;
    QString postfix;
};


/*!

*/

QValidator::QValidator( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = 0;
}


/*!

*/

QValidator::~QValidator()
{
    delete d;
    d = 0;
}


/*!
  
*/

QValidator::Result QValidator::validate( const char * input )
{
    if ( !input )
	return Unknown;
    if ( !d )
	return Good;
    QString s1( "^" );
    s1.append( prefix() );
    QRegExp r1( s1, caseSensitive(), FALSE );
    int p1 = 0;
    if ( r1.match( input, 0, &p1 ) < 0 )
	return Revert;

    QString s2( postfix() );
    s2.append( "$" );
    QRegExp r2( s2, caseSensitive(), FALSE );
    int p2 = r2.match( input, p1 );

    if ( p2 < 0 )
	return Revert;

    QString pureData( input );
    pureData = pureData.mid( p1, p2-p1 );
    return validatePureData( pureData );
}


/*!

*/

QValidator::Result QValidator::validatePureData( QString s )
{
    if ( d->useInt ) {
	bool ok;
	long int tmp = s.toLong( &ok );
	return ok && tmp >= d->iBottom && tmp <= d->iTop ? Good : Bad;
    } else if ( d->useDouble ) {
	bool ok;
	double tmp = s.toDouble( &ok );
	return ok && tmp >= d->dBottom && tmp <= d->dTop ? Good : Bad;
    }
    return Good;
}


/*!

*/

void QValidator::setInteger( int bottom, int top )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->useInt = TRUE;
    d->useDouble = FALSE;
    d->iBottom = bottom;
    d->iTop = top;
}


/*!

*/

void QValidator::setDouble( double bottom, double top, int decimals )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->useInt = FALSE;
    d->useDouble = TRUE;
    d->dBottom = bottom;
    d->dTop = top;
    d->decimals = decimals;
}


/*!

*/

void QValidator::setPrefix( const char * p )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->prefix = p;
}


/*!

*/

const char * QValidator::prefix() const
{
    if (d && d->prefix.length())
	return d->prefix;
    return 0;
}


/*!

*/

void QValidator::setPostfix( const char * p )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->postfix = p;
}


/*!

*/

const char * QValidator::postfix() const
{
    if (d && d->postfix.length())
	return d->postfix;
    return 0;
}


/*!

*/

void QValidator::setCaseSensitive( bool yes )
{
    if ( !d )
	d = new QValidatorPrivate();
    d->cs = yes;
}


/*!

*/

bool QValidator::caseSensitive() const
{
    return d ? d->cs : TRUE;
}
