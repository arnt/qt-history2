/****************************************************************************
** $Id: $
**
**  Splitter widget
**
**  Created:  980105
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qsplitter.h"
#ifndef QT_NO_SPLITTER

#include "qpainter.h"
#include "qdrawutil.h"
#include "qbitmap.h"
#include "../kernel/qlayoutengine_p.h"
#include "qptrlist.h"
#include "qmemarray.h"
#include "qobjectlist.h"
#include "qstyle.h"
#include "qapplication.h" //sendPostedEvents

class QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle( Qt::Orientation o,
		       QSplitter *parent, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    QSize sizeHint() const;

    int id() const { return myId; } // data->list.at(id())->wid == this
    void setId( int i ) { myId = i; }

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    Qt::Orientation orient;
    bool opaq;
    int myId;

    QSplitter *s;
};

#include "qsplitter.moc"

static int mouseOffset;
static int opaqueOldPos = -1; //### there's only one mouse, but this is a bit risky


QSplitterHandle::QSplitterHandle( Qt::Orientation o,
				  QSplitter *parent, const char * name )
    : QWidget( parent, name )
{
    s = parent;
    setOrientation(o);
}

QSize QSplitterHandle::sizeHint() const
{
    int sw = style().pixelMetric(QStyle::PM_SplitterWidth, this);
    return (style().sizeFromContents(QStyle::CT_Splitter, s, QSize(sw, sw)).
	    expandedTo(QApplication::globalStrut()));
}

void QSplitterHandle::setOrientation( Qt::Orientation o )
{
    orient = o;
#ifndef QT_NO_CURSOR
    if ( o == QSplitter::Horizontal )
	setCursor( splitHCursor );
    else
	setCursor( splitVCursor );
#endif
}


void QSplitterHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !(e->state()&LeftButton) )
	return;
    QCOORD pos = s->pick(parentWidget()->mapFromGlobal(e->globalPos()))
		 - mouseOffset;
    if ( opaque() ) {
	s->moveSplitter( pos, id() );
    } else {
	int min = pos; int max = pos;
	s->getRange( id(), &min, &max );
	s->setRubberband( QMAX( min, QMIN(max, pos )));
    }
}

void QSplitterHandle::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	mouseOffset = s->pick(e->pos());
}

void QSplitterHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !opaque() && e->button() == LeftButton ) {
	QCOORD pos = s->pick(parentWidget()->mapFromGlobal(e->globalPos()))
		     - mouseOffset;
	s->setRubberband( -1 );
	s->moveSplitter( pos, id() );
    }
}

void QSplitterHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawPrimitive(QStyle::PE_Splitter, &p, rect(), colorGroup(),
			  (orientation() == Qt::Horizontal ?
			   QStyle::Style_Horizontal : 0));
}

class QSplitterLayoutStruct
{
public:
    QSplitter::ResizeMode mode;
    QCOORD sizer;
    bool isSplitter;
    QWidget *wid;
};

class QSplitterData
{
public:
    QSplitterData() : opaque( FALSE ), firstShow( TRUE ) {}

    QPtrList<QSplitterLayoutStruct> list;
    bool opaque;
    bool firstShow;
};


/*!
  \class QSplitter qsplitter.h
  \brief The QSplitter class implements a splitter widget.

  \ingroup organizers
  \mainclass

  A splitter lets the user control the size of child widgets by
  dragging the boundary between the children. Any number of widgets
  may be controlled.

  To show a QListBox, a QListView and a QTextEdit side by side:

  \code
    QSplitter *split = new QSplitter( parent );
    QListBox *lb = new QListBox( split );
    QListView *lv = new QListView( split );
    QTextEdit *ed = new QTextEdit( split );
  \endcode

  In QSplitter the boundary can be either horizontal or vertical.  The
  default is horizontal (the children are side by side) but you
  can use setOrientation( QSplitter::Vertical ) to set it to vertical.

  By default, all widgets can be as large or as small as the user
  wishes, down to \link QWidget::minimumSizeHint()
  minimumSizeHint()\endlink. You can also use setMinimumSize() and
  setMaximumSize() on the children. Use setResizeMode() to specify
  that a widget should keep its size when the splitter is resized.

  Although QSplitter normally resizes the children only at the end of a
  resize operation, if you call setOpaqueResize( TRUE ) the
  widgets are resized as often as possible.

  The initial distribution of size between the widgets is determined
  by the initial size of each widget. You can also use setSizes() to
  set the sizes of all the widgets. The function sizes() returns the
  sizes set by the user.

  If you hide() a child its space will be distributed among the other
  children. It will be reinstated when you show() it again. It is also
  possible to reorder the widgets within the splitter using
  moveToFirst() and moveToLast().

  <img src=qsplitter-m.png> <img src=qsplitter-w.png>

  \sa QTabBar
*/



