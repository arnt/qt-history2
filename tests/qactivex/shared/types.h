#ifndef TYPES_H
#define TYPES_H

#include <math.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qmetaobject.h>
#include <private/qcom_p.h>
#include <private/qucomextra_p.h>


/*! 
    Helper functions 
*/
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

static inline QString typedescToQString( TYPEDESC typedesc )
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
    else if ( ptype == "USERDEFINED" ) // most USERDEFINED types are long or ints, or interfaces
	ptype = "int";
    else if ( ptype == "USERDEFINED*" )
	ptype = "IUnknown*";
	
    return ptype;
}

static inline bool checkHRESULT( HRESULT hres )
{
    switch( hres ) {
    case S_OK:
	return TRUE;
    case DISP_E_BADPARAMCOUNT:
	return FALSE;
    case DISP_E_BADVARTYPE:
	return FALSE;
    case DISP_E_EXCEPTION:
	return FALSE;
    case DISP_E_MEMBERNOTFOUND:
	return FALSE;
    case DISP_E_NONAMEDARGS:
	return FALSE;
    case DISP_E_OVERFLOW:
	return FALSE;
    case DISP_E_PARAMNOTFOUND:
	return FALSE;
    case DISP_E_TYPEMISMATCH:
	return FALSE;
    case DISP_E_UNKNOWNINTERFACE:
	return FALSE;
    case DISP_E_UNKNOWNLCID:
	return FALSE;
    case DISP_E_PARAMNOTOPTIONAL:
	return FALSE;
    default:
	return FALSE;
    }
}


// those functions are not ours
static int monthdays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
#define HALF_SECOND  (1.0/172800.0)

static inline QDateTime DATEToQDateTime( DATE ole )
{
    int year, month, day, wday, yday;
    int hour, min, sec;

    long nDays;             // Number of days since Dec. 30, 1899
    long nDaysAbsolute;     // Number of days since 1/1/0
    long nSecsInDay;        // Time in seconds since midnight
    long nMinutesInDay;     // Minutes in day
    
    long n400Years;         // Number of 400 year increments since 1/1/0
    long n400Century;       // Century within 400 year block (0,1,2 or 3)
    long n4Years;           // Number of 4 year increments since 1/1/0
    long n4Day;             // Day within 4 year block
    //  (0 is 1/1/yr1, 1460 is 12/31/yr4)
    long n4Yr;              // Year within 4 year block (0,1,2 or 3)
    BOOL bLeap4 = TRUE;     // TRUE if 4 year block includes leap year
    
    double dblDate = ole; // tempory serial date
    
    // If a valid date, then this conversion should not overflow
    nDays = (long)dblDate;
    
    // Round to the second
    dblDate += ((ole > 0.0) ? HALF_SECOND : -HALF_SECOND);
    
    nDaysAbsolute = (long)dblDate + 693959L; // Add days from 1/1/0 to 12/30/1899
    
    dblDate = fabs(dblDate);
    nSecsInDay = (long)((dblDate - floor(dblDate)) * 86400.);
    
    // Calculate the day of week (sun=1, mon=2...)
    //   -1 because 1/1/0 is Sat.  +1 because we want 1-based
    wday = (int)((nDaysAbsolute - 1) % 7L) + 1;
    
    // Leap years every 4 yrs except centuries not multiples of 400.
    n400Years = (long)(nDaysAbsolute / 146097L);
    
    // Set nDaysAbsolute to day within 400-year block
    nDaysAbsolute %= 146097L;
    
    // -1 because first century has extra day
    n400Century = (long)((nDaysAbsolute - 1) / 36524L);
    
    // Non-leap century
    if (n400Century != 0) {
	// Set nDaysAbsolute to day within century
	nDaysAbsolute = (nDaysAbsolute - 1) % 36524L;
	
	// +1 because 1st 4 year increment has 1460 days
	n4Years = (long)((nDaysAbsolute + 1) / 1461L);
	
	if (n4Years != 0) {
	    n4Day = (long)((nDaysAbsolute + 1) % 1461L);
	} else {
	    bLeap4 = FALSE;
	    n4Day = (long)nDaysAbsolute;
	}
    } else {
	// Leap century - not special case!
	n4Years = (long)(nDaysAbsolute / 1461L);
	n4Day = (long)(nDaysAbsolute % 1461L);
    }
    
    if (bLeap4) {
	// -1 because first year has 366 days
	n4Yr = (n4Day - 1) / 365;
	
	if (n4Yr != 0)
	    n4Day = (n4Day - 1) % 365;
    } else {
	n4Yr = n4Day / 365;
	n4Day %= 365;
    }
    
    // n4Day is now 0-based day of year. Save 1-based day of year, year number
    yday = (int)n4Day + 1;
    year = n400Years * 400 + n400Century * 100 + n4Years * 4 + n4Yr;
    
    // Handle leap year: before, on, and after Feb. 29.
    if (n4Yr == 0 && bLeap4) {
	// Leap Year
	if (n4Day == 59) {
	    /* Feb. 29 */
	    month = 2;
	    day = 29;
	    goto DoTime;
	}
	
	// Pretend it's not a leap year for month/day comp.
	if (n4Day >= 60)
	    --n4Day;
    }
    
    // Make n4DaY a 1-based day of non-leap year and compute
    //  month/day for everything but Feb. 29.
    ++n4Day;
    
    // Month number always >= n/32, so save some loop time */
    for ( month = (n4Day >> 5) + 1; n4Day > monthdays[month]; month++ )
	;
    
    day = (int)(n4Day - monthdays[month-1]);
    
DoTime:
    if (nSecsInDay == 0) {
	hour = min = sec = 0;
    } else {
	sec = (int)nSecsInDay % 60L;
	nMinutesInDay = nSecsInDay / 60L;
	min = (int)nMinutesInDay % 60;
	hour = (int)nMinutesInDay / 60;
    }

    QDateTime dt;
    dt.setDate( QDate( year, month, day ) );
    dt.setTime( QTime( hour, min, sec ) );
    return dt;
}

