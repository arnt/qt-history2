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

#include <limits.h>

//#define DEBUG_COLLECTION

class QTextEditDocument;
class QTextEditCommand;
class QTextEdit;
class QTextEditString;
class QTextEditSyntaxHighlighter;
class QTextEditCommandHistory;
class QTextEditFormat;
class QTextEditCursor;
class QTextEditParag;
class QTextEditFormatter;
class QTextEditIndent;
class QTextEditFormatCollection;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditCursor
{
public:
    QTextEditCursor( QTextEditDocument *d );

    QTextEditParag *parag() const;
    int index() const;
    void setParag( QTextEditParag *s );

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

    void setIndex( int i );

    bool checkParens();
    void checkIndex();

private:
    bool checkOpenParen();
    bool checkClosedParen();

    QTextEditParag *string;
    QTextEditDocument *doc;
    int idx, tmpIndex;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditCommandHistory
{
public:
    QTextEditCommandHistory( int s ) : current( -1 ), steps( s ) { history.setAutoDelete( TRUE ); }

    void addCommand( QTextEditCommand *cmd );
    QTextEditCursor *undo( QTextEditCursor *c );
    QTextEditCursor *redo( QTextEditCursor *c );

private:
    QList<QTextEditCommand> history;
    int current, steps;

};

class QTextEditCommand
{
public:
    enum Commands { Invalid, Insert, Delete, Join, Split };
    QTextEditCommand( QTextEditDocument *d ) : doc( d ), cursor( d ) {}
    virtual ~QTextEditCommand() {}
    virtual Commands type() const { return Invalid; };

    virtual QTextEditCursor *execute( QTextEditCursor *c ) = 0;
    virtual QTextEditCursor *unexecute( QTextEditCursor *c ) = 0;

protected:
    QTextEditDocument *doc;
    QTextEditCursor cursor;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditDeleteCommand : public QTextEditCommand
{
public:
    QTextEditDeleteCommand( QTextEditDocument *d, int i, int idx, const QString &str )
	: QTextEditCommand( d ), id( i ), index( idx ), text( str ) {}
    virtual Commands type() const { return Delete; };

    virtual QTextEditCursor *execute( QTextEditCursor *c );
    virtual QTextEditCursor *unexecute( QTextEditCursor *c );

protected:
    int id, index;
    QString text;

};

class QTextEditInsertCommand : public QTextEditDeleteCommand
{
public:
    QTextEditInsertCommand( QTextEditDocument *d, int i, int idx, const QString &str )
	: QTextEditDeleteCommand( d, i, idx, str ) {}
    Commands type() const { return Insert; };

