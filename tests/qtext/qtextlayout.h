#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include <qstring.h>
#include <qpoint.h>
#include <qrect.h>
#include <qlist.h>
#include <qapplication.h>
#include <qtextstream.h>
#include <qdict.h>

class QPainter;
class QRichTextFormat;
class QParagraph;

class QBidiContext {
public:
    QBidiContext(unsigned char level, QChar::Direction embedding, QBidiContext *parent = 0, bool override = false);
    ~QBidiContext();

    void ref() const;
    void deref() const;

    unsigned char level;
    bool override : 1;
    QChar::Direction dir : 5;

    QBidiContext *parent;

    // refcounting....
    mutable int count;
};

struct QBidiStatus {
    QBidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
    }
    QChar::Direction eor 		: 5;
    QChar::Direction lastStrong 	: 5;
    QChar::Direction last		: 5;
};


// =======================================================================

class QRichTextString
{
    friend class QParagraph;
public:
    struct Char {
    public:
	Char() : format( 0 ), lineStart( 0 ) {}
	~Char() { format = 0; }
	QChar c;
	ushort x;
	QRichTextFormat *format;
	uint lineStart : 1;
    };

    QRichTextString();
    QRichTextString(const QString &str, QRichTextFormat *f);
    QRichTextString(const QRichTextString &);
    QRichTextString &operator = (const QRichTextString &);
    
    QString toString() const;

    Char &at( int i ) const;
    int length() const;

    void insert( int index, const QString &s, QRichTextFormat *f );
    void truncate( int index );
    void remove( int index, int len );
    void append(Char c);
    void clear();
    void setLength(int newLen);

    void setFormat( int index, QRichTextFormat *f, bool useCollection );

private:
    Char *data;
    int len;
    int maxLen;
    QString cache;

};


// =======================================================================

class QTextRow {
public:
    QTextRow(const QRichTextString &text, int from, int length, QTextRow *previous);
    virtual ~QTextRow();

    QTextRow *previousLine() const { return prev; }
    QTextRow *nextLine() const { return next; }
    void setPreviousLine(QTextRow *l) { prev = l; }
    void setNextLine(QTextRow *l) { next = l; }

    QBidiContext *startEmbedding() { return startEmbed; }
    QBidiContext *endEmbedding() { return endEmbed; }
    int from() { return start; }
    int length() { return len; }

    virtual void paint(QPainter *p, int x, int y);

    void setPosition(int _x, int _y);
    int width() const { return w; }
    int height() const { return h; }
    int x() const { return xPos; }
    int y() const { return yPos; }

    void setBoundingRect(const QRect &r);
    QRect boundingRect();

private:
    bool hasComplexText();
    void bidiReorderLine();

    bool complexText : 1;

    QBidiContext *startEmbed;
    QBidiContext *endEmbed;
    QBidiStatus bidiStatus;
    int start;
    short len;
    int w;
    short h;
    int xPos;
    int yPos;
    QRichTextString text;
    QRichTextString reorderedText;

    QTextRow *prev;
    QTextRow *next;
};

class QTextArea;

class QParagraph {
public:
    QParagraph(const QRichTextString &, QTextArea *, QParagraph *last = 0);
    virtual ~QParagraph();

    QRect boundingRect() const { return bRect; }
    QPoint nextLine() const;

    void paint(QPainter *p, int x, int y);

protected:
    virtual void layout();

    int findLineBreak(int pos);
    void addLine(int pos, int length);

private:
    QTextArea *area;
    QTextRow *first;
    QTextRow *last;

    QRichTextString text;

    int xPos;
    int yPos;
    QRect bRect;
};


class QTextArea {
public:
    QTextArea();
    QTextArea(int width);
    virtual ~QTextArea();

    virtual int lineWidth(int x, int y, int h = 0) const;
    virtual QRect lineRect(int x, int y, int h) const;

    void appendParagraph(const QRichTextString &);
    void insertParagraph(const QRichTextString &, int pos);
    void removeParagraph(int pos);

    virtual QParagraph *createParagraph(const QRichTextString &text, QParagraph *before);

    void paint(QPainter *p, int x, int y);

private:
    int width;
    QList<QParagraph> paragraphs;
};


// =======================================================================

class QRichTextFormat
{
    friend class QRichTextFormatCollection;

public:
    enum Flags {
	Bold = 1,
	Italic = 2,
	Underline = 4,
	Family = 8,
	Size = 16,
	Color = 32,
	Font = Bold | Italic | Underline | Family | Size,
	Format = Font | Color
    };

    QRichTextFormat( const QFont &f, const QColor &c );
    QRichTextFormat( const QRichTextFormat &fm );
    QColor color() const;
    QFont font() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int width( const QChar &c ) const;
    int height() const;
    int ascent() const;
    int descent() const;

