/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.cpp#28 $
**
** Implementation of the QTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtextview.h"
#include "../kernel/qrichtextintern.cpp"

#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"

#include "qstack.h"
#include "stdio.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qlayout.h"
#include "qbitmap.h"
#include "qtimer.h"
#include "qimage.h"
#include "qmime.h"
#include "qdragobject.h"
#include "qclipboard.h"
#include "qdragobject.h"




/*!
  \class QTextView qtextview.h
  \brief A sophisticated single-page rich text viewer.
  \ingroup realwidgets
  \ingroup helpsystem

  Unlike QSimpleRichText, which merely draws small pieces of rich
  text, a QTextView is a real widget, with scrollbars when necessary,
  for showing large text documents.

  The rendering style and available tags are defined by a
  styleSheet(). Currently, a small XML/CSS1 subset including embedded
  images and tables is supported. See QStyleSheet for
  details. Possible images within the text document are resolved by
  using a QMimeSourceFactory.  See setMimeSourceFactory() for details.

  Using QTextView is quite similar to QLabel. It's mainly a call to
  setText() to set the contents. Setting the background color is
  slighty different from other widgets, since a text view is a
  scrollable widget that naturally provides a scrolling background. You
  can specify the colorgroup of the displayed text with
  setPaperColorGroup() or directly define the paper background with
  setPaper(). QTextView supports both plain color and complex pixmap
  backgrounds.

  Note that we do not intend to add a full-featured web browser widget
  to Qt (since that would easily double Qt's size and only few
  applications would benefit from it). In particular, the rich text
  support in Qt is supposed to provide a fast, portable and sufficient
  way to add reasonable online help facilities to applications. We
  will, however, extend it to some degree in future versions of Qt.

  For even more, like hypertext capabilities, see QTextBrowser.
*/

class QTextViewData
{
public:
    QStyleSheet* sheet_;
    QRichText* doc_;
    QMimeSourceFactory* factory_;
    QString original_txt;
    QString txt;
    QString contxt;
    QColorGroup mypapcolgrp;
    QColorGroup papcolgrp;
    QColor mylinkcol;
    QColor paplinkcol;
    bool linkunderline;
    QTimer* resizeTimer;
    QTimer* dragTimer;
    QTimer* scrollTimer;
    Qt::TextFormat textformat;
    QTextCursor* fcresize;
    QPoint cursor;
    uint selection :1;
    uint dirty :1;
    uint dragSelection :1;
};


/*!
  Constructs an empty QTextView
  with the standard \a parent and \a name optional arguments.
*/
QTextView::QTextView(QWidget *parent, const char *name)
    : QScrollView( parent, name, WNorthWestGravity )
{
    init();
}


/*!
  Constructs a QTextView displaying the contents \a text with context
  \a context, with the standard \a parent and \a name optional
  arguments.
*/
QTextView::QTextView( const QString& text, const QString& context,
		      QWidget *parent, const char *name)
    : QScrollView( parent, name, WNorthWestGravity | WRepaintNoErase )
{
    init();
    setText( text, context );
}


void QTextView::init()
{
    d = new QTextViewData;
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;
    d->mylinkcol = blue;
    d->paplinkcol = d->mylinkcol;
    d->linkunderline = TRUE;
    d->fcresize = 0;

    setKeyCompression( TRUE );
    setVScrollBarMode( QScrollView::Auto );
    setHScrollBarMode( QScrollView::Auto );

    d->doc_ = 0;
    d->sheet_ = 0;
    d->factory_ = 0;
    d->txt = QString::fromLatin1("<p></p>");
    d->textformat = AutoText;
    d->dirty = TRUE;
    d->selection = FALSE;
    d->dragSelection = FALSE;

    viewport()->setBackgroundMode( PaletteBase );
    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( WheelFocus );

    d->resizeTimer = new QTimer( this );
    connect( d->resizeTimer, SIGNAL( timeout() ), this, SLOT( doResize() ));
    d->dragTimer = new QTimer( this );
    connect( d->dragTimer, SIGNAL( timeout() ), this, SLOT( doStartDrag() ));
    d->scrollTimer = new QTimer( this );
    connect( d->scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ));
}

/*!
  Destructs the view.
*/
QTextView::~QTextView()
{
    delete d->doc_;
    delete d;
}

