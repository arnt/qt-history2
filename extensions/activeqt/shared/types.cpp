/****************************************************************************
**
** Implementation of type conversion routines.
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define QT_NO_CAST_TO_ASCII

#include <ocidl.h>
#include <olectl.h>

#include "types.h"
#include <qpixmap.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qobject.h>
#ifdef QAX_SERVER
#include <qaxfactory.h>
#include <qlibrary.h>
extern ITypeLib *qAxTypeLibrary;

CLSID CLSID_QRect = { 0x34030f30, 0xe359, 0x4fe6, {0xab, 0x82, 0x39, 0x76, 0x6f, 0x5d, 0x91, 0xee } };
CLSID CLSID_QSize = { 0xcb5f84b3, 0x29e5, 0x491d, {0xba, 0x18, 0x54, 0x72, 0x48, 0x8e, 0xef, 0xba } };
CLSID CLSID_QPoint = { 0x3be838a3, 0x3fac, 0xbfc4, {0x4c, 0x6c, 0x37, 0xc4, 0x4d, 0x03, 0x02, 0x52 } };

GUID IID_IAxServerBase = { 0xbd2ec165, 0xdfc9, 0x4319, { 0x8b, 0x9b, 0x60, 0xa5, 0x74, 0x78, 0xe9, 0xe3} };
#endif

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
    fdesc.sWeight = font.weight() * 10;

    IFontDisp *f;
    HRESULT res = OleCreateFontIndirect( &fdesc, IID_IFontDisp, (void**)&f );
    if ( res != S_OK ) {
	if ( f ) f->Release();
	f = 0;
#if defined(QT_CHECK_STATE)
	qWarning( "QFontToIFont: Failed to create IFont" );
#endif
    }
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
    QFont font( BSTRToQString(name), size.Lo/9750, weight / 97, italic );
    font.setBold( bold );
    font.setStrikeOut( strike );
    font.setUnderline( underline );
    SysFreeString(name);

    return font;
}

IPictureDisp *QPixmapToIPicture( const QPixmap &pixmap )
{
    IPictureDisp *pic = 0;

    PICTDESC desc;
    desc.cbSizeofstruct = sizeof(PICTDESC);
    desc.picType = PICTYPE_BITMAP;

    desc.bmp.hbitmap = 0;
    desc.bmp.hpal = QColor::hPal();

    if ( !pixmap.isNull() ) {
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

	if ( !res ) {
#if defined(QT_CHECK_STATE)
	    qSystemWarning( "QPixmapToIPicture: Failed to BitBlt bitmap" );
#endif
	    return 0;
	}

	desc.bmp.hbitmap = hbm;
    }

    HRESULT res = OleCreatePictureIndirect( &desc, IID_IPictureDisp, TRUE, (void**)&pic );
    if ( res != S_OK ) {
	if ( pic ) pic->Release();
	pic = 0;
#if defined(QT_CHECK_STATE)
	qWarning( "QPixmapToIPicture: Failed to create IPicture" );
#endif
    }
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

uint QColorToOLEColor( const QColor &col )
{
    return qRgba( col.blue(), col.green(), col.red(), 0x00 );
}

QColor OLEColorToQColor( uint col )
{
    COLORREF cref;
    OleTranslateColor( col, QColor::hPal(), &cref );
    return QColor( GetRValue(cref),GetGValue(cref),GetBValue(cref) );
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
bool QVariantToVARIANT( const QVariant &var, VARIANT &arg, const QString &type )
{
    arg.vt = VT_EMPTY;

    QVariant qvar = var;
    // "type" is the expected type, so coerce if necessary
    QVariant::Type proptype = type ? QVariant::nameToType(type.latin1()) : QVariant::Invalid;
    if ( proptype != QVariant::Invalid && proptype != qvar.type() ) {
	if ( qvar.canCast( proptype ) )
	    qvar.cast( proptype );
    }

    switch ((int)qvar.type()) {
    case QVariant::String:
	arg.vt = VT_BSTR;
	arg.bstrVal = QStringToBSTR( qvar.toString() );
	break;
    case QVariant::Int:
	arg.vt = VT_I4;
	arg.lVal = qvar.toInt();
	break;
    case QVariant::UInt:
	arg.vt = VT_UINT;
	arg.uintVal = qvar.toUInt();
	break;
    case QVariant::Bool:
	arg.vt = VT_BOOL;
	arg.boolVal = qvar.toBool() ? -1 : 0;
	break;
    case QVariant::Double:
	arg.vt = VT_R8;
	arg.dblVal = qvar.toDouble();
	break;
    case QVariant::Color:
	arg.vt = VT_COLOR;
	arg.lVal = QColorToOLEColor( qvar.toColor() );
	break;

    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
	arg.vt = VT_DATE;
	arg.date = QDateTimeToDATE( qvar.toDateTime() );
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
	    const QList<QCoreVariant> list = qvar.toList();
	    const int count = list.count();

	    arg.parray = SafeArrayCreateVector( VT_VARIANT, 0, count );
	    QList<QCoreVariant>::ConstIterator it = list.begin();
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

    case QVariant::StringList:
	{
	    arg.vt = VT_ARRAY|VT_BSTR;
	    const QStringList strings = qvar.toStringList();
	    const int count = strings.count();

	    arg.parray = SafeArrayCreateVector( VT_BSTR, 0, count );
	    QStringList::ConstIterator it = strings.begin();
	    LONG index = 0;
	    while ( it != strings.end() ) {
		QString string = *it;
		++it;
		BSTR bstr = QStringToBSTR(string);
		SafeArrayPutElement( arg.parray, &index, bstr );
		SysFreeString(bstr);
		++index;
	    }
	}
	break;

    case QVariant::ByteArray:
	{
	    arg.vt = VT_ARRAY|VT_UI1;
	    const QByteArray array = qvar.toByteArray();
	    uint size = array.size();
	    arg.parray = SafeArrayCreateVector( VT_UI1, 0, size );

	    if ( size ) {
		const char *data = array.data();
		char *dest;
		SafeArrayAccessData( arg.parray, (void **)&dest );
		memcpy( dest, data, size );
		SafeArrayUnaccessData( arg.parray );
	    }
	}
	break;

    case QVariant::LongLong:
	arg.vt = VT_CY;
	arg.cyVal.int64 = qvar.toLongLong();
	break;
    case QVariant::ULongLong:
	arg.vt = VT_CY;
	arg.cyVal.int64 = qvar.toULongLong();
	break;

#ifdef QAX_SERVER
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::Point:
	{
	    typedef HRESULT(WINAPI* PGetRecordInfoFromTypeInfo)(ITypeInfo *, IRecordInfo **);
	    static PGetRecordInfoFromTypeInfo pGetRecordInfoFromTypeInfo = 0;
	    static bool resolved = FALSE;
	    if (!resolved) {
		resolved = TRUE;
		pGetRecordInfoFromTypeInfo = (PGetRecordInfoFromTypeInfo)QLibrary::resolve("oleaut32", "GetRecordInfoFromTypeInfo");
	    }
	    if (!pGetRecordInfoFromTypeInfo)
		break;

	    ITypeInfo *typeInfo = 0;
	    IRecordInfo *recordInfo = 0;
	    CLSID clsid = qvar.type() == QVariant::Rect ? CLSID_QRect
		:qvar.type() == QVariant::Size ? CLSID_QSize
		:CLSID_QPoint;
	    qAxTypeLibrary->GetTypeInfoOfGuid(clsid, &typeInfo);
	    if (!typeInfo)
		break;
	    pGetRecordInfoFromTypeInfo(typeInfo, &recordInfo);
	    typeInfo->Release();
	    if (!recordInfo)
		break;

	    void *record = 0;
	    switch (qvar.type()) {
	    case QVariant::Rect:
		{
		    QRect qrect(qvar.toRect());
		    recordInfo->RecordCreateCopy(&qrect, &record);
		}
		break;
	    case QVariant::Size:
		{
		    QSize qsize(qvar.toSize());
		    recordInfo->RecordCreateCopy(&qsize, &record);
		}
		break;
	    case QVariant::Point:
		{
		    QPoint qpoint(qvar.toPoint());
		    recordInfo->RecordCreateCopy(&qpoint, &record);
		}
		break;
	    }

	    arg.vt = VT_RECORD;
	    arg.pRecInfo = recordInfo,
	    arg.pvRecord = record;
	}
	break;
#endif // QAX_SERVER

    case 1000: // rawAccess in QAxBase::toVariant
	if (type == "IDispatch*") {
	    arg.vt = VT_DISPATCH;
	    arg.pdispVal = (IDispatch*)qvar.rawAccess();
	    if ( arg.pdispVal )
		arg.pdispVal->AddRef();
	} else {
	    arg.vt = VT_UNKNOWN;
	    arg.punkVal = (IUnknown*)qvar.rawAccess();
	    if ( arg.punkVal )
		arg.punkVal->AddRef();
	}
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

/*!
    Copies the data in \a var into \a data.

    Used by:
    QAxBase:
    - internalProperty(ReadProperty)
*/
bool QVariantToVoidStar(const QVariant &var, void *data)
{
    switch (var.type()) {
    case QVariant::String:
	*(QString*)data = var.toString();
	break;
    case QVariant::Int:
	*(int*)data = var.toInt();
	break;
    case QVariant::UInt:
	*(uint*)data = var.toUInt();
	break;
    case QVariant::Bool:
	*(bool*)data = var.toBool();
	break;
    case QVariant::Double:
	*(double*)data = var.toDouble();
	break;
    case QVariant::Color:
	*(QColor*)data = var.toColor();
	break;
    case QVariant::Date:
	*(QDate*)data = var.toDate();
	break;
    case QVariant::Time:
	*(QTime*)data = var.toTime();
	break;
    case QVariant::DateTime:
	*(QDateTime*)data = var.toDateTime();
	break;
    case QVariant::Font:
	*(QFont*)data = var.toFont();
	break;
    case QVariant::Pixmap:
	*(QPixmap*)data = var.toPixmap();
	break;
    case QVariant::List:
	*(QList<QCoreVariant>*)data = var.toList();
	break;
    case QVariant::StringList:
	*(QStringList*)data = var.toStringList();
	break;
    case QVariant::ByteArray:
	*(QByteArray*)data = var.toByteArray();
	break;
    case QVariant::LongLong:
	*(Q_LLONG*)data = var.toLongLong();
	break;
    case QVariant::ULongLong:
	*(Q_ULLONG*)data = var.toULongLong();
	break;
    case QVariant::Rect:
	*(QRect*)data = var.toRect();
	break;
    case QVariant::Size:
	*(QSize*)data = var.toSize();
	break;
    case QVariant::Point:
	*(QPoint*)data = var.toPoint();
	break;
/*
    case 1000: // rawAccess in QAxBase::toVariant
	if (type == "IDispatch*") {
	    arg.vt = VT_DISPATCH;
	    arg.pdispVal = (IDispatch*)qvar.rawAccess();
	    if ( arg.pdispVal )
		arg.pdispVal->AddRef();
	} else {
	    arg.vt = VT_UNKNOWN;
	    arg.punkVal = (IUnknown*)qvar.rawAccess();
	    if ( arg.punkVal )
		arg.punkVal->AddRef();
	}
	break;
*/
    default:
	return false;
    }

    return true;
}