    virtual QTextEditCursor *execute( QTextEditCursor *c ) { return QTextEditDeleteCommand::unexecute( c ); }
    virtual QTextEditCursor *unexecute( QTextEditCursor *c ) { return QTextEditDeleteCommand::execute( c ); }

};


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditDocument
{
public:
    enum SelectionIds {
	Standard = 0,
	ParenMismatch,
	ParenMatch,
	Search,
	Temp // This selection must not be drawn, it's used e.g. by undo/redo to remove multiple lines with removeSelectedText()
    };

    enum Bullet {
	FilledCircle,
	FilledSquare,
	OutlinedCircle,
	OutlinedSquare
    };

    QTextEditDocument( const QString &fn, bool tabify );

    int x() const;
    int y() const;
    int width() const;
    void setWidth( int w );

    QTextEditParag *firstParag() const;
    QTextEditParag *lastParag() const;
    void setFirstParag( QTextEditParag *p );
    void setLastParag( QTextEditParag *p );

    void invalidate();

    void setSyntaxHighlighter( QTextEditSyntaxHighlighter *sh );
    QTextEditSyntaxHighlighter *syntaxHighlighter() const;

    void setFormatter( QTextEditFormatter *f );
    QTextEditFormatter *formatter() const;

    void setIndent( QTextEditIndent *i );
    QTextEditIndent *indent() const;

    void setParenCheckingEnabled( bool b );
    bool isParenCheckingEnabled() const;

    QColor selectionColor( int id ) const;
    bool hasSelection( int id ) const;
    void setSelectionStart( int id, QTextEditCursor *cursor );
    bool setSelectionEnd( int id, QTextEditCursor *cursor );
    bool removeSelection( int id );
    void selectionStart( int id, int &paragId, int &index );
    void setFormat( int id, QTextEditFormat *f, int flags );
    QTextEditParag *selectionStart( int id );
    QTextEditParag *selectionEnd( int id );

    void load( const QString &fn );
    void save( const QString &fn = QString::null );
    QString fileName() const;

    QString selectedText( int id ) const;
    void copySelectedText( int id );
    void removeSelectedText( int id, QTextEditCursor *cursor );

    void setCompletionEnabled( bool b );
    bool isCompletionEnabled() const;

    void addCompletionEntry( const QString &s );
    QStringList completionList( const QString &s ) const;

    void updateCompletionMap();

    QTextEditParag *paragAt( int i ) const;

    void addCommand( QTextEditCommand *cmd );
    QTextEditCursor *undo( QTextEditCursor *c = 0 );
    QTextEditCursor *redo( QTextEditCursor *c  = 0 );

    QTextEditFormatCollection *formatCollection() const;

    int listIndent( int depth ) const;
    Bullet bullet( int depth ) const;

private:
    struct Selection {
	QTextEditParag *startParag, *endParag;
	int startIndex;
    };

    int cx, cy, cw;
    QTextEditParag *fParag, *lParag;
    QTextEditSyntaxHighlighter *syntaxHighlighte;
    QMap<int, QColor> selectionColors;
    QMap<int, Selection> selections;
    QString filename;
    bool parenCheck, completion;
    QTextEditCommandHistory *commandHistory;
    QTextEditFormatter *pFormatter;
    QTextEditIndent *indenter;
    QTextEditFormatCollection *fCollection;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditString
{
public:
    struct Char {
    public:
	Char() : format( 0 ), lineStart( 0 ) {}
	~Char() { format = 0; }
	QChar c;
	ushort x;
	QTextEditFormat *format;
	uint lineStart : 1;
    private:
	Char &operator=( const Char & ) {
	    return *this;
	}
	
    };

    QTextEditString( QTextEditParag *p );

    QString toString() const;

    Char &at( int i ) const;
    int length() const;

    void insert( int index, const QString &s, QTextEditFormat *f );
    void truncate( int index );
    void remove( int index, int len );

    void setFormat( int index, QTextEditFormat *f, bool useCollection );

private:
    QArray<Char> data;
    QTextEditParag *parag;
    QString cache;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditParag
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
	Normal,
	BulletList,
	EnumList
    };

    QTextEditParag( QTextEditDocument *d, QTextEditParag *pr, QTextEditParag *nx, bool updateIds = TRUE );

    Type type() const;
    void setType( Type t );

    QTextEditString *string() const;
    QTextEditString::Char *at( int i ) const; // maybe remove later
    int length() const; // maybe remove later

    QTextEditDocument *document() const;

    QRect rect() const;

    QTextEditParag *prev() const;
    QTextEditParag *next() const;
    void setPrev( QTextEditParag *s );
    void setNext( QTextEditParag *s );

    void insert( int index, const QString &s );
    void append( const QString &s );
    void truncate( int index );
    void remove( int index, int len );
    void join( QTextEditParag *s );

    void invalidate( int chr );

    void move( int dy );
    void format( int start = -1, bool doMove = TRUE );

    bool isValid() const;
    bool hasChanged() const;
    void setChanged( bool b );

    int lineHeightOfChar( int i, int *bl = 0, int *y = 0 ) const;
    QTextEditString::Char *lineStartOfChar( int i, int *index = 0, int *line = 0 ) const;
    int lines() const;
    QTextEditString::Char *lineStartOfLine( int line, int *index = 0 ) const;
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

    void setFormat( int index, int len, QTextEditFormat *f, bool useCollection, int flags = -1 );

    int leftIndent() const;
    int listDepth() const;
    void setListDepth( int d );

private:
    struct Selection {
	int start, end;
    };

    QMap<int, LineStart*> lineStarts;
    int invalid;
    QRect r;
    QTextEditParag *p, *n;
    QTextEditDocument *doc;
    bool changed;
    bool firstFormat, firstHilite;
    QMap<int, Selection> selections;
    int state, id;
    bool needHighlighte;
    ParenList parens;
    int lastLenForCompletion;
    QTextEditString *str;
    Type typ;
    int left;
    int depth;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditFormatter
{
public:
    QTextEditFormatter( QTextEditDocument *d );
    virtual ~QTextEditFormatter() {}
    virtual int format( QTextEditParag *parag, int start ) = 0;

protected:
    QTextEditDocument *doc;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditFormatterBreakInWords : public QTextEditFormatter
{
public:
    QTextEditFormatterBreakInWords( QTextEditDocument *d );
    int format( QTextEditParag *parag, int start );

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditFormatterBreakWords : public QTextEditFormatter
{
public:
    QTextEditFormatterBreakWords( QTextEditDocument *d );
    int format( QTextEditParag *parag, int start );

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditIndent
{
public:
    QTextEditIndent( QTextEditDocument *d );
    virtual void indent( QTextEditParag *parag, int *oldIndent = 0, int *newIndent = 0 ) = 0;

protected:
    QTextEditDocument *doc;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditSyntaxHighlighter
{
public:
    enum Ids {
	Standard = 0
    };

    QTextEditSyntaxHighlighter( QTextEditDocument *d );
    virtual void highlighte( QTextEditParag *, int, bool = TRUE ) = 0;
    virtual QTextEditFormat *format( int id ) = 0;

protected:
    QTextEditDocument *doc;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditFormat
{
    friend class QTextEditFormatCollection;
    
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
    
    QTextEditFormat( const QFont &f, const QColor &c );
    QTextEditFormat( const QTextEditFormat &fm );
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

    bool operator==( const QTextEditFormat &f ) const;    
    QTextEditFormatCollection *parent() const;
    QString key() const;
    
    void addRef();
    void removeRef();
    
private:
    void update();
    void generateKey();
    const QFontMetrics *fontMetrics() const;
    QTextEditFormat() {}

private:
    QFont fn;
    QColor col;
    QFontMetrics *fm;
    int leftBearing, rightBearing;
    int widths[ 65536 ];
    int hei, asc, dsc;
    QTextEditFormatCollection *collection;
    int ref;
    QString k;
    
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class QTextEditFormatCollection
{
public:
    QTextEditFormatCollection();

    void setDefaultFormat( QTextEditFormat *f );
    QTextEditFormat *defaultFormat() const;
    QTextEditFormat *format( QTextEditFormat *f );
    QTextEditFormat *format( QTextEditFormat *of, QTextEditFormat *nf, int flags );
    void remove( QTextEditFormat *f );
    
private:
    QTextEditFormat *defFormat, *lastFormat;
    QDict<QTextEditFormat> cKey;
    QTextEditFormat *cres;
    QString kof, knf;
    int cflags;
    
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextEditParag *QTextEditCursor::parag() const
{
    return string;
}

inline int QTextEditCursor::index() const
{
    return idx;
}

inline void QTextEditCursor::setIndex( int i )
{
    tmpIndex = -1;
    idx = i;
}

inline bool QTextEditCursor::checkParens()
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

inline void QTextEditCursor::setParag( QTextEditParag *s )
{
    idx = 0;
    string = s;
    tmpIndex = -1;
}

inline void QTextEditCursor::checkIndex()
{
    if ( idx >= string->length() )
	idx = string->length() - 1;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline int QTextEditDocument::x() const
{
    return cx;
}

inline int QTextEditDocument::y() const
{
    return cy;
}

inline int QTextEditDocument::width() const
{
    return cw;
}

inline QTextEditParag *QTextEditDocument::firstParag() const
{
    return fParag;
}

inline QTextEditParag *QTextEditDocument::lastParag() const
{
    return lParag;
}

inline void QTextEditDocument::setFirstParag( QTextEditParag *p )
{
    fParag = p;
}

inline void QTextEditDocument::setLastParag( QTextEditParag *p )
{
    lParag = p;
}

inline void QTextEditDocument::setWidth( int w )
{
    cw = w;
}

inline QTextEditSyntaxHighlighter *QTextEditDocument::syntaxHighlighter() const
{
    return syntaxHighlighte;
}

inline void QTextEditDocument::setSyntaxHighlighter( QTextEditSyntaxHighlighter * sh )
{
    syntaxHighlighte = sh;
}

inline void QTextEditDocument::setFormatter( QTextEditFormatter *f )
{
    pFormatter = f;
}

inline QTextEditFormatter *QTextEditDocument::formatter() const
{
    return pFormatter;
}

inline void QTextEditDocument::setIndent( QTextEditIndent *i )
{
    indenter = i;
}

inline QTextEditIndent *QTextEditDocument::indent() const
{
    return indenter;
}

inline void QTextEditDocument::setParenCheckingEnabled( bool b )
{
    parenCheck = b;
}

inline bool QTextEditDocument::isParenCheckingEnabled() const
{
    return parenCheck;
}

inline QColor QTextEditDocument::selectionColor( int id ) const
{
    return selectionColors[ id ];
}

inline bool QTextEditDocument::hasSelection( int id ) const
{
    return selections.find( id ) != selections.end();
}

inline void QTextEditDocument::setSelectionStart( int id, QTextEditCursor *cursor )
{
    Selection sel;
    sel.startParag = cursor->parag();
    sel.endParag = cursor->parag();
    sel.startParag->setSelection( id, cursor->index(), cursor->index() );
    sel.startIndex = cursor->index();
    selections[ id ] = sel;
}

inline QTextEditParag *QTextEditDocument::paragAt( int i ) const
{
    QTextEditParag *s = fParag;
    while ( s ) {
	if ( s->paragId() == i )
	    return s;
	s = s->next();
    }
    return 0;
}

inline void QTextEditDocument::setCompletionEnabled( bool b )
{
    completion = b;
}

inline bool QTextEditDocument::isCompletionEnabled() const
{
    return completion;
}

inline QTextEditFormatCollection *QTextEditDocument::formatCollection() const
{
    return fCollection;
}

inline int QTextEditDocument::listIndent( int depth ) const
{
    // #######
    return ( depth + 1 ) * 15;
}

inline QTextEditDocument::Bullet QTextEditDocument::bullet( int depth ) const
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

inline QTextEditFormat::QTextEditFormat( const QFont &f, const QColor &c )
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

inline QTextEditFormat::QTextEditFormat( const QTextEditFormat &f )
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

inline void QTextEditFormat::update()
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

inline const QFontMetrics *QTextEditFormat::fontMetrics() const
{
    return fm;
}

inline QColor QTextEditFormat::color() const
{
    return col;
}

inline QFont QTextEditFormat::font() const
{
    return fn;
}

inline int QTextEditFormat::minLeftBearing() const
{
    return leftBearing;
}

inline int QTextEditFormat::minRightBearing() const
{
    return rightBearing;
}

inline int QTextEditFormat::width( const QChar &c ) const
{
    if ( c == '\t' )
	return 30;
    int w = widths[ c.unicode() ];
    if ( w == 0 ) {
	w = fm->width( c );
	( (QTextEditFormat*)this )->widths[ c.unicode() ] = w;
    }
    return w;
}

inline int QTextEditFormat::height() const
{
    return hei;
}

inline int QTextEditFormat::ascent() const
{
    return asc;
}

inline int QTextEditFormat::descent() const
{
    return dsc;
}

inline bool QTextEditFormat::operator==( const QTextEditFormat &f ) const 
{
    return k == f.k;
}
    
inline QTextEditFormatCollection *QTextEditFormat::parent() const
{
    return collection;
}

inline void QTextEditFormat::addRef()
{
    ref++;
#ifdef DEBUG_COLLECTION
    qDebug( "add ref of '%s' to %d (%p)", k.latin1(), ref, this ); 
#endif
}

inline void QTextEditFormat::removeRef()
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

inline QString QTextEditFormat::key() const
{
    return k;
}

inline void QTextEditFormat::generateKey()
{
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family();
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextEditString::Char &QTextEditString::at( int i ) const
{
    return data[ i ];
}

inline QString QTextEditString::toString() const
{
    return cache;
}

inline int QTextEditString::length() const
{
    return data.size();
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextEditString::Char *QTextEditParag::at( int i ) const
{
    return &str->at( i );
}

inline int QTextEditParag::length() const
{
    return str->length();
}

inline bool QTextEditParag::isValid() const
{
    return invalid == -1;
}

inline bool QTextEditParag::hasChanged() const
{
    return changed;
}

inline void QTextEditParag::setChanged( bool b )
{
    changed = b;
}

inline void QTextEditParag::append( const QString &s )
{
    insert( str->length(), s );
}

inline QRect QTextEditParag::rect() const
{
    return r;
}

inline QTextEditParag *QTextEditParag::prev() const
{
    return p;
}

inline QTextEditParag *QTextEditParag::next() const
{
    return n;
}

inline void QTextEditParag::setSelection( int id, int start, int end )
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

inline void QTextEditParag::removeSelection( int id )
{
    selections.remove( id );
    changed = TRUE;
}

inline int QTextEditParag::selectionStart( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return -1;
    return ( *it ).start;
}

inline int QTextEditParag::selectionEnd( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return -1;
    return ( *it ).end;
}

inline bool QTextEditParag::hasSelection( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;
    return ( *it ).start != ( *it ).end;
}

inline bool QTextEditParag::hasAnySelection() const
{
    return !selections.isEmpty();
}

inline bool QTextEditParag::fullSelected( int id ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;
    return ( *it ).start == 0 && ( *it ).end == str->length() - 1;
}

inline void QTextEditParag::setEndState( int s )
{
    if ( s == state )
	return;
    state = s;
}

inline int QTextEditParag::endState() const
{
    return state;
}

inline void QTextEditParag::setParagId( int i )
{
    id = i;
}

inline int QTextEditParag::paragId() const
{
    if ( id == -1 )
	qWarning( "invalid parag id!!!!!!!! (%p)", this );
    return id;
}

inline bool QTextEditParag::firstHighlighte() const
{
    return firstHilite;
}

inline void QTextEditParag::setFirstHighlighte( bool b )
{
    firstHilite = b;
}

inline QTextEditParag::ParenList &QTextEditParag::parenList()
{
    return parens;
}

inline QMap<int, QTextEditParag::LineStart*> &QTextEditParag::lineStartList()
{
    return lineStarts;
}

inline int QTextEditParag::lastLengthForCompletion() const
{
    return lastLenForCompletion;
}

inline void QTextEditParag::setLastLengthFotCompletion( int l )
{
    lastLenForCompletion = l;
}

inline int QTextEditParag::lineY( int l ) const
{
    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->y;
}

inline int QTextEditParag::lineBaseLine( int l ) const
{
    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->baseLine;
}

inline int QTextEditParag::lineHeight( int l ) const
{
    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->h;
}

inline void QTextEditParag::lineInfo( int l, int &y, int &h, int &bl ) const
{
    QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    y = ( *it )->y;
    h = ( *it )->h;
    bl = ( *it )->baseLine;
}

inline QTextEditString *QTextEditParag::string() const
{
    return str;
}

inline QTextEditDocument *QTextEditParag::document() const
{
    return doc;
}

inline QTextEditParag::Type QTextEditParag::type() const
{
    return typ;
}

inline void QTextEditParag::setType( Type t )
{
    if ( t != typ )
	invalidate( 0 );
    typ = t;
    if ( t == Normal )
	left = 0;
}

inline int QTextEditParag::leftIndent() const
{
    return left;
}

inline int QTextEditParag::listDepth() const
{
    return depth;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void QTextEditFormatCollection::setDefaultFormat( QTextEditFormat *f )
{
    defFormat = f;
}

inline QTextEditFormat *QTextEditFormatCollection::defaultFormat() const
{
    return defFormat;
}

#endif
