/****************************************************************************
** $Id$
**
** Implementation of the QtTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtextview.h"
#include "qrichtextintern.h"

#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"

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




/*!
  \class QtTextView qtextview.h
  \brief A sophisticated single-page rich text viewer.
  \ingroup realwidgets

  Unlike QSimpleRichText, which merely draws small pieces of rich
  text, a QtTextView is a real widget, with scrollbars when necessary,
  for showing large text documents.

  The rendering style and available tags are defined by a
  styleSheet(). Currently, a small XML/CSS1 subset including embedded
  images and tables is supported. See QStyleSheet for
  details. Possible images within the text document are resolved by
  using a QMimeSourceFactory.  See setMimeSourceFactory() for details.

  Using QtTextView is quite similar to QLabel. It's mainly a call to
  setText() to set the contents. Setting the background color is
  slightly different from other widgets, since a text view is a
  scrollable widget that naturally provides a scrolling background. You
  can specify the colorgroup of the displayed text with
  setPaperColorGroup() or directly define the paper background with
  setPaper(). QtTextView supports both plain color and complex pixmap
  backgrounds.

  Note that we do not intend to add a full-featured web browser widget
  to Qt (since that would easily double Qt's size and only few
  applications would benefit from it). In particular, the rich text
  support in Qt is supposed to provide a fast, portable and sufficient
  way to add reasonable online help facilities to applications. We
  will, however, extend it to some degree in future versions of Qt.

  For even more, like hypertext capabilities, see QtTextBrowser.
*/

class QtTextViewData
{
public:
    QStyleSheet* sheet_;
    QtRichText* doc_;
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
    Qt::TextFormat textformat;
    QtTextCursor* fcresize;
    QtTextCursor* cursor;
    uint selection :1;
    uint dirty :1;
    uint dragSelection :1;
};

class QtTextEditData
{
public:
    bool cursor_hidden;
    QTimer* cursorTimer;
    QtTextCursor* cursor;
};


/*!
  Constructs an empty QtTextView
  with the standard \a parent and \a name optional arguments.
*/
QtTextView::QtTextView(QWidget *parent, const char *name)
    : QScrollView( parent, name, WNorthWestGravity )
{
    init();
}


/*!
  Constructs a QtTextView displaying the contents \a text with context
  \a context, with the standard \a parent and \a name optional
  arguments.
*/
QtTextView::QtTextView( const QString& text, const QString& context,
		      QWidget *parent, const char *name)
    : QScrollView( parent, name, WNorthWestGravity | WRepaintNoErase )
{
    init();
    setText( text, context );
}


void QtTextView::init()
{
    d = new QtTextViewData;
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;
    d->mylinkcol = blue;
    d->paplinkcol = d->mylinkcol;
    d->linkunderline = TRUE;
    d->fcresize = 0;
    d->cursor = 0;

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
}

/*!
  Destructs the view.
*/
QtTextView::~QtTextView()
{
    delete d->doc_;
    delete d->cursor;
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
void QtTextView::setText( const QString& text, const QString& context)
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
    viewport()->repaint( FALSE );
}


/*!
  Appends \a text to the current text.

  Useful for log viewers.
*/
void QtTextView::append( const QString& text )
{
    richText().append( text,  mimeSourceFactory(), (QtStyleSheet*)styleSheet() );
    int y = contentsHeight();
    int h = richText().lastChild()->bottomMargin();
    updateLayout( contentsHeight() + visibleHeight()  );
    updateContents( contentsX(), y-h, visibleWidth(), h );
}

/*!
  Returns the contents of the view.

  \sa context(), setText()
*/
QString QtTextView::text() const
{
    return d->original_txt;
}

/*!
  Returns the context of the view.

  \sa text(), setText()
*/
QString QtTextView::context() const
{
    return d->contxt;
}


void QtTextView::createRichText()
{
    d->papcolgrp = d->mypapcolgrp;
    d->paplinkcol = d->mylinkcol;

    d->doc_ = new QtRichText( d->txt, viewport()->font(), d->contxt,
			     8, mimeSourceFactory(), (QtStyleSheet*)styleSheet() );
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
		qWarning("QtTextImage: cannot load %s", imageName.latin1() );
	    }
	}
	if (!pm.isNull())
	    d->papcolgrp.setBrush( QColorGroup::Base, QBrush(d->papcolgrp.base(), pm) );
    }
    delete d->cursor;
    d->cursor = new QtTextCursor( richText() );
}