/*!
    God knows why VariantChangeType can't do that...
    Probably because VariantClear does not delete the stuff?
*/
static inline void makeReference( VARIANT &arg )
{
    switch( arg.vt ) {
    case VT_BSTR:
	arg.pbstrVal = new BSTR(arg.bstrVal);
	break;
    case VT_BOOL:
	arg.pboolVal = new short(arg.boolVal);
	break;
    case VT_I1:
	arg.pcVal = new char(arg.cVal);
	break;
    case VT_I2:
	arg.piVal = new short(arg.iVal);
	break;
    case VT_I4:
	arg.plVal = new long(arg.lVal);
	break;
    case VT_INT:
	arg.pintVal = new int(arg.intVal);
	break;
    case VT_UI1:
	arg.pbVal = new uchar(arg.bVal);
	break;
    case VT_UI2:
	arg.puiVal = new ushort(arg.uiVal);
	break;
    case VT_UI4:
	arg.pulVal = new ulong(arg.ulVal);
	break;
    case VT_UINT:
	arg.puintVal = new uint(arg.uintVal);
	break;
    case VT_CY:
	arg.pcyVal = new CY(arg.cyVal);
	break;
    case VT_R4:
	arg.pfltVal= new float(arg.dblVal);
	break;
    case VT_R8:
	arg.pdblVal = new double(arg.dblVal);
	break;
    case VT_DATE:
	arg.pdate = new DATE(arg.date);
	break;
    case VT_DISPATCH:
	arg.ppdispVal = new IDispatch*(arg.pdispVal);
	break;
    case VT_ARRAY|VT_VARIANT:
    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_BSTR:
	arg.pparray = new SAFEARRAY*(arg.parray);
	break;
    }
    arg.vt |= VT_BYREF;
}
/*
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
*/

