/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.cpp#21 $
**
** Implementation of the QTextView class
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




/*!
  \class QTextView qtextview.h
  \brief A sophisticated single-page rich text viewer.
  \ingroup realwidgets

  Unlike QSimpleRichText, which merely draws small pieces of rich
  text, a QTextView is a real widget, with scrollbars when necessary,
  for showing large text documents.

  The rendering style and available tags are defined by a
  styleSheet(). Currently, a small XML/CSS1 subset including embedded
  images is supported. See QStyleSheet for details. Possible images
  within the text document are resolved by using a QMimeSourceFactory.
  See setMimeSourceFactory() for details.

  Using QTextView is quite similar to QLabel. It's mainly a call to
  setText() to set the contents. Setting the background color is
  slighty different from other widgets, since a text view is a
  scrollable widget that naturally provies a scrolling background. You
  can specify the colorgroup of the displayed text with
  setPaperColorGroup() or directly define the paper background with
  setPaper(). QTextView supports both plain color and complex pixmap
  backgrounds.

  Note that we do not intend to add a full-featured web browser widget
  to Qt (since that would easily double Qt's size and only few
  applications would benefit from it). In particular, the rich text
  support in Qt is supposed to provide a fast and sufficient way to
  add reasonable online help facilities to applications. We will,
  however, extend it to some degree in future versions of Qt. Most
  likely some basic table support will be put in and a new class to
  provide rich text input.

  For even more, like hypertext capabilities, see QTextBrowser.

  \bug No selection possible.
  No table support (yet).
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
    QTimer* resizeTimer;
    Qt::TextFormat textformat;
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
    : QScrollView( parent, name, WNorthWestGravity )
{
    init();
    setText( text, context );
}


void QTextView::init()
{
    d = new QTextViewData;
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;

    setKeyCompression( TRUE );
    setVScrollBarMode( QScrollView::Auto );
    setHScrollBarMode( QScrollView::Auto );
//     setHScrollBarMode( AlwaysOff );

    d->doc_ = 0;
    d->sheet_ = 0;
    d->factory_ = 0;
    d->txt = QString::fromLatin1("<p></p>");
    d->textformat = AutoText;

    viewport()->setBackgroundMode(NoBackground);
    setFocusPolicy( StrongFocus );

    d->resizeTimer = new QTimer( this );
    connect( d->resizeTimer, SIGNAL( timeout() ), this, SLOT( doResize() ));
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

    if ( d->txt.isEmpty() )
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
	QPainter * p = new QPainter( this );
	// first try to use the full width of the viewport
	QSize vs( viewportSize( 1,1 ) );
	richText().setWidth( p, vs.width() );
	// if we'll need to scroll vertically, and the only reason we'll
	// need to scroll horizontally is the vertical scroll bar, try
	// to reformat so we won't need to scroll horizontally at all
	if ( richText().height > vs.height() &&
	     richText().width <= vs.width() &&
	     richText().width + /*###*/ 16 /*###*/ > vs.width() )
	    richText().setWidth( p, vs.width()-16 );
	delete p;
	resizeContents( QMAX( richText().widthUsed, richText().width), richText().height );
	viewport()->update();
	viewport()->setCursor( arrowCursor );
    }
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
    d->doc_ = new QRichText( d->txt, viewport()->font(), d->contxt,
			     8, mimeSourceFactory(), styleSheet() );
    if ( !d->doc_->attributes() )
	return;
    if (d->doc_->attributes()->contains("bgcolor")){
	QColor  col ( (*d->doc_->attributes())["bgcolor"].latin1() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Base, col );
    }
    if (d->doc_->attributes()->contains("text")){
	QColor  col ( (*d->doc_->attributes())["text"].latin1() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Text,  col );
    }
    if (d->doc_->attributes()->contains("bgpixmap")){
	QString imageName = (*d->doc_->attributes())["bgpixmap"];
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
  Returns the document title parsed from the content.
*/
QString QTextView::documentTitle() const
{
    return richText().attributes()?(*richText().attributes())["title"]:QString::null;
}

/*!
  Returns the height of the view given a width of \a w.
*/
int QTextView::heightForWidth( int w ) const
{
    QRichText doc ( d->txt, viewport()->font(), d->contxt,
		    8, mimeSourceFactory(), styleSheet() );
    {
	QPainter p( this );
	doc.setWidth(&p, w);
    }
    return doc.height;
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
    QRegion r(cx-ox, cy-oy, cw, ch);
    richText().draw(p, 0, 0, ox, oy, cx, cy, cw, ch, r, paperColorGroup(), &paper() );

    p->setClipRegion(r);

    if ( paper().pixmap() )
	p->drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			   *paper().pixmap(), ox, oy);
    else
	p->fillRect(0, 0, viewport()->width(), viewport()->height(), paper() );

    qApp->syncX();

    p->setClipping( FALSE );
}

