#include <private/qrichtext_p.h>
#include <qpalette.h>
#include <qtimer.h>
#include <qcursor.h>
#include "qlogview.h"

/* \class QLogView qlogview.h
   
   <to be integrated with QTextEdit>
*/

/*! \internal
*/
class QLogViewPrivate
{
public:
    QLogViewPrivate() : len( 0 ), numLines( 0 ), maxLineLength( 0 )
    {
	selStart.line = selStart.index = -1;
	selEnd.line = selEnd.index = -1;
    }
    ~QLogViewPrivate() {}
    int anchorX;
    int len;
    int numLines;
    int maxLineLength;
    struct selection {
	int line;
	int index;
    };
    selection selStart, selEnd;
    QMap< int, QString > lines;
};

/*! 
  Construct a QLogView object, initialising the text content to \a
  text.
*/
QLogView::QLogView( const QString & text, QWidget * parent, 
		    const char * name )
    : QScrollView( parent, name )
{
    init();
    setText( text );
}

/*!
  Construct a QLogView object.
*/
QLogView::QLogView( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    init();
}

/*! 
  Destruct the QLogView object.
*/
QLogView::~QLogView()
{
    d->lines.clear();
    delete d;
    delete scrollTimer;
}

/*! \internal
*/
void QLogView::init()
{
    d =  new QLogViewPrivate;
    d->anchorX = 0;
    viewport()->setBackgroundMode( PaletteBase );
    scrollTimer = new QTimer( this );	
    connect( scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ) );
}

/*! 
  Returns the widget text.
*/
QString QLogView::text() const
{
    QString str;    
    
    if ( d->len == 0 )
	return str;

    // concatenate all strings
    int i;
    for ( i = 0; i < d->numLines; i++ ) {
	if ( d->lines[ i ].isEmpty() ) // CR lines are empty
	    str += "\n";
	else
	    str += d->lines[ i ];
    }
    return str;
}

