#include <private/qrichtext_p.h>
#include <qpalette.h>
#include "qlogview.h"

/* \class QLogView qlogview.h
   
   <to be integrated with QTextEdit>
*/

/*! \internal
*/
class QLogViewPrivate
{
public:
    QLogViewPrivate() : mousePressed( FALSE ), len( 0 ), numLines( 0 ) 
    {
	selStart.line = selStart.index = -1;
	selEnd.line = selEnd.index = -1;
    }
    ~QLogViewPrivate() {}
    QPoint oldCoord;
    bool mousePressed;
    int anchorX;
    int len;
    int numLines;
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
}

/*! \internal
*/
void QLogView::init()
{
    d =  new QLogViewPrivate;
    d->oldCoord.setX( 0 );
    d->oldCoord.setY( 0 );
    d->anchorX = 0;
    viewport()->setBackgroundMode( PaletteBase );
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
    for ( QStringList::Iterator it = strl.begin(); it != strl.end(); ++it )
	d->lines[ d->numLines++ ] = *it;
    
    QFontMetrics fm( font() );
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
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
    
    int oldLines = d->numLines -1 > 0 ? d->numLines - 1 : 0;
    d->len += str.length();
    QStringList strl = QStringList::split( '\n', str, TRUE );
    QStringList::Iterator it = strl.begin();
    
    // append first line in str to previous line in buffer
    if ( d->numLines > 0 ) {
	d->lines[ d->numLines - 1 ].append( *it );
	++it;
    }
    
    for ( ; it != strl.end(); ++it )
	d->lines[ d->numLines++ ] = *it;

    QFontMetrics fm( font() );
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
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
    
    // we always have to fetch at least 2 lines for drawing because the
    // painter may be translated so that parts of two lines are displayed
    // in the area of a single line
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
    // find start parag and index for selection
    if ( selStart != -1 && selEnd != -1 ) {
	if ( startLine <= selStart && endLine >= selEnd )
	{
// 	    qWarning("case 1");
	    // case 1: area to paint covers entire selection
	    int paragS = selStart - startLine;
	    int paragE = paragS + (selEnd - selStart);
	    QTextParag * parag = doc->paragAt( paragS );
	    c1.setParag( parag );
	    if ( doc->text( paragS ).length() >= idxStart )
		c1.setIndex( idxStart );
	    parag = doc->paragAt( paragE );
	    c2.setParag( parag );
	    if ( doc->text( paragE ).length() >= idxEnd )
		c2.setIndex( idxEnd );
	    doc->setSelectionStart( QTextDocument::Standard, &c1 );
	    doc->setSelectionEnd( QTextDocument::Standard, &c2 );
	} else if ( startLine > selStart && endLine < selEnd )
	{
	    // case 2: area to paint is all part of the selection
	    doc->selectAll( QTextDocument::Standard );
	} else if ( startLine > selStart && endLine >= selEnd &&
		    startLine <= selEnd )
	{
// 	    qWarning("case 3");
	    // case 3: area to paint starts inside a selection, ends past it
	    c1.setParag( doc->firstParag() );
	    c1.setIndex( 0 );
	    int paragE = selEnd - startLine;
	    QTextParag * parag = doc->paragAt( paragE );
	    c2.setParag( parag );
	    if ( doc->text( paragE ).length() >= idxEnd )
		c2.setIndex( idxEnd );
	    doc->setSelectionStart( QTextDocument::Standard, &c1 );
	    doc->setSelectionEnd( QTextDocument::Standard, &c2 );
	} else if ( startLine <= selStart && endLine < selEnd &&
		    endLine > selStart )
	{
// 	    qWarning("case 4");
	    // case 4: area to paint starts before a selection, ends inside it
	    int paragS = selStart - startLine;
	    QTextParag * parag = doc->paragAt( paragS );
	    c1.setParag( parag );
	    c1.setIndex( idxStart );
	    c2.setParag( doc->lastParag() );
	    c2.setIndex( doc->lastParag()->string()->toString().length() - 1 );
	    
	    doc->setSelectionStart( QTextDocument::Standard, &c1 );
	    doc->setSelectionEnd( QTextDocument::Standard, &c2 );
	}
    }
    
//     static int pp = 0;
//     QBrush br;
//     if ( !pp ) {
// 	pp = 1;
// 	br = red;
//     } else {
// 	pp = 0;
// 	br = green;
//     }
//     QColorGroup col = colorGroup();
//     col.setBrush( QColorGroup::Base, br );
    
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
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

void QLogView::contentsMousePressEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton )
	return;
    
    QFontMetrics fm( font() );
    d->mousePressed = TRUE;
    d->selStart.line = e->y() / fm.lineSpacing();
    QString str = d->lines[ d->selStart.line ];
    if ( e->x() > fm.width( str ) ) {
	d->selStart.index = str.length();
    } else {
	int i = 0;
	int dd, dist = 10000000;
	int curpos = str.length();
	while ( i < str.length() ) {
	    dd = fm.width( str.right( i ) ) - e->x();
	    if ( QABS(dd) < dist || dist == dd ) {
		dist = QABS(dd);
		if ( e->x() >= fm.width( str.right( i ) ) ) {
		    if ( d->anchorX > e->x() )
			curpos = i;
		    else
			curpos = i + 1;
		}
	    }
	    i++;
	}
	d->selStart.index = curpos;
// 	int i = 0;
// 	while ( i < str.length() ) {
// 	    if ( fm.width( str.right( i + 1 ) ) < e->x() )
// 		i++;
// 	    else
// 		break;
// 	}
// 	d->selStart.index = i;
    }
    d->selEnd.line = d->selStart.line;
    d->selEnd.index = d->selStart.index;
    d->anchorX = e->x();
    repaintContents( FALSE );
}

