/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qtextview.cpp#12 $
**
** Implementation of the QtTextView class
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
#include "qrichtextintern.h"

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




/*!
  \class QtTextView qtextview.h
  \brief A sophisticated single-page rich text viewer.
  \ingroup realwidgets

  Unlike QSimpleRichText, which merely draws small pieces of rich
  text, a QtTextView is a real widget, with scrollbars when necessary,
  for showing large text documents.

  The rendering style and available tags are defined by a
  styleSheet(). Currently, a small XML/CSS1 subset including embedded
  images is supported. See QStyleSheet for details. Possible images
  within the text document are resolved by using a QMimeSourceFactory.
  See setMimeSourceFactory() for details.

  Using QtTextView is quite similar to QLabel. It's mainly a call to
  setText() to set the contents. Setting the background color is
  slighty different from other widgets, since a text view is a
  scrollable widget that naturally provies a scrolling background. You
  can specify the colorgroup of the displayed text with
  setPaperColorGroup() or directly define the paper background with
  setPaper(). QtTextView supports both plain color and complex pixmap
  backgrounds.

  Note that we do not intend to add a full-featured web browser widget
  to Qt (since that would easily double Qt's size and only few
  applications would benefit from it). In particular, the rich text
  support in Qt is supposed to provide a fast and sufficient way to
  add reasonable online help facilities to applications. We will,
  however, extend it to some degree in future versions of Qt. Most
  likely some basic table support will be put in and a new class to
  provide rich text input.

  For even more, like hypertext capabilities, see QtTextBrowser.

  \bug No selection possible.
  No table support (yet).
*/

class QtTextViewData
{
public:
    QStyleSheet* sheet_;
    QtRichText* doc_;
    int viewId;
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
    : QScrollView( parent, name, WNorthWestGravity )
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

    setKeyCompression( TRUE );
    setVScrollBarMode( QScrollView::Auto );
    setHScrollBarMode( QScrollView::Auto );
//     setHScrollBarMode( AlwaysOff );

    d->doc_ = 0;
    d->viewId = 0;
    d->sheet_ = 0;
    d->factory_ = 0;
    d->txt = QString::fromLatin1("<p></p>");
    d->textformat = AutoText;

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
    d->viewId = 0;

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


    if ( isVisible() ) {
	QtTextFlow* flow = richText().flow(d->viewId);
	flow->x = 0;
	delete d->fcresize;
	d->fcresize = new QtTextCursor( richText(), d->viewId );
	d->fcresize->initFlow( flow, viewportSize(0,0).width() );
	{
	    QPainter p( viewport() );
	    d->fcresize->initParagraph( &p, &richText() );
	    d->fcresize->doLayout( &p, viewport()->height() + contentsY() );
	}
	QSize vs( viewportSize( flow->widthUsed, flow->height ) );
	if ( vs.width() != viewportSize(0,0).width() ) { // we'll get a vertical scrollbar, it seems. Once again
	    d->fcresize->initFlow( flow, vs.width() );
	    {
		QPainter p( viewport() );
		d->fcresize->initParagraph( &p, &richText() );
		d->fcresize->doLayout( &p, viewport()->height() + contentsY() );
	    }
	}
	setContentsPos( 0, 0 );
	resizeContents( flow->widthUsed-1, flow->height );
	d->resizeTimer->start( 0, TRUE );
	viewport()->repaint( FALSE );
    }
    setContentsPos( 0, 0 );

}

