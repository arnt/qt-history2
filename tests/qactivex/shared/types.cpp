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

// those functions are not ours
static int monthdays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
#define HALF_SECOND  (1.0/172800.0)

QDateTime DATEToQDateTime( DATE ole )
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

DATE QDateTimeToDATE( const QDateTime &dt )
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

void QVariantToQUObject( const QVariant &var, QUObject &obj )
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
#ifndef QT_NO_DEBUG
	if ( !preset->canConvertFrom( &obj, obj.type ) ) {
	    if ( QUType::isEqual( preset, &static_QUType_ptr ) )
		qWarning( "QVariant does not support pointer types" );
	    else
		qWarning( "Can't coerce QVariant to requested type (%s to %s)", obj.type->desc(), preset->desc() );
	} else 
#endif
	{
	    preset->convertFrom( &obj, obj.type );
	}
    }
}
