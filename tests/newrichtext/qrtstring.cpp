#include "qrtstring.h"


QRTString::QRTString()
{
}

QRTString::QRTString( const QString &str )
    : string( str ), formats( QRTFormat(), str.length() )
{
}

QRTString::QRTString( const QString &str, const QRTFormat &format )
    : string( str ), formats( format, str.length() )
{
}

QRTString::QRTString( const QRTString &other )
    : string( other.string ), formats( other.formats )
{
}

QRTString &QRTString::operator = ( const QRTString &other )
{
    string = other.string;
    formats = other.formats;
    return *this;
}

QRTString::~QRTString()
{
}

void QRTString::setFormat( const QRTFormat &format, unsigned int start, int length )
{
    if ( length == -1 || start + length > string.length() )
	length = string.length() - start;
    formats.set( start, length, format );
}

QRTFormat QRTString::format( unsigned int pos )
{
    return formats[pos];
}

QRTString &QRTString::insert( uint index, const QRTString &str )
{
    if ( &str == this ) {
	// ### protect against self assignement
	return *this;
    }
    if ( !str.length() )
	return *this;

    string.insert( index, str.string );
    formats.insert( index, str.formats );

    return *this;
}

/*! uses the format at position index
 */
QRTString &QRTString::insert( uint index, const QString &str )
{
    string.insert( index, str );
    formats.stringInsert( index, str.length() );
    return *this;
}

/*! uses the format at position index
 */
QRTString &QRTString::insert( uint index, QChar ch )
{
    string.insert( index, ch );
    formats.stringInsert( index, 1 );
    return *this;
}

QRTString &QRTString::remove( uint index, uint len )
{
    string.remove( index, len );
    formats.remove( index, len );
    return *this;
}

QRTString &QRTString::replace( uint index, uint len, const QRTString &str )
{
    remove( index, len );
    insert( index, str );
    return *this;
}
