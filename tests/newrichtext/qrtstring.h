#ifndef QRTSTRING_H
#define QRTSTRING_H

#include <qstring.h>

#include "qrtformat.h"

class QRTString
{
public:
    QRTString();
    QRTString( const QString & );
    QRTString( const QString &, const QRTFormat &format );

    QRTString( const QRTString & );
    QRTString &operator = ( const QRTString & );

    ~QRTString();

    void setFormat( const QRTFormat &, unsigned int start = 0,  int length = -1 );
    QRTFormat format( unsigned int pos );

    QRTString &insert( uint index, const QRTString & );
    QRTString &insert( uint index, const QString & );
    QRTString &insert( uint index, QChar );

    QRTString &remove( uint index, uint len );

    QRTString &replace( uint index, uint len, const QRTString & );

    int length() const { return string.length(); }
    QChar charAt( unsigned int pos ) const { return string.unicode()[pos]; }

    const QString &str() const { return string; }

private:
    QString string;
    QRTFormatArray formats;

#if 0
    // I think this should be left in the paragraph, as it has nothing to do with a RT string.
    struct LayoutHint {
	int pos;
	short x : 16;
	short y : 16;
    };
    QMemArray<LayoutHint> layoutHints;
#endif
};


#endif
