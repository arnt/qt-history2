#ifndef QTEXTEDITINTERN_H
#define QTEXTEDITINTERN_H

#include <qstring.h>
#include <qlist.h>
#include <qrect.h>
#include <qfontmetrics.h>
#include <qintdict.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qfont.h>
#include <qcolor.h>
#include <qsize.h>
#include <qvaluelist.h>
#include <qvaluestack.h>
#include <qobject.h>
#include <qdict.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qstylesheet.h>
#include <qvector.h>
#include <qpainter.h>
#include <qlayout.h>

#include <limits.h>

//#define DEBUG_COLLECTION

class QTextDocument;
class QTextCommand;
class QTextEdit;
class QTextString;
class QTextSyntaxHighlighter;
class QTextCommandHistory;
class QTextFormat;
class QTextCursor;
class QTextParag;
class QTextFormatter;
class QTextIndent;
class QTextFormatCollection;
class QStyleSheetItem;
class QTextCustomItem;
class QTextFlow;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextCursor
{
public:
    QTextCursor( QTextDocument *d );

    QTextDocument *document() const { return doc; }

    QTextParag *parag() const;
    int index() const;
    void setParag( QTextParag *s, bool restore = TRUE );

    void gotoLeft();
    void gotoRight();
    void gotoUp();
    void gotoDown();
    void gotoLineEnd();
    void gotoLineStart();
    void gotoHome();
    void gotoEnd();
    void gotoPageUp( QTextEdit *view );
    void gotoPageDown( QTextEdit *view );
    void gotoWordLeft();
    void gotoWordRight();

    void insert( const QString &s, bool checkNewLine );
    void splitAndInsertEmtyParag( bool ind = TRUE, bool updateIds = TRUE );
    bool remove();
    void indent();

    bool atParagStart();
    bool atParagEnd();

    void setIndex( int i, bool restore = TRUE );

    bool checkParens();
    void checkIndex();

    int offsetX() const { return ox; }
    int offsetY() const { return oy; }

    QTextParag *topParag() const { return parags.isEmpty() ? string : parags.first(); }
    int totalOffsetX() const;
    int totalOffsetY() const;

    void place( const QPoint &pos, QTextParag *s );
    void restoreState();

private:
    enum Operation { EnterBegin, EnterEnd, Next, Prev, Up, Down };

    bool checkOpenParen();
    bool checkClosedParen();

    void push();
    void pop();
    void processNesting( Operation op );
    void invalidateNested();
    void gotoIntoNested( const QPoint &globalPos );

    QTextParag *string;
    QTextDocument *doc;
    int idx, tmpIndex;
    int ox, oy;
    QValueStack<int> indices;
    QValueStack<QTextParag*> parags;
    QValueStack<int> xOffsets;
    QValueStack<int> yOffsets;
    QValueStack<bool> nestedStack;
    bool nested;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextCommandHistory
{
public:
    QTextCommandHistory( int s ) : current( -1 ), steps( s ) { history.setAutoDelete( TRUE ); }

    void addCommand( QTextCommand *cmd );
    QTextCursor *undo( QTextCursor *c );
    QTextCursor *redo( QTextCursor *c );

private:
    QList<QTextCommand> history;
    int current, steps;

};

class QTextCommand
{
public:
    enum Commands { Invalid, Insert, Delete, Format };
    QTextCommand( QTextDocument *d ) : doc( d ), cursor( d ) {}
    virtual ~QTextCommand() {}
    virtual Commands type() const { return Invalid; };

    virtual QTextCursor *execute( QTextCursor *c ) = 0;
    virtual QTextCursor *unexecute( QTextCursor *c ) = 0;

protected:
    QTextDocument *doc;
    QTextCursor cursor;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextDeleteCommand : public QTextCommand
{
public:
    QTextDeleteCommand( QTextDocument *d, int i, int idx, const QString &str )
	: QTextCommand( d ), id( i ), index( idx ), text( str ) {}
    virtual Commands type() const { return Delete; };

    virtual QTextCursor *execute( QTextCursor *c );
    virtual QTextCursor *unexecute( QTextCursor *c );

protected:
    int id, index;
    QString text;

};

class QTextInsertCommand : public QTextDeleteCommand
{
public:
    QTextInsertCommand( QTextDocument *d, int i, int idx, const QString &str )
	: QTextDeleteCommand( d, i, idx, str ) {}
    Commands type() const { return Insert; };

    virtual QTextCursor *execute( QTextCursor *c ) { return QTextDeleteCommand::unexecute( c ); }
    virtual QTextCursor *unexecute( QTextCursor *c ) { return QTextDeleteCommand::execute( c ); }

};

class QTextFormatCommand : public QTextCommand
{
public:
    QTextFormatCommand( QTextDocument *d, int selId, QTextFormat *f, int flags );
    ~QTextFormatCommand();
    Commands type() const { return Format; }

    virtual QTextCursor *execute( QTextCursor *c );
    virtual QTextCursor *unexecute( QTextCursor *c );

protected:
    int selection;
    QTextFormat *format;
    int flags;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class QTextTableCell;
class QTextParag;
class QTextDocument
{
    friend class QTextTableCell;
    friend class QTextCursor;
    friend class QTextEdit;
    friend class QTextParag;

public:
    enum SelectionIds {
	Standard = 0,
	ParenMismatch,
	ParenMatch,
	Search,
	Temp // This selection must not be drawn, it's used e.g. by undo/redo to
	// remove multiple lines with removeSelectedText()
    };

    static const int numSelections;

    enum Bullet {
	FilledCircle,
	FilledSquare,
	OutlinedCircle,
	OutlinedSquare
    };

    QTextDocument( QTextDocument *p );

    QTextDocument *parent() const { return par; }

    void setText( const QString &text, const QString &context, bool tabify = FALSE );
    void load( const QString &fn, bool tabify = FALSE );
    QMap<QString, QString> attributes() const { return attribs; }
    void setAttributes( const QMap<QString, QString> &attr ) { attribs = attr; }

    void save( const QString &fn = QString::null, bool untabify = FALSE );
    QString fileName() const;
    QString text( bool untabify = FALSE ) const;
    QString text( int parag, bool formatted ) const;