void QtTextView::setView( QtTextView* other )
{
    delete d->doc_;
    d->doc_ =  &other->richText();
    qDebug("set view %p", d->doc_ );
    d->viewId = d->doc_->registerView( this );
    qDebug("register view %d (%p)", d->viewId, this );

    d->original_txt = other->d->original_txt;
    d->contxt = other->d->contxt;
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
    qDebug("create rich text for %p = %p", this, d->doc_ );
    d->viewId = d->doc_->registerView( this );
    qDebug("register view %d (%p)", d->viewId, this );
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

  This may be a nice pergament or marble pixmap or simply another
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
  Returns the document defining the view as drawable and querable rich
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
    QRegion r(cx-ox, cy-oy, cw, ch);

    QtTextCursor tc( richText(), d->viewId );
    tc.gotoParagraph( p, &richText() );
    QtTextParagraph* b = tc.paragraph;

    // TODO merge with update, this is only draw. Everything needs to be clean!
    QFontMetrics fm( p->fontMetrics() );
    while ( b && tc.referenceTop() <= cy + ch ) {
	// this doesn't belong here...
	if ( b && b->dirty[ d->viewId ] ) {
	    tc.initParagraph( p, b );
	    tc.doLayout( p, tc.referenceBottom() );
	}

	tc.gotoParagraph( p, b );

	if ( tc.referenceBottom() > cy ) {
	    do {
		tc.makeLineLayout( p, fm );
		QRect geom( tc.lineGeometry() );
		if ( geom.bottom() > cy && geom.top() < cy+ch )
		    tc.drawLine( p, ox, oy, r, paperColorGroup(), QtTextOptions(&paper() ) );
	    }
	    while ( tc.gotoNextLine( p, fm ) );
	}
	b = b->nextInDocument();
    };



    p->setClipRegion(r);

    if ( paper().pixmap() )
	p->drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			   *paper().pixmap(), ox, oy);
    else
	p->fillRect(0, 0, viewport()->width(), viewport()->height(), paper() );

    p->setClipping( FALSE );


     int pagesize = 500;

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
    if ( !d->fcresize->doLayout( &p, d->fcresize->referenceBottom() + 1000 ) )
	d->resizeTimer->start( 0, TRUE );
    QtTextFlow* flow = richText().flow( d->viewId );
    resizeContents( flow->widthUsed-1, flow->height );
}




void QtTextView::paragraphChanged( QtTextParagraph* b)
{
    QPainter p( viewport() );
    QtTextCursor tc( richText(), d->viewId );
    tc.gotoParagraph( &p, b );
    tc.updateParagraph( &p );
}

/*!
  \reimp
*/
void QtTextView::resizeEvent( QResizeEvent* e )
{
    QtTextFlow* flow = richText().flow(d->viewId);
    flow->x = 0;
    delete d->fcresize;
    d->fcresize = new QtTextCursor( richText(), d->viewId );
    d->fcresize->initFlow( flow, viewportSize(0,0).width() );
    {
	QPainter p( viewport() );
	d->fcresize->initParagraph( &p, &richText() );
	d->fcresize->doLayout( &p, viewport()->height() + contentsY() );
    }
    QSize vs( viewportSize( flow->widthUsed, flow->height ) );
    if ( vs.width() != viewportSize(0,0).width() ) { // we'll get a vertical scrollbar, it seems. Once again
	d->fcresize->initFlow( flow, vs.width() );
	{
	    QPainter p( viewport() );
	    d->fcresize->initParagraph( &p, &richText() );
	    d->fcresize->doLayout( &p, viewport()->height() + contentsY() );
	}
    }
    setContentsPos( 0, 0 );
    resizeContents( flow->widthUsed-1, flow->height );
    d->resizeTimer->start( 0, TRUE );
    viewport()->repaint( FALSE );
}


/*!
  \reimp
*/
void QtTextView::viewportMousePressEvent( QMouseEvent* )
{
}

/*!
  \reimp
*/
void QtTextView::viewportMouseReleaseEvent( QMouseEvent* )
{
}

/*!
  \reimp
*/
void QtTextView::viewportMouseMoveEvent( QMouseEvent* )
{
}

/*!
  Provides scrolling and paging.
*/
void QtTextView::keyPressEvent( QKeyEvent * e)
{
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
	setContentsPos(0,contentsHeight()-viewport()->height());
	break;
    case Key_PageUp:
	scrollBy( 0, -viewport()->height() );
	break;
    case Key_PageDown:
	scrollBy( 0, viewport()->height() );
	break;
    }
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
  <li> \c PlainText - all characters are displayed verbatimely,
  including all blanks and linebreaks. Word wrap is availbe
  with the \c WordBreak alignment flag (see setAlignment() for
  details).
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
    d->cursor = new QtTextCursor( richText(), richText().viewId( this ) );
}