static inline DATE QDateTimeToDATE( const QDateTime &dt )
{
    QDate date = dt.date();
    QTime time = dt.time();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    int hour = time.hour();
    int min = time.minute();
    int sec = time.second();
    int dim = date.daysInMonth();
    bool leap = date.leapYear( year );

    long oledate = year*365 + year/4 - year/100 + year/400 + monthdays[month-1] + day;
    if ( month <= 2 && leap )
	--oledate;
    oledate -= 693959;

    double oletime = (((long)hour * 3600L) + ((long)min * 60L) + ((long)sec)) / 86400.;

    DATE ole = (double) oledate + ( ( oledate >= 0 ) ? oletime : -oletime );
    return ole;
}

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

static inline QString constRefify( const QString& type )
{
    QString crtype;

    if ( type == "QString" )
	crtype = "const QString&";
    else if ( type == "QDateTime" )
	crtype = "const QDateTime&";
    else if ( type == "QVariant" )
	crtype = "const QVariant&";
    else 
	crtype = type;

    return crtype;
}

static inline void QStringToQUType( const QString& type, QUParameter *param )
{
    param->typeExtra = 0;
    if ( type == "int" || type == "long" ) {
	param->type = &static_QUType_int;
    } else if ( type == "bool" ) {
	param->type = &static_QUType_bool;
    } else if ( type == "QString" || type == "const QString&" ) {
	param->type = &static_QUType_QString;
    } else if ( type == "double" ) {
	param->type = &static_QUType_double;
    } else if ( type == "QVariant" || type == "const QVariant&" ) {
	param->type = &static_QUType_QVariant;
    } else if ( type == "IUnknown*" ) {
	param->type = &static_QUType_iface;
	param->typeExtra = "QUnknownInterface";
    } else if ( type == "IDispatch*" ) {
	param->type = &static_QUType_idisp;
	param->typeExtra = "QDispatchInterface";
    } else {
	param->type = &static_QUType_ptr;
	QString ptype = type;
	if ( ptype.right(1) == "*" )
	    ptype.remove( ptype.length()-1, 1 );
	param->typeExtra = new char[ ptype.length() + 1 ];
	param->typeExtra = qstrcpy( (char*)param->typeExtra, ptype );
    }
}

static inline void VARIANTToQUObject( VARIANT arg, QUObject *obj )
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
}

static inline VARIANT QVariantToVARIANT( const QVariant &var, const char *type = 0 )
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
    default:
	break;
    }

    return arg;
}

static inline QVariant VARIANTToQVariant( const VARIANT &arg )
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
	var = (int)arg.ulVal;
	break;
    case VT_INT:
	var = arg.intVal;
	break;
    case VT_UINT:
	var = arg.uintVal;
	break;
    case VT_DISPATCH: // IDispatch* -> int ###
	var = (int)arg.pdispVal;
	break;
    case VT_UNKNOWN: // IUnkonwn* -> int ###
	var = (int)arg.punkVal;
	break;
    case VT_EMPTY:
	// empty VARIANT type return
	break;
    default:
	break;
    }
    return var;
}

static inline void QVariantToQUObject( const QVariant &var, QUObject &obj )
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
    case QVariant::Font:
    case QVariant::Pixmap:
    case QVariant::Brush:
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::Color:
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
    if ( preset != &static_QUType_Null && preset != obj.type ) {
#ifndef QT_NO_DEBUG
	if ( !preset->canConvertFrom( &obj, obj.type ) ) {
	    qWarning( "Can't coerce QVariant to requested type (%s to %s)", obj.type->desc(), preset->desc() );
	} else 
#endif
	{
	    preset->convertFrom( &obj, obj.type );
	}
    }
}

static inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace( char x )
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar) x <= 32;
#else
    return isspace( (uchar) x );
#endif
}
// this is copied from qobject.cpp
static inline QCString qt_rmWS( const char *s )
{
    QCString result( qstrlen(s)+1 );
    char *d = result.data();
    char last = 0;
    while( *s && isSpace(*s) )			// skip leading space
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    result.truncate( (int)(d - result.data()) );
    int void_pos = result.find("(void)");
    if ( void_pos >= 0 )
	result.remove( void_pos+1, (uint)strlen("void") );
    return result;
}

#endif TYPES_H
