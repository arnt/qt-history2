/****************************************************************************
** $Id: $
**
** Implementation of type conversion routines
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
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

#include <olectl.h>

#include "types.h"
#include <qpixmap.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>

IFontDisp *QFontToIFont( const QFont &font )
{
    FONTDESC fdesc;
    memset( &fdesc, 0, sizeof(fdesc) );
    fdesc.cbSizeofstruct = sizeof(FONTDESC);
    fdesc.cySize.Lo = font.pointSize() * 10000;
    fdesc.fItalic = font.italic();
    fdesc.fStrikethrough = font.strikeOut();
    fdesc.fUnderline = font.underline();
    fdesc.lpstrName = QStringToBSTR( font.family() );
    fdesc.sWeight = font.weight();
    
    IFontDisp *f;
    OleCreateFontIndirect( &fdesc, IID_IFontDisp, (void**)&f );
    return f;
}

QFont IFontToQFont( IFont *f )
{
    BSTR name;
    BOOL bold;
    SHORT charset;
    BOOL italic;
    CY size;
    BOOL strike;
    BOOL underline;
    SHORT weight;
    f->get_Name( &name );
    f->get_Bold( &bold );
    f->get_Charset( &charset );
    f->get_Italic( &italic );
    f->get_Size( &size );
    f->get_Strikethrough( &strike );
    f->get_Underline( &underline );
    f->get_Weight( &weight );
    QFont font( BSTRToQString(name), size.Lo/10000, weight, italic );
    font.setBold( bold );
    font.setStrikeOut( strike );
    font.setUnderline( underline );
    SysFreeString(name);

    return font;
}

IPictureDisp *QPixmapToIPicture( const QPixmap &pixmap )
{
    if ( pixmap.isNull() )
	return 0;

    HDC hdc = ::CreateCompatibleDC( pixmap.handle() );
    if ( !hdc ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "QPixmapToIPicture: Failed to create compatible device context" );
#endif
	return 0;
    }
    HBITMAP hbm = ::CreateCompatibleBitmap( pixmap.handle(), pixmap.width(), pixmap.height() );
    if ( !hbm ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "QPixmapToIPicture: Failed to create compatible bitmap" );
#endif
	return 0;
    }
    ::SelectObject( hdc, hbm );
    BOOL res = ::BitBlt( hdc, 0, 0, pixmap.width(), pixmap.height(), pixmap.handle(), 0, 0, SRCCOPY );

    ::DeleteObject( hdc );

    PICTDESC desc;
    desc.cbSizeofstruct = sizeof(PICTDESC);
    desc.picType = PICTYPE_BITMAP;

    desc.bmp.hbitmap = hbm;
    desc.bmp.hpal = QColor::hPal();

    IPictureDisp *pic = 0;
    HRESULT hres = OleCreatePictureIndirect( &desc, IID_IPictureDisp, TRUE, (void**)&pic );
    return pic;
}

QPixmap IPictureToQPixmap( IPicture *ipic )
{
    SHORT type;
    ipic->get_Type( &type );
    if ( type != PICTYPE_BITMAP )
	return QPixmap();

    QPixmap pm( 1,1 );
    QPaintDeviceMetrics pdm( &pm );
    OLE_XSIZE_HIMETRIC pWidth, pHeight;
    ipic->get_Width( &pWidth );
    ipic->get_Height( &pHeight );
    QSize sz( MAP_LOGHIM_TO_PIX( pWidth, pdm.logicalDpiX() ),
	      MAP_LOGHIM_TO_PIX( pHeight, pdm.logicalDpiY() ) );

    pm.resize( sz );
    ipic->Render( pm.handle(), 0, 0, pm.width(), pm.height(), 0, pHeight, pWidth, -pHeight, 0 );

    return pm;
}

QDateTime DATEToQDateTime( DATE ole )
{
    SYSTEMTIME stime;
    if ( ole >= 949998 || VariantTimeToSystemTime( ole, &stime ) == FALSE )
	return QDateTime();

    QDate date( stime.wYear, stime.wMonth, stime.wDay );
    QTime time( stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds );
    return QDateTime( date, time );
}

DATE QDateTimeToDATE( const QDateTime &dt )
{
    if ( !dt.isValid() || dt.isNull() )
	return 949998;

    SYSTEMTIME stime;
    memset( &stime, 0, sizeof(stime) );
    QDate date = dt.date();
    QTime time = dt.time();
    if ( date.isValid() && !date.isNull() ) {
	stime.wDay = date.day();
	stime.wMonth = date.month();
	stime.wYear = date.year();
    }
    if ( time.isValid() && !time.isNull() ) {
	stime.wMilliseconds = time.msec();
	stime.wSecond = time.second();
	stime.wMinute = time.minute();
	stime.wHour = time.hour();
    }

    double vtime;
    SystemTimeToVariantTime( &stime, &vtime );

    return vtime;
}

/*
    Converts \a var to \a arg, and tries to coerce \a arg to \a type.

    Used by 
    QAxServerBase:
    - IDispatch::Invoke( PROPERTYGET )
    - IPersistPropertyBag::Save

    QAxBase
    - QAxBase::qt_property( PropertySet )
    - QAxBase::internalInvoke( properties )
    - IPropertyBag::Read.
*/
void QVariantToVARIANT( const QVariant &var, VARIANT &arg, const char *type )
{
    arg.vt = VT_EMPTY;

    QVariant qvar = var;
    // "type" is the expected type, so coerce if necessary
    QVariant::Type proptype = type ? QVariant::nameToType( type ) : QVariant::Invalid;
    if ( proptype != QVariant::Invalid && proptype != qvar.type() ) {
	if ( qvar.canCast( proptype ) )
	    qvar.cast( proptype );
    }

    switch ( qvar.type() ) {
    case QVariant::String:
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( qvar.toString() );
	break;
    case QVariant::Int:
	arg.vt = VT_I4;
	arg.lVal = qvar.toInt();
	break;
    case QVariant::UInt:
	arg.vt = VT_UI4;
	arg.ulVal = qvar.toUInt();
	break;
    case QVariant::Bool:
	arg.vt = VT_BOOL;
	arg.boolVal = qvar.toBool();
	break;
    case QVariant::Double:
	arg.vt = VT_R8;
	arg.dblVal = qvar.toDouble();
	break;
    case QVariant::CString:
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( qvar.toCString() );
	break;
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
	arg.vt = VT_DATE;
	arg.date = QDateTimeToDATE( qvar.toDateTime() );
	break;
    case QVariant::Color:
	arg.vt = VT_COLOR;
	arg.ulVal = QColorToOLEColor( qvar.toColor() );
	break;
    case QVariant::Font:
	arg.vt = VT_DISPATCH;
	arg.pdispVal = QFontToIFont( qvar.toFont() );
	break;

    case QVariant::Pixmap:
	arg.vt = VT_DISPATCH;
	arg.pdispVal = QPixmapToIPicture( qvar.toPixmap() );
	break;

    case QVariant::List:
	{
	    arg.vt = VT_ARRAY|VT_VARIANT;
	    const QValueList<QVariant> list = qvar.toList();
	    const int count = list.count();

	    arg.parray = SafeArrayCreateVector( VT_VARIANT, 0, list.count() );
	    QValueList<QVariant>::ConstIterator it = list.begin();
	    LONG index = 0;
	    while ( it != list.end() ) {
		QVariant varelem = *it;
		++it;
		VARIANT var;
		QVariantToVARIANT( varelem, var, varelem.typeName() );
		SafeArrayPutElement( arg.parray, &index, &var );
		++index;
	    }
	}
	break;

    default:
	break;
    }
}

