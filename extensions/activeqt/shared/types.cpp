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

#include <atlbase.h>

#include <math.h>

#include "types.h"

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
    SYSTEMTIME stime;
    memset( &stime, 0, sizeof(stime) );
    QDate date = dt.date();
    QTime time = dt.time();
    stime.wDay = date.day();
    stime.wMonth = date.month();
    stime.wYear = date.year();
    stime.wMilliseconds = time.msec();
    stime.wSecond = time.second();
    stime.wMinute = time.minute();
    stime.wHour = time.hour();

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
	arg.vt = VT_UI4;
	arg.ulVal = QColorToOLEColor( var.toColor() );
	break;
    case QVariant::Font:
	arg.vt = VT_DISPATCH;
	arg.pdispVal = QFontToIFont( var.toFont() );
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
		CComPtr<IFont> ifont;
		disp->QueryInterface( IID_IFont, (void**)&ifont );
		if ( ifont ) {
		    var = IFontToQFont( ifont );
		}
	    }
	}
	//var = (int)arg.pdispVal; ###
	break;
    case VT_UNKNOWN:
	//var = (int)arg.punkVal; ###
	break;
    case VT_USERDEFINED:
	var = OLEColorToQColor( arg.ulVal );
	break;
    case VT_EMPTY:
	// empty VARIANT type return
	break;
    default:
	break;
    }
    return var;
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
		const QUEnum *uEnum = (const QUEnum *)typeExtra;
		for ( uint eItem = 0; eItem<uEnum->count; ++eItem ) {
		    if ( uEnum->items[eItem].key == var.toString() ) {
			static_QUType_enum.set( &obj, uEnum->items[eItem].value );
			break;
		    }
		}
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
    } else if ( QUType::isEqual( obj->type, &static_QUType_ptr ) ) {
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

static inline QString vartypeToQt( VARTYPE vt )
{
    QString str;
    switch ( vt ) {
    case VT_EMPTY:
	// str = "[Empty]";
	break;
    case VT_NULL:
	// str = "[Null]";
	break;
    case VT_I2:
    case VT_I4:
	str = "int";
	break;
    case VT_R4:
    case VT_R8:
	str = "double";
	break;
    case VT_CY:
	str = "long long"; // ### 64bit struct CY { ulong lo, long hi };
	break;
    case VT_DATE:
	str = "QDateTime";
	break;
    case VT_BSTR:
	str = "QString";
	break;
    case VT_DISPATCH:
	str = "IDispatch*";
	break;
    case VT_ERROR:
	str = "long";
	break;
    case VT_BOOL:
	str = "bool";
	break;
    case VT_VARIANT:
	str = "QVariant";
	break;
    case VT_DECIMAL:
	// str = "[DECIMAL]";
	break;
    case VT_RECORD:
	// str = "[Usertype]";
	break;
    case VT_UNKNOWN:
	str = "IUnknown*";
	break;
    case VT_I1:
	str = "char";
	break;
    case VT_UI1:
	str = "unsigned char";
	break;
    case VT_UI2:
	str = "unsigned short";
	break;
    case VT_UI4:
	str = "unsigned int";
	break;
    case VT_INT:
	str = "int";
	break;
    case VT_UINT:
	str = "unsigned int";
	break;
    case VT_VOID:
	str = "void";
	break;
    case VT_HRESULT:
	str = "long";
	break;

    case VT_PTR:
	// str = "[Pointer]";
	break;
    case VT_SAFEARRAY:
	// str = "VT_ARRAY";
	break;
    case VT_CARRAY:
	// str = "[C array]";
	break;
    case VT_USERDEFINED:
	str = "USERDEFINED";
	break;
    case VT_LPSTR:
	str = "const char*";
	break;
    case VT_LPWSTR:
	str = "const unsigned short*";
	break;

    case VT_FILETIME:
	// str = "[FILETIME]";
	break;
    case VT_BLOB:
	// str = "[Blob]";
	break;
    case VT_STREAM:
	// str = "[Stream]";
	break;
    case VT_STORAGE:
	// str = "[Storage]";
	break;
    case VT_STREAMED_OBJECT:
	// str = "[Streamed object]";
	break;
    case VT_STORED_OBJECT:
	// str = "[Stored object]";
	break;
    case VT_BLOB_OBJECT:
	// str = "[Blob object]";
	break;
    case VT_CF:
	// str = "[Clipboard]";
	break;
    case VT_CLSID:
	// str = "GUID";
	break;
    case VT_VECTOR:
	// str = "[Vector]";
	break;

    case VT_ARRAY:
	// str = "SAFEARRAY*";
	break;
    case VT_RESERVED:
	// str = "[Reserved]";
	break;

    default:
	// str = "[Unknown]";
	break;
    }

    if ( vt & VT_BYREF )
	str += "*";

    return str;
}

QString typedescToQString( TYPEDESC typedesc )
{
    QString ptype;

    VARTYPE vt = typedesc.vt;
    if ( vt == VT_PTR ) {
	vt = typedesc.lptdesc->vt;
	ptype = vartypeToQt( vt );
	if ( !!ptype ) 
	    ptype += "*";
    } else if ( vt == VT_SAFEARRAY ) {
	vt = typedesc.lpadesc->tdescElem.vt;
	ptype = vartypeToQt( vt );
	if ( !!ptype ) 
	    ptype = ptype + "[" + QString::number( typedesc.lpadesc->cDims ) + "]";
    } else {
	ptype = vartypeToQt( vt );
    }
    if ( ptype.isEmpty() )
	ptype = "UNSUPPORTED";

    return ptype;
}