static QSize minSize( const QWidget *w )
{
    QSize min = w->minimumSize();
    QSize s;
    if ( min.height() <= 0 || min.width() <= 0 )
	s = w->minimumSizeHint();
    if ( min.height() > 0 )
	s.setHeight( min.height() );
    if ( min.width() > 0 )
	s.setWidth( min.width() );
    return s.expandedTo(QSize(0,0));
}

/*!
  Constructs a horizontal splitter with the \a parent and \a
  name arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter( QWidget *parent, const char *name )
    :QFrame(parent,name,WPaintUnclipped)
{
     orient = Horizontal;
     init();
}


/*!
  Constructs a splitter with orientation \a o with the \a parent
  and \a name arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter( Orientation o, QWidget *parent, const char *name )
    :QFrame(parent,name,WPaintUnclipped)
{
     orient = o;
     init();
}


/*!
  Destroys the splitter and any children.
*/

QSplitter::~QSplitter()
{
    data->list.setAutoDelete( TRUE );
    delete data;
}


void QSplitter::init()
{
    data = new QSplitterData;
    if ( orient == Horizontal )
	setSizePolicy( QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum) );
    else
	setSizePolicy( QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding) );
}


/*!
  \fn void QSplitter::refresh()

  Updates the splitter's state. You should not need to call this
  function.
*/


/*!
  \property QSplitter::orientation
  \brief the orientation of the splitter

  By default the orientation is horizontal (the widgets are side by side).
  The possible orientations are Qt:Vertical and Qt::Horizontal (the default).

*/

void QSplitter::setOrientation( Orientation o )
{
    if ( orient == o )
	return;
    orient = o;

    if ( orient == Horizontal )
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    else
	setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->isSplitter )
	    ((QSplitterHandle*)s->wid)->setOrientation( o );
	s = data->list.next();  // ### next at end of loop, no iterator
    }
    recalc( isVisible() );
}


/*!
  \reimp
*/
void QSplitter::resizeEvent( QResizeEvent * )
{
    doResize();
}


/*
  Inserts the widget \a w at the end (or at the beginning if \a first
  is TRUE) of the splitter's list of widgets.

  It is the responsibility of the caller of this function to make sure
  that \a w is not already in the splitter and to call recalcId if
  needed.  (If \a first is TRUE, then recalcId is very probably
  needed.)
*/

QSplitterLayoutStruct *QSplitter::addWidget( QWidget *w, bool first )
{
    QSplitterLayoutStruct *s;
    QSplitterHandle *newHandle = 0;
    if ( data->list.count() > 0 ) {
	s = new QSplitterLayoutStruct;
	s->mode = KeepSize;
	QString tmp = "qt_splithandle_";
	tmp += w->name();
	newHandle = new QSplitterHandle( orientation(), this, tmp );
	s->wid = newHandle;
	newHandle->setId(data->list.count());
	s->isSplitter = TRUE;
	s->sizer = pick( newHandle->sizeHint() );
	if ( first )
	    data->list.insert( 0, s );
	else
	    data->list.append( s );
    }
    s = new QSplitterLayoutStruct;
    s->mode = Stretch;
    s->wid = w;
    if ( !testWState( WState_Resized ) && w->sizeHint().isValid() )
	s->sizer = pick( w->sizeHint() );
    else
	s->sizer = pick( w->size() );
    s->isSplitter = FALSE;
    if ( first )
	data->list.insert( 0, s );
    else
	data->list.append( s );
    if ( newHandle && isVisible() )
	newHandle->show(); //will trigger sending of post events
    return s;
}


/*!
  Tells the splitter that a child widget has been inserted or removed.
  The event is passed in \a c.
*/