static inline bool enumValue( const QString &string, const QUEnum *uEnum, int &value )
{
    bool isInt = FALSE;
    value = string.toInt( &isInt );
    if ( isInt )
	return TRUE;
    else for ( uint eItem = 0; eItem<uEnum->count; ++eItem ) {
	if ( uEnum->items[eItem].key == string ) {
	    value = uEnum->items[eItem].value;
	    return TRUE;
	}
    }
    return FALSE;
}

/*
    Converts \a var to \a res, and tries to coerce \a res to the type of \a param.

    Used by:
    QAxBase
    - QAxBase::internalInvoke( for slots )
*/
void QVariantToVARIANT( const QVariant &var, VARIANT &res, const QUParameter *param )
{
    QUObject obj;
    obj.type = &static_QUType_Null;

    if ( !QUType::isEqual( &static_QUType_QVariant, param->type ) ) {
	if ( param->type->canConvertFrom( &obj, &static_QUType_QVariant ) ) {
	    param->type->convertFrom( &obj, &static_QUType_QVariant );
	} else if ( ( var.type() == QVariant::String || var.type() == QVariant::CString ) 
		    && QUType::isEqual( param->type, &static_QUType_enum ) ) {
	    int value;
	    if ( enumValue( var.toString(), (const QUEnum *)param->typeExtra, value ) )
		static_QUType_enum.set( &obj, value );
	}
    }

    if ( obj.type == &static_QUType_Null )
	static_QUType_QVariant.set( &obj, var );

    QUObjectToVARIANT( &obj, res, param );
    obj.type->clear( &obj );
}