/*!
  \reimp
*/
void QTextView::viewportResizeEvent(QResizeEvent* )
{
}

void QTextView::doResize()
{
    QSize vw = viewportSize( QMAX( richText().widthUsed,
				   richText().width),
			     richText().height );
    {
	QPainter p(this);
	richText().setWidth( &p, vw.width() );
    }
    resizeContents( QMAX( richText().widthUsed,
			  richText().width),
		    richText().height );
    viewport()->update();
}



/*!
  \reimp
*/
void QTextView::resizeEvent( QResizeEvent* e )
{
    if ( contentsHeight() > 4 * height() ) {
      // large document, do deferred resize
      d->resizeTimer->start( 200, TRUE );
      QScrollView::resizeEvent( e );
    }
    else {
      // small document, resize immediately
      QSize vw = viewportSize( QMAX( richText().widthUsed,
				     richText().width),
			       richText().height );
      {
	  QPainter p( this );
	  richText().setWidth( &p, vw.width() );
      }
      resizeContents( QMAX( richText().widthUsed,
			    richText().width),
		      richText().height );
      QScrollView::resizeEvent( e );
      if ( viewport()->isVisibleToTLW() )
	  viewport()->repaint( FALSE );
    }
}


/*!
  \reimp
*/
void QTextView::viewportMousePressEvent( QMouseEvent* )
{
}

/*!
  \reimp
*/
void QTextView::viewportMouseReleaseEvent( QMouseEvent* )
{
}

/*!
  \reimp
*/
void QTextView::viewportMouseMoveEvent( QMouseEvent* )
{
}