/*!
  Changes the contents of the view to the string \a text and the
  context to \a context.

  \a text may be interpreted either as plain text or as rich text,
  depending on the textFormat(). The default setting is \c AutoText,
  i.e. the text view autodetects the format from \a text.

  The \a context is used to resolve references within the text
  document, for example image sources. It is passed directly to the
  mimeSourceFactory() when quering data.

  \sa text(), setTextFormat()
*/
void QTextView::setText( const QString& text, const QString& context)
{
    delete d->doc_;
    d->doc_ = 0;

    d->original_txt = text;
    d->contxt = context;

    if ( text.isEmpty() )
	d->txt = QString::fromLatin1("<p></p>");
    else if ( d->textformat == AutoText ) {
	if ( QStyleSheet::mightBeRichText( text ) )
	    d->txt = text;
	else
	    d->txt = QStyleSheet::convertFromPlainText( text );
    }
    else if ( d->textformat == PlainText )
	d->txt = QStyleSheet::convertFromPlainText( text );
    else // rich text
	d->txt = text;


    setContentsPos( 0, 0 );
    richText().invalidateLayout();
    richText().flow()->initialize( visibleWidth() );
    updateLayout();
    viewport()->repaint();
}


/*!
  Appends \a text to the current text.

  Useful for log viewers.
*/
void QTextView::append( const QString& text )
{
    richText().append( text,  mimeSourceFactory(), styleSheet() );
    int y = contentsHeight();
    int h = richText().lastChild()->bottomMargin();
    doResize();
    updateContents( contentsX(), y-h, visibleWidth(), h );
}

/*!
  Returns the contents of the view.

  \sa context(), setText()
*/
QString QTextView::text() const
{
    return d->original_txt;
}

/*!
  Returns the context of the view.

  \sa text(), setText()
*/
QString QTextView::context() const
{
    return d->contxt;
}


void QTextView::createRichText()
{
    d->papcolgrp = d->mypapcolgrp;
    d->paplinkcol = d->mylinkcol;

    d->doc_ = new QRichText( d->txt, viewport()->font(), d->contxt,
			     8, mimeSourceFactory(), styleSheet() );
    if (d->doc_->attributes().contains("bgcolor")){
	QColor  col ( d->doc_->attributes()["bgcolor"].latin1() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Base, col );
    }
    if (d->doc_->attributes().contains("link")){
	QColor  col ( d->doc_->attributes()["link"].latin1() );
	if ( col.isValid() )
	    d->paplinkcol = col;
    }
    if (d->doc_->attributes().contains("text")){
	QColor  col ( d->doc_->attributes()["text"].latin1() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Text,  col );
    }
    if (d->doc_->attributes().contains("background")){
	QString imageName = d->doc_->attributes()["background"];
	QPixmap pm;
	const QMimeSource* m =
	    context().isNull()
		? mimeSourceFactory()->data( imageName )
		: mimeSourceFactory()->data( imageName, context() );
	if ( m ) {
	    if ( !QImageDrag::decode( m, pm ) ) {
		qWarning("QTextImage: cannot load %s", imageName.latin1() );
	    }
	}
	if (!pm.isNull())
	    d->papcolgrp.setBrush( QColorGroup::Base, QBrush(d->papcolgrp.base(), pm) );
    }
    d->cursor = QPoint(0,0);
}


/*!
  Returns the current style sheet of the view.

  \sa setStyleSheet()
*/
QStyleSheet* QTextView::styleSheet() const
{
    if (!d->sheet_)
	return QStyleSheet::defaultSheet();
    else
	return d->sheet_;

}

/*!
  Sets the style sheet of the view.

  \sa styleSheet()
*/
void QTextView::setStyleSheet( QStyleSheet* styleSheet )
{
    d->sheet_ = styleSheet;
    viewport()->update();
}


/*!
  Returns the current mime source factory  for the view.

  \sa setMimeSourceFactory()
*/
QMimeSourceFactory* QTextView::mimeSourceFactory() const
{
    if (!d->factory_)
	return QMimeSourceFactory::defaultFactory();
    else
	return d->factory_;

}

/*!
  Sets the mime source factory for the view. The factory is used to
  resolve named references within rich text documents. If no factory
  has been specified, the text view uses the default factory
  QMimeSourceFactory::defaultFactory().

  Ownership of \a factory is \e not transferred to make it possible
  for several text view widgets to share the same mime source.

  \sa mimeSourceFactory()
*/
void QTextView::setMimeSourceFactory( QMimeSourceFactory* factory )
{
    d->factory_ = factory;
    viewport()->update();
}


