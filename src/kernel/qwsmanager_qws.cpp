/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_qws.cpp#8 $
**
** Implementation of Qt/Embedded window manager
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qwsmanager_qws.h"
#include "qcursor.h"

#ifndef QT_NO_QWS_MANAGER

#include "qdrawutil.h"
#include "qapplication.h"
#include "qaccel.h"
#include "qstyle.h"
#include "qwidget.h"
#include "qpopupmenu.h"
#include "qpainter.h"
#include "qregion.h"
#include "qevent.h"
#include "qcursor.h"
#include "qgfx_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsregionmanager_qws.h"
#include "qwsdefaultdecoration_qws.h"


enum WMStyle {
    Default_WMStyle = 1, /* Starting at zero stuffs up menus */
    KDE_WMStyle,
    KDE2_WMStyle,
    BeOS_WMStyle,
    Windows_WMStyle,
    Hydro_WMStyle,
};


#ifndef QT_NO_QWS_WINDOWS_WM_STYLE
#include "qwswindowsdecoration_qws.h"
QWSDefaultDecoration *new_Windows_WMDecorations() { return new QWSWindowsDecoration(); }
#endif // QT_NO_QWS_WINDOWS_WM_STYLE
#ifndef QT_NO_QWS_KDE_WM_STYLE
#include "qwskdedecoration_qws.h"
QWSDefaultDecoration *new_KDE_WMDecorations() { return new QWSKDEDecoration(); }
#endif // QT_NO_QWS_KDE_WM_STYLE
#ifndef QT_NO_QWS_KDE2_WM_STYLE
#include "qwskde2decoration_qws.h"
QWSDefaultDecoration *new_KDE2_WMDecorations() { return new QWSKDE2Decoration(); }
#endif // QT_NO_QWS_KDE2_WM_STYLE
#ifndef QT_NO_QWS_BEOS_WM_STYLE
#include "qwsbeosdecoration_qws.h"
QWSDefaultDecoration *new_BeOS_WMDecorations() { return new QWSBeOSDecoration(); }
#endif // QT_NO_QWS_BEOS_WM_STYLE
#ifndef QT_NO_QWS_HYDRO_WM_STYLE
#include "qwshydrodecoration_qws.h"
QWSDefaultDecoration *new_Hydro_WMDecorations() { return new QWSHydroDecoration(); }
#endif // QT_NO_QWS_HYDRO_WM_STYLE

#include "qwsdefaultdecoration_qws.h"
QWSDefaultDecoration *new_Default_WMDecorations() { return new QWSDefaultDecoration(); }


struct WMStyleFactoryItem {
	WMStyle WMStyleType;
	QString WMStyleName;
	QWSDefaultDecoration *(*new_WMDecorations)();
} WMStyleList[] = {
#ifndef QT_NO_QWS_WINDOWS_WM_STYLE
    { Windows_WMStyle, "Windows", new_Windows_WMDecorations },
#endif // QT_NO_QWS_WINDOWS_WM_STYLE
#ifndef QT_NO_QWS_KDE_WM_STYLE
    { KDE_WMStyle, "KDE", new_KDE_WMDecorations },
#endif // QT_NO_QWS_KDE_WM_STYLE
#ifndef QT_NO_QWS_KDE2_WM_STYLE
    { KDE2_WMStyle, "KDE2", new_KDE2_WMDecorations },
#endif // QT_NO_QWS_KDE2_WM_STYLE
#ifndef QT_NO_QWS_BEOS_WM_STYLE
    { BeOS_WMStyle, "BeOS", new_BeOS_WMDecorations },
#endif // QT_NO_QWS_BEOS_WM_STYLE
#ifndef QT_NO_QWS_HYDRO_WM_STYLE
    { Hydro_WMStyle, "Hydro", new_Hydro_WMDecorations },
#endif // QT_NO_QWS_HYDRO_WM_STYLE

    { Default_WMStyle, "Default", new_Default_WMDecorations },
    { Default_WMStyle, NULL, NULL }
};


