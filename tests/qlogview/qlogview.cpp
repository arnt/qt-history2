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
    QLogViewPrivate() : mousePressed( FALSE ), len( 0 ), numLines( 0 ) {}
    ~QLogViewPrivate() {}
    bool mousePressed;
    int len;
    int numLines;
    QPoint selectionStart;
    QPoint selectionEnd;
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
void QLogView::drawContents( QPainter * p, int clipx, int clipy, int clipw, int cliph )
{
    QFontMetrics fm( font() );
    int startLine = clipy / fm.lineSpacing();
    int nlines = (cliph / fm.lineSpacing()) + 2;
    
    if ( startLine >= d->numLines )
	return;
    if ( (startLine + nlines) > d->numLines )
	nlines = d->numLines - startLine;
    
    int i = 0;
    QString str;
    for ( i = startLine; i < (startLine + nlines); i++ )
	str.append( d->lines[ i ] + "\n" );
    
    QTextDocument * doc = new QTextDocument( 0 );
    doc->setPlainText( str );
    doc->setFormatter( new QTextFormatterBreakWords ); // deleted by QTextDoc
    doc->formatter()->setWrapEnabled( FALSE );

    // have to align the painter so that partly visible lines are
    // drawn at the correct position within the area that needs to be
    // painted
    QRect r( 0, 0, clipw, cliph + (clipy % fm.lineSpacing()) );
    p->translate( 0, clipy - (clipy % fm.lineSpacing()) );
    doc->draw( p, r, colorGroup() );
    p->translate( 0, -(clipy - (clipy % fm.lineSpacing())) );

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
    d->mousePressed = TRUE;
    d->selectionStart = e->pos();
}

void QLogView::contentsMouseReleaseEvent( QMouseEvent * e )
{
    d->mousePressed = TRUE;
}

void QLogView::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( d->mousePressed ) {
	QPainter p( viewport() );
	p.drawLine( d->selectionStart.x(), d->selectionStart.y(),
		    e->pos().x(), e->pos().y() );
    }
}
