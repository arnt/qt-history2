#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include <qstring.h>
#include <qpoint.h>
#include <qrect.h>
#include <qptrlist.h>
#include <qapplication.h>
#include <qtextstream.h>
#include <qdict.h>
#include <qpalette.h>

class QPainter;
class QRichTextFormat;
class QParagraph;
class QTextArea;
class QTextEditFormat;
class QTextAreaCursor;

// ### move to qnamespace for 3.0
enum HAlignment { AlignAuto, AlignLeft, AlignRight, AlignJustify };


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
	Char() : f( 0 ) {}
	~Char();
	QChar c;
	ushort x;
	void setFormat(QRichTextFormat *fmt);
	QRichTextFormat *format() const { return f; }
    private:
	QRichTextFormat *f;
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
    QTextRow(QRichTextString *text, int from, int length, QTextRow *previous, int baseline, int width);
    QTextRow(QRichTextString *text, QTextRow *previous);
    virtual ~QTextRow();

    void layout();

    QTextRow *prev() const { return p; }
    QTextRow *next() const { return n; }
    void setPrev(QTextRow *l) { p = l; }
    void setNext(QTextRow *l) { n = l; }

    QBidiContext *startEmbedding() { return startEmbed; }
    QBidiContext *endEmbedding() { return endEmbed; }

    void setFrom( int f ) { start = f; }
    int from() const { return start; }
    void setLength( int l ) { len = l; }
    int length() const { return len; }

    virtual void paint(QPainter &p, int x, int y, QTextAreaCursor *, HAlignment = AlignAuto);

    void setPosition(int _x, int _y);
    int width() const { return bRect.width(); }
    int height() const { return bRect.height(); }
    int x() const { return bRect.x(); }
    int y() const { return bRect.y(); }

    void setTextWidth( int w ) { tw = w; }
    int textWidth() const { return tw; }

    void setBaseline( int b ) { bl = b; }
    int baseline() const { return bl; }

    void setBoundingRect(const QRect &r);
    QRect boundingRect();

    bool hasComplexText() const { return complexText; }

    int visualPosition(int logicalPosition) const;
    int logicalPosition(int visualPosition) const;
    
private:
    bool checkComplexText();
    int bidiReorderLine(int pos = -1, bool logicalToVisual = true);
    void drawBuffer( QPainter &painter, int x, int y, const QString &buffer, int startX,
		     int bw, bool drawSelections,
		     QRichTextFormat *lastFormat, int i, int *selectionStarts,
		     int *selectionEnds, const QColorGroup &cg );

    bool complexText : 1;

    QBidiContext *startEmbed;
    QBidiContext *endEmbed;
    QBidiStatus bidiStatus;
    int start;
    short len;
    short bl;
    short tw;
    QRichTextString *text;
    QRichTextString reorderedText;
    QRect bRect;

    QTextRow *p, *n;

};

// =================================================================

class QParagraph {
public:
    QParagraph(const QRichTextString &, QTextArea *, QParagraph *last = 0);
    virtual ~QParagraph();

    QRect boundingRect() const { return bRect; }
    QPoint nextLine() const;

    void paint(QPainter &p, int x, int y, QTextAreaCursor *c);
    QRichTextString *string() { return &text; }

    QParagraph *prev() const { return p; }
    QParagraph *next() const { return n; }
    void setPrev( QParagraph *prev ) { p = prev; }
    void setNext( QParagraph *next ) { n = next; }

    QTextRow *first() const { return firstRow; }
    QTextRow *last() const { return lastRow; }
    void setFirst(QTextRow *r) { firstRow = r; }
    void setLast(QTextRow *r) { lastRow = r; }
    int x() const { return xPos; }
    int y() const { return yPos; }

    void setHAlignment( HAlignment a ) { hAlign = a; }
    HAlignment hAlignment() const { return hAlign; }

    QChar::Direction basicDirection() const;

    int insert(int idx, const QString &str);
    
    virtual void layout();

private:
    QTextArea *area;
    QTextRow *firstRow;
    QTextRow *lastRow;
    QParagraph *p, *n;

    QRichTextString text;
    mutable QChar::Direction basicDir;

    int xPos;
    int yPos;
    QRect bRect;

    HAlignment hAlign;
};

// =================================================================

class QTextArea {
public:
    QTextArea();
    QTextArea(int width);
    virtual ~QTextArea();

    virtual QRect lineRect(int x, int y, int h = -1) const;

    void appendParagraph(const QRichTextString &);
    void insertParagraph(const QRichTextString &, int pos);
    void removeParagraph(int pos);

    QParagraph *firstParagraph() const;
    QParagraph *lastParagraph() const;

    virtual QParagraph *createParagraph(const QRichTextString &text, QParagraph *before);

    void paint(QPainter &p, int x, int y, QTextAreaCursor *c = 0);

private:
    int width;
    QParagraph *first, *last;
};

// =================================================================

class QTextAreaCursor
{
public:
    QTextAreaCursor( QTextArea *);

    QTextRow *row() const { return line; }

    QParagraph *paragraph() const { return parag; }
    void setParagraph( QParagraph *s );

    void gotoLeft();
    void gotoRight();
    void gotoUp();
    void gotoDown();
    void gotoLineEnd();
    void gotoLineStart();
    void gotoHome();
    void gotoEnd();
    void gotoPageUp();
    void gotoPageDown();
    void gotoWordLeft();
    void gotoWordRight();

    void insert( const QString &s, bool checkNewLine );
    void splitAndInsertEmtyParag( bool ind = TRUE, bool updateIds = TRUE );
    bool remove();
    void indent();

    bool atParagStart();
    bool atParagEnd();

    int index() const;
    void setIndex( int i );

    int visualIndex() const;
    
    bool checkParens();
    void checkIndex();

private:
    bool checkOpenParen();
    bool checkClosedParen();

    QTextRow *line;
    QParagraph *parag;
    QTextArea *area;
    int idx, tmpIndex;
    int visual1, visual2;
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

// =========================================================================

class QRichTextFormatter
{
public:
    QRichTextFormatter( QTextArea *a );
    virtual ~QRichTextFormatter() {}
    virtual int format( QParagraph *parag, int start ) = 0;
    virtual QTextRow *newLine(QParagraph *p, QTextRow *previous);
    QRect openLine(QParagraph *p, QTextRow *line, int from, int height = 0);
    bool closeLine(QParagraph *p, QTextRow *line, int to, int height, int baseline, int width);

protected:
    QTextArea *area;

};

// =========================================================================

class QRichTextFormatterBreakWords : public QRichTextFormatter
{
public:
    QRichTextFormatterBreakWords( QTextArea *a );
    int format( QParagraph *parag, int start );

};

#endif