/*!
    Converts \a arg to \a obj, and tries to coerce \a obj to the type of \a param.

    Used by
    - QAxServerBase::Invoke( methods )
    - QAxServerBase::qt_emit

    - QAxBase::qt_invoke( return value and out-parameters )
    - QAxEventSink::Invoke
*/
void VARIANTToQUObject( const VARIANT &arg, QUObject *obj, const QUParameter *param )
{
    if ( arg.vt & VT_BYREF ) {
	VARTYPE vt2 = arg.vt & ~VT_BYREF;
	switch ( vt2 ) {
	case VT_UI1:
	    static_QUType_ptr.set( obj, arg.pbVal );
	    break;
	case VT_I2:
	    static_QUType_ptr.set( obj, arg.piVal );
	    break;
	case VT_I4:
	    static_QUType_ptr.set( obj, arg.plVal );
	    break;
	case VT_ERROR:
	    static_QUType_ptr.set( obj, arg.pscode );
	    break;
	case VT_R4:
	    static_QUType_ptr.set( obj, arg.pfltVal );
	    break;
	case VT_R8:
	    static_QUType_ptr.set( obj, arg.pdblVal );
	    break;
	case VT_DATE:
	    static_QUType_ptr.set( obj, new QDateTime( DATEToQDateTime( *arg.pdate ) ) );
	    break;
	case VT_BOOL:
	    static_QUType_ptr.set( obj, arg.pboolVal );
	    break;
	case VT_BSTR:
	    static_QUType_ptr.set( obj, BSTRToQString( *arg.pbstrVal ) );
	    break;
	case VT_UNKNOWN:
	    static_QUType_ptr.set( obj, *arg.ppunkVal );
	    break;
	case VT_DISPATCH:
	    static_QUType_ptr.set( obj, *arg.ppdispVal );
	    break;
	default:
	    break;
	}
    } else switch ( arg.vt ) {
    case VT_UI1: // byte -> int
	static_QUType_int.set( obj, arg.bVal );
	break;
    case VT_I2: // short -> int
	static_QUType_int.set( obj, arg.iVal );
	break;
    case VT_I4: // long -> int
	static_QUType_int.set( obj, arg.lVal );
	break;
    case VT_ERROR: // SCODE == long
	static_QUType_int.set( obj, arg.scode );
	break;
    case VT_R4: // float -> double
	static_QUType_double.set( obj, arg.fltVal );
	break;
    case VT_R8: // double -> double
	static_QUType_double.set( obj, arg.dblVal );
	break;
    case VT_CY: // Currency -> ###
	break;
    case VT_DATE: // DATE -> QDateTime
	static_QUType_ptr.set( obj, new QDateTime( DATEToQDateTime( arg.date ) ) );
	break;
    case VT_BOOL: // bool -> bool
	static_QUType_bool.set( obj, arg.boolVal );
	break;
    case VT_BSTR: // bstr -> QString
	static_QUType_QString.set( obj, BSTRToQString( arg.bstrVal ) );
	break;
    case VT_UNKNOWN:  // IUnknown -> void*
	static_QUType_ptr.set( obj, arg.punkVal );
	break;
    case VT_DISPATCH: // IDispatch -> void*
	static_QUType_ptr.set( obj, arg.pdispVal );
	break;
    default:
	break;
    }

    QUType *preset = obj->type;
    if ( !QUType::isEqual(preset, &static_QUType_Null ) && !QUType::isEqual( preset, obj->type ) ) {
#ifndef QT_NO_DEBUG
	if ( !preset->canConvertFrom( obj, obj->type ) ) {
	    qWarning( "Can't coerce VARIANT type to requested type (%s to %s)", obj->type->desc(), preset->desc() );
	} else 
#endif
	{
	    preset->convertFrom( obj, obj->type );
	}
    }
}