/*
    Converts \a var to \a res, and tries to coerce \a res to the type of \a param.

    Used by:
    QAxBase
    - QAxBase::internalInvoke( for slots )
*
bool QVariantToVARIANT( const QVariant &var, VARIANT &res, const QUParameter *param )
{
    QVariant variant = var;
    const char *vartypename = 0;

    if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
	vartypename = QVariant::typeToName( (QVariant::Type)*(char*)param->typeExtra );
    } else if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
	vartypename = (const char*)param->typeExtra;
    } else if ( QUType::isEqual( param->type, &static_QUType_enum ) && param->typeExtra ) {
	if ( var.type() == QVariant::String || var.type() == QVariant::CString ) {
	    int enumval;
	    if ( enumValue( var.toString(), (const QUEnum*)param->typeExtra, enumval ) )
		variant = enumval;
	}
	vartypename = "int";
    } else {
	vartypename = param->type->desc();
    }

    if (!variant.isValid()) {
	QVariant::Type exp = QVariant::nameToType(vartypename);
	variant.cast(exp);
    }

    bool ok = QVariantToVARIANT( variant, res, vartypename );

    // short* and char* are common in OLE controls, and cannot be coerced from int*
    if (variant.type() == QVariant::Int) {
	if (param->typeExtra == (void*)2)
	    res.vt = VT_I2;
	else if (param->typeExtra == (void*)1)
	    res.vt = VT_I1;
    } else if (variant.type() == QVariant::Double) {
	if (param->typeExtra == (void*)4)
	    res.vt = VT_R4;
    }
    bool byref = param && ( param->inOut & QUParameter::Out );
    if ( byref ) {
	if ( !ok && !qstrcmp(vartypename, "QVariant" ) ) {
	    res.vt = VT_VARIANT|VT_BYREF;
	    VARIANT *variant = new VARIANT;
	    VariantInit( variant );
	    res.pvarVal = variant;
	} else {
	    makeReference( res );
	}
    }

    return ok;
}
*/