void QtTextEdit::setView( QtTextView* other )
{
    QtTextView::setView( other );
    delete d->cursor;
    d->cursor = new QtTextCursor( richText(), richText().viewId( this ) );
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
	    d->cursor->insert( &p, e->text() ); // the painter will be closed
	}

    };
    if ( p.isActive() )
	p.end();
    QRect geom ( d->cursor->caretGeometry() );
    ensureVisible( geom.center().x(), geom.center().y(), geom.width()/2, geom.height()/2 );
    showCursor();

//     if (e->key() == Key_Plus)
// 	exit(2); // profiling

    /*
    hideCursor();
    bool select = e->state() & Qt::ShiftButton;
#define CLEARSELECT if (!select) {cursor->clearSelection();updateSelection();}


    if (e->key() == Key_Right
	|| e->key() == Key_Left
	|| e->key() == Key_Up
	|| e->key() == Key_Down
	|| e->key() == Key_Home
	|| e->key() == Key_End
	|| e->key() == Key_PageUp
	|| e->key() == Key_PageDown
	) {
	// cursor movement
	CLEARSELECT
	    QtTextRow*  oldCursorRow = cursor->row;
	bool ensureVisibility = TRUE;
	{
	    QPainter p( viewport() );
	    switch (e->key()) {
	    case Key_Right:
		cursor->right(&p, select);
		p.end();
		break;
	    case Key_Left:
		cursor->left(&p, select);
		p.end();
		break;
	    case Key_Up:
		cursor->up(&p, select);
		p.end();
		break;
	    case Key_Down:
		cursor->down(&p, select);
		p.end();
		break;
	    case Key_Home:
		cursor->home(&p, select);
		p.end();
		break;
	    case Key_End:
		cursor->last(&p, select);
		p.end();
		break;
	    case Key_PageUp:
		p.end();
		ensureVisibility = FALSE;
		{
		    int oldContentsY = contentsY();
		    if (!cursor->ylineOffsetClean)
			cursor->yline-=oldContentsY;
		    scrollBy( 0, -viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = cursor->xline;
		    int oldYline = cursor->yline;
		    cursor->goTo( &p, oldXline, oldYline +  1 + contentsY(), select);
		    cursor->xline = oldXline;
		    cursor->yline = oldYline;
		    cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    case Key_PageDown:
		p.end();
		ensureVisibility = FALSE;
		{
		    int oldContentsY = contentsY();
		    if (!cursor->ylineOffsetClean)
			cursor->yline-=oldContentsY;
		    scrollBy( 0, viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = cursor->xline;
		    int oldYline = cursor->yline;
		    cursor->goTo( &p, oldXline, oldYline + 1 + contentsY(), select);
		    cursor->xline = oldXline;
		    cursor->yline = oldYline;
		    cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    }
	}
	if (cursor->row == oldCursorRow)
	    updateSelection(cursor->rowY, cursor->rowY);
	else
	    updateSelection();
	if (ensureVisibility) {
	    ensureVisible(cursor->x, cursor->y);
	}
	showCursor();
    }
    else {
	
	if (e->key() == Key_Return || e->key() == Key_Enter ) {
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    for (int i = 0; i < QMIN(4, e->count()); i++)
			cursor->enter( &p ); // can be optimized
		}
	    updateScreen();
	}
	else if (e->key() == Key_Delete) {
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    cursor->del( &p, QMIN(4, e->count() ));
		}
	    updateScreen();
	}
	else if (e->key() == Key_Backspace) {
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    cursor->backSpace( &p, QMIN(4, e->count() ) );
		}
	    updateScreen();
	}
	else if (!e->text().isEmpty() ){
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    cursor->insert( &p, e->text() );
		}
	    updateScreen();
	}
    }
    */
}


/* Updates the visible selection according to the internal
  selection. If oldY and newY is defined, then only the area between
  both horizontal lines is taken into account. */
