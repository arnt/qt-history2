#ifndef QRTFORMAT_H
#define QRTFORMAT_H

#include <qfont.h>
#include <qcolor.h>

class QRTFormatPrivate;

class QRTFormat
{
public:
    QRTFormat();
    QRTFormat( const QRTFormat & );
    QRTFormat & operator = ( const QRTFormat & );
    ~QRTFormat();

    QRTFormat( const QFont &f, const QColor &c );

    void setFont( const QFont & );
    const QFont &font() const;

    void setColor( const QColor & );
    const QColor &color() const;

    bool operator == ( const QRTFormat &other ) const { return d == other.d; }
    bool operator != ( const QRTFormat &other ) const { return d != other.d; }

    enum VerticalAlignment { AlignNormal, AlignSuperScript, AlignSubScript };

    static void statistics();
private:
    friend class QRTFormatArray;
    QRTFormat( QRTFormatPrivate *p );
    QRTFormatPrivate *d;
};

// the real class holding the data. Shared and refcounted.
class QRTFormatPrivate
{
public:
    QRTFormatPrivate() : refCount( 0 ) {}
    QRTFormatPrivate( const QFont &f, const QColor &c )
	: font( f ), color( c ), refCount( 0 ) {}
    void ref() { refCount++; }
    void deref() {
	refCount--;
	if ( refCount == 0 )
	    deleteMe();
    }

    QFont font;
    QColor color;
    QRTFormat::VerticalAlignment va;
    int refCount;
    void deleteMe();
};

inline QRTFormat::QRTFormat( QRTFormatPrivate *p ) : d(p) { d->ref(); }

class QRTFormatArray
{
public:
    QRTFormatArray();
    QRTFormatArray( const QRTFormat &f, int len );

    QRTFormatArray( const QRTFormatArray &other );
    QRTFormatArray &operator =( const QRTFormatArray &other );

    ~QRTFormatArray();

    void insert( int pos, int len, const QRTFormat & );
    void insert( int pos, const QRTFormatArray & );
    void stringInsert( int pos,  int length );

    void remove( int pos, int len );

    void set( int pos,  int len, const QRTFormat & );

    QRTFormat operator [] (int pos) const;

    int numFormats() const { return size; }

private:
    struct FormatHint {
	unsigned int length;
	QRTFormatPrivate *format;
    };
    FormatHint *formatHints;
    int size;
    int alloc;

    void resizeFormatHints( int size );
    void duplicateFormatHints( FormatHint *hints, int num );
    void derefFormatHints();
    void removeFormatHints( int from, int num );
    void insertFormatHints( int at, int num, QRTFormatPrivate *f = 0 );

    void debug( const char *prefix );
};

#endif