/*!
    Returns \a arg as a QVariant of type \a hint.

    Used by:
    QAxBase
    - QAxBase::qt_property( PropertyGet )
    - QAxBase::internalInvoke( update out parameters )
    - QAxBase::dynamicCall( return value )
    - IPropertyBag::Write

    QAxServerBase::
    - IDispatch::Invoke( PropertyPut )
    - IPersistPropertyBag::Load
*/
QVariant VARIANTToQVariant( const VARIANT &arg, const char *hint )
{
    QVariant var;
    switch( arg.vt ) {
    case VT_UI1:
	var = arg.bVal;
	break;
    case VT_UI1 | VT_BYREF:
	var = *arg.pbVal;
	break;
    case VT_I2:
	var = arg.iVal;
	break;
    case VT_ERROR:
    case VT_I4:
	if ( !qstrcmp( hint, "QColor" ) ) {
	    var = OLEColorToQColor( arg.ulVal );
	    break;
	}
	var = (int)arg.lVal;
	break;
    case VT_I4 | VT_BYREF:
	var = (int)*arg.plVal;
	break;
    case VT_R4:
	var = (double)arg.fltVal;
	break;
    case VT_R8:
	var = arg.dblVal;
	break;
    case VT_DATE:
	var = DATEToQDateTime( arg.date );
	break;
    case VT_BOOL:
	var = QVariant( arg.boolVal, 42 );
	break;
    case VT_BSTR:
	var = BSTRToQString( arg.bstrVal );
	break;
    case VT_I1:
	var = arg.cVal;
	break;
    case VT_UI2:
	var = arg.uiVal;
	break;
    case VT_UI4:
	if ( !qstrcmp( hint, "QColor" ) ) {
	    var = OLEColorToQColor( arg.ulVal );
	    break;
	}
	var = (int)arg.ulVal;
	break;
    case VT_INT:
	var = arg.intVal;
	break;
    case VT_UINT:
	var = arg.uintVal;
	break;
    case VT_DISPATCH:
	{
	    IDispatch *disp = arg.pdispVal;
	    if ( disp ) {
		IFont *ifont = 0;
		disp->QueryInterface( IID_IFont, (void**)&ifont );
		if ( ifont ) {
		    var = IFontToQFont( ifont );
		    ifont->Release();
		    break;
		}

		IPicture *ipic = 0;
		disp->QueryInterface( IID_IPicture, (void**)&ipic );
		if ( ipic ) {
		    var = IPictureToQPixmap( ipic );
		    ipic->Release();
		}
	    } else if ( hint ) {
		if ( !qstrcmp( hint, "QPixmap" ) )
		    var = QPixmap();
		else if ( !qstrcmp( hint, "QFont" ) )
		    var = QFont();
	    }
	}
	break;
    case VT_USERDEFINED:
	break;
    case VT_ARRAY|VT_VARIANT:
	{
	    SAFEARRAY *array = arg.parray;
	    QValueList<QVariant> list;

	    if ( !array || array->cDims != 1 ) {
		var = list;
		break;
	    }

	    long lBound, uBound;
	    SafeArrayGetLBound( array, 1, &lBound );
	    SafeArrayGetUBound( array, 1, &uBound );

	    for ( long i = lBound; i <= uBound; ++i ) {
		VARIANT var;
		SafeArrayGetElement( array, &i, &var );

		QVariant qvar = VARIANTToQVariant( var, 0 );
		list << qvar;
	    }

	    var = list;
	}
	break;
    default:
	break;
    }

    QVariant::Type proptype = hint ? QVariant::nameToType( hint ) : QVariant::Invalid;
    if ( proptype != QVariant::Invalid && var.type() != proptype ) {
	if ( var.canCast( proptype ) )
	    var.cast( proptype );
    }
    return var;
}