/*!
  Sets the brush to use as the background to \a pap.

  This may be a nice pergament or marble pixmap or simply another
  plain color.

  Technically, setPaper() is just a convenience function to set the
  base brush of the paperColorGroup().

  \sa paper()
*/
void QTextView::setPaper( const QBrush& pap)
{
    d->mypapcolgrp.setBrush( QColorGroup::Base, pap );
    d->papcolgrp.setBrush( QColorGroup::Base, pap );
    viewport()->update();
}

/*!
  Sets the full colorgroup of the paper to \a colgrp. If not specified
  otherwise in the document itself, any text will use
  QColorGroup::text(). The background will be painted with
  QColorGroup::brush(QColorGroup::Base).

  \sa paperColorGroup(), setPaper()
*/
void QTextView::setPaperColorGroup( const QColorGroup& colgrp)
{
    d->mypapcolgrp = colgrp;
    d->papcolgrp = colgrp;
    viewport()->update();
}

/*!
  Returns the colorgroup of the paper.

  \sa setPaperColorGroup(), setPaper()
*/
const QColorGroup& QTextView::paperColorGroup() const
{
    return d->papcolgrp;
}

/*!
  Sets the color used to display links in the document to \c col.

  \sa linkColor()
 */
void QTextView::setLinkColor( const QColor& col )
{
    d->mylinkcol = col;
    d->paplinkcol = col;
}

/*!
  Returns the current link color.

  The color may either have been set with setLinkColor() or stem from
  the document's body tag.

  \sa setLinkColor()
 */
const QColor& QTextView::linkColor() const
{
    return d->paplinkcol;
}

/*!
  Defines wether or not links should be displayed underlined.
 */
void QTextView::setLinkUnderline( bool u)
{
    d->linkunderline = u;
}

/*!
  Returns wether or not links should be displayed underlined.
 */
bool QTextView::linkUnderline() const
{
    return d->linkunderline;
}


/*!
  Returns the document title parsed from the content.
*/
QString QTextView::documentTitle() const
{
    return richText().attributes()["title"];
}

/*!
  Returns the height of the view given a width of \a w.
*/
int QTextView::heightForWidth( int w ) const
{
//     QRichText doc ( d->txt, viewport()->font(), d->contxt,
// 		    8, mimeSourceFactory(), styleSheet() );
//     {
// 	QPainter p( this );
// 	doc.setWidth(&p, w);
//     }
//     return doc.height;
    return w;
}

/*!
  Returns the document defining the view as drawable and querable rich
  text object.  This is not currently useful for applications.
*/
QRichText& QTextView::richText() const
{
    if (!d->doc_){
	QTextView* that = (QTextView*) this;
	that->createRichText();
    }
    return *d->doc_;
}

/*!
  Returns the brush used to paint the background.
*/
const QBrush& QTextView::paper()
{
    return d->papcolgrp.brush( QColorGroup::Base );
}

/*!
  \reimp
*/
void QTextView::drawContentsOffset(QPainter* p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QTextOptions to(&paper(), d->paplinkcol, d->linkunderline );
    to.offsetx = ox;
    to.offsety = oy;

    QRegion r(cx-ox, cy-oy, cw, ch);

    QTextCursor tc( richText() );
    tc.gotoParagraph( p, richText().getParBefore( cy ) );
    QTextParagraph* b = tc.paragraph;

    // TODO merge with update, this is only draw. Everything needs to be clean!
    QFontMetrics fm( p->fontMetrics() );
    while ( b && tc.y() <= cy + ch ) {

	if ( b && b->dirty ) //ensure the paragraph is layouted
	    tc.updateLayout( p, cy + ch );

	tc.gotoParagraph( p, b );

	if ( tc.y() + tc.paragraph->height > cy ) {
	    do {
		tc.makeLineLayout( p );
		QRect geom( tc.lineGeometry() );
		if ( geom.bottom() > cy && geom.top() < cy+ch )
		    tc.drawLine( p, ox, oy, cx, cy, cw, ch, r, paperColorGroup(), to );
	    }
	    while ( tc.gotoNextLine( p ) );
	}
	b = b->nextInDocument();
    };

    richText().flow()->drawFloatingItems( p, ox, oy, cx, cy, cw, ch, r, paperColorGroup(), to );
    p->setClipRegion(r);

    if ( paper().pixmap() )
	p->drawTiledPixmap(0, 0, visibleWidth(), visibleHeight(),
			   *paper().pixmap(), ox, oy);
    else
	p->fillRect(0, 0, visibleWidth(), visibleHeight(), paper() );

    p->setClipping( FALSE );


//     const int pagesize = 100000;

//      for (int page = cy / pagesize; page <= (cy+ch) / pagesize; ++page ) {

// 	 p->setPen( DotLine );

// 	 p->drawLine( cx-ox, page * pagesize - oy, cx-ox+cw, page*
// 		      pagesize - oy );
//      }
}

