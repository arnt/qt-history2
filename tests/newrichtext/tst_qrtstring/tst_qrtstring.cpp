// tst_QString.cpp: implementation of the tst_QString class.
//
//////////////////////////////////////////////////////////////////////

#include <qregexp.h>

#include "tst_qrtstring.h"
#include "../qrtformat.h"
#include "../qrtstring.h"

tst_QRTString::tst_QRTString( int argc, char* argv[] ) : QTestCase( argc, argv )
{
}

tst_QRTString::~tst_QRTString()
{
}

void tst_QRTString::init()
{
}

void tst_QRTString::cleanup()
{
}

QTestTable* tst_QRTString::createTestTable( const QString &funcName )
{
    if ( funcName == "setFormat" ||
	 funcName == "length" ||
	 funcName == "charAt" ||
	 funcName == "qstring" ||
	 funcName == "format" )
	return table1();
    return QTestCase::createTestTable( funcName );
}

QTestTable* tst_QRTString::table1()
{
    QTestTable *t = new QTestTable( this );
    t->defineElement( "QString", "str" );
    t->defineElement( "int", "from" );
    t->defineElement( "int", "length" );
    t->defineElement( "QColor", "color" );

    *NEW_DATA( t, "null_0" )  << QString::null << 0 << 0 << QColor( red );
    *NEW_DATA( t, "null_1" )  << QString::null << 0 << -1 << QColor( red );
    *NEW_DATA( t, "null_2" )  << QString::null << 1 << 0 << QColor( red );
    *NEW_DATA( t, "null_3" )  << QString::null << 0 << 1 << QColor( red );
    *NEW_DATA( t, "null_4" )  << QString::null << 1 << 1 << QColor( red );
    *NEW_DATA( t, "empty_0" )  << "" << 0 << 0 << QColor( red );
    *NEW_DATA( t, "empty_1" )  << "" << 0 << -1 << QColor( red );
    *NEW_DATA( t, "empty_2" )  << "" << 1 << 0 << QColor( red );
    *NEW_DATA( t, "empty_3" )  << "" << 0 << 1 << QColor( red );
    *NEW_DATA( t, "empty_4" )  << "" << 1 << 1 << QColor( red );

    *NEW_DATA( t, "data_0" )  << "test" << 0 << 0 << QColor( red );
    *NEW_DATA( t, "data_1" )  << "test" << 0 << -1 << QColor( red );
    *NEW_DATA( t, "data_2" )  << "test" << 1 << 0 << QColor( red );
    *NEW_DATA( t, "data_3" )  << "test" << 0 << 1 << QColor( red );
    *NEW_DATA( t, "data_4" )  << "test" << 1 << 1 << QColor( red );

    return t;
}


//     int length() const { return string.length(); }
void tst_QRTString::length( QTestData *data )
{
    {
	FETCH( QString, str );

	QRTString string( str );
	COMPARE( string.length(), (int)str.length() );
    }
    COMPARE( QRTFormat::statistics(),  TRUE );
}

//     QChar charAt( unsigned int pos ) const { return string.unicode()[pos]; }
void tst_QRTString::charAt( QTestData *data )
{
    {
	FETCH( QString, str );

	QRTString string( str );
	for ( int i = 0; i < str.length(); i++ )
	    VERIFY( string.charAt( i ) == str.at( i ) );

    }
    COMPARE( QRTFormat::statistics(),  TRUE );
}


//     const QString &str() const { return string; }
void tst_QRTString::qstring( QTestData *data )
{
    {
	FETCH( QString, str );

	QRTString string( str );
	COMPARE( string.qstring(), str );

    }
    COMPARE( QRTFormat::statistics(),  TRUE );
}


//     void setFormat( const QRTFormat &, unsigned int start = 0,  int length = -1 );
void tst_QRTString::setFormat( QTestData *data)
{
    // we do this in a scope of it's own, so we can check the refcounting of the formats afterwards.
    {
	FETCH( QString, str );
	FETCH( int, from );
	FETCH( int, length );
	FETCH( QColor, color );
	QRTFormat format;
	format.setColor( color );

	QRTString string( str );
	int len = string.length();
	string.setFormat( format, from, length );
	COMPARE( len, string.length() );

	for ( int i = 0; i < len; i++ ) {
	    QRTFormat f = string.format( i );
	    if ( i >= from && (length == -1 || i < from + length) )
		COMPARE( f.color(), color );
	    else
		COMPARE( f.color(), QColor( Qt::black ) );
	}
    }
    COMPARE( QRTFormat::statistics(),  TRUE );
}

//     QRTFormat format( unsigned int pos );
void tst_QRTString::format( QTestData *data)
{
    // ### rather similar to setFormat, maybe make more distinct
    {
	FETCH( QString, str );
	FETCH( int, from );
	FETCH( int, length );

	QRTString string( str );
	int len = string.length();

	for ( int i = 0; i < len; i++ ) {
	    QRTFormat f = string.format( i );
	    VERIFY( f == QRTFormat() );
	}

	FETCH( QColor, color );
	QRTFormat format;
	format.setColor( color );
	string.setFormat( format, from, length );
	COMPARE( len, string.length() );

	for ( int i = 0; i < len; i++ ) {
	    QRTFormat f = string.format( i );
	    if ( i >= from && (length == -1 || i < from + length) )
		VERIFY( f == format );
	    else
		VERIFY( f == QRTFormat() );
	}

    }
    COMPARE( QRTFormat::statistics(),  TRUE );
}

//     QRTString &insert( uint index, const QRTString & );
void tst_QRTString::insert_qrtstring( QTestData *)
{
}

//     QRTString &insert( uint index, const QString & );
void tst_QRTString::insert_qstring( QTestData *)
{
}

//     QRTString &insert( uint index, QChar );
void tst_QRTString::insert_qchar( QTestData *)
{
}

//     QRTString &remove( uint index, uint len );
void tst_QRTString::remove( QTestData *)
{
}

//     QRTString &replace( uint index, uint len, const QRTString & );
void tst_QRTString::replace( QTestData *)
{
}