/*!
    Converts \a arg to \a obj, and tries to coerce \a obj to the type of \a param.

    Used by
    - QAxServerBase::Invoke( methods )
    - QAxServerBase::qt_emit

    - QAxBase::qt_invoke( return value and out-parameters )
    - QAxEventSink::Invoke
*/
bool VARIANTToQUObject( const VARIANT &arg, QUObject *obj, const QUParameter *param )
{
    // three dummy variables to avoid code duplication for VT_I2/4 etc.
    int intvalue = 0;
    uint uintvalue = 0;
/*
    switch ( arg.vt ) {
    case VT_BSTR:
	{
	    QString str = BSTRToQString( arg.bstrVal );
	    if ( QUType::isEqual( param->type, &static_QUType_QString ) ) {
		static_QUType_QString.set( obj, str );
	    } else if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
		QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
		switch( vartype ) {
		case QVariant::CString:
		    static_QUType_varptr.set( obj, new QCString( str.local8Bit() ) );
		    break;
		case QVariant::String:
		    static_QUType_varptr.set( obj, new QString( str ) );
		    break;
                default:
                    break;
		}
	    } else {
		static_QUType_QString.set( obj, str );
	    }
	}
	break;
    case VT_BSTR|VT_BYREF:
	{
	    QString str = BSTRToQString( *arg.pbstrVal );
	    if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
		&& (QVariant::Type)*(char*)param->typeExtra == QVariant::CString ) {
		QCString *reference = (QCString*)static_QUType_varptr.get( obj );
		if ( reference )
		    *reference = str.local8Bit();
		else
		    reference = new QCString( str.local8Bit() );
		static_QUType_varptr.set( obj, reference );
	    } else {
		static_QUType_QString.set( obj, str );
	    }
	}
	break;
    case VT_BOOL:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	    && (QVariant::Type)*(char*)param->typeExtra == QVariant::Bool )
	    static_QUType_varptr.set( obj, new bool(arg.boolVal) );
	else
	    static_QUType_bool.set( obj, arg.boolVal );
	break;
    case VT_BOOL|VT_BYREF:
	static_QUType_bool.set( obj, *arg.pboolVal );
	break;
    case VT_I1:
    case VT_I2:
    case VT_I4:
    case VT_INT:
	switch ( arg.vt ) {
	case VT_I1:
	    intvalue = arg.cVal;
	    break;
	case VT_I2:
	    intvalue = arg.iVal;
	    break;
	case VT_I4:
	    intvalue = arg.lVal;
	    break;
	case VT_INT:
	    intvalue = arg.intVal;
	    break;
        default:
            break;
	}
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
	    const QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
	    switch ( vartype ) {
	    case QVariant::Color:
		static_QUType_varptr.set( obj, new QColor( OLEColorToQColor(intvalue) ) );
		break;
	    case QVariant::Int:
		static_QUType_varptr.set( obj, new int(intvalue) );
		break;
            default:
                break;
	    }
	} else {
	    static_QUType_int.set( obj, intvalue );
	}
	break;
    case VT_I1|VT_BYREF:
    case VT_I2|VT_BYREF:
    case VT_I4|VT_BYREF:
    case VT_INT|VT_BYREF:
	switch ( arg.vt ) {
	case VT_I1|VT_BYREF:
	    intvalue = *arg.pcVal;
	    break;
	case VT_I2|VT_BYREF:
	    intvalue = *arg.piVal;
	    break;
	case VT_I4|VT_BYREF:
	    intvalue = *arg.plVal;
	    break;
	case VT_INT|VT_BYREF:
	    intvalue = *arg.pintVal;
	    break;
        default:
            break;
	}
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	    && (QVariant::Type)*(char*)param->typeExtra == QVariant::Color ) {
	    QColor *reference = (QColor*)static_QUType_varptr.get( obj );
	    if ( reference )
		*reference = OLEColorToQColor( intvalue );
	    else
		reference = new QColor(OLEColorToQColor( intvalue ));
	    static_QUType_varptr.set( obj, reference );
	} else {
	    static_QUType_int.set( obj, intvalue );
	}
	break;
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_UINT:
	switch ( arg.vt ) {
	case VT_UI1:
	    uintvalue = arg.bVal;
	    break;
	case VT_UI2:
	    uintvalue = arg.uiVal;
	    break;
	case VT_UI4:
	    uintvalue = arg.ulVal;
	    break;
	case VT_UINT:
	    uintvalue = arg.uintVal;
	    break;
        default:
            break;
	}
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
	    const QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
	    switch( vartype ) {
	    case QVariant::Color:
		static_QUType_varptr.set( obj, new QColor( OLEColorToQColor( uintvalue ) ) );
		break;
	    case QVariant::UInt:
		static_QUType_varptr.set( obj, new uint( uintvalue ) );
		break;
            default:
                break;
	    }
	}
	break;
    case VT_UI1|VT_BYREF:
    case VT_UI2|VT_BYREF:
    case VT_UI4|VT_BYREF:
    case VT_UINT|VT_BYREF:
	switch ( arg.vt ) {
	case VT_UI1|VT_BYREF:
	    uintvalue = *arg.pbVal;
	    break;
	case VT_UI2|VT_BYREF:
	    uintvalue = *arg.puiVal;
	    break;
	case VT_UI4|VT_BYREF:
	    uintvalue = *arg.pulVal;
	    break;
	case VT_UINT|VT_BYREF:
	    uintvalue = *arg.puintVal;
	    break;
        default:
            break;
	}
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
	    const QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
	    switch ( vartype ) {
	    case QVariant::Color:
		{
		    QColor *reference = (QColor*)static_QUType_varptr.get( obj );
		    if ( reference )
			*reference = OLEColorToQColor( uintvalue );
		    else
			reference = new QColor(OLEColorToQColor( uintvalue ));
		    static_QUType_varptr.set( obj, reference );
		}
		break;
	    case QVariant::UInt:
		{
		    uint *reference = (uint*)static_QUType_varptr.get( obj );
		    if ( reference )
			*reference = uintvalue;
		    else
			reference = new uint( uintvalue );
		    static_QUType_varptr.set( obj, reference );
		}
		break;
            default:
                break;
	    }
	} 
	break;
    case VT_CY:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
	    const QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
	    switch( vartype ) {
	    case QVariant::ULongLong:
		static_QUType_varptr.set( obj, new Q_ULLONG(arg.cyVal.int64) );
		break;
	    case QVariant::LongLong:
		static_QUType_varptr.set( obj, new Q_LLONG(arg.cyVal.int64) );
		break;
            default:
                break;
	    }
	}
	break;
    case VT_CY|VT_BYREF:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra ) {
	    const QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
	    switch( vartype ) {
	    case QVariant::ULongLong:
		{
		    Q_ULLONG *reference = (Q_ULLONG*)static_QUType_varptr.get( obj );
		    if ( reference )
			*reference = arg.pcyVal->int64;
		    else
			reference = new Q_ULLONG(arg.pcyVal->int64);
		    static_QUType_varptr.set( obj, reference );
		}
		break;
	    case QVariant::LongLong:
		{
		    Q_LLONG *reference = (Q_LLONG*)static_QUType_varptr.get( obj );
		    if ( reference )
			*reference = arg.pcyVal->int64;
		    else
			reference = new Q_LLONG(arg.pcyVal->int64);
		    static_QUType_varptr.set( obj, reference );
		}
		break;
            default:
                break;
	    }
	}
	break;
    case VT_R4:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	    && (QVariant::Type)*(char*)param->typeExtra == QVariant::Double )
	    static_QUType_varptr.set( obj, new double(arg.fltVal) );
	else
	    static_QUType_double.set( obj, arg.fltVal );
	break;
    case VT_R8:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	    && (QVariant::Type)*(char*)param->typeExtra == QVariant::Double )
	    static_QUType_varptr.set( obj, new double(arg.dblVal) );
	else
	    static_QUType_double.set( obj, arg.dblVal );
	break;
    case VT_R4|VT_BYREF:
	static_QUType_double.set( obj, *arg.pfltVal );
	break;
    case VT_R8|VT_BYREF:
	static_QUType_double.set( obj, *arg.pdblVal );
	break;
    case VT_DATE: // DATE -> QDateTime
	static_QUType_varptr.set( obj, new QDateTime( DATEToQDateTime( arg.date ) ) );
	break;
    case VT_DATE|VT_BYREF:
	{
	    QDateTime *reference = (QDateTime*)static_QUType_varptr.get( obj );
	    if ( reference )
		*reference = DATEToQDateTime( *arg.pdate );
	    else
		reference = new QDateTime(DATEToQDateTime( *arg.pdate ));
	    static_QUType_varptr.set( obj, reference );
	}
	break;
    case VT_DISPATCH:
    case VT_DISPATCH|VT_BYREF:
	{
	    IDispatch *disp = 0;
	    if ( arg.vt & VT_BYREF )
		disp = *arg.ppdispVal;
	    else
		disp = arg.pdispVal;

	    if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
		&& (QVariant::Type)*(char*)param->typeExtra == QVariant::Font ) {
		IFont *ifont = 0;
		QFont qfont;
		QFont *reference = (QFont*)static_QUType_varptr.get( obj );
		if ( disp )
		    disp->QueryInterface( IID_IFont, (void**)&ifont );
		if ( ifont ) {
		    qfont = IFontToQFont( ifont );
		    ifont->Release();
		} else {
		    qfont = QFont();
		}
		if ( reference )
		    *reference = qfont;
		else
		    reference = new QFont( qfont );
		static_QUType_varptr.set( obj, reference );
	    } else if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
		       && (QVariant::Type)*(char*)param->typeExtra == QVariant::Pixmap ) {
		IPicture *ipic = 0;
		QPixmap qpixmap;
		QPixmap *reference = (QPixmap*)static_QUType_varptr.get( obj );
		if ( disp )
		    disp->QueryInterface( IID_IPicture, (void**)&ipic );
		if ( ipic ) {
		    qpixmap = IPictureToQPixmap( ipic );
		    ipic->Release();
		} else {
		    qpixmap = QPixmap();
		}
		if ( reference )
		    *reference = qpixmap;
		else
		    reference = new QPixmap( qpixmap );
		static_QUType_varptr.set( obj, reference );
	    } else {
		IAxServerBase *qax = 0;
		if (disp)
		    disp->QueryInterface(IID_IAxServerBase, (void**)&qax);
		if (qax) {
		    QObject *theObject = qax->qObject();
		    // verify that parameter expect the QObject
		    if (theObject && !qstrcmp(theObject->className(),(const char*)param->typeExtra))
			static_QUType_ptr.set(obj, theObject);
		    qax->Release();
		}
		if (!obj->payload.ptr)
		    static_QUType_ptr.set( obj, disp );
	    }
	}
	break;

    case VT_ARRAY|VT_VARIANT:
    case VT_ARRAY|VT_VARIANT|VT_BYREF:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	     && *(char*)param->typeExtra == QVariant::List ) {
	    // parray and pparrayare a union
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;
	    QList<QVariant> list;
	    QList<QVariant> *reference = (QList<QVariant>*)static_QUType_varptr.get( obj );

	    if ( array && array->cDims == 1 ) {
		long lBound, uBound;
		SafeArrayGetLBound( array, 1, &lBound );
		SafeArrayGetUBound( array, 1, &uBound );

		for ( long i = lBound; i <= uBound; ++i ) {
		    VARIANT var;
		    VariantInit( &var );
		    SafeArrayGetElement( array, &i, &var );

		    QVariant qvar = VARIANTToQVariant( var, 0 );
		    clearVARIANT( &var );
		    list << qvar;
		}
	    }
	    if ( reference )
		*reference = list;
	    else
		reference = new QList<QVariant>( list );
	    static_QUType_varptr.set( obj, reference );
	}
	break;

    case VT_ARRAY|VT_BSTR:
    case VT_ARRAY|VT_BSTR|VT_BYREF:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	     && *(char*)param->typeExtra == QVariant::StringList ) {
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;
	    QStringList strings;
	    QStringList *reference = (QStringList*)static_QUType_varptr.get( obj );

	    if ( array && array->cDims == 1 ) {
		long lBound, uBound;
		SafeArrayGetLBound( array, 1, &lBound );
		SafeArrayGetUBound( array, 1, &uBound );

		for (long i = lBound; i <= uBound; ++i ) {
		    BSTR bstr;
		    SafeArrayGetElement( array, &i, &bstr );
		    QString qstr = BSTRToQString(bstr);
		    strings << qstr;
		}
	    }
	    if ( reference )
		*reference = strings;
	    else
		reference = new QStringList(strings);
	    static_QUType_varptr.set( obj, reference );
	}
	break;

    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI1|VT_BYREF:
	if ( QUType::isEqual( param->type, &static_QUType_varptr ) && param->typeExtra 
	     && *(char*)param->typeExtra == QVariant::ByteArray ) {
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;
	    QByteArray bytes;
	    QByteArray *reference = (QByteArray*)static_QUType_varptr.get( obj );

	    if ( array && array->cDims == 1 ) {
		long lBound, uBound;
		SafeArrayGetLBound( array, 1, &lBound );
		SafeArrayGetUBound( array, 1, &uBound );

		if ( uBound != -1 ) {
		    bytes.resize( uBound - lBound + 1 );
		    char *data = bytes.data();
		    char *src;
		    SafeArrayAccessData( array, (void**)&src );
		    memcpy( data, src, bytes.size() );
		    SafeArrayUnaccessData( array );
		}
	    }
	    if ( reference )
		*reference = bytes;
	    else
		reference = new QByteArray( bytes );
	    static_QUType_varptr.set( obj, reference );
	}
	break;

    case VT_UNKNOWN:  // IUnknown -> void*
	static_QUType_ptr.set( obj, arg.punkVal );
	break;
    case VT_UNKNOWN|VT_BYREF:
	static_QUType_ptr.set( obj, *arg.ppunkVal );
	break;
    case VT_VARIANT: 
    case VT_VARIANT|VT_BYREF:
	if ( arg.pvarVal ) {
	    QVariant var = VARIANTToQVariant( *arg.pvarVal, 0 );
	    static_QUType_QVariant.set( obj, var );
	}
	break;
    case VT_RECORD|VT_BYREF:
	if (arg.pvRecord && arg.pRecInfo) {
	    QVariant var = VARIANTToQVariant(arg, 0);
	    void *reference = static_QUType_varptr.get(obj);
	    switch(var.type()) {
	    case QVariant::Rect:
		if (reference)
		    *(QRect*)reference = var.toRect();
		else
		    reference = new QRect(var.toRect());
		break;
	    case QVariant::Size:
		if (reference)
		    *(QSize*)reference = var.toSize();
		else
		    reference = new QSize(var.toSize());
		break;
	    case QVariant::Point:
		if (reference)
		    *(QPoint*)reference = var.toPoint();
		else
		    reference = new QPoint(var.toPoint());
		break;
	    default:
		break;
	    }
	    static_QUType_varptr.set(obj, reference);
	}
	break;
    default:
	return FALSE;
    }

    if ( !QUType::isEqual(  param->type, obj->type ) ) {
	if ( !param->type->canConvertFrom( obj, obj->type ) ) {
#ifndef QT_NO_DEBUG
	    qWarning( "Can't coerce VARIANT type to requested type (%s to %s)", obj->type->desc(), param->type->desc() );
#endif
	    obj->type->clear( obj );
	    return FALSE;
	} else {
	    param->type->convertFrom( obj, obj->type );
	}
    }
*/
    return TRUE;
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
QVariant VARIANTToQVariant( const VARIANT &arg, const QString &hint )
{
    QVariant var;
    switch( arg.vt ) {
    case VT_BSTR:
	var = BSTRToQString( arg.bstrVal );
	break;
    case VT_BSTR|VT_BYREF:
	var = BSTRToQString( *arg.pbstrVal );
	break;
    case VT_BOOL:
	var = QVariant( (bool)arg.boolVal );
	break;
    case VT_BOOL|VT_BYREF:
	var = QVariant( (bool)*arg.pboolVal );
	break;
    case VT_I1:
	var = arg.cVal;
	break;
    case VT_I1|VT_BYREF:
	var = *arg.pcVal;
	break;
    case VT_I2:
	var = arg.iVal;
	break;
    case VT_I2|VT_BYREF:
	var = *arg.piVal;
	break;
    case VT_I4:
	if (hint == "QColor")
	    var = OLEColorToQColor( arg.lVal );
	else
	    var = (int)arg.lVal;
	break;
    case VT_I4|VT_BYREF:
	if (hint == "QColor")
	    var = OLEColorToQColor( (int)*arg.plVal );
	else
	    var = (int)*arg.plVal;
	break;
    case VT_INT:
	var = arg.intVal;
	break;
    case VT_INT|VT_BYREF:
	var = *arg.pintVal;
	break;
    case VT_UI1:
	var = arg.bVal;
	break;
    case VT_UI1|VT_BYREF:
	var = *arg.pbVal;
	break;
    case VT_UI2:
	var = arg.uiVal;
	break;
    case VT_UI2|VT_BYREF:
	var = *arg.puiVal;
	break;
    case VT_UI4:
	if (hint == "QColor")
	    var = OLEColorToQColor( arg.ulVal );
	else
	    var = (int)arg.ulVal;
	break;
    case VT_UI4|VT_BYREF:
	if (hint == "QColor")
	    var = OLEColorToQColor( (uint)*arg.pulVal );
	else
	    var = (int)*arg.pulVal;
	break;
    case VT_UINT:
	var = arg.uintVal;
	break;
    case VT_UINT|VT_BYREF:
	var = *arg.puintVal;
	break;
    case VT_CY:
	var = arg.cyVal.int64;
	break;
    case VT_CY|VT_BYREF:
	var = arg.pcyVal->int64;
	break;
    case VT_R4:
	var = arg.fltVal;
	break;
    case VT_R4|VT_BYREF:
	var = *arg.pfltVal;
	break;
    case VT_R8:
	var = arg.dblVal;
	break;
    case VT_R8|VT_BYREF:
	var = *arg.pdblVal;
	break;
    case VT_DATE:
	var = DATEToQDateTime( arg.date );
	break;
    case VT_DATE|VT_BYREF:
	var = DATEToQDateTime( *arg.pdate );
	break;
    case VT_VARIANT:
    case VT_VARIANT|VT_BYREF:
	if ( arg.pvarVal )
	    var = VARIANTToQVariant( *arg.pvarVal, hint );
	break;

    case VT_DISPATCH:
    case VT_DISPATCH|VT_BYREF:
	{
	    // pdispVal and ppdispVal are a union
	    IDispatch *disp = 0;
	    if ( arg.vt & VT_BYREF )
		disp = *arg.ppdispVal;
	    else
		disp = arg.pdispVal;
	    if (hint == "QFont") {
		IFont *ifont = 0;
		if ( disp )
		    disp->QueryInterface( IID_IFont, (void**)&ifont );
		if ( ifont ) {
		    var = IFontToQFont( ifont );
		    ifont->Release();
		} else {
		    var = QFont();
		}
	    } else if (hint == "QPixmap") {
		IPicture *ipic = 0;
		if ( disp )
		    disp->QueryInterface( IID_IPicture, (void**)&ipic );
		if ( ipic ) {
		    var = IPictureToQPixmap( ipic );
		    ipic->Release();
		} else {
		    var = QPixmap();
		}
	    } else {
		var.rawAccess( (IUnknown*)disp, (QVariant::Type)1000 );
	    }
	}
	break;
    case VT_UNKNOWN:
    case VT_UNKNOWN|VT_BYREF:
	{
	    IUnknown *unkn = 0;
	    if ( arg.vt & VT_BYREF )
		unkn = *arg.ppunkVal;
	    else
		unkn = arg.punkVal;
	    var.rawAccess( unkn, (QVariant::Type)1000 );
	}
	break;
    case VT_ARRAY|VT_VARIANT:
    case VT_ARRAY|VT_VARIANT|VT_BYREF:
	{
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;

	    QList<QVariant> list;
	    if ( !array || array->cDims != 1 ) {
		var = list;
		break;
	    }

	    long lBound, uBound;
	    SafeArrayGetLBound( array, 1, &lBound );
	    SafeArrayGetUBound( array, 1, &uBound );

	    for ( long i = lBound; i <= uBound; ++i ) {
		VARIANT var;
		VariantInit( &var );
		SafeArrayGetElement( array, &i, &var );

		QVariant qvar = VARIANTToQVariant( var, 0 );
		clearVARIANT( &var );
		list << qvar;
	    }

	    var = list;
	}
	break;

    case VT_ARRAY|VT_BSTR:
    case VT_ARRAY|VT_BSTR|VT_BYREF:
	{
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;

	    QStringList strings;
	    if (!array || array->cDims != 1 ) {
		var = strings;
		break;
	    }

	    long lBound, uBound;
	    SafeArrayGetLBound( array, 1, &lBound );
	    SafeArrayGetUBound( array, 1, &uBound );

	    for ( long i = lBound; i <= uBound; ++i ) {
		BSTR bstr;
		SafeArrayGetElement( array, &i, &bstr );
		QString str = BSTRToQString( bstr );
		strings << str;
	    }

	    var = strings;
	}
	break;

    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI1|VT_BYREF:
	{
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;

	    QByteArray bytes;
	    if ( !array || array->cDims != 1 ) {
		var = bytes;
		break;
	    }

	    long lBound, uBound;
	    SafeArrayGetLBound( array, 1, &lBound );
	    SafeArrayGetUBound( array, 1, &uBound );

	    if ( uBound != -1 ) { // non-empty array
		bytes.resize( uBound - lBound + 1 );
		char *data = bytes.data();
		char *src;
		SafeArrayAccessData( array, (void**)&src );
		memcpy( data, src, bytes.size() );
		SafeArrayUnaccessData( array );
	    }

	    var = bytes;
	}
	break;

#if defined(QAX_SERVER)
    case VT_RECORD:
    case VT_RECORD|VT_BYREF:
	if (arg.pvRecord && arg.pRecInfo) {
	    IRecordInfo *recordInfo = arg.pRecInfo;
	    void *record = arg.pvRecord;
	    GUID guid;
	    recordInfo->GetGuid(&guid);

	    if (guid == CLSID_QRect) {
		QRect qrect;
		recordInfo->RecordCopy(record, &qrect);
		var = qrect;
	    } else if (guid == CLSID_QSize) {
		QSize qsize;
		recordInfo->RecordCopy(record, &qsize);
		var = qsize;
	    } else if (guid == CLSID_QPoint) {
		QPoint qpoint;
		recordInfo->RecordCopy(record, &qpoint);
		var = qpoint;
	    }
	}
	break;
#endif // QAX_SERVER
    default:
	break;
    }

    QVariant::Type proptype = hint ? QVariant::nameToType(hint.latin1()) : QVariant::Invalid;
    if ( proptype != QVariant::Invalid && var.type() != proptype ) {
	if ( var.canCast( proptype ) ) {
	    var.cast( proptype );
	} else if (proptype == QVariant::StringList && var.type() == QVariant::List) {
	    bool allStrings = TRUE;
	    QStringList strings;
	    const QList<QCoreVariant> list(var.toList());
	    for (QList<QCoreVariant>::ConstIterator it(list.begin()); it != list.end(); ++it) {
		QVariant variant = *it;
		if (variant.canCast(QVariant::String))
		    strings << variant.toString();
		else
		    allStrings = FALSE;
	    }
	    if (allStrings)
		var = strings;
	}
    }
    return var;
}

static inline void updateReference( VARIANT &dest, VARIANT &src, bool byref )
{
    if ( dest.vt == (src.vt|VT_BYREF) ) {
	switch( src.vt ) {
	case VT_BSTR:
	    *dest.pbstrVal = src.bstrVal;
	    break;
	case VT_BOOL:
	    *dest.pboolVal = src.boolVal;
	    break;
	case VT_I1:
	    *dest.pcVal = src.cVal;
	    break;
	case VT_I2:
	    *dest.piVal = src.iVal;
	    break;
	case VT_I4:
	    *dest.plVal = src.lVal;
	    break;
	case VT_INT:
	    *dest.pintVal = src.intVal;
	    break;
	case VT_UI1:
	    *dest.pbVal = src.bVal;
	    break;
	case VT_UI2:
	    *dest.puiVal = src.uiVal;
	    break;
	case VT_UI4:
	    *dest.plVal = src.lVal;
	    break;
	case VT_UINT:
	    *dest.puintVal = src.uintVal;
	    break;
	case VT_CY:
	    *dest.pcyVal = src.cyVal;
	    break;
	case VT_R4:
	    *dest.pfltVal = src.fltVal;
	    break;
	case VT_R8:
	    *dest.pdblVal = src.dblVal;
	    break;
	case VT_DATE:
	    *dest.pdate = src.date;
	    break;
	case VT_DISPATCH:
	    if ( *dest.ppdispVal ) (*dest.ppdispVal)->Release();
	    *dest.ppdispVal = src.pdispVal;
	    break;
	case VT_ARRAY|VT_VARIANT:
	case VT_ARRAY|VT_UI1:
	case VT_ARRAY|VT_BSTR:
	    if ( *dest.pparray ) SafeArrayDestroy( *dest.pparray );
	    *dest.pparray = src.parray;
	    break;
	case VT_RECORD:
	    src.pRecInfo->RecordCopy(src.pvRecord, dest.pvRecord);
	    break;
	default:
	    dest = src;
	    dest.vt = src.vt|VT_BYREF;
	    break;
	}
    } else {
	if ( byref && dest.vt != VT_EMPTY )
	    clearVARIANT( &dest );
	dest = src;
    }
}

/*!
    Converts \a obj to \a arg, and tries to coerce \a var to the type of \a param.

    Used by
    QAxServerBase:
    - qt_emit
    - IDispatch::Invoke( update references in method )

    QAxBase:
    - QAxBase::qt_invoke
*/
bool QUObjectToVARIANT( QUObject *obj, VARIANT &arg, const QUParameter *param )
{
/*
    bool byref = param && ( param->inOut & QUParameter::Out ) && ( param->inOut != QUParameter::Out );
    if ( param && !QUType::isEqual( param->type, obj->type ) && param->type->canConvertFrom( obj, obj->type ) )
	param->type->convertFrom( obj, obj->type );

    // map the QUObject's type to the VARIANT
    if ( QUType::isEqual( obj->type, &static_QUType_int ) ) {
	if ( byref && ( arg.vt == (VT_I4|VT_BYREF) ) ) {
	    // short* and char* are common in OLE controls, and cannot be coerced from int*
	    if (param->typeExtra == (void*)2) {
		arg.vt = VT_I2|VT_BYREF;
		*arg.piVal = static_QUType_int.get( obj );
	    } else if (param->typeExtra == (void*)1) {
		arg.vt = VT_I1|VT_BYREF;
		*arg.pbVal = static_QUType_int.get( obj );
	    } else {
		*arg.plVal = static_QUType_int.get( obj );
	    }
	} else {
	    arg.vt = VT_I4;
	    arg.lVal = static_QUType_int.get( obj );
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_QString ) ) {
	if ( byref && ( arg.vt == (VT_BSTR|VT_BYREF) ) ) {
	    SysFreeString( *arg.pbstrVal );
	    *arg.pbstrVal = QStringToBSTR( static_QUType_QString.get( obj ) );
	} else {
	    arg.vt = VT_BSTR;
	    arg.bstrVal = QStringToBSTR( static_QUType_QString.get( obj ) );
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_bool ) ) {
	if ( byref && ( arg.vt == (VT_BOOL|VT_BYREF) ) ) {
	    *arg.pboolVal = static_QUType_bool.get( obj ) ? -1 : 0;
	} else {
	    arg.vt = VT_BOOL;
	    arg.boolVal = static_QUType_bool.get( obj ) ? -1 : 0;
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_double ) ) {
	if ( byref && ( arg.vt == (VT_R8|VT_BYREF) ) ) {
	    *arg.pdblVal = static_QUType_double.get( obj );
	} else {
	    arg.vt = VT_R8;
	    arg.dblVal = static_QUType_double.get( obj );
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_enum ) ) {
	if ( byref && ( arg.vt == (VT_I4|VT_BYREF) ) ) {
	    *arg.plVal = static_QUType_enum.get( obj );
	} else {
	    arg.vt = VT_I4;
	    arg.lVal = static_QUType_enum.get( obj );
	}
    } else if ( QUType::isEqual( obj->type, &static_QUType_QVariant ) && param ) {
	QVariant value = static_QUType_QVariant.get( obj );
	const char *vartype;
	if ( QUType::isEqual( param->type, &static_QUType_QVariant ) ||
	     QUType::isEqual( param->type, &static_QUType_varptr ) ) {
	    if ( param->typeExtra )
		vartype = QVariant::typeToName( (QVariant::Type)*(char*)param->typeExtra );
	    else
		vartype = value.typeName();
	} else if ( QUType::isEqual( param->type, &static_QUType_ptr ) )
	    vartype = (const char*)param->typeExtra;
	else
	    vartype = param->type->desc();

	VARIANT var;
	VariantInit( &var );
	if ( !QVariantToVARIANT( value, var, vartype ) )
	    return FALSE;

	updateReference( arg, var, byref );
    } else if ( QUType::isEqual( obj->type, &static_QUType_varptr ) ||
		QUType::isEqual( obj->type, &static_QUType_ptr ) && param ) {
	void *ptrvalue = static_QUType_varptr.get( obj );
	const char *vartype;
	QVariant value;
	VARIANT var;
	VariantInit( &var );

	if ( QUType::isEqual( param->type, &static_QUType_varptr ) ||
	    QUType::isEqual( param->type, &static_QUType_QVariant ) ) {
	    QVariant::Type vart;
	    if ( param->typeExtra )
		vart = (QVariant::Type)*(char*)param->typeExtra ;
	    else
		vart = value.type();

	    switch( vart ) {
	    case QVariant::UInt:
		value = *(uint*)ptrvalue;
		break;
	    case QVariant::ULongLong:
		value = *(Q_ULLONG*)ptrvalue;
		break;
	    case QVariant::LongLong:
		value = *(Q_LLONG*)ptrvalue;
		break;
	    case QVariant::CString:
		value = *(QCString*)ptrvalue;
		break;
	    case QVariant::Color:
		value = *(QColor*)ptrvalue;
		break;
	    case QVariant::Date:
		value = *(QDate*)ptrvalue;
		break;
	    case QVariant::Time:
		value = *(QTime*)ptrvalue;
		break;
	    case QVariant::DateTime:
		value = *(QDateTime*)ptrvalue;
		break;
	    case QVariant::Pixmap:
		value = *(QPixmap*)ptrvalue;
		break;
	    case QVariant::Font:
		value = *(QFont*)ptrvalue;
		break;
	    case QVariant::List:
		value = *(QList<QVariant>*)ptrvalue;
		break;
	    case QVariant::ByteArray:
		value = *(QByteArray*)ptrvalue;
		break;
	    case QVariant::StringList:
		value = *(QStringList*)ptrvalue;
		break;
	    case QVariant::Rect:
		value = *(QRect*)ptrvalue;
		break;
	    case QVariant::Size:
		value = *(QSize*)ptrvalue;
		break;
	    case QVariant::Point:
		value = *(QPoint*)ptrvalue;
		break;
	    default:
		break;
	    }
	    vartype = QVariant::typeToName( vart );
	} else if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
	    vartype = (const char*)param->typeExtra;
	    if ( !qstrcmp( vartype, "IDispatch*" ) || !qstrcmp( vartype, "IDispatch" ) ) {
		var.vt = VT_DISPATCH;
		var.pdispVal = (IDispatch*)ptrvalue;
		if ( ptrvalue )
		    var.pdispVal->AddRef();
	    } else if ( !qstrcmp( vartype, "IUnknown*" ) || !qstrcmp( vartype, "IUnknown" ) ) {
		var.vt = VT_UNKNOWN;
		var.punkVal = (IUnknown*)ptrvalue;
		if ( ptrvalue )
		    var.punkVal->AddRef();
#ifdef QAX_SERVER
	    } else if (qAxFactory()->featureList().contains(vartype)) {
		var.vt = VT_DISPATCH;
		
		if (ptrvalue)
		    qAxFactory()->createObjectWrapper((QObject*)ptrvalue, &var.pdispVal);
		else
		    var.pdispVal = 0;
#endif
	    }
	} else {
	    vartype = param->type->desc();
	}

	if( var.vt == VT_EMPTY && !QVariantToVARIANT( value, var, vartype ) )
	    return FALSE;

	updateReference( arg, var, byref );
    } else {
	qDebug( "QUObjectToVARIANT: Unhandled QUType %s!", obj->type->desc() );
	arg.vt = VT_EMPTY;
	return FALSE;
    }
    if ( byref && !(arg.vt & VT_BYREF) )
	makeReference( arg );
*/
    return TRUE;
}

void clearQUObject( QUObject *obj, const QUParameter *param )
{
/*
    if ( !param || !QUType::isEqual( param->type, &static_QUType_varptr ) || !QUType::isEqual( param->type, obj->type ) ) {
	obj->type->clear( obj );
    } else if ( param->typeExtra ) {
	const QVariant::Type vartype = (QVariant::Type)*(char*)param->typeExtra;
	void *ptrvalue = static_QUType_varptr.get( obj );
	switch( vartype ) {
	case QVariant::Bool:
	    delete (bool*)ptrvalue;
	    break;
	case QVariant::Int:
	    delete (int*)ptrvalue;
	    break;
	case QVariant::UInt:
	    delete (uint*)ptrvalue;
	    break;
	case QVariant::LongLong:
	    delete (Q_LLONG*)ptrvalue;
	    break;
	case QVariant::ULongLong:
	    delete (Q_ULLONG*)ptrvalue;
	    break;
	case QVariant::Double:
	    delete (double*)ptrvalue;
	    break;
	case QVariant::String:
	    delete (QString*)ptrvalue;
	    break;
	case QVariant::CString:
	    delete (QCString*)ptrvalue;
	    break;
	case QVariant::Color:
	    delete (QColor*)ptrvalue;
	    break;
	case QVariant::Date:
	    delete (QDate*)ptrvalue;
	    break;
	case QVariant::Time:
	    delete (QTime*)ptrvalue;
	    break;
	case QVariant::DateTime:
	    delete (QDateTime*)ptrvalue;
	    break;
	case QVariant::Pixmap:
	    delete (QPixmap*)ptrvalue;
	    break;
	case QVariant::Font:
	    delete (QFont*)ptrvalue;
	    break;
	case QVariant::List:
	    delete (QList<QVariant>*)ptrvalue;
	    break;
	case QVariant::ByteArray:
	    delete (QByteArray*)ptrvalue;
	    break;
	case QVariant::StringList:
	    delete (QStringList*)ptrvalue;
	    break;
	case QVariant::Rect:
	    delete (QRect*)ptrvalue;
	    break;
	case QVariant::Size:
	    delete (QSize*)ptrvalue;
	    break;
	case QVariant::Point:
	    delete (QPoint*)ptrvalue;
	    break;
	default:
	    break;
	}
	obj->payload.ptr = 0;
    }
*/
}

void clearVARIANT( VARIANT *var )
{
    if ( var->vt & VT_BYREF ) {
	switch( var->vt ) {
	case VT_BSTR|VT_BYREF:
	    SysFreeString( *var->pbstrVal );
	    delete var->pbstrVal;
	    break;
	case VT_BOOL|VT_BYREF:
	    delete var->pboolVal;
	    break;
	case VT_I1|VT_BYREF:
	    delete var->pcVal;
	    break;
	case VT_I2|VT_BYREF:
	    delete var->piVal;
	    break;
	case VT_I4|VT_BYREF:
	    delete var->plVal;
	    break;
	case VT_INT|VT_BYREF:
	    delete var->pintVal;
	    break;
	case VT_UI1|VT_BYREF:
	    delete var->pbVal;
	    break;
	case VT_UI2|VT_BYREF:
	    delete var->puiVal;
	    break;
	case VT_UI4|VT_BYREF:
	    delete var->pulVal;
	    break;
	case VT_UINT|VT_BYREF:
	    delete var->puintVal;
	    break;
	case VT_CY|VT_BYREF:
	    delete var->pcyVal;
	    break;
	case VT_R4|VT_BYREF:
	    delete var->pfltVal;
	    break;
	case VT_R8|VT_BYREF:
	    delete var->pdblVal;
	    break;
	case VT_DATE|VT_BYREF:
	    delete var->pdate;
	    break;
	case VT_DISPATCH|VT_BYREF:
	    (*var->ppdispVal)->Release();
	    delete var->ppdispVal;
	    break;
	case VT_ARRAY|VT_VARIANT|VT_BYREF:
	case VT_ARRAY|VT_UI1|VT_BYREF:
	    SafeArrayDestroy( *var->pparray );
	    delete var->pparray;
	    break;
	case VT_VARIANT|VT_BYREF:
	    delete var->pvarVal;
	    break;
	}
	VariantInit( var );
    } else {
	VariantClear( var );
    }
}