void QLogView::contentsMouseReleaseEvent( QMouseEvent * e )
{
    QFontMetrics fm( font() );
    d->mousePressed = FALSE;
    d->selEnd.line = e->y() / fm.lineSpacing();
    QString str = d->lines[ d->selEnd.line ];
    if ( e->x() > fm.width( str ) ) {
	d->selEnd.index = str.length();
    } else {
	int i = 0;
	int dd, dist = 10000000;
	int curpos = str.length();
	while ( i < str.length() ) {
	    dd = fm.width( str.right( i ) ) - e->x();
	    if ( QABS(dd) < dist || dist == dd ) {
		dist = QABS(dd);
		if ( e->x() >= fm.width( str.right( i ) ) ) {
		    if ( d->anchorX > e->x() )
			curpos = i;
		    else
			curpos = i + 1;
		}
	    }
	    i++;
	}
	d->selEnd.index = curpos;
    }
    if ( d->selEnd.line < d->selStart.line ) {
	int tmp = d->selStart.line;
	d->selStart.line = d->selEnd.line;
	d->selEnd.line = tmp;
	tmp = d->selStart.index;
	d->selStart.index = d->selEnd.index;
	d->selEnd.index = tmp;
    }
    repaintContents( FALSE );
}

void QLogView::contentsMouseMoveEvent( QMouseEvent * e )
{
    QFontMetrics fm( font() );
    if ( d->mousePressed ) {
	d->selEnd.line = e->y() / fm.lineSpacing();
	if ( d->selEnd.line < 0 )
	    d->selEnd.line = 0;
	QString str = d->lines[ d->selEnd.line ];
	if ( e->x() > fm.width( str ) ) {
	    d->selEnd.index = str.length();
	} else {
	    int i = 0;
	    int dd, dist = 10000000;
	    int curpos = 0;
	    while ( i < str.length() ) {
		dd = fm.width( str.right( i ) ) - e->x();
		if ( QABS(dd) < dist || dist == dd ) {
		    dist = QABS(dd);
		    if ( e->x() >= fm.width( str.right( i ) ) ) {
			if ( d->anchorX > e->x() )
			    curpos = i;
			else
			    curpos = i + 1;
		    }
		}
		i++;
 	    }
	    d->selEnd.index = curpos;
	}
	// calc height of rect that needs redrawing
	int h = QABS(e->y() - d->oldCoord.y()) + fm.lineSpacing() * 2;
	int y;
	if ( d->oldCoord.y() < e->y() )
	    y = d->oldCoord.y() - fm.lineSpacing() * 1;
	else
	    y = e->y() - fm.lineSpacing() * 1;
	if ( y < 0 )
	    y = 0;
	repaintContents( 0, y, width(), h, FALSE ); 
	d->oldCoord = e->pos();
    }
}
