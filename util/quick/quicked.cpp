#include "quicked.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qframe.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qobjcoll.h>
#include <stdlib.h>
#include <qscrollview.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qptrdict.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfile.h>

DesignerInfo::DesignerInfo( QWidget* w, bool t ) :
    widget(w), top(t)
{
}


// Ugly-hack (similar to an ugle-hack) to call QWidget::paintEvent
// without going via event filters, needed because we want to call it
// from such a filter! 
class QDWidget : public QWidget {
public:
    void directPaintEvent(QPaintEvent* e)
    {
        paintEvent(e);
    }
};

static const int HL_SIZE = 4;       // Thickness of highlight
static const int CNR_SIZE = 10;     // Maximum size of selection corners

/*!
  \class QuickEditedWidget quicked.h
  \brief An editable widget.

  By \link monitor() monitoring\endlink sub-widgets of this, those
  sub-widgets are flagged for editing.  The widgets monitored may be
  composed of other widgets, which will not be editable unless they are
  also explicitly monitored.  
*/ 

/*!
  Construct a QuickEditedWidget with an empty selection.  Initially
  it only monitors itself.

  \link QApplication::setGlobalMouseTracking() Global mouse tracking\endlink
  is enabled for the life of the widget.
*/
QuickEditedWidget::QuickEditedWidget(QWidget* parent, const char* name, WFlags f) :
    QWidget(parent,name,f)
{
    QApplication::setGlobalMouseTracking(TRUE);
    primary_selection = 0;
    monitor(this);
}

/*!
  Destructs the QuickEditedWidget.
*/
QuickEditedWidget::~QuickEditedWidget()
{
    QApplication::setGlobalMouseTracking(FALSE);
}

/*!
  Flags \a w to be monitored.  The widget must be a subwidget of
  this QuickEditedWidget.  \a top is for internal use and should
  always be TRUE.  The monitored widget is resized to its sizeHint(),
  in preparation for editing <b>(this may be inappropriate)</b>.
*/
void QuickEditedWidget::monitor(QWidget* w, bool top=TRUE)
{
    DesignerInfo *info = new DesignerInfo(w, top);

    if (!infoDict.find(w)) // may be already monitored
	infoDict.insert(w,info);

    if (top && w->sizeHint().isValid())
	w->resize(w->sizeHint());
    w->installEventFilter(this);
    if ( w->children() ) {
	QObjectListIt it(*w->children());
	QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( obj->isWidgetType() ) {
		QWidget *child = ((QWidget*)obj);
		monitor( child, FALSE );
	    }
	}
    }
}

/*!
  Begins manipulating \a decendant as the primary selection.  The widget
  must already have been monitored.
  \a child_pos is the position within \a decendant which started the
  manipulation (it is in the coordinate space of \a decendant).
*/
void QuickEditedWidget::startManip( QWidget* decendant, QPoint child_pos )
{
    stopManip();
    drag_last_pos = decendant->mapToGlobal( child_pos );
    manip_pos = nearCorner( decendant, child_pos );
    if (this != decendant || manip_pos == BottomRight) {
	primary_selection = decendant;
    }
    QRect dirty = mapToMe( decendant, decendant->rect() );
    repaintWithChildren( dirty.left()-HL_SIZE, dirty.top()-HL_SIZE,
		         dirty.width()+HL_SIZE*2, dirty.height()+HL_SIZE*2 );
}

/*!
  Stops manipulating the primary selection.  The primary selection
  becomes 0.
*/
void QuickEditedWidget::stopManip()
{
    if ( primary_selection ) {
	QRect dirty = mapToMe( primary_selection, primary_selection->rect() );
	primary_selection = 0;
	repaintWithChildren( dirty.left()-HL_SIZE, dirty.top()-HL_SIZE,
			 dirty.width()+HL_SIZE*2, dirty.height()+HL_SIZE*2 );
    }
}

/*!
  Returns \a r with corner \c manipulated by a displacement of \a delta.
*/
QRect QuickEditedWidget::manipRect( QRect r, Corner c, QPoint delta )
{
    switch ( c ) {
      case NoCorner:
	r.moveBy( delta.x(), delta.y() );
	break;
      case TopLeft:
      case TopRight:
	r.setTop( r.top() + delta.y() );
	break;
      case BottomLeft:
      case BottomRight:
	r.setBottom( r.bottom() + delta.y() );
	break;
    }
    switch ( c ) {
      case NoCorner:
	break;
      case TopLeft:
      case BottomLeft:
	r.setLeft( r.left() + delta.x() );
	break;
      case TopRight:
      case BottomRight:
	r.setRight( r.right() + delta.x() );
	break;
    }

    return r;
}

