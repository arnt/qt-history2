// EduKeyGenerator.cpp : Implementation of CEduKeyGenerator
#include "stdafx.h"
#include "Licensekeys.h"
#include "EduKeyGenerator.h"

#include <qstring.h>
#include <qdatetime.h>
/////////////////////////////////////////////////////////////////////////////
// CEduKeyGenerator


static QString BSTR2QString( BSTR src )
{
    QString tmp;

    for( unsigned int i = 0; i < SysStringLen( src ); i++ ) {
	QChar c( src[ i ] );
	tmp += c;
    }
    return tmp;
}

static QString simplifyUserString( const QString& str )
{
    QString qsi( str.lower() );
    if( qsi.isEmpty() )
	qsi = "dummy";
    
    QString qso;
    for( int i = 0; i < (int)qsi.length(); i++ ) {
	QChar c = qsi[ i ];
	if( ( ( c >= 'a' ) && ( c <='z' ) ) || ( ( c >= '0' ) && ( c <='9' ) ) )
	    qso += c;
    }
    if ( qso.length() > 60 )
	qso.truncate( 60 );
    
    return qso;
}


static QString dateToString( const QDateTime& dt )
{
    const char * const weekdayNames[] ={
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    const char * const monthNames[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    QString dateString;

    // text like: Wed May 20 03:40:13 1998
    dateString.sprintf( "%s %s %2d %.2d:%.2d:%.2d %d", weekdayNames[ dt.date().dayOfWeek() - 1 ],
						       monthNames[ dt.date().month() - 1 ],
						       dt.date().day(),
						       dt.time().hour(),
						       dt.time().minute(),
						       dt.time().second(),
						       dt.date().year() );
    return dateString;
}

static QString trToSerialNo( const uchar* code )
{
    QString res;

    char cchars[] = "qmnbvcx2zlk3jhg5fdsa6poi9uy1rewt";

    for( int i = 0; i < 8; i++ ) {
	int r = 0;
	for( int j = 0; j < 5; j++ ) {
	    r <<= 1;
	    r |= (( code[j] >> i ) & 0x01);
	}
	res += cchars[r];
    }
    return res;
}

static QString trMakeChecksum( const QString& name, const QString& company, const QString& code )
{
    QString s = name + company + code;
    int crc = qChecksum( s.latin1(), s.length() );

    return QString( "%1" ).arg( crc, 4, 16 );
}

static QString trMakeSerialNo( const QString& company )
{
    QString gCompany = simplifyUserString( company );
    QString gName = simplifyUserString( "" );

    QDate exp = QDate::currentDate().addDays( 31 );

    QString ts = gName;
    ts += gCompany;
    ts += dateToString( exp );

    int crc = qChecksum( ts.latin1(), ts.length() );

    uchar y = ((Q_UINT8)(exp.year() - 1999 )) & 0x07;
    uchar m = ((Q_UINT8)exp.month()) & 0x0f;
    uchar d = ((Q_UINT8)exp.day()) & 0x1f;

    uchar code[5];

    code[0] = (uchar)(crc & 0xff);
    code[1] = (uchar)(( d << 3 ) | y);
    code[2] = (uchar)(QTime::currentTime().msec()) & 0xff;
    code[3] = (uchar)m;
    code[4] = (uchar)(crc >> 8 & 0xff);

    QString codeStr = trToSerialNo( code );

    QString checkStr = trMakeChecksum( gName, gCompany, codeStr );

    codeStr += checkStr;

    return codeStr;

}

STDMETHODIMP CEduKeyGenerator::newKey(BSTR institute, /*[out,retval]*/BSTR* key)
{
    QString s = trMakeSerialNo( BSTR2QString( institute ) );

//    s = "*" + s;
    *key = ::SysAllocString( (TCHAR*)qt_winTchar( s, TRUE ) );
    return S_OK;
}