void QSplitter::childEvent( QChildEvent *c )
{
    if ( c->type() == QEvent::ChildInserted ) {
	if ( !c->child()->isWidgetType() )
	    return;

	if ( ((QWidget*)c->child())->testWFlags( WType_TopLevel ) )
	    return;

	QSplitterLayoutStruct *s = data->list.first();
	while ( s ) {
	    if ( s->wid == c->child() )
		return;
	    s = data->list.next();
	}
	addWidget( (QWidget*)c->child() );
	recalc( isVisible() );

    } else if ( c->type() == QEvent::ChildRemoved ) {
	QSplitterLayoutStruct *p = 0;
	if ( data->list.count() > 1 )
	    p = data->list.at(1); //remove handle _after_ first widget.
	QSplitterLayoutStruct *s = data->list.first();
	while ( s ) {
	    if ( s->wid == c->child() ) {
		data->list.removeRef( s );
		delete s;
		if ( p && p->isSplitter ) {
		    data->list.removeRef( p );
		    delete p->wid; //will call childEvent
		    delete p;
		}
		recalcId();
		doResize();
		return;
	    }
	    p = s;
	    s = data->list.next();
	}
    }
}


/*!
  Shows a rubber band at position \a p. If \a p is negative, the
  rubber band is removed.
*/

void QSplitter::setRubberband( int p )
{
    QPainter paint( this );
    paint.setPen( gray );
    paint.setBrush( gray );
    paint.setRasterOp( XorROP );
    QRect r = contentsRect();
    const int rBord = 3; //Themable????
    int sw = style().pixelMetric(QStyle::PM_SplitterWidth, this);
    if ( orient == Horizontal ) {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( opaqueOldPos + sw/2 - rBord , r.y(),
			    2*rBord, r.height() );
	if ( p >= 0 )
	    paint.drawRect( p  + sw/2 - rBord, r.y(), 2*rBord, r.height() );
    } else {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( r.x(), opaqueOldPos + sw/2 - rBord,
			    r.width(), 2*rBord );
	if ( p >= 0 )
	    paint.drawRect( r.x(), p + sw/2 - rBord, r.width(), 2*rBord );
    }
    opaqueOldPos = p;
}


/*! \reimp */

bool QSplitter::event( QEvent *e )
{
    if ( e->type() == QEvent::LayoutHint || ( e->type() == QEvent::Show && data->firstShow ) ) {
	recalc( isVisible() );
	if ( e->type() == QEvent::Show )
	    data->firstShow = FALSE;
    }
    return QWidget::event( e );
}


/*!
  \obsolete
  
  Draws the splitter handle in the rectangle described by \a x, \a y,
  \a w, \a h using painter \a p.
  \sa QStyle::drawPrimitive()
*/

// ### Remove this in 4.0

void QSplitter::drawSplitter( QPainter *p,
			      QCOORD x, QCOORD y, QCOORD w, QCOORD h )
{
    style().drawPrimitive(QStyle::PE_Splitter, p, QRect(x, y, w, h), colorGroup(),
			  (orientation() == Qt::Horizontal ?
			   QStyle::Style_Horizontal : 0));
}


/*!
  Returns the id of the splitter to the right of or below the widget \a w,
  or 0 if there is no such splitter
  (i.e. it is either not in this QSplitter or it is at the end).
*/

int QSplitter::idAfter( QWidget* w ) const
{
    QSplitterLayoutStruct *s = data->list.first();
    bool seen_w = FALSE;
    while ( s ) {
	if ( s->isSplitter && seen_w )
	    return data->list.at();
	if ( !s->isSplitter && s->wid == w )
	    seen_w = TRUE;
	s = data->list.next();
    }
    return 0;
}


/*!
  Moves the left/top edge of the splitter handle with id \a id as
  close as possible to position \a p, which is the distance from the
  left (or top) edge of the widget.

  For Arabic and Hebrew the layout is reversed, and using this
  function to set the position of the splitter might lead to
  unexpected results, since in Arabic and Hebrew the position of
  splitter one is to the left of the position of splitter zero.

  \sa idAfter()
*/
void QSplitter::moveSplitter( QCOORD p, int id )
{
    p = adjustPos( p, id );

    QSplitterLayoutStruct *s = data->list.at(id);
    int oldP = orient == Horizontal ? s->wid->x() : s->wid->y();
    bool upLeft;
    if ( QApplication::reverseLayout() && orient == Horizontal ) {
	p += s->wid->width();
	upLeft = p > oldP;
    } else
	upLeft = p < oldP;

    moveAfter( p, id, upLeft );
    moveBefore( p-1, id-1, upLeft );

    storeSizes();
}