QWSDecoration *QWSManager::newDefaultDecoration()
{
    return new QWSDefaultDecoration;
}


/*!
  \class QWSDecoration qwsmanager_qws.h
  \brief The QWSDecoration class allows the appearance of the Qt/Embedded Window
  Manager to be customized.

  Qt/Embedded provides window management to top level windows.  The
  appearance of the borders and buttons (the decoration) around the
  managed windows can be customized by creating your own class derived
  from QWSDecoration and overriding a few methods.

  This class is non-portable.  It is available \e only in Qt/Embedded.

  \sa QApplication::qwsSetDecoration()
*/

/*!
  \fn QWSDecoration::QWSDecoration()

  Constructs a decorator.
*/

/*!
  \fn QWSDecoration::~QWSDecoration()

  Destructs a decorator.
*/

/*!
  \enum QWSDecoration::Region

  This enum describes the regions in the window decorations.

  <ul>
  <li> \c None - used internally.
  <li> \c All - the entire region used by the window decoration.
  <li> \c Title - Displays the window title and allows the window to be
	  moved by dragging.
  <li> \c Top - allows the top of the window to be resized.
  <li> \c Bottom - allows the bottom of the window to be resized.
  <li> \c Left - allows the left edge of the window to be resized.
  <li> \c Right - allows the right edge of the window to be resized.
  <li> \c TopLeft - allows the top-left of the window to be resized.
  <li> \c TopRight - allows the top-right of the window to be resized.
  <li> \c BottomLeft - allows the bottom-left of the window to be resized.
  <li> \c BottomRight - allows the bottom-right of the window to be resized.
  <li> \c Close - clicking in this region closes the window.
  <li> \c Minimize - clicking in this region minimizes the window.
  <li> \c Maximize - clicking in this region maximizes the window.
  <li> \c Normalize - returns a maximized window to previous size.
  <li> \c Menu - clicking in this region opens the window operations menu.
  </ul>
*/

/*!
  \fn QRegion QWSDecoration::region( const QWidget *widget, const QRect &rect, Region type )

  Returns the requested region \a type which will contain \a widget
  with geometry \a rect.
*/

/*!
  Called when the user clicks in the \c Close region.

  \a widget is the QWidget to be closed.

  The default behaviour is to close the widget.
*/
void QWSDecoration::close( QWidget *widget )
{
    widget->close(FALSE);
}


#include <qdialog.h>

/*

#include <qbitmap.h>

class MinimisedWindow : public QWidget
{
public:
    MinimisedWindow( QWidget *restore ) : 
	QWidget( (QWidget *)restore->parent(), restore->caption(), WStyle_Customize | WStyle_NoBorder ),
	w(restore)
    {
	w->hide();
	QPixmap p( "../pics/tux.png" );
	setBackgroundPixmap( p );
	setFixedSize( p.size() );
	setMask( p.createHeuristicMask() );
	show();
    }
 
    void mouseDoubleClickEvent( QMouseEvent * ) { w->show(); delete this; }
    void mousePressEvent( QMouseEvent *e ) { clickPos = e->pos(); }
    void mouseMoveEvent( QMouseEvent *e ) { move( e->globalPos() - clickPos ); }

    QWidget *w;
    QPoint clickPos;
};

*/


/*!
  Called when the user clicks in the \c Minimize region.

  \a widget is the QWidget to be minimized.

  The default behaviour is to ignore this action.
*/
void QWSDecoration::minimize( QWidget * )
{
//      new MinimisedWindow( w );
    
    //    qDebug("No minimize functionality provided");
}