/*!
  \reimp
*/
void QTextView::viewportResizeEvent(QResizeEvent* )
{
}

void QTextView::doResize()
{
    QPainter p( viewport() );
    if ( !d->fcresize->updateLayout( &p, d->fcresize->y() + d->fcresize->paragraph->height + 1000 ) )
	d->resizeTimer->start( 0, TRUE );
    QTextFlow* flow = richText().flow();
    resizeContents( QMAX( flow->widthUsed-1, visibleWidth() ), flow->height );
}

/*!
  \reimp
*/
void QTextView::resizeEvent( QResizeEvent* e )
{
    setUpdatesEnabled( FALSE ); // to hinder qscrollview from showing/hiding scrollbars. Safe since we call resizeContents later!
    QScrollView::resizeEvent( e );
    setUpdatesEnabled( TRUE);
    richText().invalidateLayout();
    richText().flow()->initialize( visibleWidth() );
    updateLayout();
    viewport()->repaint( FALSE );
}


/*!
  \reimp
*/
void QTextView::viewportMousePressEvent( QMouseEvent* e )
{
    if ( e->button() != LeftButton )
	return;
    QPainter p( viewport() );
    d->cursor = e->pos() + QPoint( contentsX(), contentsY() );
    bool sel = richText().toggleSelection( &p, d->cursor, d->cursor );
    p.end();
    if ( !sel ) {
	clearSelection();
	d->dragSelection = TRUE;
    } else {
	d->dragTimer->start( QApplication::startDragTime(), TRUE );
    }
}

/*!
  \reimp
*/
void QTextView::viewportMouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	d->scrollTimer->stop();
	if ( d->dragSelection ) {
#if defined(_WS_X11_)
	copy();
#else
	if ( style() == MotifStyle )
	    copy();
#endif
	d->dragSelection = FALSE;
	} else {
	    clearSelection();
	}
    }
}



/*!  Returns TRUE if there is any text selected, FALSE otherwise.
  
  \sa selectedText()
*/
bool QTextView::hasSelectedText() const
{
    return d->selection;
}

/*!  Returns a copy of the selected text in plain text format.
  
  \sa hasSelectedText()
 */
QString QTextView::selectedText() const
{
    return richText().selectedText();
}
    

/*!
  Copies the marked text to the clipboard.
*/
void QTextView::copy()
{
#if defined(_WS_X11_)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
#endif
    QString t = richText().selectedText();
#if defined(_OS_WIN32_)
    // Need to convert NL to CRLF
    QRegExp nl("\\n");
    t.replace( nl, "\r\n" );
#endif
    QApplication::clipboard()->setText( t );
#if defined(_WS_X11_)
    connect( QApplication::clipboard(), SIGNAL(dataChanged()),
	     this, SLOT(clipboardChanged()) );
#endif
}

/*!
  Selects all text.
*/
void QTextView::selectAll()
{
    richText().selectAll();
    viewport()->repaint( FALSE );
    d->selection = TRUE;
#if defined(_WS_X11_)
    copy();
#endif
}

/*!
  \reimp
*/
void QTextView::viewportMouseMoveEvent( QMouseEvent* e)
{

    if (e->state() & LeftButton ) {
	if (d->dragSelection ) {
	    doSelection( e->pos() );
	    ensureVisible( d->cursor.x(), d->cursor.y() );
	} else if ( d->dragTimer->isActive() ) {
	    d->dragTimer->stop();
	    doStartDrag();
	}
    } 
}