void QSplitter::setG( QWidget *w, int p, int s, bool isSplitter )
{
    if ( orient == Horizontal ) {
	if ( QApplication::reverseLayout() && orient == Horizontal && !isSplitter )
	    p = contentsRect().width() - p - s;
	w->setGeometry( p, contentsRect().y(), s, contentsRect().height() );
    } else
	w->setGeometry( contentsRect().x(), p, contentsRect().width(), s );
}


/*
  Places the right/bottom edge of the widget at \a id at position \a pos.

  \sa idAfter()
*/

void QSplitter::moveBefore( int pos, int id, bool upLeft )
{
    if( id < 0 )
	return;
    QSplitterLayoutStruct *s = data->list.at(id);
    if ( !s )
	return;
    QWidget *w = s->wid;
    if ( w->isHidden() ) {
	moveBefore( pos, id-1, upLeft );
    } else if ( s->isSplitter ) {
	int pos1, pos2;
	int dd = s->sizer;
	if( QApplication::reverseLayout() && orient == Horizontal ) {
	    pos1 = pos;
	    pos2 = pos + dd;
	} else {
	    pos2 = pos - dd;
	    pos1 = pos2 + 1;
	}
	if ( upLeft ) {
	    setG( w, pos1, dd, TRUE );
	    moveBefore( pos2, id-1, upLeft );
	} else {
	    moveBefore( pos2, id-1, upLeft );
	    setG( w, pos1, dd, TRUE );
	}
    } else {
	int dd, newLeft, nextPos;
	if( QApplication::reverseLayout() && orient == Horizontal ) {
	    dd = w->geometry().right() - pos;
	    dd = QMAX( pick(minSize(w)), QMIN(dd, pick(w->maximumSize())));
	    newLeft = pos+1;
	    nextPos = newLeft + dd;
	} else {
	    dd = pos - pick( w->pos() ) + 1;
	    dd = QMAX( pick(minSize(w)), QMIN(dd, pick(w->maximumSize())));
	    newLeft = pos-dd+1;
	    nextPos = newLeft - 1;
	}
	setG( w, newLeft, dd, TRUE );
	moveBefore( nextPos, id-1, upLeft );
    }
}


/*
  Places the left/top edge of the widget at \a id at position \a pos.

  \sa idAfter()
*/

void QSplitter::moveAfter( int pos, int id, bool upLeft )
{
    QSplitterLayoutStruct *s = id < int(data->list.count()) ?
			       data->list.at(id) : 0;
    if ( !s )
	return;
    QWidget *w = s->wid;
    if ( w->isHidden() ) {
	moveAfter( pos, id+1, upLeft );
    } else if ( pick( w->pos() ) == pos ) {
	//No need to do anything if it's already there.
	return;
    } else if ( s->isSplitter ) {
	int dd = s->sizer;
	int pos1, pos2;
	if( QApplication::reverseLayout() && orient == Horizontal ) {
	    pos2 = pos - dd;
	    pos1 = pos2 + 1;
	} else {
	    pos1 = pos;
	    pos2 = pos + dd;
	}
	if ( upLeft ) {
	    setG( w, pos1, dd, TRUE );
	    moveAfter( pos2, id+1, upLeft );
	} else {
	    moveAfter( pos2, id+1, upLeft );
	    setG( w, pos1, dd, TRUE );
	}
    } else {
	int left = pick( w->pos() );
	int right, dd,/* newRight,*/ newLeft, nextPos;
	if ( QApplication::reverseLayout() && orient == Horizontal ) {
	    dd = pos - left + 1;
	    dd = QMAX( pick(minSize(w)), QMIN(dd, pick(w->maximumSize())));
	    newLeft = pos-dd+1;
	    nextPos = newLeft - 1;
	} else {
	    right = pick( w->geometry().bottomRight() );
	    dd = right - pos + 1;
	    dd = QMAX( pick(minSize(w)), QMIN(dd, pick(w->maximumSize())));
	    /*newRight = pos+dd-1;*/
	    newLeft = pos;
	    nextPos = newLeft + dd;
	}
	setG( w, newLeft, dd, TRUE );
	/*if( right != newRight )*/
	moveAfter( nextPos, id+1, upLeft );
    }
}