/*!
  Sets the widget text to \a str.
*/
void QLogView::setText( const QString & str )
{
    if ( str.isEmpty() || str.isNull() )
	return;
    
    d->numLines = 0;
    d->lines.clear();
    d->len = str.length();
    QStringList strl = QStringList::split( '\n', str, TRUE );
    QFontMetrics fm( font() );
    int lWidth;
    for ( QStringList::Iterator it = strl.begin(); it != strl.end(); ++it ) {
	d->lines[ d->numLines++ ] = *it;
	lWidth = fm.width( *it );
	if ( lWidth > d->maxLineLength )
	    d->maxLineLength = lWidth;
    }
    
    resizeContents( d->maxLineLength + 4, d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

/*! 
  Appends \a str to the widget contents, and the text color to be \a
  col.
*/
void QLogView::append( const QString & str )
{
    if ( str.isEmpty() || str.isNull() )
	return;
    
    d->len += str.length();
    QStringList strl = QStringList::split( '\n', str, TRUE );
    QStringList::Iterator it = strl.begin();
    
    QFontMetrics fm( font() );
    int lWidth;
    // append first line in str to previous line in buffer
    if ( d->numLines > 0 ) {
	d->lines[ d->numLines - 1 ].append( *it );
	lWidth = fm.width( d->lines[ d->numLines -1 ] );
	if ( lWidth > d->maxLineLength )
	    d->maxLineLength = lWidth;
	++it;
    }
    
    for ( ; it != strl.end(); ++it ) {
	d->lines[ d->numLines++ ] = *it;
	lWidth = fm.width( *it );
	if ( lWidth > d->maxLineLength )
	    d->maxLineLength = lWidth;
    }

    resizeContents( d->maxLineLength + 4, d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}


/*! 
*/
int QLogView::find( const QRegExp & /*reg*/ )
{
    return -1;
}

/*! 
  Returns the text length.
*/
int QLogView::length()
{
    return d->len;
}

/*! \reimp
*/
void QLogView::drawContents( QPainter * p, int clipx, int clipy, int clipw,
			     int cliph )
{
    QFontMetrics fm( font() );
    int startLine = clipy / fm.lineSpacing();
    
    // we always have to fetch at least two lines for drawing because the
    // painter may be translated so that parts of two lines cover the area
    // of a single line
    int nLines = (cliph / fm.lineSpacing()) + 2;
    int endLine = startLine + nLines;
    
    if ( startLine >= d->numLines )
	return;
    if ( (startLine + nLines) > d->numLines )
	nLines = d->numLines - startLine;
    
    int i = 0;
    QString str;
    for ( i = startLine; i < (startLine + nLines); i++ )
	str.append( d->lines[ i ] + "\n" );
    
    QTextDocument * doc = new QTextDocument( 0 );
    doc->setPlainText( str );
    doc->setFormatter( new QTextFormatterBreakWords ); // deleted by QTextDoc
    doc->formatter()->setWrapEnabled( FALSE );

    // if there is a selection, make sure that the selection in the
    // part we need to redraw is correct
    if ( d->selStart.line != -1 && d->selEnd.line != -1 ) {
	QTextCursor c1( doc );
	QTextCursor c2( doc );
	int selStart = d->selStart.line;
	int idxStart = d->selStart.index;
	int selEnd = d->selEnd.line;
	int idxEnd = d->selEnd.index;
	if ( selEnd < selStart ) {
	    selStart = d->selEnd.line;
	    idxStart = d->selEnd.index;
	    selEnd = d->selStart.line;
	    idxEnd = d->selStart.index;
	}
	if ( startLine <= selStart && endLine >= selEnd )
	{
	    // case 1: area to paint covers entire selection
	    int paragS = selStart - startLine;
	    int paragE = paragS + (selEnd - selStart);
	    QTextParag * parag = doc->paragAt( paragS );
	    c1.setParag( parag );
	    if ( doc->text( paragS ).length() >= (uint) idxStart )
		c1.setIndex( idxStart );
	    parag = doc->paragAt( paragE );
	    if ( parag ) {
		c2.setParag( parag );
		if ( doc->text( paragE ).length() >= (uint) idxEnd )
		    c2.setIndex( idxEnd );
	    }
	} else if ( startLine > selStart && endLine < selEnd )
	{
	    // case 2: area to paint is all part of the selection
	    doc->selectAll( QTextDocument::Standard );
	} else if ( startLine > selStart && endLine >= selEnd &&
		    startLine <= selEnd )
	{
	    // case 3: area to paint starts inside a selection, ends past it
	    c1.setParag( doc->firstParag() );
	    c1.setIndex( 0 );
	    int paragE = selEnd - startLine;
	    QTextParag * parag = doc->paragAt( paragE );
	    if ( parag ) {
		c2.setParag( parag );
		if ( doc->text( paragE ).length() >= (uint) idxEnd )
		    c2.setIndex( idxEnd );
	    }
	} else if ( startLine <= selStart && endLine < selEnd &&
		    endLine > selStart )
	{
	    // case 4: area to paint starts before a selection, ends inside it
	    int paragS = selStart - startLine;
	    QTextParag * parag = doc->paragAt( paragS );
	    c1.setParag( parag );
	    c1.setIndex( idxStart );
	    c2.setParag( doc->lastParag() );
	    c2.setIndex( doc->lastParag()->string()->toString().length() - 1 );
	    
	}
	// previously selected?
	if ( !doc->hasSelection( QTextDocument::Standard ) ) {
	    doc->setSelectionStart( QTextDocument::Standard, &c1 );
	    doc->setSelectionEnd( QTextDocument::Standard, &c2 );
	}
    }
        
    // have to align the painter so that partly visible lines are
    // drawn at the correct position within the area that needs to be
    // painted
    int offset = clipy % fm.lineSpacing() + 2;
    QRect r( clipx, 0, clipw, cliph + offset );
    p->translate( 0, clipy - offset );
    doc->draw( p, r.x(), r.y(), r.width(), r.height(), colorGroup() );
    p->translate( 0, -(clipy - offset) );
    delete doc;
}

/*! \reimp
*/
void QLogView::fontChange( const QFont & )
{
    QFontMetrics fm( font() );
    resizeContents( d->maxLineLength + 4, d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

void QLogView::contentsMousePressEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton )
	return;
    
    QFontMetrics fm( font() );
    mousePressed = TRUE;
    d->selStart.line = e->y() / fm.lineSpacing();
    QString str = d->lines[ d->selStart.line ];
    mousePos = e->pos();
    d->selStart.index = charIndex( str );
    d->selEnd.line = d->selStart.line;
    d->selEnd.index = d->selStart.index;
    d->anchorX = e->x();
    oldMousePos = e->pos();
    repaintContents( FALSE );
}

void QLogView::contentsMouseReleaseEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton )
	return;

    QFontMetrics fm( font() );
    mousePressed = FALSE;
    d->selEnd.line = e->y() / fm.lineSpacing();
    QString str = d->lines[ d->selEnd.line ];
    mousePos = e->pos();
    d->selEnd.index = charIndex( str );
    if ( d->selEnd.line < d->selStart.line ) {
	int tmp = d->selStart.line;
	d->selStart.line = d->selEnd.line;
	d->selEnd.line = tmp;
	tmp = d->selStart.index;
	d->selStart.index = d->selEnd.index;
	d->selEnd.index = tmp;
    }
    oldMousePos = e->pos();
    repaintContents( FALSE );
}

void QLogView::contentsMouseMoveEvent( QMouseEvent * e )
{
    mousePos = e->pos();
    doAutoScroll();
    oldMousePos = mousePos;
}

void QLogView::doAutoScroll()
{
    if ( !mousePressed )
	return;
    
    QFontMetrics fm( font() );
    QPoint pos( mapFromGlobal( QCursor::pos() ) );
    bool doScroll = FALSE;
    int xx = contentsX() + pos.x();
    int yy = contentsY() + pos.y();
        
    // find out how much we have to scroll in either dir.
    if ( pos.x() < 0 || pos.x() > viewport()->width() ||
	 pos.y() < 0 || pos.y() > viewport()->height() ) {
	int my = yy;
	if ( pos.x() < 0 )
	    xx = contentsX() - fm.width( 'w');
	else if ( pos.x() > viewport()->width() )
	    xx = contentsX() + viewport()->width() + fm.width('w');

	if ( pos.y() < 0 ) {
	    my = contentsY() - 1;
	    yy = (my / fm.lineSpacing()) * fm.lineSpacing() + 1;
	} else if ( pos.y() > viewport()->height() ) {
	    my = contentsY() + viewport()->height() + 1;
	    yy = (my / fm.lineSpacing() + 1) * fm.lineSpacing() - 1;
	}	
	d->selEnd.line = my / fm.lineSpacing();
  	mousePos.setX( xx );
 	mousePos.setY( my );
	doScroll = TRUE;
    } else {
	d->selEnd.line = mousePos.y() / fm.lineSpacing();
    }
	
    if ( d->selEnd.line < 0 )
	d->selEnd.line = 0;
    
    QString str = d->lines[ d->selEnd.line ];
    d->selEnd.index = charIndex( str );
    
    // have to have a valid index before generating a paint event
    if ( doScroll )
	ensureVisible( xx, yy, 1, 1 );

    // calc pos and height of rect that needs redrawing
    int h = QABS(mousePos.y() - oldMousePos.y()) + fm.lineSpacing() * 2;
    int y;
    if ( oldMousePos.y() < mousePos.y() )
	y = oldMousePos.y() - fm.lineSpacing();
    else
	y = mousePos.y() - fm.lineSpacing();
    if ( y < 0 )
	y = 0;
    repaintContents( contentsX(), y, width(), h, FALSE );

    if ( !scrollTimer->isActive() && pos.y() < 0 || pos.y() > height() )
	scrollTimer->start( 100, FALSE );
    else if ( scrollTimer->isActive() && pos.y() >= 0 && pos.y() <= height() )
	scrollTimer->stop();
}

int QLogView::charIndex( const QString & str )
{
    QFontMetrics fm( font() );
    uint i = 0;
    int dd, dist = 10000000;
    int curpos = str.length();

    if ( mousePos.x() > fm.width( str ) )
	return str.length();
    
    while ( i < str.length() ) {
	dd = fm.width( str.right( i ) ) - mousePos.x();
	if ( QABS(dd) < dist || dist == dd ) {
	    dist = QABS(dd);
	    if ( mousePos.x() >= fm.width( str.right( i ) ) ) {
		if ( d->anchorX > mousePos.x() )
		    curpos = i;
		else
		    curpos = i + 1;
	    }
	}
	i++;
    }
    return curpos;
}
