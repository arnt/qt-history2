/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qdecoration_qws.h"

#ifndef QT_NO_QWS_MANAGER

#include "qapplication.h"
#include "qdrawutil.h"
#include "qpainter.h"
#include "qregion.h"



/*!
    \class QDecoration qdecoration_qws.h
    \brief The QDecoration class allows the appearance of the Qt/Embedded Window
    Manager to be customized.

    \ingroup qws

    Qt/Embedded provides window management to top level windows. The
    appearance of the borders and buttons (the decoration) around the
    managed windows can be customized by creating your own class
    derived from QDecoration and overriding a few methods.

    This class is non-portable. It is available \e only in Qt/Embedded.

    \sa QApplication::qwsSetDecoration()
*/

/*!
    \fn QDecoration::QDecoration()

    Constructs a decorator.
*/

/*!
    \fn QDecoration::~QDecoration()

    Destroys a decorator.
*/

/*!
    \enum QDecoration::Region

    This enum describes the regions in the window decorations.

    \value All the entire region used by the window decoration.
    \value Title displays the window title and allows the window to be
            moved by dragging.
    \value Top allows the top of the window to be resized.
    \value Bottom allows the bottom of the window to be resized.
    \value Left allows the left edge of the window to be resized.
    \value Right allows the right edge of the window to be resized.
    \value TopLeft allows the top-left of the window to be resized.
    \value TopRight allows the top-right of the window to be resized.
    \value BottomLeft allows the bottom-left of the window to be resized.
    \value BottomRight allows the bottom-right of the window to be resized.
    \value Close clicking in this region closes the window.
    \value Minimize clicking in this region minimizes the window.
    \value Maximize clicking in this region maximizes the window.
    \value Normalize returns a maximized window to its previous size.
    \value Menu clicking in this region opens the window operations
            (system) menu.
    \value Help clicking in this region provides context-sensitive help.
    \omitvalue None
    \omitvalue LastRegion
*/

/*!
    \fn QRegion QDecoration::region(const QWidget *widget, const QRect &rect, Region type)

    Returns the requested region \a type which will contain \a widget
    with geometry \a rect.
*/

/*!
    Called when the user clicks in the \c Close region.

    \a widget is the widget to be closed.

    The default behavior is to close the widget.
*/
void QDecoration::close(QWidget *widget)
{
    widget->close();
}


#include <qdialog.h>

/*

#include <qbitmap.h>

class MinimisedWindow : public QWidget
{
public:
    MinimisedWindow(QWidget *restore) :
        QWidget((QWidget *)restore->parent(), restore->windowCaption(), Qt::WStyle_Customize | Qt::WStyle_NoBorder),
        w(restore)
    {
        w->hide();
        QPixmap p("../pics/tux.png");
        setBackgroundPixmap(p);
        setFixedSize(p.size());
        setMask(p.createHeuristicMask());
        show();
    }

    void mouseDoubleClickEvent(QMouseEvent *) { w->show(); delete this; }
    void mousePressEvent(QMouseEvent *e) { clickPos = e->pos(); }
    void mouseMoveEvent(QMouseEvent *e) { move(e->globalPos() - clickPos); }

    QWidget *w;
    QPoint clickPos;
};

*/


/*!
    Called when the user clicks in the \c Minimize region.

    \a widget is the widget to be minimized.

    The default behavior is to ignore this action.
*/
void QDecoration::minimize(QWidget * /* widget */)
{
//      new MinimisedWindow(w);

    //    qDebug("No minimize functionality provided");
}


/*!
    Called when the user clicks in the \c Maximize region.

    \a widget is the widget to be maximized.

    The default behavior is to resize the widget to be full-screen.
    This method can be overridden e.g. to allow room for launch
    panels.
*/
void QDecoration::maximize(QWidget *widget)
{
    QRect nr;

    // find out how much space the decoration needs
    extern QRect qt_maxWindowRect;
    QRect desk = qt_maxWindowRect;

/*
#ifdef QPE_WM_LOOK_AND_FEEL
    if (wmStyle == QtEmbedded_WMStyle) {
        QRect dummy(0, 0, desk.width(), 1);
        QRegion r = region(widget, dummy, Title);
        QRect rect = r.boundingRect();
        nr = QRect(desk.x(), desk.y()-rect.y(),
            desk.width(), desk.height() - rect.height());
    } else
#endif
*/
    {
        QRect dummy(0, 0, 1, 1);
        QRegion r = region(widget, dummy);
        if (r.isEmpty()) {
            nr = desk;
        } else {
            QRect rect = r.boundingRect();
            nr = QRect(desk.x()-rect.x(), desk.y()-rect.y(),
                    desk.width() - (rect.width()==1 ? 0 : rect.width()), // ==1 -> dummy
                    desk.height() - (rect.height()==1 ? 0 : rect.height()));
        }
    }
    widget->setGeometry(nr);
}

/*!
    \fn void QDecoration::paint(QPainter *painter, const QWidget *widget)

    Override to paint the border and title decoration around \a widget
    using \a painter.
*/

/*!
    \fn void QDecoration::paintButton(QPainter *painter, const QWidget *widget, Region type, int state)

    Override to paint a button of type \a type using \a painter.

    \a widget is the widget whose button is to be drawn.
    \a state is the state of the button. It can be a combination of the
    following OR-ed together:
    \list
    \i \c QWSButton::MouseOver
    \i \c QWSButton::Clicked
    \i \c QWSButton::On
    \endlist
*/

/*!
    Returns first region which contains \a point.
    If none of the regions contain the point it returns None.
*/
int QDecoration::regionAt(const QWidget *w, const QPoint &point)
{
    int regions[] = {
        TopLeft, Top, TopRight, Left, Right, BottomLeft, Bottom, BottomRight, // Borders first
        Menu, Title, Help, Minimize, Maximize, Close,                         // then buttons
        None
    };

//     char *regions_str[] = {
//         "TopLeft", "Top", "TopRight", "Left", "Right", "BottomLeft", "Bottom", "BottomRight",
//         "Menu", "Title", "Help", "Minimize", "Maximize", "Close",
//         "None"
//     };

    // First check to see if within all regions at all
    QRegion reg = region(w, w->geometry(), All);
    if (!reg.contains(point)) {
        return None;
    }

    int i = 0;
    while (regions[i]) {
        reg = region(w, w->geometry(), regions[i]);
        if (reg.contains(point)) {
//            qDebug("In region %s", regions_str[i]);
            return regions[i];
        }
        ++i;
    }
    return None;
}

#endif