/*!
  Returns the valid range of the splitter with id \a id in \a *min and \a *max.

  \sa idAfter()
*/

void QSplitter::getRange( int id, int *min, int *max )
{
    int minB = 0;	//before
    int maxB = 0;
    int minA = 0;
    int maxA = 0;	//after
    int n = data->list.count();
    if ( id < 0 || id >= n )
	return;
    int i;
    for ( i = 0; i < id; i++ ) {
	QSplitterLayoutStruct *s = data->list.at(i);
	if ( s->wid->isHidden() ) {
	    //ignore
	} else if ( s->isSplitter ) {
	    minB += s->sizer;
	    maxB += s->sizer;
	} else {
	    minB += pick( minSize(s->wid) );
	    maxB += pick( s->wid->maximumSize() );
	}
    }
    for ( i = id; i < n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at(i);
	if ( s->wid->isHidden() ) {
	    //ignore
	} else if ( s->isSplitter ) {
	    minA += s->sizer;
	    maxA += s->sizer;
	} else {
	    minA += pick( minSize(s->wid) );
	    maxA += pick( s->wid->maximumSize() );
	}
    }
    QRect r = contentsRect();
    if ( orient == Horizontal && QApplication::reverseLayout() ) {
	int splitterWidth = style().pixelMetric(QStyle::PM_SplitterWidth, this);
	if ( min )
	    *min = pick(r.topRight()) - QMIN( maxB, pick(r.size())-minA ) - splitterWidth;
	if ( max )
	    *max = pick(r.topRight()) - QMAX( minB, pick(r.size())-maxA ) - splitterWidth;
    } else {
	if ( min )
	    *min = pick(r.topLeft()) + QMAX( minB, pick(r.size())-maxA );
	if ( max )
	    *max = pick(r.topLeft()) + QMIN( maxB, pick(r.size())-minA );
    }
}


/*!
  Returns the closest legal position to \a p of the splitter with id \a id.

  \sa idAfter()
*/

int QSplitter::adjustPos( int p, int id )
{
    int min = 0;
    int max = 0;
    getRange( id, &min, &max );
    p = QMAX( min, QMIN( p, max ) );

    return p;
}


void QSplitter::doResize()
{
    QRect r = contentsRect();
    int i;
    int n = data->list.count();
    QMemArray<QLayoutStruct> a( n );
    for ( i = 0; i< n; i++ ) {
	a[i].init();
	QSplitterLayoutStruct *s = data->list.at(i);
	if ( s->wid->isHidden() ) {
	    a[i].stretch = 0;
	    a[i].sizeHint = a[i].minimumSize = 0;
	    a[i].maximumSize = 0;
	} else if ( s->isSplitter ) {
	    a[i].stretch = 0;
	    a[i].sizeHint = a[i].minimumSize = a[i].maximumSize = s->sizer;
	    a[i].empty = FALSE;
	} else if ( s->mode == KeepSize ) {
	    a[i].stretch = 0;
	    a[i].minimumSize = pick( minSize(s->wid) );
	    a[i].sizeHint = s->sizer;
	    a[i].maximumSize = pick( s->wid->maximumSize() );
	    a[i].empty = FALSE;
	} else if ( s->mode == FollowSizeHint ) {
	    a[i].stretch = 0;
	    a[i].minimumSize = a[i].sizeHint = pick( s->wid->sizeHint() );
	    a[i].maximumSize = pick( s->wid->maximumSize() );
	    a[i].empty = FALSE;
	} else { //proportional
	    a[i].stretch = s->sizer;
	    a[i].maximumSize = pick( s->wid->maximumSize() );
	    a[i].sizeHint = a[i].minimumSize = pick( minSize(s->wid) );
	    a[i].empty = FALSE;
	}
    }

    qGeomCalc( a, 0, n, pick( r.topLeft() ), pick( r.size() ), 0 );

    for ( i = 0; i< n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at(i);
	setG( s->wid, a[i].pos, a[i].size );
    }

}


