#include <private/qrichtext_p.h>
#include <qpalette.h>
#include "qlogview.h"

/* \class QLogView qlogview.h
   
   This is a class that is designed for displaying/storing large text
   documents, like log files. The internal format is UTF8, which is an
   efficient way of storing large blocks of text. This widget will
   typically only need to allocate the number of bytes used to store
   the actual text - there is very little overhead involved.
 */

class QLogViewPrivate
{
public:
    QLogViewPrivate() : len(0), numLines(0), index(0), text(0) {}
    ~QLogViewPrivate() {}    
    int len;
    int numLines;
    QMemArray<int>  index;
    QMemArray<char> text;
};

/*! 
*/
QLogView::QLogView( const QString & text, QWidget * parent, 
		    const char * name )
    : QScrollView( parent, name )
{
    init();
    setText( text );
}

/*! 
*/
QLogView::QLogView( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    init();
}

/*! 
*/
QLogView::~QLogView()
{
    delete d;
}

/*! 
*/
void QLogView::init()
{
    d =  new QLogViewPrivate;
    d->index.resize( 100 );
    d->index.data()[ 0 ] = 0;
    
    viewport()->setBackgroundMode( PaletteBase );
}

/*! 
*/
QString QLogView::text() const
{
    if ( d->len )
	return QString::fromUtf8( d->text.data() );
    return QString();
}

/*! 
*/
void QLogView::setText( const QString & str )
{
    if ( str.isEmpty() || str.isNull() )
	return;
    
    QCString utf8 = str.utf8();
    int i = 0;
    d->numLines = 0;
    // count lines and build an index of where each line starts
    while ( (i = utf8.find( '\n', i )) != -1 ) {
	if ( d->index.size() <= d->numLines )
	    d->index.resize( d->numLines + 100 );
	d->index[ d->numLines ] = i;
	d->numLines++;
	i++;
    }
    
    // allocate 50% more space than the text needs - save # mallocs
    d->text.resize( utf8.length() * 1.2 );
    d->len = utf8.length();
    qmemmove( d->text.data(), (const char *) utf8, utf8.length() );
    d->text[ d->len + 1 ] = 0;
    
    QFontMetrics fm( font() );
    int h = d->numLines * fm.lineSpacing() + fm.descent() + 1;
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

/*! 
*/
void QLogView::append( const QString & str )
{
    if ( str.isEmpty() || str.isNull() )
	return;
    
    QCString utf8 = str.utf8();    
    int i = 0;
    // count lines and build index
    while ( (i = utf8.find( '\n', i )) != -1 ) {
	if ( d->index.size() <= d->numLines )
	    d->index.resize( d->numLines + 100 );
	d->index[ d->numLines ] = d->len + i;	
	d->numLines++;
	i++;
    }
    
    int tlen = d->len;
    // allocate 50% more space than the text needs - save # mallocs
    if ( d->text.size() < utf8.length() + d->len )
	d->text.resize( (utf8.length() + d->len) * 1.2 );
    d->len = utf8.length() + tlen;
    qmemmove( d->text.data() + tlen, (const char *) utf8, utf8.length() );

    QFontMetrics fm( font() );
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

/*! 
*/
int QLogView::find( const QRegExp & reg )
{
    return -1;
}

/*! 
*/
int QLogView::length()
{
    return QString::fromUtf8( d->text.data() ).length();
}

/*! 
*/
void QLogView::drawContents( QPainter * p, int clipx, int clipy, int clipw,
			     int cliph )
{
    QFontMetrics fm( font() );
    int lines = 0;
    int startLine = clipy / fm.lineSpacing();

    QStringList strs = lineRange( startLine, (cliph / fm.lineSpacing()) + 2 );
    for ( QStringList::Iterator it = strs.begin(); it != strs.end(); ++it )
	p->drawText( contentsX(), (startLine + lines++ + 1) * fm.lineSpacing(),
		     *it );
}

/*! \internal
  
  Returns a string list of  \a numLines lines, starting with
  line \a startLine.
 */
QStringList QLogView::lineRange( int startLine, int numLines ) const
{
    QString str;
    
    if ( startLine > d->numLines )
	return QStringList();
    
    // find and extract the range of lines requested
    const char * data = d->text.data();
    int i = 0;
    int startIdx = -1, endIdx = -1;
    
    // calc indices for the requested block
    if ( startLine != 0 )
	startIdx = d->index[ startLine - 1 ] + 1;
    
    if ( (startLine + numLines) < d->numLines )
	endIdx = d->index[ startLine + numLines ];
    
    if ( d->len != 0 ) {
	 char * tmp = 0;
	 int len = 0;
	 if ( startIdx == -1 )
	     startIdx = 0;
	 if ( endIdx != - 1 ) {
	     len = endIdx - startIdx;
	 } else {
	     len = d->len - startIdx;
	 }
	 if ( len != 0 ) {
	     tmp = new char[ len + 1 ]; // for zero termination
	     qmemmove( tmp, data + startIdx, len );
	     tmp[ len ] = 0;
	     str = QString::fromUtf8( tmp );
	     delete[] tmp;
	 }
    }
    QStringList strs = QStringList::split( '\n', str, TRUE );
    return strs;
}

/*! \reimp
 */
void QLogView::fontChange( const QFont & oldFont )
{
    QFontMetrics fm( font() );
    resizeContents( contentsWidth(), d->numLines * fm.lineSpacing() + 
		    fm.descent() + 1 );
}

/*! \internal
  
  Returns the color for line \a line.
*/
QColor QLogView::lineColor( int line ) const
{
    if ( line > d->numLines )
	return QColor();
}
