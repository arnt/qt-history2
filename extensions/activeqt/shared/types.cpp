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
    ipic->Render( pm.handle(), 0, 0, pm.width(), pm.height(), 0, 0, pWidth, pHeight, 0 );

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

VARIANT QVariantToVARIANT( const QVariant &var, const char *type )
{
    VARIANT arg;
    arg.vt = VT_EMPTY;

    switch ( var.type() ) {
    case QVariant::String:
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( var.toString() );
	break;
    case QVariant::Int:
	if( !qstrcmp( type, "bool" ) ) {
	    arg.vt = VT_BOOL;
	    arg.boolVal = var.toBool();
	    break;
	}
	arg.vt = VT_I4;
	arg.lVal = var.toInt();
	break;
    case QVariant::UInt:
	arg.vt = VT_UI4;
	arg.ulVal = var.toUInt();
	break;
    case QVariant::Bool:
	arg.vt = VT_BOOL;
	arg.boolVal = var.toBool();
	break;
    case QVariant::Double:
	arg.vt = VT_R8;
	arg.dblVal = var.toDouble();
	break;
    case QVariant::CString:
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( var.toCString() );
	break;
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
	arg.vt = VT_DATE;
	arg.date = QDateTimeToDATE( var.toDateTime() );
	break;
    case QVariant::Color:
	arg.vt = VT_COLOR;
	arg.ulVal = QColorToOLEColor( var.toColor() );
	break;
    case QVariant::Font:
	arg.vt = VT_DISPATCH;
	arg.pdispVal = QFontToIFont( var.toFont() );
	break;

    case QVariant::Pixmap:
	arg.vt = VT_DISPATCH;
	arg.pdispVal = QPixmapToIPicture( var.toPixmap() );
	break;

    default:
	break;
    }

    return arg;
}

void VARIANTToQUObject( VARIANT arg, QUObject *obj )
{
    QUType *preset = obj->type;
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

QVariant VARIANTToQVariant( const VARIANT &arg, const char *hint )
{
    QVariant var;
    switch( arg.vt ) {
    case VT_UI1:
	var = arg.bVal;
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
    case VT_R4:
	var = (double)arg.fltVal;
	break;
    case VT_R8:
	var = arg.dblVal;
	break;
    case VT_CY: // __int64 -> int ###
	var = (int)arg.cyVal.Hi;
	break;
    case VT_DATE: // DATE -> QDateTime
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
    case VT_UNKNOWN:
	//var = (int)arg.punkVal; ###
	break;
    case VT_USERDEFINED:
	break;
    case VT_EMPTY:
	// empty VARIANT type return
	break;
    default:
	break;
    }
    return var;
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

VARIANT QVariantToVARIANT( const QVariant &var, const QUParameter *param )
{
    QUObject obj;
    obj.type = &static_QUType_Null;

    QUType *preset = param->type;
    if ( !QUType::isEqual( &static_QUType_QVariant, preset ) ) {
	if ( !preset->canConvertFrom( &obj, &static_QUType_QVariant ) ) {
	    if ( param->typeExtra && ( var.type() == QVariant::String || var.type() == QVariant::CString )
		&& QUType::isEqual( preset, &static_QUType_enum ) ) {
		int value;
		if ( enumValue( var.toString(), (const QUEnum *)param->typeExtra, value ) )
		    static_QUType_enum.set( &obj, value );
	    }
	} else {
	    preset->convertFrom( &obj, &static_QUType_QVariant );
	}
    }

    if ( obj.type == &static_QUType_Null )
	static_QUType_QVariant.set( &obj, var );

    VARIANT res;
    QUObjectToVARIANT( &obj, res, param );
    if ( param->inOut & QUParameter::Out )
	res.vt |= VT_BYREF;

    obj.type->clear( &obj );

    return res;
}

void QVariantToQUObject( const QVariant &var, QUObject &obj, const void *typeExtra )
{
    QUType *preset = obj.type;
    switch ( var.type() ) {
    case QVariant::Invalid:
    case QVariant::Map:
    case QVariant::List:
    break;
    case QVariant::String:
	static_QUType_QString.set( &obj, var.toString() );
	break;
    case QVariant::StringList:
	break;
    case QVariant::Font:
	static_QUType_ptr.set( &obj, QFontToIFont( var.toFont() ) );
	break;
    case QVariant::Pixmap:
	static_QUType_ptr.set( &obj, QPixmapToIPicture( var.toPixmap() ) );
	break;

    case QVariant::Brush:
    case QVariant::Rect:
    case QVariant::Size:
	break;
    case QVariant::Color:
	static_QUType_int.set( &obj, QColorToOLEColor( var.toColor() ) );
	break;
    case QVariant::Palette:
    case QVariant::ColorGroup:
    case QVariant::IconSet:
    case QVariant::Point:
    case QVariant::Image:
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
    case QVariant::PointArray:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
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
    case QVariant::ByteArray:
    case QVariant::BitArray:
    case QVariant::KeySequence:
    default:
	return;
	break;
    }
    if ( !QUType::isEqual(preset, &static_QUType_Null ) && !QUType::isEqual( preset, obj.type ) ) {
	if ( !preset->canConvertFrom( &obj, obj.type ) ) {
	    if ( typeExtra && ( var.type() == QVariant::String || var.type() == QVariant::CString )
		&& QUType::isEqual( preset, &static_QUType_enum ) ) {
		int value;
		if ( enumValue( var.toString(), (const QUEnum *)typeExtra, value ) )
		    static_QUType_enum.set( &obj, value );
	    }
#ifndef QT_NO_DEBUG
	    else if ( QUType::isEqual( preset, &static_QUType_ptr ) )
		qWarning( "QVariant does not support pointer types" );
	    else
		qWarning( "Can't coerce QVariant to requested type (%s to %s)", obj.type->desc(), preset->desc() );
#endif
	} else {
	    preset->convertFrom( &obj, obj.type );
	}
    }
}

void QUObjectToVARIANT( QUObject *obj, VARIANT &arg, const QUParameter *param )
{
    // map the QUObject's type to the VARIANT
    if ( QUType::isEqual( obj->type, &static_QUType_int ) ) {
	arg.vt = VT_I4;
	arg.lVal = static_QUType_int.get( obj );
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
	arg = QVariantToVARIANT( static_QUType_QVariant.get( obj ) );
    } else if ( QUType::isEqual( obj->type, &static_QUType_idisp ) ) {
	arg.vt = VT_DISPATCH;
	arg.pdispVal = (IDispatch*)static_QUType_ptr.get( obj );
    } else if ( QUType::isEqual( obj->type, &static_QUType_iface ) ) {
	arg.vt = VT_UNKNOWN;
	arg.punkVal = (IUnknown*)static_QUType_ptr.get( obj );
    } else if ( QUType::isEqual( obj->type, &static_QUType_ptr ) && param ) {
	const char *type = (const char*)param->typeExtra;
	if ( !qstrcmp( type, "int" ) ) {
	    arg.vt = VT_I4;
	    arg.lVal = *(int*)static_QUType_ptr.get( obj );
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