    int x() const;
    int y() const;
    int width() const;
    int widthUsed() const;
    int height() const;
    void setWidth( int w );
    int minimumWidth() const;
    bool setMinimumWidth( int w, QTextParag *parag );

    QTextParag *firstParag() const;
    QTextParag *lastParag() const;
    void setFirstParag( QTextParag *p );
    void setLastParag( QTextParag *p );

    void invalidate();

    void setSyntaxHighlighter( QTextSyntaxHighlighter *sh );
    QTextSyntaxHighlighter *syntaxHighlighter() const;

    void setFormatter( QTextFormatter *f );
    QTextFormatter *formatter() const;

    void setIndent( QTextIndent *i );
    QTextIndent *indent() const;

    void setParenCheckingEnabled( bool b );
    bool isParenCheckingEnabled() const;

    QColor selectionColor( int id ) const;
    bool invertSelectionText( int id ) const;
    bool hasSelection( int id ) const;
    void setSelectionStart( int id, QTextCursor *cursor );
    bool setSelectionEnd( int id, QTextCursor *cursor );
    bool removeSelection( int id );
    void selectionStart( int id, int &paragId, int &index );
    void selectionEnd( int id, int &paragId, int &index );
    void setFormat( int id, QTextFormat *f, int flags );
    QTextParag *selectionStart( int id );
    QTextParag *selectionEnd( int id );

    QString selectedText( int id ) const;
    void copySelectedText( int id );
    void removeSelectedText( int id, QTextCursor *cursor );
    void indentSelection( int id );

    void setCompletionEnabled( bool b );
    bool isCompletionEnabled() const;

    void addCompletionEntry( const QString &s );
    QStringList completionList( const QString &s ) const;

    void updateCompletionMap();

    QTextParag *paragAt( int i ) const;

    void addCommand( QTextCommand *cmd );
    QTextCursor *undo( QTextCursor *c = 0 );
    QTextCursor *redo( QTextCursor *c  = 0 );

    QTextFormatCollection *formatCollection() const;

    int listIndent( int depth ) const;
    Bullet bullet( int depth ) const;

    bool find( const QString &expr, bool cs, bool wo, bool forward, int *parag, int *index, QTextCursor *cursor );

    void setTextFormat( Qt::TextFormat f );
    Qt::TextFormat textFormat() const;

    bool inSelection( int selId, const QPoint &pos ) const;

    const QStyleSheet *styleSheet() const { return sheet_; }
    const QMimeSourceFactory *mimeSourceFactory() const { return factory_; }
    QString context() const { return contxt; }

    void setStyleSheet( const QStyleSheet *s ) { if ( s ) sheet_ = s; }
    void setMimeSourceFactory( const QMimeSourceFactory *f ) { if ( f ) factory_ = f; }
    void setContext( const QString &c ) { if ( !c.isEmpty() ) contxt = c; }

    void setLinkColor( const QColor &c ) { linkC = c; }
    QColor linkColor() const { return linkC; }

    void setUnderlineLinks( bool b ) { underlLinks = b; }
    bool underlineLinks() const { return underlLinks; }

    void setPaper( const QBrush *brush ) { backBrush = brush; }
    const QBrush *paper() { return backBrush; }

    void doLayout( QPainter *p, int w );
    void draw( QPainter *p, const QRegion &reg, const QColorGroup &cg, const QBrush *paper = 0 );
    void drawParag( QPainter *p, QTextParag *parag, int cx, int cy, int cw, int ch,
		    QPixmap *&doubleBuffer, const QColorGroup &cg,
		    bool drawCursor, QTextCursor *cursor );
    QTextParag *draw( QPainter *p, int cx, int cy, int cw, int ch, const QColorGroup &cg,
		      bool onlyChanged = FALSE, bool drawCursor = FALSE, QTextCursor *cursor = 0 );
    void setDefaultFont( const QFont &f );

    void registerCustomItem( QTextCustomItem *i, QTextParag *p );
    void unregisterCustomItem( QTextCustomItem *i, QTextParag *p );

    QTextFlow *flow() const { return flow_; }
    bool verticalBreak() const { return pages; }
    void setVerticalBreak( bool b ) { pages = b; }

    void setUseFormatCollection( bool b ) { useFC = b; }
    bool useFormatCollection() const { return useFC; }

    QTextTableCell *tableCell() const { return tc; }
    void setTableCell( QTextTableCell *c ) { tc = c; }

    void setPlainText( const QString &text, bool tabify = FALSE );
    void setRichText( const QString &text, const QString &context );
    QString richText( QTextParag *p = 0, bool formatted = FALSE ) const;
    QString plainText( QTextParag *p = 0, bool formatted = FALSE, bool untabify = FALSE ) const;

    bool focusNextPrevChild( bool next );

private:
    void clear();
    QPixmap *bufferPixmap( const QSize &s );
    // HTML parser
    bool hasPrefix(const QString& doc, int pos, QChar c);
    bool hasPrefix(const QString& doc, int pos, const QString& s);
    QTextCustomItem* parseTable( const QMap<QString, QString> &attr, const QTextFormat &fmt, const QString &doc, int& pos );
    bool eatSpace(const QString& doc, int& pos, bool includeNbsp = FALSE );
    bool eat(const QString& doc, int& pos, QChar c);
    QString parseOpenTag(const QString& doc, int& pos, QMap<QString, QString> &attr, bool& emptyTag);
    QString parseCloseTag( const QString& doc, int& pos );
    QChar parseHTMLSpecialChar(const QString& doc, int& pos);
    QString parseWord(const QString& doc, int& pos, bool lower = TRUE);
    QChar parseChar(const QString& doc, int& pos, QStyleSheetItem::WhiteSpaceMode wsm );

private:
    struct Selection {
	QTextParag *startParag, *endParag;
	int startIndex;
    };

    struct Focus {
	QTextParag *parag;
	int start, len;
	QString href;
    };

