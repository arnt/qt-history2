/****************************************************************************
** $Id: $
**
** Declaration and implementation of data conversion routines
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <private/qcom_p.h>
#include <private/qucomextra_p.h>

/*! 
    Helper functions 
*/
static inline QString BSTRToQString( BSTR bstr )
{
    QString str;
    if ( !bstr )
	return str;

    int len = wcslen( bstr );
    str.setUnicode( (QChar*)bstr, len );
    return str;
}

static inline BSTR QStringToBSTR( const QString &str )
{
    BSTR bstrVal;

    int wlen = str.length();
    bstrVal = SysAllocStringByteLen( 0, wlen*2 );
    memcpy( bstrVal, str.unicode(), sizeof(QChar)*(wlen) );
    bstrVal[wlen] = 0;

    return bstrVal;
}

extern QDateTime DATEToQDateTime( DATE ole );
extern DATE QDateTimeToDATE( const QDateTime &dt );
extern void VARIANTToQUObject( VARIANT arg, QUObject *obj );

struct IFont;
struct IFontDisp;

extern IFontDisp *QFontToIFont( const QFont &font );
extern QFont IFontToQFont( IFont *f );

static uint QColorToOLEColor( const QColor &col )
{
    return qRgb( col.blue(), col.green(), col.red() );
}

static QColor OLEColorToQColor( uint col )
{
    return QColor( qBlue(col), qGreen(col), qRed(col) );
}

extern VARIANT QVariantToVARIANT( const QVariant &var, const char *type = 0 );
extern VARIANT QVariantToVARIANT( const QVariant &var, const QUParameter *param );
extern QVariant VARIANTToQVariant( const VARIANT &arg, const char *hint = 0 );
extern void QVariantToQUObject( const QVariant &var, QUObject &obj, const void *typeExtra = 0 );
extern void QUObjectToVARIANT( QUObject *obj, VARIANT &var, const QUParameter *param );

#endif //TYPES_H