/*!
  Called when the user clicks in the \c Maximize region.

  \a widget is the QWidget to be maximized.

  The default behaviour is to resize the widget to be full-screen.
  This method can be overridden to, e.g. avoid launch panels.
*/
void QWSDecoration::maximize( QWidget *widget )
{
    QRect nr;

    // find out how much space the decoration needs
    extern QRect qt_maxWindowRect;
    QRect desk = qt_maxWindowRect;

/*
#ifdef QPE_WM_LOOK_AND_FEEL
    if (wmStyle == QtEmbedded_WMStyle) {
        QRect dummy( 0, 0, desk.width(), 1 );
	QRegion r = region(widget, dummy, Title);
        QRect rect = r.boundingRect();
        nr = QRect(desk.x(), desk.y()-rect.y(),
            desk.width(), desk.height() - rect.height());
    } else
#endif
*/
    {
        QRect dummy;
        QRegion r = region(widget, dummy);
        QRect rect = r.boundingRect();
        nr = QRect(desk.x()-rect.x(), desk.y()-rect.y(),
	    desk.width() - rect.width(),
	    desk.height() - rect.height());
    }
    widget->setGeometry(nr);
}

/*!
  Called to create a QPopupMenu containing the valid menu operations.

  The default implementation adds all possible window operations.
*/

#ifndef QT_NO_POPUPMENU
QPopupMenu *QWSDecoration::menu(QWSManager *manager, const QWidget *, const QPoint &)
{
    QPopupMenu *m = new QPopupMenu();

    m->insertItem(QObject::tr("&Restore"), (int)Normalize);
    m->insertItem(QObject::tr("&Move"), (int)Title);
    m->insertItem(QObject::tr("&Size"), (int)BottomRight);
    m->insertItem(QObject::tr("Mi&nimize"), (int)Minimize);
    m->insertItem(QObject::tr("Ma&ximize"), (int)Maximize);
    m->insertSeparator();
    
    // Style Menu
    QPopupMenu *styleMenu = new QPopupMenu();
    for (int i = 0; WMStyleList[i].WMStyleName != NULL; i++)
	styleMenu->insertItem( QObject::tr(WMStyleList[i].WMStyleName), WMStyleList[i].WMStyleType );
    styleMenu->connect(styleMenu, SIGNAL(activated(int)), manager, SLOT(styleMenuActivated(int)));
    m->insertItem(QObject::tr("Style"), styleMenu);
    m->insertSeparator();

    m->insertItem(QObject::tr("Close"), (int)Close);

    return m;
}
#endif

/*!
  \fn void QWSDecoration::paint( QPainter *painter, const QWidget *widget )

  Override to paint the border and title decoration around \a widget using
  \a painter.

*/

/*!
  \fn void QWSDecoration::paintButton( QPainter *painter, const QWidget *widget, Region type, int state )

  Override to paint a button \a type using \a painter.

  \a widget is the widget whose button is to be drawn.
  \a state is the state of the button.  It can be a combination of the
  following ORed together:
  <ul>
  <li> \c QWSButton::MouseOver
  <li> \c QWSButton::Clicked
  <li> \c QWSButton::On
  </ul>
*/


QWidget *QWSManager::active = 0;
QPoint QWSManager::mousePos;

QWSManager::QWSManager(QWidget *w)
    : QObject(), activeRegion(QWSDecoration::None), managed(w), popup(0)
{
    dx = 0;
    dy = 0;

    menuBtn = new QWSButton(this, QWSDecoration::Menu);
    closeBtn = new QWSButton(this, QWSDecoration::Close);
    minimizeBtn = new QWSButton(this, QWSDecoration::Minimize);
    maximizeBtn = new QWSButton(this, QWSDecoration::Maximize, TRUE);
}

QWSManager::~QWSManager()
{
#ifndef QT_NO_POPUPMENU
    if (popup)
	delete popup;
#endif
    delete menuBtn;
    delete closeBtn;
    delete minimizeBtn;
    delete maximizeBtn;

}

QRegion QWSManager::region()
{
    return QApplication::qwsDecoration().region(managed, managed->geometry());
}