    int cx, cy, cw;
    QTextParag *fParag, *lParag;
    QTextSyntaxHighlighter *syntaxHighlighte;
    QMap<int, QColor> selectionColors;
    QMap<int, Selection> selections;
    QMap<int, bool> selectionText;
    QString filename;
    bool parenCheck, completion;
    QTextCommandHistory *commandHistory;
    QTextFormatter *pFormatter;
    QTextIndent *indenter;
    QTextFormatCollection *fCollection;
    Qt::TextFormat txtFormat;
    bool preferRichText;
    QTextFlow *flow_;
    QList<QTextCustomItem> customItems;
    bool pages;
    QTextDocument *par;
    bool useFC;
    QTextTableCell *tc;
    bool withoutDoubleBuffer;
    QTextCursor *tmpCursor;
    bool underlLinks;
    QColor linkC;
    const QBrush *backBrush;
    QPixmap *buf_pixmap;
    bool nextDoubleBuffered;
    Focus focusIndicator;
    int minw;
    QTextParag *minwParag;
    const QStyleSheet* sheet_;
    const QMimeSourceFactory* factory_;
    QString contxt;
    QMap<QString, QString> attribs;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextString
{
public:
    class Char
    {
	friend class QTextString;
    public:
	Char() : lineStart( 0 ), isCustom( 0 ), d( 0 ) {} // this is never called, initialize variables in QTextString::insert()!!!
	~Char();
	QChar c;
	uint lineStart : 1;
	uint isCustom : 1;
	int x;
	int width() const;
	int height() const;
	int ascent() const;
	int descent() const;
	QTextFormat *format() const;
	QTextCustomItem *customItem() const;
	void setFormat( QTextFormat *f );
	void setCustomItem( QTextCustomItem *i );
	
    private:
	struct CharData
	{
	    QTextFormat *format;
	    QTextCustomItem *custom;
	};
	
	Char &operator=( const Char & ) {
	    return *this;
	}

	void *d;
    };

    QTextString();

    QString toString() const;

    Char &at( int i ) const;
    int length() const;

    void insert( int index, const QString &s, QTextFormat *f );
    void insert( int index, Char *c );
    void truncate( int index );
    void remove( int index, int len );

    void setFormat( int index, QTextFormat *f, bool useCollection );

private:
    QArray<Char> data;
    QString cache;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextParag
{
public:
    struct LineStart {
	LineStart() : y( 0 ), baseLine( 0 ), h( 0 ) {}
	LineStart( ushort y_, ushort bl, ushort h_ ) : y( y_ ), baseLine( bl ), h( h_ ) {}
	ushort y, baseLine, h;
    };

    struct Paren {
	Paren() : type( Open ), chr( ' ' ), pos( -1 ) {}
	Paren( int t, const QChar &c, int p ) : type( (Type)t ), chr( c ), pos( p ) {}
	enum Type { Open, Closed };
	Type type;
	QChar chr;
	int pos;
    };

    typedef QValueList<Paren> ParenList;

    enum Type {
	Normal = 0,
	BulletList,
	EnumList
    };

    QTextParag( QTextDocument *d, QTextParag *pr = 0, QTextParag *nx = 0, bool updateIds = TRUE );
    virtual ~QTextParag() {}

    QTextString *string() const;
    QTextString::Char *at( int i ) const; // maybe remove later
    int length() const; // maybe remove later

    void setListStyle( QStyleSheetItem::ListStyle ls );
    QStyleSheetItem::ListStyle listStyle() const;

    void setList( bool b, int listStyle );
    void incDepth();
    void decDepth();

    void setFormat( QTextFormat *fm );
    QTextFormat *paragFormat() const;

    QTextDocument *document() const;

    QRect rect() const;

    QTextParag *prev() const;
    QTextParag *next() const;
    void setPrev( QTextParag *s );
    void setNext( QTextParag *s );

    void insert( int index, const QString &s );
    void append( const QString &s, bool reallyAtEnd = FALSE );
    void truncate( int index );
    void remove( int index, int len );
    void join( QTextParag *s );

    void invalidate( int chr );

    void move( int dy );
    void format( int start = -1, bool doMove = TRUE );

    bool isValid() const;
    bool hasChanged() const;
    void setChanged( bool b );

    int lineHeightOfChar( int i, int *bl = 0, int *y = 0 ) const;
    QTextString::Char *lineStartOfChar( int i, int *index = 0, int *line = 0 ) const;
    int lines() const;
    QTextString::Char *lineStartOfLine( int line, int *index = 0 ) const;
    int lineY( int l ) const;
    int lineBaseLine( int l ) const;
    int lineHeight( int l ) const;
    void lineInfo( int l, int &y, int &h, int &bl ) const;

    void setSelection( int id, int start, int end );
    void removeSelection( int id );
    int selectionStart( int id ) const;
    int selectionEnd( int id ) const;
    bool hasSelection( int id ) const;
    bool hasAnySelection() const;
    bool fullSelected( int id ) const;

    void setEndState( int s );
    int endState() const;

    void setParagId( int i );
    int paragId() const;

    bool firstHighlighte() const;
    void setFirstHighlighte( bool b );

    void indent( int *oldIndent = 0, int *newIndent = 0 );

    ParenList &parenList();
    QMap<int, LineStart*> &lineStartList();

    int lastLengthForCompletion() const;
    void setLastLengthFotCompletion( int l );

    void setFormat( int index, int len, QTextFormat *f, bool useCollection = TRUE, int flags = -1 );

    void setAlignment( int a );
    int alignment() const;

    virtual void paint( QPainter &painter, const QColorGroup &cg, QTextCursor *cusror = 0, bool drawSelections = FALSE,
			int clipx = -1, int clipy = -1, int clipw = -1, int cliph = -1 );

    void setStyleSheetItems( const QVector<QStyleSheetItem> &vec );
    QVector<QStyleSheetItem> styleSheetItems() const;
    QStyleSheetItem *style() const;

    int topMargin() const;
    int bottomMargin() const;
    int leftMargin() const;
    int rightMargin() const;

    int numberOfSubParagraph() const;
    void registerFloatingItem( QTextCustomItem *i );
    void unregisterFloatingItem( QTextCustomItem *i );

    void setFullWidth( bool b ) { fullWidth = b; }
    bool isFullWidth() const { return fullWidth; }

    QTextTableCell *tableCell() const { return tc; }
    void setTableCell( QTextTableCell *c ) { tc = c; }

    void addCustomItem();
    void removeCustomItem();
    int customItems() const;

    QBrush *background() const;

private:
    void drawLabel( QPainter* p, int x, int y, int w, int h, int base, const QColorGroup& cg );
    void drawParagBuffer( QPainter &painter, const QString &buffer, int startX,
			  int lastY, int baseLine, int bw, int h, bool drawSelections,
			  QTextFormat *lastFormat, int i, int *selectionStarts,
			  int *selectionEnds, const QColorGroup &cg  );

private:
    struct Selection {
	int start, end;
    };

    QMap<int, LineStart*> lineStarts;
    int invalid;
    QRect r;
    QTextParag *p, *n;
    QTextDocument *doc;
    bool changed;
    bool firstFormat, firstHilite;
    QMap<int, Selection> selections;
    int state, id;
    bool needHighlighte;
    ParenList parens;
    int lastLenForCompletion;
    QTextString *str;
    int align;
    QVector<QStyleSheetItem> styleSheetItemsVec;
    QStyleSheetItem::ListStyle listS;
    int numSubParag;
    int tm, bm, lm, rm;
    QTextFormat *defFormat;
    QList<QTextCustomItem> floatingItems;
    bool fullWidth;
    QTextTableCell *tc;
    int numCustomItems;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextFormatter
{
public:
    QTextFormatter( QTextDocument *d );
    virtual ~QTextFormatter() {}
    virtual int format( QTextParag *parag, int start ) = 0;

protected:
    QTextDocument *doc;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextFormatterBreakInWords : public QTextFormatter
{
public:
    QTextFormatterBreakInWords( QTextDocument *d );
    int format( QTextParag *parag, int start );

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextFormatterBreakWords : public QTextFormatter
{
public:
    QTextFormatterBreakWords( QTextDocument *d );
    int format( QTextParag *parag, int start );

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextIndent
{
public:
    QTextIndent( QTextDocument *d );
    virtual void indent( QTextParag *parag, int *oldIndent = 0, int *newIndent = 0 ) = 0;

protected:
    QTextDocument *doc;

};
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextSyntaxHighlighter
{
public:
    enum Ids {
	Standard = 0
    };

    QTextSyntaxHighlighter( QTextDocument *d );
    virtual void highlighte( QTextParag *, int, bool = TRUE ) = 0;
    virtual QTextFormat *format( int id ) = 0;

protected:
    QTextDocument *doc;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextFormat
{
    friend class QTextFormatCollection;

public:
    enum Flags {
	Bold = 1,
	Italic = 2,
	Underline = 4,
	Family = 8,
	Size = 16,
	Color = 32,
	Misspelled = 64,
	Font = Bold | Italic | Underline | Family | Size,
	Format = Font | Color | Misspelled
    };

    QTextFormat();
    QTextFormat( const QFont &f, const QColor &c );
    QTextFormat( const QTextFormat &fm );
    QTextFormat makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr ) const;
    QTextFormat& operator=( const QTextFormat &fm );
    QColor color() const;
    QFont font() const;
    bool isMisspelled() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int width( const QChar &c ) const;
    int height() const;
    int ascent() const;
    int descent() const;
    QString anchorHref() const;
    QString anchorName() const;
    bool isAnchor() const;

    void setBold( bool b );
    void setItalic( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setFont( const QFont &f );
    void setColor( const QColor &c );
    void setMisspelled( bool b );

    bool operator==( const QTextFormat &f ) const;
    QTextFormatCollection *parent() const;
    QString key() const;

    static QString getKey( const QFont &f, const QColor &c, bool misspelled, const QString &lhref, const QString &lnm );

    void addRef();
    void removeRef();

    QString makeFormatChangeTags( QTextFormat *f ) const;
    QString makeFormatEndTags() const;

    void setPainter( QPainter *p );

private:
    void update();
    void generateKey();

private:
    QFont fn;
    QColor col;
    QFontMetrics fm;
    bool missp;
    int leftBearing, rightBearing;
    uchar widths[ 256 ];
    int hei, asc, dsc;
    QTextFormatCollection *collection;
    int ref;
    QString k;
    int logicalFontSize;
    int stdPointSize;
    QString anchor_href;
    QString anchor_name;
    QPainter *painter;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextCustomItem : public QShared
{
public:
    QTextCustomItem( QTextDocument *p )
	:  xpos(0), ypos(-1), width(-1), height(0), parent( p )
    {}
    virtual ~QTextCustomItem() {}
    virtual void draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg ) = 0;

    virtual void adjustToPainter( QPainter* ) { width = 0; }

    enum Placement { PlaceInline = 0, PlaceLeft, PlaceRight };
    virtual Placement placement() const { return PlaceInline; }
    bool placeInline() { return placement() == PlaceInline; }

    virtual bool ownLine() const { return FALSE; }
    virtual void resize( QPainter*, int nwidth ){ width = nwidth; };
    virtual void invalidate() {};

    virtual bool isNested() const { return FALSE; }
    virtual int minimumWidth() const { return 0; }

    int xpos; // used for floating items
    int ypos; // used for floating items
    int width;
    int height;

    virtual void enter( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd = FALSE ) {
	doc = doc; parag = parag; idx = idx; ox = ox; oy = oy; Q_UNUSED( atEnd )
    }
    virtual void enterAt( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, const QPoint & ) {
	doc = doc; parag = parag; idx = idx; ox = ox; oy = oy;
    }
    virtual void next( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy ) {
	doc = doc; parag = parag; idx = idx; ox = ox; oy = oy;
    }
    virtual void prev( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy ) {
	doc = doc; parag = parag; idx = idx; ox = ox; oy = oy;
    }
    virtual void down( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy ) {
	doc = doc; parag = parag; idx = idx; ox = ox; oy = oy;
    }
    virtual void up( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy ) {
	doc = doc; parag = parag; idx = idx; ox = ox; oy = oy;
    }

    QTextDocument *parent;
};

class QTextImage : public QTextCustomItem
{
public:
    QTextImage( QTextDocument *p, const QMap<QString, QString> &attr, const QString& context,
		const QMimeSourceFactory &factory);
    ~QTextImage();

    Placement placement() const { return place; }
    void adjustToPainter( QPainter* );

    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg );

private:
    QRegion* reg;
    QPixmap pm;
    Placement place;
    int tmpwidth, tmpheight;
};

class QTextHorizontalLine : public QTextCustomItem
{
public:
    QTextHorizontalLine( QTextDocument *p );
    ~QTextHorizontalLine();
    void realize( QPainter* );
    void draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg );

    bool ownLine() const { return TRUE; }

private:
};

class QTextFlow
{
    friend class QTextDocument;
    friend class QTextTableCell;

public:
    QTextFlow();
    ~QTextFlow();

    void setWidth( int w );
    void setPageSize( int ps ) { pagesize = ps; }
    int pageSize() const { return pagesize; }

    int adjustLMargin( int yp, int margin, int space );
    int adjustRMargin( int yp, int margin, int space );

    void registerFloatingItem( QTextCustomItem* item, bool right = FALSE );
    void unregisterFloatingItem( QTextCustomItem* item );
    void drawFloatingItems(QPainter* p, int cx, int cy, int cw, int ch, const QColorGroup& cg );
    void adjustFlow( int  &yp, int w, int h, bool pages = TRUE );

    bool isEmpty() { return leftItems.isEmpty() && rightItems.isEmpty(); }
    void updateHeight( QTextCustomItem *i );

private:
    int width;
    int height;

    int pagesize;

    QList<QTextCustomItem> leftItems;
    QList<QTextCustomItem> rightItems;

};

class QTextTable;

class QTextTableCell : public QLayoutItem
{
    friend QTextTable;

public:
    QTextTableCell( QTextTable* table,
		    int row, int column,
		    const QMap<QString, QString> &attr,
		    const QStyleSheetItem* style,
		    const QTextFormat& fmt, const QString& context,
		    const QMimeSourceFactory &factory, const QStyleSheet *sheet, const QString& doc );
    QTextTableCell( QTextTable* table, int row, int column );

    ~QTextTableCell();
    QSize sizeHint() const ;
    QSize minimumSize() const ;
    QSize maximumSize() const ;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& ) ;
    QRect geometry() const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

    void adjustToPainter();

    int row() const { return row_; }
    int column() const { return col_; }
    int rowspan() const { return rowspan_; }
    int colspan() const { return colspan_; }
    int stretch() const { return stretch_; }

    QTextDocument* richText()  const { return richtext; }
    QTextTable* table() const { return parent; }

    void draw( int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg );

    QBrush *backGround() const { return background; }

private:
    QPainter* painter() const;
    QRect geom;
    QTextTable* parent;
    QTextDocument* richtext;
    int row_;
    int col_;
    int rowspan_;
    int colspan_;
    int stretch_;
    int maxw;
    int minw;
    bool hasFixedWidth;
    QBrush *background;
};

class QTextTable: public QTextCustomItem
{
public:
    QTextTable( QTextDocument *p, const QMap<QString, QString> &attr );
    ~QTextTable();
    void adjustToPainter( QPainter *p );
    void verticalBreak( int  y, QTextFlow* flow );
    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch,
	       const QColorGroup& cg );

    bool noErase() const { return TRUE; };
    bool ownLine() const { return TRUE; }
    Placement placement() const { return place; }
    bool isNested() const { return TRUE; }
    void resize( QPainter*, int nwidth );
    virtual void invalidate() { cachewidth = -1; };
    QString anchorAt( QPainter* p, int x, int y );

    virtual void enter( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd = FALSE );
    virtual void enterAt( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, const QPoint &pos );
    virtual void next( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual void prev( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual void down( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual void up( QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );

    int minimumWidth() const { return layout ? layout->minimumSize().width() : 0; }

private:
    QGridLayout* layout;
    QList<QTextTableCell> cells;

    friend class QTextTableCell;
    void addCell( QTextTableCell* cell );
    QPainter* painter;
    int cachewidth;
    int fixwidth;
    int cellpadding;
    int cellspacing;
    int border;
    int outerborder;
    int stretch;
    int innerborder;
    int lastX, lastY;

    int currCell;

    Placement place;
};


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextFormatCollection
{
    friend class QTextDocument;

public:
    QTextFormatCollection();

    void setDefaultFormat( QTextFormat *f );
    QTextFormat *defaultFormat() const;
    QTextFormat *format( QTextFormat *f );
    QTextFormat *format( QTextFormat *of, QTextFormat *nf, int flags );
    QTextFormat *format( const QFont &f, const QColor &c );
    void remove( QTextFormat *f );

    void debug();

    void setPainter( QPainter *p );

private:
    QTextFormat *defFormat, *lastFormat, *cachedFormat;
    QDict<QTextFormat> cKey;
    QTextFormat *cres;
    QFont cfont;
    QColor ccol;
    QString kof, knf;
    int cflags;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextParag *QTextCursor::parag() const
{
    return string;
}

inline int QTextCursor::index() const
{
    return idx;
}

inline void QTextCursor::setIndex( int i, bool restore )
{
    if ( restore )
	restoreState();
    tmpIndex = -1;
    idx = i;
}

inline bool QTextCursor::checkParens()
{
    QChar c( string->at( idx )->c );
    if ( c == '{' || c == '(' || c == '[' ) {
	return checkOpenParen();
    } else if ( idx > 0 ) {
	c = string->at( idx - 1 )->c;
	if ( c == '}' || c == ')' || c == ']' ) {
	    return checkClosedParen();
	}
    }

    return FALSE;
}

inline void QTextCursor::setParag( QTextParag *s, bool restore )
{
    if ( restore )
	restoreState();
    idx = 0;
    string = s;
    tmpIndex = -1;
}

inline void QTextCursor::checkIndex()
{
    if ( idx >= string->length() )
	idx = string->length() - 1;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline int QTextDocument::x() const
{
    return cx;
}

inline int QTextDocument::y() const
{
    return cy;
}

inline int QTextDocument::width() const
{
    return QMAX( cw, flow_->width );
}

inline int QTextDocument::height() const
{
    if ( lParag )
	return QMAX( flow_->height, lParag->rect().top() + lParag->rect().height() + 1 );
    return 0;
}

inline QTextParag *QTextDocument::firstParag() const
{
    return fParag;
}

inline QTextParag *QTextDocument::lastParag() const
{
    return lParag;
}

inline void QTextDocument::setFirstParag( QTextParag *p )
{
    fParag = p;
}

inline void QTextDocument::setLastParag( QTextParag *p )
{
    lParag = p;
}

inline void QTextDocument::setWidth( int w )
{
    cw = QMAX( w, minw );
    flow_->setWidth( cw );
}

inline int QTextDocument::minimumWidth() const
{
    return minw;
}

inline QTextSyntaxHighlighter *QTextDocument::syntaxHighlighter() const
{
    return syntaxHighlighte;
}

inline void QTextDocument::setSyntaxHighlighter( QTextSyntaxHighlighter * sh )
{
    syntaxHighlighte = sh;
}

inline void QTextDocument::setFormatter( QTextFormatter *f )
{
    pFormatter = f;
}

inline QTextFormatter *QTextDocument::formatter() const
{
    return pFormatter;
}

inline void QTextDocument::setIndent( QTextIndent *i )
{
    indenter = i;
}

inline QTextIndent *QTextDocument::indent() const
{
    return indenter;
}

inline void QTextDocument::setParenCheckingEnabled( bool b )
{
    parenCheck = b;
}

inline bool QTextDocument::isParenCheckingEnabled() const
{
    return parenCheck;
}

inline QColor QTextDocument::selectionColor( int id ) const
{
    return selectionColors[ id ];
}

inline bool QTextDocument::invertSelectionText( int id ) const
{
    return selectionText[ id ];
}

inline bool QTextDocument::hasSelection( int id ) const
{
    return selections.find( id ) != selections.end();
}

inline void QTextDocument::setSelectionStart( int id, QTextCursor *cursor )
{
    Selection sel;
    sel.startParag = cursor->parag();
    sel.endParag = cursor->parag();
    sel.startParag->setSelection( id, cursor->index(), cursor->index() );
    sel.startIndex = cursor->index();
    selections[ id ] = sel;
}

inline QTextParag *QTextDocument::paragAt( int i ) const
{
    QTextParag *s = fParag;
    while ( s ) {
	if ( s->paragId() == i )
	    return s;
	s = s->next();
    }
    return 0;
}

inline void QTextDocument::setCompletionEnabled( bool b )
{
    completion = b;
}

inline bool QTextDocument::isCompletionEnabled() const
{
    return completion;
}

inline QTextFormatCollection *QTextDocument::formatCollection() const
{
    return fCollection;
}

inline int QTextDocument::listIndent( int depth ) const
{
    // #######
    return ( depth + 1 ) * 15;
}

inline QTextDocument::Bullet QTextDocument::bullet( int depth ) const
{
    if ( depth == 0 )
	return FilledCircle;
    else if ( depth == 1 )
	return FilledSquare;
    else if ( depth == 2 )
	return OutlinedCircle;
    else if ( depth == 3 )
	return OutlinedSquare;
    else
	return FilledCircle;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextFormat::QTextFormat()
    : fm( QFontMetrics( fn ) ), logicalFontSize( 3 ), stdPointSize( 12 ), painter( 0 )
{
    missp = FALSE;
    collection = 0;
}

inline QTextFormat::QTextFormat( const QFont &f, const QColor &c )
    : fn( f ), col( c ), fm( QFontMetrics( f ) ),
      logicalFontSize( 3 ), stdPointSize( f.pointSize() ), painter( 0 )
{
    collection = 0;
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.height();
    asc = fm.ascent() + fm.leading();
    dsc = fm.descent();
    missp = FALSE;
    memset( widths, 0, 256 );
    generateKey();
    addRef();
}

inline QTextFormat::QTextFormat( const QTextFormat &f )
    : fm( f.fm )
{
    collection = 0;
    fn = f.fn;
    col = f.col;
    painter = f.painter;
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    memset( widths, 0, 256 );
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    stdPointSize = f.stdPointSize;
    logicalFontSize = f.logicalFontSize;
    missp = f.missp;
    k = f.k;
    anchor_name = f.anchor_name;
    anchor_href = f.anchor_href;
    addRef();
}

inline QTextFormat& QTextFormat::operator=( const QTextFormat &f )
{
    collection = f.collection;
    fn = f.fn;
    col = f.col;
    fm = f.fm;
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    memset( widths, 0, 256 );
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    stdPointSize = f.stdPointSize;
    logicalFontSize = f.logicalFontSize;
    missp = f.missp;
    k = f.k;
    anchor_name = f.anchor_name;
    anchor_href = f.anchor_href;
    addRef();
    return *this;
}

inline void QTextFormat::update()
{
    fm = QFontMetrics( fn );
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.height();
    asc = fm.ascent() + fm.leading();
    dsc = fm.descent();
    memset( widths, 0, 256 );
    generateKey();
}

inline QColor QTextFormat::color() const
{
    return col;
}

inline QFont QTextFormat::font() const
{
    return fn;
}

inline bool QTextFormat::isMisspelled() const
{
    return missp;
}

inline int QTextFormat::minLeftBearing() const
{
    if ( !painter )
	return leftBearing;
    painter->setFont( fn );
    return painter->fontMetrics().minLeftBearing();
}

inline int QTextFormat::minRightBearing() const
{
    if ( !painter )
	return rightBearing;
    painter->setFont( fn );
    return painter->fontMetrics().minRightBearing();
}

inline int QTextFormat::width( const QChar &c ) const
{
    if ( !painter ) {
	if ( c == '\t' )
	    return 30;
	int w = widths[ c.unicode() ];
	if ( w == 0 ) {
	    w = fm.width( c );
	    if ( w < 256 )
		( (QTextFormat*)this )->widths[ c.unicode() ] = w;
	}
	return w;
    }
    painter->setFont( fn );
    return painter->fontMetrics().width( c );
}

inline int QTextFormat::height() const
{
    if ( !painter )
	return hei;
    painter->setFont( fn );
    return painter->fontMetrics().height();
}

inline int QTextFormat::ascent() const
{
    if ( !painter )
	return asc;
    painter->setFont( fn );
    return painter->fontMetrics().height() + painter->fontMetrics().leading();
}

inline int QTextFormat::descent() const
{
    if ( !painter )
	return dsc;
    painter->setFont( fn );
    return painter->fontMetrics().descent();
}

inline bool QTextFormat::operator==( const QTextFormat &f ) const
{
    return k == f.k;
}

inline QTextFormatCollection *QTextFormat::parent() const
{
    return collection;
}

inline void QTextFormat::addRef()
{
    ref++;
#ifdef DEBUG_COLLECTION
    qDebug( "add ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
}

inline void QTextFormat::removeRef()
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

inline QString QTextFormat::key() const
{
    return k;
}

inline void QTextFormat::generateKey()
{
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family()
       << (int)isMisspelled()
       << anchor_href
       << anchor_name;
}

inline QString QTextFormat::getKey( const QFont &fn, const QColor &col, bool misspelled, const QString &lhref, const QString &lnm )
{
    QString k;
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family()
       << (int)misspelled
       << lhref
       << lnm;
    return k;
}

inline QString QTextFormat::anchorHref() const
{
    return anchor_href;
}

inline QString QTextFormat::anchorName() const
{
    return anchor_name;
}

inline bool QTextFormat::isAnchor() const
{
    return !anchor_href.isEmpty()  || !anchor_name.isEmpty();
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextString::Char &QTextString::at( int i ) const
{
    return data[ i ];
}

inline QString QTextString::toString() const
{
    return cache;
}

inline int QTextString::length() const
{
    return data.size();
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextString::Char *QTextParag::at( int i ) const
{
    return &str->at( i );
}

inline int QTextParag::length() const
{
    return str->length();
}

inline bool QTextParag::isValid() const
{
    return invalid == -1;
}

inline bool QTextParag::hasChanged() const
{
    return changed;
}

inline void QTextParag::setChanged( bool b )
{
    changed = b;
}

inline void QTextParag::append( const QString &s, bool reallyAtEnd )
{
    if ( reallyAtEnd )
	insert( str->length(), s );
    else
	insert( QMAX( str->length() - 1, 0 ), s );
}

inline QRect QTextParag::rect() const
{
    return r;
}

inline QTextParag *QTextParag::prev() const
{
    return p;
}

inline QTextParag *QTextParag::next() const
{
    return n;
}

inline void QTextParag::setSelection( int id, int start, int end )
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it != selections.end() ) {
	if ( start == ( *it ).start && end == ( *it ).end )
	    return;
    }

    Selection sel;
    sel.start = start;
    sel.end = end;
    selections[ id ] = sel;
    changed = TRUE;
}

inline void QTextParag::removeSelection( int id )
{
    selections.remove( id );
    changed = TRUE;
}

inline int QTextParag::selectionStart( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return -1;
    return ( *it ).start;
}

inline int QTextParag::selectionEnd( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return -1;
    return ( *it ).end;
}

inline bool QTextParag::hasSelection( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;
    return ( *it ).start != ( *it ).end || length() == 1;
}

inline bool QTextParag::hasAnySelection() const
{
    return !selections.isEmpty();
}

inline bool QTextParag::fullSelected( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;
    return ( *it ).start == 0 && ( *it ).end == str->length() - 1;
}

inline void QTextParag::setEndState( int s )
{
    if ( s == state )
	return;
    state = s;
}

inline int QTextParag::endState() const
{
    return state;
}

inline void QTextParag::setParagId( int i )
{
    id = i;
}

inline int QTextParag::paragId() const
{
    if ( id == -1 )
	qWarning( "invalid parag id!!!!!!!! (%p)", this );
    return id;
}

inline bool QTextParag::firstHighlighte() const
{
    return firstHilite;
}

inline void QTextParag::setFirstHighlighte( bool b )
{
    firstHilite = b;
}

inline QTextParag::ParenList &QTextParag::parenList()
{
    return parens;
}

inline QMap<int, QTextParag::LineStart*> &QTextParag::lineStartList()
{
    return lineStarts;
}

inline int QTextParag::lastLengthForCompletion() const
{
    return lastLenForCompletion;
}

inline void QTextParag::setLastLengthFotCompletion( int l )
{
    lastLenForCompletion = l;
}

inline int QTextParag::lineY( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "QTextParag::lineY: line %d out of range!", l );
	return 0;
    }

    if ( !isValid() )
	( (QTextParag*)this )->format();

    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->y;
}

inline int QTextParag::lineBaseLine( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "QTextParag::lineBaseLine: line %d out of range!", l );
	return 10;
    }

    if ( !isValid() )
	( (QTextParag*)this )->format();

    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->baseLine;
}

inline int QTextParag::lineHeight( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "QTextParag::lineHeight: line %d out of range!", l );
	return 15;
    }

    if ( !isValid() )
	( (QTextParag*)this )->format();

    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->h;
}

inline void QTextParag::lineInfo( int l, int &y, int &h, int &bl ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "QTextParag::lineInfo: line %d out of range!", l );
	qDebug( "%d %d", lineStarts.count() - 1, l );
	y = 0;
	h = 15;
	bl = 10;
	return;
    }

    if ( !isValid() )
	( (QTextParag*)this )->format();

    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    y = ( *it )->y;
    h = ( *it )->h;
    bl = ( *it )->baseLine;
}

inline QTextString *QTextParag::string() const
{
    return str;
}

inline QTextDocument *QTextParag::document() const
{
    return doc;
}

inline void QTextParag::setAlignment( int a )
{
    if ( a == align )
	return;
    align = a;
    invalidate( 0 );
}

inline int QTextParag::alignment() const
{
    if ( align != -1 )
	return align;
    QStyleSheetItem *item = style();
    if ( !item )
	return Qt::AlignLeft;
    for ( int i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	item = styleSheetItemsVec[ i ];
	if ( item->alignment() != QStyleSheetItem::Undefined )
	    return item->alignment();
    }
    return Qt::AlignLeft;
}

inline QVector<QStyleSheetItem> QTextParag::styleSheetItems() const
{
    QVector<QStyleSheetItem> vec;
    vec.resize( styleSheetItemsVec.size() );
    for ( int i = 0; i < (int)vec.size(); ++i )
	vec.insert( i, styleSheetItemsVec[ i ] );
    return vec;
}

inline QStyleSheetItem *QTextParag::style() const
{
    if ( styleSheetItemsVec.size() == 0 )
	return 0;
    return styleSheetItemsVec[ styleSheetItemsVec.size() - 1 ];
}

inline int QTextParag::topMargin() const
{
    if ( tm != -1 )
	return tm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->tm = 0;
	return 0;
    }

    int m = 0;
    if ( item->margin( QStyleSheetItem::MarginTop ) != QStyleSheetItem::Undefined )
	m = item->margin( QStyleSheetItem::MarginTop );
    QStyleSheetItem *it = 0;
    QStyleSheetItem *p = prev() ? prev()->style() : 0;
    for ( int i = (int)styleSheetItemsVec.size() - 2 ; i >= 0; --i ) {
	it = styleSheetItemsVec[ i ];
	if ( it != p )
	    break;
	int mar = it->margin( QStyleSheetItem::MarginTop );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
	if ( it->displayMode() != QStyleSheetItem::DisplayInline )
	    break;
    }

    ( (QTextParag*)this )->tm = m;
    return tm;
}

inline int QTextParag::bottomMargin() const
{
    if ( bm != -1 )
	return bm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->bm = 0;
	return 0;
    }