void QSplitter::recalc( bool update )
{
    int fi = 2*frameWidth();
    int maxl = fi;
    int minl = fi;
    int maxt = QWIDGETSIZE_MAX;
    int mint = fi;
    int n = data->list.count();
    bool first = TRUE;
    /*
      The splitter before a hidden widget is always hidden.
      The splitter before the first visible widget is hidden.
      The splitter before any other visible widget is visible.
    */
    for ( int i = 0; i< n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at(i);
	if ( !s->isSplitter ) {
	    QSplitterLayoutStruct *p = (i > 0) ? p = data->list.at( i-1 ) : 0;
	    if ( p && p->isSplitter )
		if ( first || s->wid->isHidden() )
		    p->wid->hide(); //may trigger new recalc
		else
		    p->wid->show(); //may trigger new recalc
	    if ( !s->wid->isHidden() )
		first = FALSE;
	}
    }

    bool empty=TRUE;
    for ( int j = 0; j< n; j++ ) {
	QSplitterLayoutStruct *s = data->list.at(j);
	if ( !s->wid->isHidden() ) {
	    empty = FALSE;
	    if ( s->isSplitter ) {
		minl += s->sizer;
		maxl += s->sizer;
	    } else {
		QSize minS = minSize(s->wid);
		minl += pick( minS );
		maxl += pick( s->wid->maximumSize() );
		mint = QMAX( mint, trans( minS ));
		int tm = trans( s->wid->maximumSize() );
		if ( tm > 0 )
		    maxt = QMIN( maxt, tm );
	    }
	}
    }
    if ( empty )
	maxl = maxt = 0;
    else
	maxl = QMIN( maxl, QWIDGETSIZE_MAX );
    if ( maxt < mint )
	maxt = mint;

    if ( orient == Horizontal ) {
	setMaximumSize( maxl, maxt );
	setMinimumSize( minl, mint );
    } else {
	setMaximumSize( maxt, maxl );
	setMinimumSize( mint, minl );
    }
    if ( update )
	doResize();
}

/*! \enum QSplitter::ResizeMode

  This enum type describes how QSplitter will resize each of its child widgets.  The currently defined values are:

  \value Stretch  the widget will be resized when the splitter
  itself is resized.

  \value KeepSize  QSplitter will try to keep this widget's size
  unchanged.

  \value FollowSizeHint  QSplitter will resize the widget when the
  widget's size hint changes.
*/

/*!
  Sets resize mode of \a w to \a mode.

  \sa ResizeMode
*/

void QSplitter::setResizeMode( QWidget *w, ResizeMode mode )
{
    processChildEvents();
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->wid == w  ) {
	    s->mode = mode;
	    return;
	}
	s = data->list.next();
    }
    s = addWidget( w, TRUE );
    s->mode = mode;
}


/*!
  Returns TRUE if opaque resize is on; otherwise returns FALSE.

  \sa setOpaqueResize()
*/

bool QSplitter::opaqueResize() const
{
    return data->opaque;
}


/*!
    If \a on is TRUE then opaque resizing is turned on; otherwise
    opaque resizing is turned off.
  Opaque resizing is initially turned off.

  \sa opaqueResize()
*/

void QSplitter::setOpaqueResize( bool on )
{
    data->opaque = on;
}


/*!
  Moves widget \a w to the leftmost/top position.
*/

void QSplitter::moveToFirst( QWidget *w )
{
    processChildEvents();
    bool found = FALSE;
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->wid == w  ) {
	    found = TRUE;
	    QSplitterLayoutStruct *p = data->list.prev();
	    if ( p ) { // not already at first place
		data->list.take(); //take p
		data->list.take(); // take s
		data->list.insert( 0, p );
		data->list.insert( 0, s );
	    }
	    break;
	}
	s = data->list.next();
    }
     if ( !found )
	addWidget( w, TRUE );
     recalcId();
}


/*!
  Moves widget \a w to the rightmost/bottom position.
*/

void QSplitter::moveToLast( QWidget *w )
{
    processChildEvents();
    bool found = FALSE;
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->wid == w  ) {
	    found = TRUE;
	    data->list.take(); // take s
	    QSplitterLayoutStruct *p = data->list.current();
	    if ( p ) { // the splitter handle after s
		data->list.take(); //take p
		data->list.append( p );
	    }
	    data->list.append( s );
	    break;
	}
	s = data->list.next();
    }
     if ( !found )
	addWidget( w);
     recalcId();
}