QWSDecoration::Region QWSManager::pointInRegion(const QPoint &p)
{
    QWSDecoration &dec = QApplication::qwsDecoration();
    QRect rect(managed->geometry());

    for (int i = QWSDecoration::Title; i <= QWSDecoration::LastRegion; i++) {
	if (dec.region(managed, rect, (QWSDecoration::Region)i).contains(p))
	    return (QWSDecoration::Region)i;
    }

    return QWSDecoration::None;
}

bool QWSManager::event(QEvent *e)
{
    switch (e->type()) {
	case QEvent::MouseMove:
	    mouseMoveEvent( (QMouseEvent*)e );
            break;

        case QEvent::MouseButtonPress:
            mousePressEvent( (QMouseEvent*)e );
            break;

        case QEvent::MouseButtonRelease:
            mouseReleaseEvent( (QMouseEvent*)e );
            break;

        case QEvent::MouseButtonDblClick:
            mouseDoubleClickEvent( (QMouseEvent*)e );
            break;

	case QEvent::Paint:
	    paintEvent( (QPaintEvent*)e );
            break;

	default:
	    return FALSE;
	    break;
    }

    return TRUE;
}

void QWSManager::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
	mousePos = e->globalPos();
	dx = 0;
	dy = 0;
	activeRegion = pointInRegion(mousePos);
	switch (activeRegion) {
	    case QWSDecoration::Menu:
		menu(managed->pos());
		break;
	    case QWSDecoration::Close:
		closeBtn->setClicked(TRUE);
		break;
	    case QWSDecoration::Minimize:
		minimizeBtn->setClicked(TRUE);
		break;
	    case QWSDecoration::Maximize:
		maximizeBtn->setClicked(TRUE);
		break;
	    default:
		break;
	}
	if ( activeRegion != QWSDecoration::None &&
	     activeRegion != QWSDecoration::Menu ) {
	    active = managed;
	    managed->grabMouse();
	}
	if ( activeRegion != QWSDecoration::None &&
	     activeRegion != QWSDecoration::Close &&
	     activeRegion != QWSDecoration::Minimize &&
	     activeRegion != QWSDecoration::Menu) {
	    managed->raise();
	    managed->setActiveWindow();
	}
    } else if (e->button() == Qt::RightButton) {
	menu(e->globalPos());
    }
}

void QWSManager::mouseReleaseEvent(QMouseEvent *e)
{
    managed->releaseMouse();
    if (e->button() == Qt::LeftButton) {
	handleMove();
	mousePos = e->globalPos();
	QWSDecoration::Region rgn = pointInRegion(e->globalPos());
	switch (activeRegion) {
	    case QWSDecoration::Close:
		closeBtn->setClicked(FALSE);
		if (rgn == QWSDecoration::Close) {
		    close();
		    return;
		}
		break;
	    case QWSDecoration::Minimize:
		minimizeBtn->setClicked(FALSE);
		if (rgn == QWSDecoration::Minimize)
		    minimize();
		break;
	    case QWSDecoration::Maximize:
		maximizeBtn->setClicked(FALSE);
		if (rgn == QWSDecoration::Maximize)
		    toggleMaximize();
		break;
	    default:
		break;
	}

	activeRegion = QWSDecoration::None;
    }

    if (activeRegion == QWSDecoration::None)
	active = 0;
}