/*!
  If there is a primary selection, this function manipulates all widgets
  in the selection according to \a newpos and the drag_last_pos.
  drag_last_pos is changed to newpos.
*/
void QuickEditedWidget::manip(QPoint newpos)
{
    if ( primary_selection ) {
	emit changed();

	QWidget* w;
	for (QListIterator<QWidget> i(selection); (w=i.current()); ++i) {
	    if ( w!=this || selection.count()==1 ) {
		QRect dirty = mapToMe( w, w->rect() );

		QRect mwg = 
		    manipRect( w->geometry(),
				      manip_pos, newpos - drag_last_pos );
		w->setGeometry( mwg );

		dirty = dirty.unite(mapToMe( w, w->rect() ));
		dirty.setLeft( dirty.left()-HL_SIZE );
		dirty.setTop( dirty.top()-HL_SIZE );
		dirty.setRight( dirty.right()+HL_SIZE );
		dirty.setBottom( dirty.bottom()+HL_SIZE );

		repaintWithChildren( dirty );
	    }
	}

	drag_last_pos = newpos;
    }
}

/*!
  Deselects everything.
*/
void QuickEditedWidget::clearSelection()
{
    QList<QWidget> t = selection;
    selection.clear();

    for (QWidget* w = t.first(); w; w = t.next()) {
	QPoint tl = mapToMe( w, QPoint(0,0) );
	for (Corner c=BottomLeft; c!=NoCorner; c=Corner(c-1)) {
	    QRect ca = cornerRect(w,c);
	    ca.moveBy(tl.x(),tl.y());
	    repaintWithChildren(ca);
	}
    }

    primary_selection = 0;
}

/*!
  Adds \a w to the selection, repainting as required.
*/
void QuickEditedWidget::select(QWidget *w)
{
    selection.append(w);

    QPoint tl = mapToMe( w, QPoint(0,0) );
    for (Corner c=BottomLeft; c!=NoCorner; c=Corner(c-1)) {
	QRect ca = cornerRect(w,c);
	ca.moveTopLeft(tl);
	repaintWithChildren(ca);
    }
}

/*!
  Returns a widget from the selection for which \a mepos is
  a coordinate in a corner of the widget.  \a mepos is in
  the coordinate space of this QuickEditedWidget.
*/
QWidget* QuickEditedWidget::selectionCorner( QPoint mepos )
{
    for (QWidget* w = selection.first(); w; w = selection.next()) {
	QPoint tl = mapToMe( w, QPoint(0,0) );
	for (Corner c=BottomLeft; c!=NoCorner; c=Corner(c-1)) {
	    QRect ca = cornerRect(w,c);
	    ca.moveBy(tl.x(),tl.y());
	    if (ca.contains(mepos))
		return w;
	}
    }
    return 0;
}

/*!
  Repaints all children of \a parent, clipped to \a r in the coordinate
  space of \a parent.
*/
void QuickEditedWidget::repaintChildren( QWidget* parent, const QRect& r )
{
    if ( parent->children() ) {
	QObjectListIt it(*parent->children());
	QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( obj->isWidgetType() ) {
		QWidget *child = ((QWidget*)obj);
		if ( child->geometry().intersects( r ) ) {
		    QRect cr = r;
		    cr.moveBy( -child->x(), -child->y() );
		    repaintChildren( child, cr );
		}
	    }
	}
    }
    parent->repaint( r.x(), r.y(), r.width(), r.height() );
}

/*!
  Repaints this widget, <em>and any children</em> clipped to \a cliprect.
  The overlay is also painted over the top of all other painting.
*/
void QuickEditedWidget::repaintWithChildren( const QRect& cliprect )
{
    repaintChildren( this, cliprect );
    paintOverlay( cliprect );
}

/*!
  \overload
*/
void QuickEditedWidget::repaintWithChildren( int x, int y, int w, int h )
{
    repaintWithChildren( QRect(x, y, w, h) );
}