/*!
  Provides scrolling and paging.
*/
void QTextView::keyPressEvent( QKeyEvent * e)
{
    int unknown = 0;
    switch (e->key()) {
    case Key_Right:
	scrollBy( 10, 0 );
	break;
    case Key_Left:
	scrollBy( -10, 0 );
	break;
    case Key_Up:
	scrollBy( 0, -10 );
	break;
    case Key_Down:
	scrollBy( 0, 10 );
	break;
    case Key_Home:
	setContentsPos(0,0);
	break;
    case Key_End:
	setContentsPos(0,contentsHeight()-visibleHeight());
	break;
    case Key_PageUp:
	scrollBy( 0, -visibleHeight() );
	break;
    case Key_PageDown:
	scrollBy( 0, visibleHeight() );
	break;
    case Key_C:
	if ( e->state() & ControlButton )
	    copy();
#if defined (_WS_WIN_)
    case Key_Insert:
	if ( e->state() & ControlButton )
	    copy();
#endif
	break;
    default:
	unknown++;
    }
    if ( unknown )				// unknown key
	e->ignore();
}

/*!
  \reimp
*/
void QTextView::paletteChange( const QPalette & p )
{
    QScrollView::paletteChange( p );
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;
}


/*!
  Returns the current text format.

  \sa setTextFormat()
 */
Qt::TextFormat QTextView::textFormat() const
{
    return d->textformat;
}

/*!
  Sets the text format to \a format. Possible choices are
  <ul>
  <li> \c PlainText - all characters are displayed verbatimely,
  including all blanks and linebreaks.
  <li> \c RichText - rich text rendering. The available
  styles are defined in the default stylesheet
  QStyleSheet::defaultSheet().
  <li> \c AutoText - this is also the default. The label
  autodetects which rendering style suits best, \c PlainText
  or \c RichText. Technically, this is done by using the
  QStyleSheet::mightBeRichText() heuristic.
  </ul>
 */
void QTextView::setTextFormat( Qt::TextFormat format )
{
    d->textformat = format;
    setText( d->original_txt, d->contxt ); // trigger update
}


/*!\internal
 */
void QTextView::updateLayout()
{
    if ( !isVisible() ) {
	d->dirty = TRUE;
	return;
    }

    QSize cs( viewportSize( contentsWidth(), contentsHeight() ) );
    int ymax = contentsY() + cs.height() + 1;

    delete d->fcresize;
    d->fcresize = new QTextCursor( richText() );

    {
	QPainter p( viewport() );
	d->fcresize->initParagraph( &p, &richText() );
	d->fcresize->updateLayout( &p, ymax );
    }

    QTextFlow* flow = richText().flow();
    QSize vs( viewportSize( flow->widthUsed, flow->height ) );

    if ( vs.width() != visibleWidth() ) {
	flow->initialize( vs.width() );
	richText().invalidateLayout();
	QPainter p( viewport() );
	d->fcresize->gotoParagraph( &p, &richText() );
	d->fcresize->updateLayout( &p, ymax );
    }

    resizeContents( QMAX( flow->widthUsed-1, vs.width() ), flow->height );
    d->resizeTimer->start( 0, TRUE );
    d->dirty = FALSE;
}


/*!\reimp
 */
void QTextView::showEvent( QShowEvent* )
{
    if ( d->dirty )
	updateLayout();
}

void QTextView::clearSelection()
{
    d->dragTimer->stop();
    if ( !d->selection )
	return; // nothing to do

    richText().clearSelection();
    d->selection = FALSE;
    repaintContents( richText().flow()->updateRect(), FALSE );
    richText().flow()->validateRect();
}

void QTextView::doStartDrag()
{
    QTextDrag* drag = new QTextDrag( selectedText(), this ) ;
    drag->drag();
}

void QTextView::doAutoScroll()
{
    QPoint pos = viewport()->mapFromGlobal( QCursor::pos() );
    if ( pos.y() < 0 )
	scrollBy( 0, -32 );
    else if (pos.y() > visibleHeight() )
	scrollBy( 0, 32 );
    doSelection( pos );
}

void QTextView::doSelection( const QPoint& pos ) 
{
    QPainter p(viewport());

    QPoint to( pos + QPoint( contentsX(), contentsY()  ) );
    if ( to != d->cursor ) {
	richText().toggleSelection( &p, d->cursor, to );
	d->selection = TRUE;
	d->cursor = to;
	repaintContents( richText().flow()->updateRect(), FALSE );
	richText().flow()->validateRect();
    }
    
    if ( pos.y() < 0 || pos.y() > visibleHeight() )
	d->scrollTimer->start( 100, FALSE );
    else
	d->scrollTimer->stop();
}    

void QTextView::clipboardChanged()
{
#if defined(_WS_X11_)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    clearSelection();
#endif
}