/*!
    Converts \a var to \a obj, and tries to coerce \a obj to the type of \a param.

    Used by
    QAxBase:
    - QAxEventSink::OnChanged
*/
void QVariantToQUObject( const QVariant &var, QUObject &obj, const QUParameter *param )
{
    if ( QUType::isEqual( param->type, &static_QUType_QVariant ) ) {
	static_QUType_QVariant.set( &obj, var );
    } else switch ( var.type() ) {
    case QVariant::String:
	static_QUType_QString.set( &obj, var.toString() );
	break;
    case QVariant::Font:
	static_QUType_ptr.set( &obj, QFontToIFont( var.toFont() ) );
	break;
    case QVariant::Pixmap:
	static_QUType_ptr.set( &obj, QPixmapToIPicture( var.toPixmap() ) );
	break;
    case QVariant::Color:
	static_QUType_int.set( &obj, QColorToOLEColor( var.toColor() ) );
	break;
    case QVariant::Int:
	static_QUType_int.set( &obj, var.toInt() );
	break;
    case QVariant::UInt:
	static_QUType_int.set( &obj, var.toUInt() );
	break;
    case QVariant::Bool:
	static_QUType_bool.set( &obj, var.toDouble() );
	break;
    case QVariant::Double:
	static_QUType_double.set( &obj, var.toDouble() );
	break;
    case QVariant::CString:
	static_QUType_charstar.set( &obj, var.toCString() );
	break;
    case QVariant::Date:
	static_QUType_ptr.set( &obj, new QDateTime( var.toDate() ) );
	break;
    case QVariant::Time:
	static_QUType_ptr.set( &obj, new QDateTime( QDate(), var.toTime() ) );
	break;
    case QVariant::DateTime:
	static_QUType_ptr.set( &obj, new QDateTime( var.toDateTime() ) );
	break;
    case QVariant::List:
	static_QUType_ptr.set( &obj, new QValueList<QVariant>( var.toList() ) );
	break;
    default:
	return;
    }

    if ( !QUType::isEqual( param->type, obj.type ) ) {
	if ( param->type->canConvertFrom( &obj, obj.type ) ) {
	    param->type->convertFrom( &obj, obj.type );
	} else if ( ( var.type() == QVariant::String || var.type() == QVariant::CString )
		    && QUType::isEqual( param->type, &static_QUType_enum ) ) {
	    int value;
	    if ( enumValue( var.toString(), (const QUEnum *)param->typeExtra, value ) )
		static_QUType_enum.set( &obj, value );
	}
#ifndef QT_NO_DEBUG
	else {
	    const char *type = param->type->desc();
	    if ( QUType::isEqual( param->type, &static_QUType_ptr ) )
		type = (const char*)param->typeExtra;
	    qWarning( "Can't coerce QVariant to requested type (%s to %s)", var.typeName(), type );
	}
#endif
    }
}