/*!
  Paints the overlay, clipped to \a cliprect.
  The overlay highlights the selection, with additional
  highlighting for the primary selection.
  Note that the overlay is not clipped by child widgets,
  and so can contain arbitrary construction graphics.
*/
void QuickEditedWidget::paintOverlay( const QRect& cliprect )
{
    if ( primary_selection || !selection.isEmpty()) {
	bool s = testWFlags(WPaintUnclipped);
	if (!s) setWFlags(WPaintUnclipped);
	QPainter p;
	p.begin(this);
	p.setClipRect( cliprect );  // Cannot do this if children don't clip
	if ( primary_selection ) {
	    QRect wrect = mapToMe( primary_selection, primary_selection->rect() );
	    p.setPen(green);
	    p.setBrush(NoBrush);
	    for (int i=1; i<=HL_SIZE; i++)
		p.drawRect( wrect.left()-i, wrect.top()-i,
			    wrect.width()+2*i, wrect.height()+2*i );
	}
	if ( !selection.isEmpty() ) {
	    p.setPen(NoPen);
	    p.setBrush(red);
	    for (QWidget* w = selection.first(); w; w = selection.next()) {
		QPoint tl = mapToMe( w, QPoint(0,0) );
		if ( w == this ) {
		    QPointArray cp = cornerPolygon(w,BottomRight);
		    cp.translate(tl.x(),tl.y());
		    p.drawPolygon(cp);
		} else {
		    for (Corner c=BottomLeft; c!=NoCorner; c=Corner(c-1)) {
			QPointArray cp = cornerPolygon(w,c);
			cp.translate(tl.x(),tl.y());
			p.drawPolygon(cp);
		    }
		}
	    }
	}
	p.end();
	if (!s) clearWFlags(WPaintUnclipped);
    }
}

/*!
  Maps \a p from the coordinate space of \a decendant
  to the coordinate space of \a ancestor.
*/
QPoint QuickEditedWidget::mapTo( QWidget* ancestor, QWidget* decendant, QPoint p )
{
    while ( decendant != ancestor )
    {
#if defined(CHECK_RANGE)
	if ( !decendant ) {
	    fatal( "QuickEditedWidget::mapTo - "
		   " ancestor is not ancestor of decendant" );
	}
#endif
	p = decendant->mapToParent( p );
	decendant = decendant->parentWidget();
    }
    return p;
}

/*!
  Maps \a p from the coordinate space of \a decendant
  to the coordinate space of this QuickEditedWidget.
*/
QPoint QuickEditedWidget::mapToMe( QWidget* decendant, QPoint p )
{
    return mapTo( this, decendant, p );
}

/*!
  Maps \a r from the coordinate space of \a decendant
  to the coordinate space of \a ancestor.
*/
QRect QuickEditedWidget::mapTo( QWidget* ancestor, QWidget* decendant, QRect r )
{
    r.moveTopLeft( mapTo( ancestor, decendant, r.topLeft() ) );
    return r;
}

/*!
  Maps \a r from the coordinate space of \a decendant
  to the coordinate space of this QuickEditedWidget.
*/
QRect QuickEditedWidget::mapToMe( QWidget* decendant, QRect r )
{
    return mapTo( this, decendant, r );
}

/*!
  Returns the Corner of \a w indicated by \a p.  \a p is in the
  coordinate-space of \a w.
*/
QuickEditedWidget::Corner QuickEditedWidget::nearCorner( QWidget* w, QPoint p )
{
    int lrmargin = QMIN( CNR_SIZE, w->width()/4+2 );
    int tbmargin = QMIN( CNR_SIZE, w->height()/4+2 );

    if ( p.x() < lrmargin ) {
	if ( p.y() < tbmargin ) return TopLeft;
	if ( p.y() >= w->height()-tbmargin ) return BottomLeft;
    } else if ( p.x() >= w->width()-lrmargin ) {
	if ( p.y() < tbmargin ) return TopRight;
	if ( p.y() >= w->height()-tbmargin ) return BottomRight;
    }

    return NoCorner;
}

/*!
  Returns a polygon appropriate for graphically representing the
  given corner of the given widget.  The returned QRect is in the
  coordinate-space of \a w.

  \sa cornerRect()
*/
QPointArray QuickEditedWidget::cornerPolygon( QWidget* w, Corner c )
{
    QRect r = cornerRect(w,c);
    QPointArray result(3);

    switch (c) {
      case NoCorner:
	result = r;
	break;
      case TopLeft:
	result.setPoint(0,r.topLeft());
	result.setPoint(1,r.topRight());
	result.setPoint(2,r.bottomLeft());
	break;
      case TopRight:
	result.setPoint(0,r.topLeft());
	result.setPoint(1,r.topRight());
	result.setPoint(2,r.bottomRight());
	break;
      case BottomLeft:
	result.setPoint(0,r.topLeft());
	result.setPoint(1,r.bottomRight());
	result.setPoint(2,r.bottomLeft());
	break;
      case BottomRight:
	result.setPoint(0,r.bottomRight());
	result.setPoint(1,r.topRight());
	result.setPoint(2,r.bottomLeft());
    }

    return result;
}