void QSplitter::recalcId()
{
    int n = data->list.count();
    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at(i);
	if ( s->isSplitter )
	    ((QSplitterHandle*)s->wid)->setId(i);
    }
}


/*!\reimp
*/
QSize QSplitter::sizeHint() const
{
    constPolish();
    int l = 0;
    int t = 0;
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() &&
		 !((QWidget*)o)->isHidden() ) {
		QSize s = ((QWidget*)o)->sizeHint();
		if ( s.isValid() ) {
		    l += pick( s );
		    t = QMAX( t, trans( s ) );
		}
	    }
	}
    }
    return orientation() == Horizontal ? QSize( l, t ) : QSize( t, l );
}


/*!
\reimp
*/

QSize QSplitter::minimumSizeHint() const
{
    constPolish();
    int l = 0;
    int t = 0;
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() &&
		 !((QWidget*)o)->isHidden() ) {
		QSize s = minSize((QWidget*)o);
		if ( s.isValid() ) {
		    l += pick( s );
		    t = QMAX( t, trans( s ) );
		}
	    }
	}
    }
    return orientation() == Horizontal ? QSize( l, t ) : QSize( t, l );
}


/*
  Calculates stretch parameters from current sizes
*/

void QSplitter::storeSizes()
{
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( !s->isSplitter )
	    s->sizer = pick( s->wid->size() );
	s = data->list.next();
    }
}


#if 0 // ### remove this code ASAP

/*!
  Hides \a w if \a hide is TRUE and updates the splitter.

  \warning Due to a limitation in the current implementation,
  calling QWidget::hide() will not work.
*/

void QSplitter::setHidden( QWidget *w, bool hide )
{
    if ( w == w1 ) {
	w1show = !hide;
    } else if ( w == w2 ) {
	w2show = !hide;
    } else {
#ifdef QT_CHECK_RANGE
	qWarning( "QSplitter::setHidden(), unknown widget" );
#endif
	return;
    }
    if ( hide )
	w->hide();
    else
	w->show();
    recalc( TRUE );
}


/*!
  Returns the hidden status of \a w
*/

bool QSplitter::isHidden( QWidget *w ) const
{
    if ( w == w1 )
	return !w1show;
     else if ( w == w2 )
	return !w2show;
#ifdef QT_CHECK_RANGE
    else
	qWarning( "QSplitter::isHidden(), unknown widget" );
#endif
    return FALSE;
}
#endif


/*!
  Returns a list of the size parameters of all the widgets in this
  splitter.

  Giving the values to another splitter's setSizes() function will
  produce a splitter with the same layout as this one.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QValueList<int> list = mySplitter.sizes();
    QValueList<int>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa setSizes()
*/

QValueList<int> QSplitter::sizes() const
{
    if ( !testWState(WState_Polished) ) {
	QWidget* that = (QWidget*) this;
	that->polish();
    }
    QValueList<int> list;
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( !s->isSplitter )
	    list.append( s->sizer );
	s = data->list.next();
    }
    return list;
}



/*!
  Sets the size parameters to the values given in \a list.
  If the splitter is horizontal, the values set the sizes from
  left to right. If it is vertical, the sizes are applied from
  top to bottom.
  Extra values in \a list are ignored.

  If \a list contains too few values, the result is undefined
  but the program will still be well-behaved.

  \sa sizes()
*/

void QSplitter::setSizes( QValueList<int> list )
{
    processChildEvents();
    QValueList<int>::Iterator it = list.begin();
    QSplitterLayoutStruct *s = data->list.first();
    while ( s && it != list.end() ) {
	if ( !s->isSplitter ) {
	    s->sizer = *it;
	    ++it;
	}
	s = data->list.next();
    }
    doResize();
}


/*!
  Gets all posted child events, ensuring that the internal state of
  the splitter is consistent.
*/

void QSplitter::processChildEvents()
{
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );
}


/*!
  \reimp
*/

void QSplitter::styleChange( QStyle& old )
{
    int sw = style().pixelMetric(QStyle::PM_SplitterWidth, this);
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->isSplitter )
	    s->sizer = sw;
	s = data->list.next();
    }
    doResize();
    QFrame::styleChange( old );
}

#endif
