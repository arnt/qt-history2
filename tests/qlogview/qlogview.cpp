#include <private/qrichtext_p.h>
#include <qpalette.h>
#include "qlogview.h"

/* \class QLogView qlogview.h
   
   This class is designed for displaying and storing plain text more
   efficiently than QTextView. Storing large amounts of plain text in
   a QTextView widget is simply not feasible since the rich text
   engine incurs a noticeable overhead for each character that is
   stored. This is not the case with the QLogView class since it does
   not support editing or complex formatting. You can, however, assign
   a color to each line in the QLogView.
*/

/*! \internal
*/
class QLogViewPrivate
{
public:
    QLogViewPrivate() : len(0), numLines(0) {}
    ~QLogViewPrivate() {}
    int len;
    int numLines;
    QMap< int, QString > lines;
    QMap< int, QColor >  colors;
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

/*! \internal
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
void QLogView::setText( const QString & str, const QColor & col )
{
    if ( str.isEmpty() || str.isNull() )
	return;
    
    d->numLines = 0;
    d->lines.clear();
    d->colors.clear();    
    d->len = str.length();
    QStringList strl = QStringList::split( '\n', str, TRUE );
    for ( QStringList::Iterator it = strl.begin(); it != strl.end(); ++it )
	d->lines[ d->numLines++ ] = *it;
    
    if ( col != QColor() ) {
	int i;
	for ( i = 0; i < d->numLines; i++ )
	    d->colors[ i ] = col;
    }
    
    QFontMetrics fm( font() );
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

/*! 
  Appends \a str to the widget contents, and the text color to be \a
  col.
*/
void QLogView::append( const QString & str, const QColor & col )
{
    if ( str.isEmpty() || str.isNull() )
	return;
    
    int oldLines = d->numLines -1 > 0 ? d->numLines - 1 : 0;
    d->len += str.length();
    QStringList strl = QStringList::split( '\n', str, TRUE );
    QStringList::Iterator it = strl.begin();
    
    if ( d->numLines > 0 ) {
	d->lines[ d->numLines - 1 ].append( *it );
	++it;
    }
    
    for ( ; it != strl.end(); ++it )
	d->lines[ d->numLines++ ] = *it;

    if ( col != QColor() ) {
	int i;
	for ( i = oldLines; i < d->numLines; i++ )
	    d->colors[ i ] = col;
    }

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
void QLogView::drawContents( QPainter * p, int /*clipx*/, int clipy, int /*clipw*/, int cliph )
{
    QFontMetrics fm( font() );
    int startLine = clipy / fm.lineSpacing();
    int nlines = (cliph / fm.lineSpacing()) + 2;
    
    if ( startLine >= d->numLines )
	return;
    if ( (startLine + nlines) > d->numLines )
	nlines = d->numLines - startLine;

    int i = 0;
    for ( i = startLine; i < (startLine + nlines); i++ ) {
	if ( d->colors.find( i ) != d->colors.end() )
	    p->setPen( d->colors[ i ] );
	else
	    p->setPen( colorGroup().text() );
 	p->drawText( contentsX(), (i + 1) * fm.lineSpacing(), d->lines[ i ] );
    }
}

/*! \reimp
*/
void QLogView::fontChange( const QFont & )
{
    QFontMetrics fm( font() );
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

/*!
  Set the color of \a line to be \a col.
*/
void QLogView::setLineColor( int line, const QColor & col )
{
    if ( col != QColor() )
	d->colors[ line ] = col;
}