/*!
  Returns a rectangle equal to the 
  given corner of the given widget.  The returned QRect is in the
  coordinate-space of \a w.

  \sa cornerPolygon()
*/
QRect QuickEditedWidget::cornerRect( QWidget* w, Corner c )
{
    int lrmargin = QMIN( CNR_SIZE, w->width()/4+2 );
    int tbmargin = QMIN( CNR_SIZE, w->height()/4+2 );

    switch (c) {
      case NoCorner:
	break;
      case TopLeft:
	return QRect(0,0,lrmargin,tbmargin);
      case TopRight:
	return QRect(w->width()-lrmargin,0,lrmargin,tbmargin);
      case BottomLeft:
	return QRect(0,w->height()-tbmargin,lrmargin,tbmargin);
      case BottomRight:
	return QRect(w->width()-lrmargin,w->height()-tbmargin,lrmargin,tbmargin);
    }

    return QRect(0,0,-1,-1);
}

/*!
  Returns the top-monitored widget that is an ancestor of \a decendant.
  A top-monitored widget is a \e w which was monitored via
  \link monitor() monitor(w, TRUE)\endlink.
*/
QWidget* QuickEditedWidget::top(QWidget* decendant)
{
    DesignerInfo* info = infoDict.find(decendant);
    while (!info->top) {
	decendant = decendant->parentWidget();
	info = infoDict.find(decendant);
    }
    return decendant;
}

/*!
  Returns the widget under \a root which is \e visually the parent
  of a widget with area \a wrect.  That is, the logically-lowest widget
  that completely encloses
  \a wrect.  In searching, the widget \a exclude is ignored.
  If more than one such widget exists, the uppermost one is returned.
  \a wrect is relative to \a root.
*/
QWidget* QuickEditedWidget::visualParent(QWidget* root, QRect wrect, QWidget* exclude)
{
    QWidget* result = root;
    if ( root->children() ) {
	QObjectListIt it(*root->children());
	QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( obj->isWidgetType() && obj != exclude ) {
		QWidget *child = ((QWidget*)obj);
		if (child->geometry().contains(wrect)) {
		    QRect r = wrect; r.moveBy( -child->x(), -child->y() );
		    result = visualParent(child, r, exclude);
		}
	    }
	}
    }
    return result;
}

/*!
  Filter events to all \link monitor() monitored\endlink widgets,
  including this QuickEditedWidget.  This is the central controller
  of editing.
*/
bool QuickEditedWidget::eventFilter(QObject* receiver, QEvent* event)
{
    QWidget *w = (QWidget*)receiver;

    DesignerInfo* info = infoDict.find(w);

    ASSERT( info );

    switch ( event->type() ) {
      case Event_Paint:
	((QDWidget*)w)->directPaintEvent((QPaintEvent*)event);
	paintOverlay( mapToMe( w, ((QPaintEvent*)event)->rect() ) );
	return TRUE;
	break;
      case Event_MouseButtonDblClick:
      case Event_MouseButtonPress:
      case Event_MouseButtonRelease:
      case Event_MouseMove:
	return mouseEventFilter((QMouseEvent*)event, w);
    }

    return FALSE;
}

/*!
  A sub-task of eventFilter() which deals with mouse event \e sent
  to widgets \a w.
*/
bool QuickEditedWidget::mouseEventFilter( QMouseEvent* e, QWidget* w )
{
    switch (e->type()) {
      case Event_MouseButtonDblClick:
	return TRUE;
      case Event_MouseButtonPress:
	{
	    QWidget* t = top(w);
	    QPoint t_pos = mapTo(t,w,e->pos());
	    QPoint me_pos = mapToMe(t,t_pos);
	    QWidget* s;
	    if ( (s = selectionCorner( me_pos )) ) {
		t = s;
		t_pos = me_pos - mapToMe(s,QPoint(0,0));
	    } else if ( !selection.contains(t) ) {
		if ( !selection.isEmpty()
		  && !(e->state()&ControlButton) )
		    clearSelection();
		select(t);
	    }
	    startManip( t, t_pos );
	}
	return TRUE;
      case Event_MouseButtonRelease:
	if ( primary_selection ) {
	    QWidget *visual_parent =
		visualParent(this,
		    mapToMe(primary_selection, primary_selection->rect()),
		    primary_selection);
	    if (visual_parent != primary_selection->parentWidget()
	    && 0!=strcmp("qt_viewport", primary_selection->parentWidget()->name()))
	    {
		if (QMessageBox::warning(this, "Reparenting",
		    "The widget is now visually in\n"
		    "a different parent. Should the\n"
		    "structure be changed accordingly?",
		    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No ) == QMessageBox::Yes)
		{
		    primary_selection->recreate(
			visual_parent,
			0,
			mapToMe( primary_selection, QPoint(0,0) )
			-mapToMe( visual_parent, QPoint(0,0) ),
			TRUE
		    );
		}
	    }
	}
	stopManip();
	return TRUE;
      case Event_MouseMove:
	manip( QCursor::pos() );
	return TRUE;
    }

    return FALSE;
}

/*!
  \fn void QuickEditedWidget::changed()

  This signal is emitted every time the widget changes.
*/
