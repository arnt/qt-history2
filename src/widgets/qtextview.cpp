/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.cpp#12 $
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





/*!
  \class QTextView qtextview.h
  \brief A sophisticated single-page rich text viewer.
  \ingroup realwidgets

  Unlike QSimpleRichText, which merely draws small pieces of rich text,
  a QTextView is a real widget, with scrollbars when necessary, for showing
  large text documents.

  For even more, like hypertext capabilities, see QTextBrowser.
*/

class QTextViewData
{
public:
    QStyleSheet* sheet_;
    QRichText* doc_;
    QMLProvider* provider_;
    QString txt;
    QColorGroup mypapcolgrp;
    QColorGroup papcolgrp;
    QTimer* resizeTimer;
};


/*!
  Constructs an empty QTextView
  with the standard \a parent and \a name optional arguments.
*/
QTextView::QTextView(QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();
}


/*!
  Constructs a QTextView displaying the contents \a doc,
  with the standard \a parent and \a name optional arguments.
*/
QTextView::QTextView( const QString& doc, QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();
    d->txt = doc;
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
    d->provider_ = 0;
    d->txt = QString::fromLatin1("<p></p>");

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
  Changes the contents of the view to the string \a text.

  \sa contents()
*/
void QTextView::setText( const QString& text)
{
    delete d->doc_;
    d->doc_ = 0;

    // ###### TODOdo format stuff
    if ( QStyleSheet::mightBeRichText( text ) ) 
      d->txt = text;
    else 
      d->txt = QStyleSheet::convertFromPlainText( text );

    if ( d->txt.isEmpty() )
	d->txt = QString::fromLatin1("<p></p>");
    if ( isVisible() ) {
	QPainter * p = new QPainter( this );
	// first try to use the full width of the viewport
	QSize vs( viewportSize( 1,1 ) );
	currentDocument().setWidth( p, vs.width() );
	// if we'll need to scroll vertically, and the only reason we'll
	// need to scroll horizontally is the vertical scroll bar, try
	// to reformat so we won't need to scroll horizontally at all
	if ( currentDocument().height > vs.height() &&
	     currentDocument().width <= vs.width() &&
	     currentDocument().width + /*###*/ 16 /*###*/ > vs.width() )
	    currentDocument().setWidth( p, vs.width()-16 );
	delete p;
	resizeContents( QMAX( currentDocument().widthUsed, currentDocument().width), currentDocument().height );
	viewport()->update();
	viewport()->setCursor( arrowCursor );
    }
}


/*!
  Returns the contents of the view.

  \sa setContents()
*/
QString QTextView::text() const
{
    return d->txt;
}


void QTextView::createDocument()
{
    d->papcolgrp = d->mypapcolgrp;
    d->doc_ = new QRichText( d->txt, viewport()->font(), 8, provider(), styleSheet() );
    if ( !d->doc_->attributes() )
	return;
    if (d->doc_->attributes()->find("bgcolor")){
	QColor  col ( d->doc_->attributes()->find("bgcolor")->ascii() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Base, col );
    }
    if (d->doc_->attributes()->find("text")){
	QColor  col ( d->doc_->attributes()->find("text")->ascii() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Text,  col );
    }
    if (d->doc_->attributes()->find("bgpixmap")){
	QPixmap pm = provider()->image(*d->doc_->attributes()->find("bgpixmap"));
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
  Returns the current provider for the view.

  \sa setProvider()
*/
QMLProvider* QTextView::provider() const
{
    if (!d->provider_)
	return QMLProvider::defaultProvider();
    else
	return d->provider_;

}

/*!
  Sets the provider for the view.

  \sa provider()
*/
void QTextView::setProvider( QMLProvider* newProvider )
{
    d->provider_ = newProvider;
    viewport()->update();
}


/*!
  Sets the brush to use as the background to \a pap.

  \sa paper()
*/
void QTextView::setPaper( const QBrush& pap)
{
    d->mypapcolgrp.setBrush( QColorGroup::Base, pap );
    d->papcolgrp.setBrush( QColorGroup::Base, pap );
    viewport()->update();
}

/*!
  Sets the full colorgroup of the background to \a colgrp.
*/
void QTextView::setPaperColorGroup( const QColorGroup& colgrp)
{
    d->mypapcolgrp = colgrp;
    d->papcolgrp = colgrp;
    viewport()->update();
}

/*!
  Returns the colorgroup of the background.
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
    QString* s = currentDocument().attributes()?currentDocument().attributes()->find("title"):0;
    if (s)
	return *s;
    else
	return QString();
}

/*!
  Returns the height of the view given a width of \a w.
*/
int QTextView::heightForWidth( int w ) const
{
    QRichText doc ( d->txt, viewport()->font(), 8, provider(), styleSheet());
    {
	QPainter p( this );
	doc.setWidth(&p, w);
    }
    return doc.height;
}

/*!
  Returns the current document defining the view.  This is not currently
  useful for applications.
*/
QRichText& QTextView::currentDocument() const
{
    if (!d->doc_){
	QTextView* that = (QTextView*) this;
	that->createDocument();
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
    currentDocument().draw(p, 0, 0, ox, oy, cx, cy, cw, ch, r, paperColorGroup(), &paper() );

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
    QSize vw = viewportSize( QMAX( currentDocument().widthUsed,
				   currentDocument().width),
			     currentDocument().height );
    {
	QPainter p(this);
	currentDocument().setWidth( &p, vw.width() );
    }
    resizeContents( QMAX( currentDocument().widthUsed,
			  currentDocument().width),
		    currentDocument().height );
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
      QSize vw = viewportSize( QMAX( currentDocument().widthUsed,
				     currentDocument().width),
			       currentDocument().height );
      QPainter p;
      p.begin( this );
      currentDocument().setWidth( &p, vw.width() );
      p.end();
      resizeContents( QMAX( currentDocument().widthUsed,
			    currentDocument().width),
		      currentDocument().height );
      QScrollView::resizeEvent( e );
      viewport()->update();
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
void QTextView::paletteChange( const QPalette & )
{
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;
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
void QTextEdit::setText( const QString& contents)
{
    QTextView::setContents( contents );
    delete cursor;
    cursor = new QTextCursor(currentDocument());
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
    currentDocument().draw(&p, 0, 0, contentsX(), contentsY(),
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
	currentDocument().draw(&p, 0, 0, contentsX(), contentsY(),
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
    resizeContents( QMAX( currentDocument().widthUsed, currentDocument().width(), currentDocument().height );
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