void QWSManager::mouseMoveEvent(QMouseEvent *e)
{
#ifndef QT_NO_CURSOR
    static CursorShape shape[] = { ArrowCursor, ArrowCursor, ArrowCursor,
			    SizeVerCursor, SizeVerCursor, SizeHorCursor,
			    SizeHorCursor, SizeFDiagCursor, SizeBDiagCursor,
			    SizeBDiagCursor, SizeFDiagCursor, ArrowCursor,
			    ArrowCursor, ArrowCursor, ArrowCursor, ArrowCursor};

    // cursor
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();
    if (activeRegion == QWSDecoration::None)
    {
	if ( !QWidget::mouseGrabber() ) {
	    QWSDecoration::Region r = pointInRegion(e->globalPos());
	    qwsd->selectCursor(managed, shape[r]);
	}
    } else
	qwsd->selectCursor(managed, shape[activeRegion]);
#endif //QT_NO_CURSOR
    // resize/move regions
    dx = e->globalX() - mousePos.x();
    dy = e->globalY() - mousePos.y();

    handleMove();

    // button regions
    QWSDecoration::Region r = pointInRegion(e->globalPos());
    menuBtn->setMouseOver(r == QWSDecoration::Menu);
    closeBtn->setMouseOver(r == QWSDecoration::Close);
    minimizeBtn->setMouseOver(r == QWSDecoration::Minimize);
    maximizeBtn->setMouseOver(r == QWSDecoration::Maximize);
}

void QWSManager::handleMove()
{
    if (!dx && !dy)
	return;

    int x = managed->x();
    int y = managed->y();
    int w = managed->width();
    int h = managed->height();

    QRect geom(managed->geometry());

    switch (activeRegion) {
	case QWSDecoration::Title:
	    geom = QRect(x + dx, y + dy, w, h);
	    break;
	case QWSDecoration::Top:
	    geom = QRect(x, y + dy, w, h - dy);
	    break;
	case QWSDecoration::Bottom:
	    geom = QRect(x, y, w, h + dy);
	    break;
	case QWSDecoration::Left:
	    geom = QRect(x + dx, y, w - dx, h);
	    break;
	case QWSDecoration::Right:
	    geom = QRect(x, y, w + dx, h);
	    break;
	case QWSDecoration::TopRight:
	    geom = QRect(x, y + dy, w + dx, h - dy);
	    break;
	case QWSDecoration::TopLeft:
	    geom = QRect(x + dx, y + dy, w - dx, h - dy);
	    break;
	case QWSDecoration::BottomLeft:
	    geom = QRect(x + dx, y, w - dx, h + dy);
	    break;
	case QWSDecoration::BottomRight:
	    geom = QRect(x, y, w + dx, h + dy);
	    break;
	default:
	    return;
    }

    if (geom.width() >= managed->minimumWidth()
	    && geom.width() <= managed->maximumWidth()) {
	mousePos.setX(mousePos.x() + dx);
    }
    else {
	geom.setX(x);
	geom.setWidth(w);
    }
    if (geom.height() >= managed->minimumHeight()
	    && geom.height() <= managed->maximumHeight()) {
	mousePos.setY(mousePos.y() + dy);
    }
    else {
	geom.setY(y);
	geom.setHeight(h);
    }

    if (geom != managed->geometry()) {
	QApplication::sendPostedEvents();
	managed->setGeometry(geom);
    }

    dx = 0;
    dy = 0;
}