    int m = 0;
    if ( item->margin( QStyleSheetItem::MarginBottom ) != QStyleSheetItem::Undefined )
	m = item->margin( QStyleSheetItem::MarginBottom );
    QStyleSheetItem *it = 0;
    QStyleSheetItem *n = next() ? next()->style() : 0;
    for ( int i =(int)styleSheetItemsVec.size() - 2 ; i >= 0; --i ) {
	it = styleSheetItemsVec[ i ];
	if ( it != n )
	    break;
	int mar = it->margin( QStyleSheetItem::MarginBottom );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
	if ( it->displayMode() != QStyleSheetItem::DisplayInline )
	    break;
    }

    ( (QTextParag*)this )->bm = m;
    return bm;
}

inline int QTextParag::leftMargin() const
{
    if ( lm != -1 )
	return lm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->lm = 0;
	return 0;
    }
    int m = 0;
    for ( int i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	item = styleSheetItemsVec[ i ];
	int mar = item->margin( QStyleSheetItem::MarginLeft );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
	if ( item->name() == "ol" || item->name() == "ul" ) {
	    m += defFormat->width( '1' ) +
		 defFormat->width( '2' ) +
		 defFormat->width( '3' ) +
		 defFormat->width( '.' );
	}
    }
    ( (QTextParag*)this )->lm = m;
    return lm;
}

inline int QTextParag::rightMargin() const
{
    if ( rm != -1 )
	return rm;
    QStyleSheetItem *item = style();
    if ( !item ) {
	( (QTextParag*)this )->rm = 0;
	return 0;
    }
    int m = 0;
    for ( int i = 0; i < (int)styleSheetItemsVec.size(); ++i ) {
	item = styleSheetItemsVec[ i ];
	int mar = item->margin( QStyleSheetItem::MarginRight );
	m += mar != QStyleSheetItem::Undefined ? mar : 0;
    }
    ( (QTextParag*)this )->rm = m;
    return rm;
}

inline int QTextParag::numberOfSubParagraph() const
{
    if ( numSubParag != -1 )
 	return numSubParag;
    int n = 0;
    QTextParag *p = (QTextParag*)this;
    while ( p && style() == p->style() && listStyle() == p->listStyle() ) {
	++n;
	p = p->prev();
    }
    ( (QTextParag*)this )->numSubParag = n;
    return n;
}

inline void QTextParag::setListStyle( QStyleSheetItem::ListStyle ls )
{
    listS = ls;
    invalidate( 0 );
}

inline QStyleSheetItem::ListStyle QTextParag::listStyle() const
{
    return listS;
}

inline void QTextParag::setFormat( QTextFormat *fm )
{
    defFormat = doc->formatCollection()->format( fm );
}

inline QTextFormat *QTextParag::paragFormat() const
{
    return defFormat;
}

inline void QTextParag::registerFloatingItem( QTextCustomItem *i )
{
    floatingItems.append( i );
}

inline void QTextParag::unregisterFloatingItem( QTextCustomItem *i )
{
    floatingItems.removeRef( i );
}

inline void QTextParag::addCustomItem()
{
    numCustomItems++;
}

inline void QTextParag::removeCustomItem()
{
    numCustomItems--;
}

inline int QTextParag::customItems() const
{
    return numCustomItems;
}

inline QBrush *QTextParag::background() const
{
    return tc ? tc->backGround() : 0;
};


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void QTextFormatCollection::setDefaultFormat( QTextFormat *f )
{
    defFormat = f;
}

inline QTextFormat *QTextFormatCollection::defaultFormat() const
{
    return defFormat;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextString::Char::~Char()
{
    if ( format() )
	format()->removeRef();
    if ( isCustom )
	delete (QTextCustomItem*)d;
}

inline QTextFormat *QTextString::Char::format() const
{
    return !isCustom ? (QTextFormat*)d : ( (CharData*)d )->format;
}


inline QTextCustomItem *QTextString::Char::customItem() const
{
    return isCustom ? ( (CharData*)d )->custom : 0;
}

inline void QTextString::Char::setFormat( QTextFormat *f )
{
    if ( !isCustom ) {
	d = (void*)f;
    } else {
	if ( !d ) {
	    d = new CharData;
	    ( (CharData*)d )->custom = 0;
	}
	( (CharData*)d )->format = f;
    }
}

inline void QTextString::Char::setCustomItem( QTextCustomItem *i )
{
    if ( !isCustom ) {
	QTextFormat *f = (QTextFormat*)d;
	d = new CharData;
	( (CharData*)d )->format = f;
	isCustom = TRUE;
    } else {
	delete ( (CharData*)d )->custom;
    }
    ( (CharData*)d )->custom = i;
}

inline int QTextString::Char::width() const
{
    return !isCustom ? format()->width( c ) : ( customItem()->placement() == QTextCustomItem::PlaceInline ? customItem()->width : 0 );
}

inline int QTextString::Char::height() const
{
    return !isCustom ? format()->height() : ( customItem()->placement() == QTextCustomItem::PlaceInline ? customItem()->height : 0 );
}

inline int QTextString::Char::ascent() const
{
    return !isCustom ? format()->ascent() : ( customItem()->placement() == QTextCustomItem::PlaceInline ? customItem()->height : 0 );
}

inline int QTextString::Char::descent() const
{
    return !isCustom ? format()->descent() : 0;
}

#endif