void QtTextEdit::updateSelection(int oldY, int newY)
{
    oldY = newY = 0; // shut up.
//     if (!cursor || !cursor->selectionDirty)
// 	return;

//     if (oldY > newY) {
// 	int tmp = oldY;
// 	oldY = newY;
// 	newY = tmp;
//     }

//     QPainter p(viewport());
//     int minY = oldY>=0?QMAX(QMIN(oldY, newY), contentsY()):contentsY();
//     int maxY = newY>=0?QMIN(QMAX(oldY, newY), contentsY()+viewport()->height()):contentsY()+viewport()->height();
//     QRegion r;
//     richText().draw(&p, 0, 0, contentsX(), contentsY(),
// 			   contentsX(), minY,
// 			   viewport()->width(), maxY-minY,
// 			   r, paperColorGroup(), QtTextOptions(&paper()), FALSE, TRUE);
//     cursor->selectionDirty = FALSE;
}

void QtTextEdit::viewportMousePressEvent( QMouseEvent * e )
{
    if ( !d->cursor )
	return;
     hideCursor();
     e = 0; //#####
//      cursor->clearSelection();
//      updateSelection();
     {
//  	QPainter p( viewport() );
//  	cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
     }
     showCursor();
}

void QtTextEdit::viewportMouseReleaseEvent( QMouseEvent * )
{
    // nothing
}

void QtTextEdit::viewportMouseMoveEvent( QMouseEvent * )
{
//     if (e->state() & LeftButton) {
// 	hideCursor();
// 	QtTextRow*  oldCursorRow =c ursor->row;
// 	{
// 	    QPainter p(viewport());
// 	    cursor->goTo( &p, e->pos().x() + contentsX(),
// 			  e->pos().y() + contentsY(), TRUE);
// 	}
// 	if (cursor->row == oldCursorRow)
// 	    updateSelection(cursor->rowY, cursor->rowY );
// 	else
// 	    updateSelection();
// 	if (cursor->y + cursor->height > contentsY() + viewport()->height()) {
// 	    scrollBy(0, cursor->y + cursor->height-contentsY()-viewport()->height());
// 	}
// 	else if (cursor->y < contentsY())
// 	    scrollBy(0, cursor->y - contentsY() );
// 	showCursor();
//     }
}

void QtTextEdit::drawContentsOffset(QPainter*p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QtTextView::drawContentsOffset(p, ox, oy, cx, cy, cw, ch);
    d->cursor->update( p ); //##### it may be on a dirty paragraph
    return;
//     if (!d->cursor_hidden)
//  	cursor->draw(p, ox, oy, cx, cy, cw, ch);
}

void QtTextEdit::cursorTimerDone()
{
    return;
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
  		 viewport()->width(), viewport()->height());
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



/*!  Updates the visible screen according to the (changed) internal
  data structure.
*/
void QtTextEdit::updateScreen()
{
    /*
    {
	QPainter p( viewport() );
	QRegion r(0, 0, viewport()->width(), viewport()->height());
	richText().draw(&p, 0, 0, contentsX(), contentsY(),
			       contentsX(), contentsY(),
			       viewport()->width(), viewport()->height(),
			       r, paperColorGroup(), QtTextOptions( &paper() ), TRUE);
	p.setClipRegion(r);
	if ( paper().pixmap() )
	    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			      *paperColorGroup().brush( QColorGroup::Base ).pixmap(), contentsX(), contentsY());
	else
	    p.fillRect(0, 0, viewport()->width(), viewport()->height(), paper());
    }
    showCursor();
    resizeContents( QMAX( richText().widthUsed, richText().width ), richText().height );
    ensureVisible(cursor->x, cursor->y);
    */
}

void QtTextEdit::resizeEvent( QResizeEvent* e )
{
    QtTextView::resizeEvent( e );
    if ( d->cursor ) {
 	QPainter p( this );
  	d->cursor->update(&p); //##### it may be on a dirty paragraph
    }
}

void QtTextEdit::viewportResizeEvent(QResizeEvent* e)
{
    QtTextView::viewportResizeEvent(e);
}