/*!
    Converts \a obj to \a arg, and tries to coerce \a var to the type of \a param.

    Used by
    QVariantToVariant

    QAxServerBase:
    - qt_emit
    - IDispatch::Invoke( update references in method )

    QAxBase:
    - QAxBase::qt_invoke
*/
void QUObjectToVARIANT( QUObject *obj, VARIANT &arg, const QUParameter *param )
{
    bool byref = param && ( param->inOut & QUParameter::Out );
    if ( param && param->type->canConvertFrom( obj, obj->type ) )
	param->type->convertFrom( obj, obj->type );

    // map the QUObject's type to the VARIANT
    if ( QUType::isEqual( obj->type, &static_QUType_int ) ) {
	if ( byref ) {
	    arg.vt = VT_I4|VT_BYREF;
	    arg.plVal = new long(static_QUType_int.get( obj ));
	} else {
	    arg.vt = VT_I4;
	    arg.lVal = static_QUType_int.get( obj );
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_QString ) ) {
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( static_QUType_QString.get( obj ) );
    } else if ( QUType::isEqual( obj->type, &static_QUType_charstar ) ) {
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( static_QUType_charstar.get( obj ) );
    } else if ( QUType::isEqual( obj->type, &static_QUType_bool ) ) {
	arg.vt = VT_BOOL;
	arg.boolVal = static_QUType_bool.get( obj );
    } else if ( QUType::isEqual( obj->type, &static_QUType_double ) ) {
	arg.vt = VT_R8;
	arg.dblVal = static_QUType_double.get( obj );
    } else if ( QUType::isEqual( obj->type, &static_QUType_enum ) ) {
	arg.vt = VT_I4;
	arg.lVal = static_QUType_enum.get( obj );
    } else if ( QUType::isEqual( obj->type, &static_QUType_QVariant ) ) {
	QVariant value = static_QUType_QVariant.get( obj );
	if ( byref && QUType::isEqual( param->type, &static_QUType_ptr ) ) {
	    const char *type = (const char*)param->typeExtra;
	    if ( !qstrcmp( type, "int" ) ) {
		arg.vt = VT_I4 | VT_BYREF;
		arg.plVal = new long(value.toInt());
	    } else if ( !qstrcmp( type, "QString" ) || !qstrcmp( type, "const QString&" ) ) {
		arg.vt = VT_BSTR | VT_BYREF;
		arg.pbstrVal = new BSTR(QStringToBSTR( value.toString() ) );
	    }
	} else {
	    const char *type = param->type->desc();
	    if ( QUType::isEqual( param->type, &static_QUType_ptr ) )
		type = (const char*)param->typeExtra;
	    QVariantToVARIANT( value, arg, type );
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_ptr ) && param ) {
	const char *type = (const char*)param->typeExtra;
	if ( !qstrcmp( type, "int" ) ) {
	    if ( byref ) {
		arg.vt = VT_I4 | VT_BYREF;
		*arg.plVal = *(int*)static_QUType_ptr.get( obj );
	    } else {
		arg.vt = VT_I4;
		arg.lVal = *(int*)static_QUType_ptr.get( obj );
	    }
	} else if ( !qstrcmp( type, "QString" ) || !qstrcmp( type, "const QString&" ) ) {
	    arg.vt = VT_BSTR;
	    arg.bstrVal = QStringToBSTR( *(QString*)static_QUType_ptr.get( obj ) );
	} else if ( !qstrcmp( type, "QDateTime" ) || !qstrcmp( type, "const QDateTime&" ) ) {
	    arg.vt = VT_DATE;
	    arg.date = QDateTimeToDATE( *(QDateTime*)static_QUType_ptr.get( obj ) );
	} else {
	    arg.vt = VT_UI4;
	    arg.ulVal = (Q_ULONG)static_QUType_ptr.get( obj );
	}
	//###
    } else {
	arg.vt = VT_EMPTY;
    }
}