void QWSManager::paintEvent(QPaintEvent *)
{
    QWSDecoration &dec = QApplication::qwsDecoration();
    QPainter painter(managed);

    // Adjust our widget region to contain the window
    // manager decoration instead of the widget itself.
    QRegion r = managed->topData()->decor_allocated_region;
    int rgnIdx = managed->alloc_region_index;
    if ( rgnIdx >= 0 ) {
	QWSDisplay::grab();
	const int *rgnRev = qt_fbdpy->regionManager()->revision( rgnIdx );
	if ( managed->alloc_region_revision != *rgnRev ) {
	    QRegion newRegion = qt_fbdpy->regionManager()->region( rgnIdx );
	    QSize s( qt_screen->deviceWidth(), qt_screen->deviceHeight() );
	    newRegion = qt_screen->mapFromDevice( newRegion, s );
	    r &= newRegion;
	}
	painter.internalGfx()->setGlobalRegionIndex( rgnIdx );
	QWSDisplay::ungrab();
    }
    painter.internalGfx()->setWidgetRegion( r );

    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paint(&painter, managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paintButton(&painter, managed, QWSDecoration::Menu, menuBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Close, closeBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Minimize, minimizeBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Maximize, maximizeBtn->state());
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_POPUPMENU
    if (!popup) {
	popup = QApplication::qwsDecoration().menu(this, managed, managed->pos());
	connect(popup, SIGNAL(activated(int)), SLOT(menuActivated(int)));
    }
    popup->setItemEnabled(QWSDecoration::Maximize, normalSize.isNull());
    popup->setItemEnabled(QWSDecoration::Normalize, !normalSize.isNull());
    popup->popup(pos);
#endif
}

#include <qcdestyle.h>
#include <qcommonstyle.h>
#include <qcompactstyle.h>
#include <qmotifplusstyle.h>
#include <qmotifstyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>
#include <qwindowsstyle.h>

void QWSManager::styleMenuActivated(int id)
{
    for (int i = 0; WMStyleList[i].WMStyleName != NULL; i++) {
    	if (id == WMStyleList[i].WMStyleType) {
	    qApp->qwsSetDecoration( WMStyleList[i].new_WMDecorations() );
    	}
    }

    // Force a repaint of the WM regions
    const QSize s = managed->size();
    managed->resize( s.width() + 1, s.height() );
    managed->resize( s.width(), s.height() );
}

void QWSManager::menuActivated(int id)
{
    switch (id) {
	case QWSDecoration::Close:
	    close();
	    return;
	case QWSDecoration::Minimize:
	    minimize();
	    break;
	case QWSDecoration::Maximize:
	case QWSDecoration::Normalize:
	    toggleMaximize();
	    break;
	case QWSDecoration::Title:
	    mousePos = QCursor::pos();
	    activeRegion = QWSDecoration::Title;
	    active = managed;
	    managed->grabMouse();
	    break;
	case QWSDecoration::BottomRight:
	    mousePos = QCursor::pos();
	    activeRegion = QWSDecoration::BottomRight;
	    active = managed;
	    managed->grabMouse();
	    break;
	default:
	    break;
    }
}

void QWSManager::close()
{
    active = 0;
    QApplication::qwsDecoration().close(managed);
}

void QWSManager::minimize()
{
    QApplication::qwsDecoration().minimize(managed);
}


void QWSManager::maximize()
{
    QApplication::qwsDecoration().maximize(managed);
}

void QWSManager::toggleMaximize()
{
    if (normalSize.isNull()) {
	normalSize = managed->geometry();
	managed->showMaximized();
	maximizeBtn->setOn(TRUE);
    } else {
	managed->setGeometry(normalSize);
	maximizeBtn->setOn(FALSE);
	normalSize = QRect();
    }
}

/*
*/
QWSButton::QWSButton(QWSManager *m, QWSDecoration::Region t, bool tb)
    : flags(0), toggle(tb), type(t), manager(m)
{
}

void QWSButton::setMouseOver(bool m)
{
    int s = state();
    if (m) flags |= MouseOver;
    else flags &= ~MouseOver;
    if (state() != s)
	paint();
}

void QWSButton::setClicked(bool c)
{
    int s = state();
    if (c) flags |= Clicked;
    else flags &= ~Clicked;
    if (state() != s)
	paint();
}

void QWSButton::setOn(bool o)
{
    int s = state();
    if (o) flags |= On;
    else flags &= ~On;
    if (state() != s)
	paint();
}

void QWSButton::paint()
{
    QWSDecoration &dec = QApplication::qwsDecoration();
    QPainter painter(manager->widget());
    painter.setClipRegion(dec.region(manager->widget(), manager->widget()->rect()));
    dec.paintButton(&painter, manager->widget(), type, state());
}

#endif //QT_NO_QWS_MANAGER