/*!
  Returns the current style sheet of the view.

  \sa setStyleSheet()
*/
QStyleSheet* QtTextView::styleSheet() const
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
void QtTextView::setStyleSheet( QStyleSheet* styleSheet )
{
    d->sheet_ = styleSheet;
    viewport()->update();
}


/*!
  Returns the current mime source factory  for the view.

  \sa setMimeSourceFactory()
*/
QMimeSourceFactory* QtTextView::mimeSourceFactory() const
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
void QtTextView::setMimeSourceFactory( QMimeSourceFactory* factory )
{
    d->factory_ = factory;
    viewport()->update();
}


/*!
  Sets the brush to use as the background to \a pap.

  This may be a nice parchment or marble pixmap or simply another
  plain color.

  Technically, setPaper() is just a convenience function to set the
  base brush of the paperColorGroup().

  \sa paper()
*/
void QtTextView::setPaper( const QBrush& pap)
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
void QtTextView::setPaperColorGroup( const QColorGroup& colgrp)
{
    d->mypapcolgrp = colgrp;
    d->papcolgrp = colgrp;
    viewport()->update();
}

/*!
  Returns the colorgroup of the paper.

  \sa setPaperColorGroup(), setPaper()
*/
const QColorGroup& QtTextView::paperColorGroup() const
{
    return d->papcolgrp;
}

/*!
  Sets the color used to display links in the document to \c col.

  \sa linkColor()
 */
void QtTextView::setLinkColor( const QColor& col )
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
const QColor& QtTextView::linkColor() const
{
    return d->paplinkcol;
}

/*!
  Defines wether or not links should be displayed underlined.
 */
void QtTextView::setLinkUnderline( bool u)
{
    d->linkunderline = u;
}

/*!
  Returns wether or not links should be displayed underlined.
 */
bool QtTextView::linkUnderline() const
{
    return d->linkunderline;
}


/*!
  Returns the document title parsed from the content.
*/
QString QtTextView::documentTitle() const
{
    return richText().attributes()["title"];
}

/*!
  Returns the height of the view given a width of \a w.
*/
int QtTextView::heightForWidth( int w ) const
{
//     QtRichText doc ( d->txt, viewport()->font(), d->contxt,
// 		    8, mimeSourceFactory(), (QtStyleSheet*)styleSheet() );
//     {
// 	QPainter p( this );
// 	doc.setWidth(&p, w);
//     }
//     return doc.height;
    return w;
}

/*!
  Returns the document defining the view as drawable and queryable rich
  text object.  This is not currently useful for applications.
*/
QtRichText& QtTextView::richText() const
{
    if (!d->doc_){
	QtTextView* that = (QtTextView*) this;
	that->createRichText();
    }
    return *d->doc_;
}

/*!
  Returns the brush used to paint the background.
*/
const QBrush& QtTextView::paper()
{
    return d->papcolgrp.brush( QColorGroup::Base );
}