    void setBold( bool b );
    void setItalic( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setFont( const QFont &f );
    void setColor( const QColor &c );

    bool operator==( const QRichTextFormat &f ) const;
    QRichTextFormatCollection *parent() const;
    QString key() const;

    static QString getKey( const QFont &f, const QColor &c );

    void addRef();
    void removeRef();

    QString makeFormatChangeTags( QRichTextFormat *f ) const;
    QString makeFormatEndTags() const;

private:
    void update();
    void generateKey();
    const QFontMetrics *fontMetrics() const;
    QRichTextFormat() {}

private:
    QFont fn;
    QColor col;
    QFontMetrics *fm;
    int leftBearing, rightBearing;
    int widths[ 65536 ];
    int hei, asc, dsc;
    QRichTextFormatCollection *collection;
    int ref;
    QString k;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QRichTextFormatCollection
{
public:
    QRichTextFormatCollection();

    void setDefaultFormat( QRichTextFormat *f );
    QRichTextFormat *defaultFormat() const;
    QRichTextFormat *format( QRichTextFormat *f );
    QRichTextFormat *format( QRichTextFormat *of, QRichTextFormat *nf, int flags );
    QRichTextFormat *format( const QFont &f, const QColor &c );
    void remove( QRichTextFormat *f );

    void debug();

private:
    QRichTextFormat *defFormat, *lastFormat, *cachedFormat;
    QDict<QRichTextFormat> cKey;
    QRichTextFormat *cres;
    QFont cfont;
    QColor ccol;
    QString kof, knf;
    int cflags;

};

// =======================================================================

inline QRichTextString::QRichTextString(const QRichTextString &other)
{
    cache = other.cache;
    len = other.len;
    maxLen = len;
    data = new Char[len];
    memcpy(data, other.data, len*sizeof(Char));    
}
    
inline QRichTextString &QRichTextString::operator = (const QRichTextString &other)
{
    delete [] data;
    cache = other.cache;
    len = other.len;
    maxLen = len;
    data = new Char[len];
    memcpy(data, other.data, len*sizeof(Char));    
    return *this;
}

inline QRichTextString::Char &QRichTextString::at( int i ) const
{
    return data[ i ];
}

inline QString QRichTextString::toString() const
{
    return cache;
}

inline int QRichTextString::length() const
{
    return len;
}

inline void QRichTextString::append(Char c)
{
    if(len + 1 >= maxLen)
	setLength(2*maxLen);
    data[len] = c;
    cache += c.c;
    len++;
}

inline void QRichTextString::setLength(int newLen)
{
    Char *newData = new Char[newLen];
    memcpy(newData, data, len*sizeof(Char));
    delete [] data;
    data = newData;
    maxLen = newLen;
}


inline void QRichTextString::clear()
{
    delete [] data;
    data = 0;
}

// ========================================================================


inline QRichTextFormat::QRichTextFormat( const QFont &f, const QColor &c )
    : fn( f ), col( c ), fm( new QFontMetrics( f ) )
{
    leftBearing = fm->minLeftBearing();
    rightBearing = fm->minRightBearing();
    hei = fm->height();
    asc = fm->ascent();
    dsc = fm->descent();
    for ( int i = 0; i < 65536; ++i )
	widths[ i ] = 0;
    generateKey();
    addRef();
}

inline QRichTextFormat::QRichTextFormat( const QRichTextFormat &f )
{
    fn = f.fn;
    col = f.col;
    fm = new QFontMetrics( fn );
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    for ( int i = 0; i < 65536; ++i )
	widths[ i ] = f.widths[ i ];
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    generateKey();
    addRef();
}

inline void QRichTextFormat::update()
{
    *fm = QFontMetrics( fn );
    leftBearing = fm->minLeftBearing();
    rightBearing = fm->minRightBearing();
    hei = fm->height();
    asc = fm->ascent();
    dsc = fm->descent();
    for ( int i = 0; i < 65536; ++i )
	widths[ i ] = 0;
    generateKey();
}

inline const QFontMetrics *QRichTextFormat::fontMetrics() const
{
    return fm;
}

inline QColor QRichTextFormat::color() const
{
    return col;
}

inline QFont QRichTextFormat::font() const
{
    return fn;
}

inline int QRichTextFormat::minLeftBearing() const
{
    return leftBearing;
}

inline int QRichTextFormat::minRightBearing() const
{
    return rightBearing;
}

inline int QRichTextFormat::width( const QChar &c ) const
{
    if ( c == '\t' )
	return 30;
    int w = widths[ c.unicode() ];
    if ( w == 0 ) {
	w = fm->width( c );
	( (QRichTextFormat*)this )->widths[ c.unicode() ] = w;
    }
    return w;
}

inline int QRichTextFormat::height() const
{
    return hei;
}

inline int QRichTextFormat::ascent() const
{
    return asc;
}

inline int QRichTextFormat::descent() const
{
    return dsc;
}

inline bool QRichTextFormat::operator==( const QRichTextFormat &f ) const
{
    return k == f.k;
}

inline QRichTextFormatCollection *QRichTextFormat::parent() const
{
    return collection;
}

inline void QRichTextFormat::addRef()
{
    ref++;
#ifdef DEBUG_COLLECTION
    qDebug( "add ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
}

inline void QRichTextFormat::removeRef()
{
    ref--;
    if ( !collection )
	return;
#ifdef DEBUG_COLLECTION
    qDebug( "remove ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
    if ( ref == 0 )
	collection->remove( this );
}

inline QString QRichTextFormat::key() const
{
    return k;
}

inline void QRichTextFormat::generateKey()
{
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family();
}

inline QString QRichTextFormat::getKey( const QFont &fn, const QColor &col )
{
    QString k;
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family();
    return k;
}

// ===============================================================================

inline void QRichTextFormatCollection::setDefaultFormat( QRichTextFormat *f )
{
    defFormat = f;
}

inline QRichTextFormat *QRichTextFormatCollection::defaultFormat() const
{
    return defFormat;
}


#endif