/*!
  Provides scrolling and paging.
*/
void QTextView::keyPressEvent( QKeyEvent * e)
{
    switch (e->key()) {
    case Key_Right:
	break;
    case Key_Left:
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
void QTextView::setTextFormat( Qt::TextFormat format )
{
    d->textformat = format;
    setText( d->original_txt, d->contxt ); // trigger update
}


//************************************************************************

#if 0
QTextEdit::QTextEdit(QWidget *parent, const char *name)
    : QTextView( parent, name )
{
    setKeyCompression( TRUE );
    setVScrollBarMode( AlwaysOn );
    cursor_hidden = FALSE;
    cursorTimer = new QTimer( this );
    cursorTimer->start(200, TRUE);
    connect( cursorTimer, SIGNAL( timeout() ), this, SLOT( cursorTimerDone() ));

    cursor = 0;

}

QTextEdit::~QTextEdit()
{
}



/*!
  reimplemented for internal purposes
 */
void QTextEdit::setText( const QString& text, const QString& context  )
{
    QTextView::setText( text, context );
    delete cursor;
    cursor = new QTextCursor(richText());
}

/*!
  Make a tree dump
 */
QString QTextEdit::text()
{
    qDebug("not yet implemented");
    return "not yet implemented";
}

void QTextEdit::keyPressEvent( QKeyEvent * e)
{

//     if (e->key() == Key_Plus)
// 	exit(2); // profiling


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
	    QTextRow*  oldCursorRow = cursor->row;
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
		cursor->end(&p, select);
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
}


/* Updates the visible selection according to the internal
  selection. If oldY and newY is defined, then only the area between
  both horizontal lines is taken into account. */
void QTextEdit::updateSelection(int oldY, int newY)
{
    if (!cursor || !cursor->selectionDirty)
	return;

    if (oldY > newY) {
	int tmp = oldY;
	oldY = newY;
	newY = tmp;
    }

    QPainter p(viewport());
    int minY = oldY>=0?QMAX(QMIN(oldY, newY), contentsY()):contentsY();
    int maxY = newY>=0?QMIN(QMAX(oldY, newY), contentsY()+viewport()->height()):contentsY()+viewport()->height();
    QRegion r;
    richText().draw(&p, 0, 0, contentsX(), contentsY(),
			   contentsX(), minY,
			   viewport()->width(), maxY-minY,
			   r, paperColorGroup(), &paper(), FALSE, TRUE);
    cursor->selectionDirty = FALSE;
}

void QTextEdit::viewportMousePressEvent( QMouseEvent * e)
{
    hideCursor();
    cursor->clearSelection();
    updateSelection();
    {
	QPainter p( viewport() );
	cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
    }
    showCursor();
}

void QTextEdit::viewportMouseReleaseEvent( QMouseEvent * )
{
    // nothing
}

void QTextEdit::viewportMouseMoveEvent( QMouseEvent * e)
{
    if (e->state() & LeftButton) {
	hideCursor();
	QTextRow*  oldCursorRow = cursor->row;
	{
	    QPainter p(viewport());
	    cursor->goTo( &p, e->pos().x() + contentsX(),
			  e->pos().y() + contentsY(), TRUE);
	}
	if (cursor->row == oldCursorRow)
	    updateSelection(cursor->rowY, cursor->rowY );
	else
	    updateSelection();
	if (cursor->y + cursor->height > contentsY() + viewport()->height()) {
	    scrollBy(0, cursor->y + cursor->height-contentsY()-viewport()->height());
	}
	else if (cursor->y < contentsY())
	    scrollBy(0, cursor->y - contentsY() );
	showCursor();
    }
}

void QTextEdit::drawContentsOffset(QPainter*p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QTextView::drawContentsOffset(p, ox, oy, cx, cy, cw, ch);
    if (!cursor_hidden)
	cursor->draw(p, ox, oy, cx, cy, cw, ch);
}

void QTextEdit::cursorTimerDone()
{
    if (cursor_hidden) {
	if (QTextEdit::hasFocus())
	    showCursor();
	else
	    cursorTimer->start(400, TRUE);
    }
    else {
	hideCursor();
    }
}

void QTextEdit::showCursor()
{
    cursor_hidden = FALSE;
    QPainter p( viewport() );
    cursor->draw(&p, contentsX(), contentsY(),
		 contentsX(), contentsY(),
		 viewport()->width(), viewport()->height());
    cursorTimer->start(400, TRUE);
}

void QTextEdit::hideCursor()
{
    if (cursor_hidden)
	return;
    cursor_hidden = TRUE;
    repaintContents(cursor->x, cursor->y,
		    cursor->width(), cursor->height);
    cursorTimer->start(300, TRUE);
}



/*!  Updates the visible screen according to the (changed) internal
  data structure.
*/
void QTextEdit::updateScreen()
{
    {
	QPainter p( viewport() );
	QRegion r(0, 0, viewport()->width(), viewport()->height());
	richText().draw(&p, 0, 0, contentsX(), contentsY(),
			       contentsX(), contentsY(),
			       viewport()->width(), viewport()->height(),
			       r, paperColorGroup(), &paper(), TRUE);
	p.setClipRegion(r);
	if ( paper().pixmap() )
	    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			      *paperColorGroup().brush( QColorGroup::Base ).pixmap(), contentsX(), contentsY());
	else
	    p.fillRect(0, 0, viewport()->width(), viewport()->height(), paper());
    }
    showCursor();
    resizeContents( QMAX( richText().widthUsed, richText().width(), richText().height );
    ensureVisible(cursor->x, cursor->y);
}

void QTextEdit::viewportResizeEvent(QResizeEvent* e)
{
    QTextView::viewportResizeEvent(e);
    {
	QPainter p( this );
	cursor->calculatePosition(&p);
    }
}


#endif