/*!
  \reimp
*/
void QtTextView::drawContentsOffset(QPainter* p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QtTextOptions to(&paper() );
    to.offsetx = ox;
    to.offsety = oy;

    QRegion r(cx-ox, cy-oy, cw, ch);

    QtTextCursor tc( richText() );
    tc.gotoParagraph( p, &richText() );
    QtTextParagraph* b = tc.paragraph;

    // TODO merge with update, this is only draw. Everything needs to be clean!
    QFontMetrics fm( p->fontMetrics() );
    while ( b && tc.y() <= cy + ch ) {

	if ( b && b->dirty ) //ensure the paragraph is laid out
	    tc.updateLayout( p, cy + ch );

	tc.gotoParagraph( p, b );

	if ( tc.y() + tc.paragraph->height > cy ) {
	    do {
		tc.makeLineLayout( p, fm );
		QRect geom( tc.lineGeometry() );
		if ( geom.bottom() > cy && geom.top() < cy+ch )
		    tc.drawLine( p, ox, oy, cx, cy, cw, ch, r, paperColorGroup(), to );
	    }
	    while ( tc.gotoNextLine( p, fm ) );
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


    const int pagesize = 100000;

     for (int page = cy / pagesize; page <= (cy+ch) / pagesize; ++page ) {

	 p->setPen( DotLine );

	 p->drawLine( cx-ox, page * pagesize - oy, cx-ox+cw, page*
		      pagesize - oy );
     }
}

/*!
  \reimp
*/
void QtTextView::viewportResizeEvent(QResizeEvent* )
{
}

void QtTextView::doResize()
{
    QPainter p( viewport() );
    if ( !d->fcresize->updateLayout( &p, d->fcresize->y() + d->fcresize->paragraph->height + 1000 ) )
	d->resizeTimer->start( 0, TRUE );
    QtTextFlow* flow = richText().flow();
    resizeContents( QMAX( flow->widthUsed-1, visibleWidth() ), flow->height );
}




// void QtTextView::paragraphChanged( QtTextParagraph* b)
// {
//     QPainter p( viewport() );
//     QtTextCursor tc( richText() );
//     tc.gotoParagraph( &p, b );
//     tc.updateParagraph( &p );
// }

/*!
  \reimp
*/
void QtTextView::resizeEvent( QResizeEvent* e )
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
void QtTextView::viewportMousePressEvent( QMouseEvent* e )
{
    if ( !d->cursor )
	return;
    if ( e->button() != LeftButton )
	return;
    QPainter p( viewport() );
    d->cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
    p.end();
    if ( !d->cursor->isSelected() ) {
	clearSelection();
	d->dragSelection = TRUE;
    } else {
    }
}

/*!
  \reimp
*/
void QtTextView::viewportMouseReleaseEvent( QMouseEvent* e )
{
    if ( d->dragSelection && e->button() == LeftButton ) {
	//### TODO put selection into clipboard
#if defined(Q_WS_X11)
	copy();
#else
	if ( style() == MotifStyle )
	    copy();
#endif
	d->dragSelection = FALSE;
    }
}


void QtTextView::copy()
{
#if defined(Q_WS_X11)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
#endif
#if defined(Q_OS_WIN32)
    // Need to convert NL to CRLF
    QRegExp nl("\\n");
    t.replace( nl, "\r\n" );
#endif
    QApplication::clipboard()->setText( richText().selectedText() );
#if defined(Q_WS_X11)
    connect( QApplication::clipboard(), SIGNAL(dataChanged()),
	     this, SLOT(clipboardChanged()) );
#endif
}

void QtTextView::selectAll()
{
}

/*!
  \reimp
*/
void QtTextView::viewportMouseMoveEvent( QMouseEvent* e)
{

     if (e->state() & LeftButton && d->dragSelection ) {
	 if ( !d->cursor )
	     return;
	QPainter p(viewport());
 	d->cursor->split();
	QtTextCursor oldc( *d->cursor );
	d->cursor->goTo( &p, e->pos().x() + contentsX(),
			 e->pos().y() + contentsY() );
  	if ( d->cursor->split() ) {
	    if ( (oldc.paragraph == d->cursor->paragraph) && (oldc.current >= d->cursor->current) ) {
		oldc.current++;
		oldc.update( &p );
	    }
 	}
	int oldy = oldc.y();
	int oldx = oldc.x();
	int oldh = oldc.height;
	int newy = d->cursor->y();
	int newx = d->cursor->x();
	int newh = d->cursor->height;

	QtTextCursor start( richText() ), end( richText() );

	bool oldIsFirst = (oldy < newy) || (oldy == newy && oldx <= newx);
	if ( oldIsFirst ) {
	    start = oldc;
	    end = *d->cursor;
	} else {
	    start = *d->cursor;
	    end = oldc;
	}

	while ( start.paragraph != end.paragraph ) {
 	    start.setSelected( !start.isSelected() );
	    start.rightOneItem( &p );
	    d->selection = TRUE;
	}
	while ( !start.atEnd() && start.paragraph == end.paragraph && start.current < end.current ) {
 	    start.setSelected( !start.isSelected() );
	    start.rightOneItem( &p );
	    d->selection = TRUE;
	}
	p.end();
	repaintContents( 0, QMIN(oldy, newy),
			 contentsWidth(),
			 QMAX(oldy+oldh, newy+newh)-QMIN(oldy,newy),
			 FALSE);
	QRect geom ( d->cursor->caretGeometry() );
	ensureVisible( geom.center().x(), geom.center().y(), geom.width()/2, geom.height()/2 );
     }
}

/*!
  Provides scrolling and paging.
*/
void QtTextView::keyPressEvent( QKeyEvent * e)
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
#if defined (Q_WS_WIN)
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
void QtTextView::paletteChange( const QPalette & p )
{
    QScrollView::paletteChange( p );
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;
}


/*!
  Returns the current text format.

  \sa setTextFormat()
 */
Qt::TextFormat QtTextView::textFormat() const
{
    return d->textformat;
}

/*!
  Sets the text format to \a format. Possible choices are
  <ul>
  <li> \c PlainText - all characters are displayed verbatim,
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
void QtTextView::setTextFormat( Qt::TextFormat format )
{
    d->textformat = format;
    setText( d->original_txt, d->contxt ); // trigger update
}


/*!\internal
 */
void QtTextView::updateLayout( int ymax )
{
    if ( !isVisible() ) {
	d->dirty = TRUE;
	return;
    }

    QSize cs( viewportSize( contentsWidth(), contentsHeight() ) );
    if ( ymax < 0 )
	ymax = contentsY() + cs.height() + 1;

    delete d->fcresize;
    d->fcresize = new QtTextCursor( richText() );

    {
	QPainter p( viewport() );
	d->fcresize->initParagraph( &p, &richText() );
	d->fcresize->updateLayout( &p, ymax );
    }

    QtTextFlow* flow = richText().flow();
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
void QtTextView::showEvent( QShowEvent* )
{
    if ( d->dirty )
	updateLayout();
}

void QtTextView::clearSelection()
{
    if ( !d->selection ) 
	return; // nothing to do
    
    richText().clearSelection();
    d->selection = FALSE;
    repaintContents( richText().flow()->updateRect(), FALSE );
    richText().flow()->validateRect();
}

void QtTextView::clipboardChanged()
{
#if defined(Q_WS_X11)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    clearSelection();
#endif
}

//************************************************************************

QtTextEdit::QtTextEdit(QWidget *parent, const char *name)
    : QtTextView( parent, name )
{
    setKeyCompression( TRUE );
    setVScrollBarMode( AlwaysOn );
    d = new QtTextEditData;
    d->cursor = 0;
    d->cursor_hidden = FALSE;
    d->cursorTimer = new QTimer( this );
    d->cursorTimer->start(200, TRUE);
    connect( d->cursorTimer, SIGNAL( timeout() ), this, SLOT( cursorTimerDone() ));
    setKeyCompression( TRUE );
}

QtTextEdit::~QtTextEdit()
{
}



/*!
  reimplemented for internal purposes
 */
void QtTextEdit::setText( const QString& text, const QString& context  )
{
    QtTextView::setText( text, context );
    delete d->cursor;
    d->cursor = new QtTextCursor( richText() );
    {
	QPainter p( this );
	d->cursor->gotoParagraph( &p, &richText() );
    }
}


/*!
  Make a tree dump
 */
QString QtTextEdit::text()
{
    qDebug("not yet implemented");
    return "not yet implemented";
}

void QtTextEdit::keyPressEvent( QKeyEvent * e )
{

    if ( !d->cursor )
	return;
    //    bool select = e->state() & Qt::ShiftButton;
    hideCursor();
    QPainter p( viewport() );
    switch (e->key()) {
    case Key_Right:
	d->cursor->right( &p );
	break;
    case Key_Left:
	d->cursor->left( &p );
	break;
    case Key_Up:
	d->cursor->up( &p );
	break;
    case Key_Down:
	d->cursor->down( &p );
	break;
    default:
	if (!e->text().isEmpty() ) {
	    d->cursor->insert( &p, e->text() );
	    p.end();
	    repaintContents( richText().flow()->updateRect(), FALSE );
	    richText().flow()->validateRect();
	}

    };
    if ( p.isActive() )
	p.end();

    QRect geom ( d->cursor->caretGeometry() );
    ensureVisible( geom.center().x(), geom.center().y(), geom.width()/2, geom.height()/2 );
    showCursor();

}



void QtTextEdit::viewportMousePressEvent( QMouseEvent * e )
{
    if ( !d->cursor )
	return;
    hideCursor();
    {
	QPainter p( viewport() );
	d->cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
    }
    showCursor();
}

void QtTextEdit::viewportMouseReleaseEvent( QMouseEvent * )
{
    // nothing
}

void QtTextEdit::viewportMouseMoveEvent( QMouseEvent * e)
{
     if (e->state() & LeftButton ) {
 	hideCursor();
	QPainter p(viewport());
 	d->cursor->split();
	QtTextCursor oldc( *d->cursor );
	d->cursor->goTo( &p, e->pos().x() + contentsX(),
			 e->pos().y() + contentsY() );
  	if ( d->cursor->split() ) {
	    if ( (oldc.paragraph == d->cursor->paragraph) && (oldc.current >= d->cursor->current) ) {
		oldc.current++;
		oldc.update( &p );
	    }

 	}
	int oldy = oldc.y();
	int oldx = oldc.x();
	int oldh = oldc.height;
	int newy = d->cursor->y();
	int newx = d->cursor->x();
	int newh = d->cursor->height;

	QtTextCursor start( richText() ), end( richText() );

	bool oldIsFirst = (oldy < newy) || (oldy == newy && oldx <= newx);
	if ( oldIsFirst ) {
	    start = oldc;
	    end = *d->cursor;
	} else {
	    start = *d->cursor;
	    end = oldc;
	}

	while ( start.paragraph != end.paragraph ) {
 	    start.setSelected( !start.isSelected() );
	    start.rightOneItem( &p );
	}
	while ( !start.atEnd() && start.paragraph == end.paragraph && start.current < end.current ) {
 	    start.setSelected( !start.isSelected() );
	    start.rightOneItem( &p );
	}
	p.end();
	repaintContents( 0, QMIN(oldy, newy),
			 contentsWidth(),
			 QMAX(oldy+oldh, newy+newh)-QMIN(oldy,newy),
			 FALSE);
	QRect geom ( d->cursor->caretGeometry() );
	ensureVisible( geom.center().x(), geom.center().y(), geom.width()/2, geom.height()/2 );
     }
}

void QtTextEdit::drawContentsOffset(QPainter*p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QtTextView::drawContentsOffset(p, ox, oy, cx, cy, cw, ch);
}

void QtTextEdit::cursorTimerDone()
{
    if ( !d->cursor )
	return;
     if (d->cursor_hidden) {
 	if (QtTextEdit::hasFocus())
 	    showCursor();
 	else
 	    d->cursorTimer->start(400, TRUE);
     }
     else {
 	hideCursor();
     }
}

void QtTextEdit::showCursor()
{
    d->cursor_hidden = FALSE;
    QPainter p( viewport() );
    d->cursor->draw(&p, contentsX(), contentsY(),
		 contentsX(), contentsY(),
  		 visibleWidth(), visibleHeight());
    d->cursorTimer->start(400, TRUE);
}

void QtTextEdit::hideCursor()
{
    if (d->cursor_hidden)
 	return;
    d->cursor_hidden = TRUE;

    QRect geom( d->cursor->caretGeometry() );
    repaintContents(geom.x(), geom.y(), geom.width(), geom.height() );

    d->cursorTimer->start(300, TRUE);
}



void QtTextEdit::resizeEvent( QResizeEvent* e )
{
    QtTextView::resizeEvent( e );
    if ( d->cursor ) {
 	QPainter p( this );
    }
}

void QtTextEdit::viewportResizeEvent(QResizeEvent* e)
{
    QtTextView::viewportResizeEvent(e);
}



void QtTextEdit::temporary()
{
    qDebug("temporary ");
    QPainter p( viewport() );
    QtTextCursor start( richText() );
    bool all_bold = TRUE;
    {
	start.gotoParagraph( &p, &richText() );
	bool modified = FALSE;
	do {
	    if ( start.isSelected() ) {
		all_bold &= start.paragraph->text.bold( start.current );
		start.paragraph->text.setBold( start.current, TRUE );
		modified = TRUE;
	    }
	    if ( modified && start.atEnd() ) {
		QtTextCursor tc( start );
		tc.gotoParagraph( &p, tc.paragraph );
		tc.updateParagraph( &p );
		modified = FALSE;
	    }
	} while ( start.rightOneItem( &p ) );
    }
    if ( all_bold ) {
	start.gotoParagraph( &p, &richText() );
	bool modified = FALSE;
	do {
	    if ( start.isSelected() ) {
		start.paragraph->text.setBold( start.current, FALSE );
		modified = TRUE;
	    }
	    if ( modified && start.atEnd() ) {
		QtTextCursor tc( start );
		tc.gotoParagraph( &p, tc.paragraph );
		tc.updateParagraph( &p );
		modified = FALSE;
	    }
	} while ( start.rightOneItem( &p ) );
    }

    d->cursor->update( &p );
    p.end();
    repaintContents( richText().flow()->updateRect(), FALSE );
    richText().flow()->validateRect();
    QRect geom ( d->cursor->caretGeometry() );
    ensureVisible( geom.center().x(), geom.center().y(), geom.width()/2, geom.height()/2 );
}
