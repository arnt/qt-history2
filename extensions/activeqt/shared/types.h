/****************************************************************************
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <private/qcom_p.h>
//#include <private/qucomextra_p.h>

struct QUParameter;

extern GUID IID_IAxServerBase;
struct IAxServerBase : public IUnknown
{
    virtual IUnknown *clientSite() const = 0;
    virtual void emitPropertyChanged( const char*, long dispid = -1 ) = 0;
    virtual bool emitRequestPropertyChange( const char*, long dispid = -1 ) = 0;
    virtual QObject *qObject() const = 0;
};

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )


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
    return SysAllocStringLen( (OLECHAR*)str.unicode(), str.length() );
}

extern QDateTime DATEToQDateTime( DATE ole );
extern DATE QDateTimeToDATE( const QDateTime &dt );

struct IFont;
struct IFontDisp;

extern IFontDisp *QFontToIFont( const QFont &font );
extern QFont IFontToQFont( IFont *f );

extern uint QColorToOLEColor( const QColor &col );
extern QColor OLEColorToQColor( uint col );

extern bool QVariantToVARIANT( const QVariant &var, VARIANT &arg, const char *type );
extern bool QVariantToVARIANT( const QVariant &var, VARIANT &arg, const QUParameter *param );
extern QVariant VARIANTToQVariant( const VARIANT &arg, const char *type );

extern bool QVariantToQUObject( const QVariant &var, QUObject &obj, const QUParameter *param );

extern bool VARIANTToQUObject( const VARIANT &arg, QUObject *obj, const QUParameter *param );
extern bool QUObjectToVARIANT( QUObject *obj, VARIANT &var, const QUParameter *param );

extern void clearQUObject( QUObject *obj, const QUParameter *param );
extern void clearVARIANT( VARIANT *var );

#endif //TYPES_H
