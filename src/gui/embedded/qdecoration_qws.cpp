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
#include "qwhatsthis.h"

#include "qmenu.h"
#include "private/qwidget_p.h"
#include "qwsmanager_qws.h"


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
    \enum QDecoration::DecorationRegion

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
    \fn QRegion QDecoration::region(const QWidget *widget, const QRect &rect, int type)

    Returns the requested region \a type which will contain \a widget
    with geometry \a rect.
*/

/*!
    \fn void QDecoration::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                                DecorationState state)
)

    Override to paint the border and title decoration around \a widget
    using \a painter.
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

/*!
    Builds the system \a menu for the TLW \a widget.
*/
void QDecoration::buildSysMenu(QWidget *widget, QMenu *menu)
{
    QDecorationAction *act = new QDecorationAction("Restore", menu, Maximize);
    act->setEnabled(widget->windowState() & Qt::WindowMaximized);
    menu->addAction(act);
    menu->addAction(new QDecorationAction("Move", menu, Move));
    menu->addAction(new QDecorationAction("Size", menu, Resize));
    act = new QDecorationAction("Minimize", menu, Minimize);
    menu->addAction(act);
    act = new QDecorationAction("Maximize", menu, Maximize);
    act->setDisabled(widget->windowState() & Qt::WindowMaximized);
    menu->addAction(act);
    menu->addSeparator();
    menu->addAction(new QDecorationAction("Close", menu, Close));
}

void QDecoration::menuTriggered(QWidget *widget, QAction *action)
{
    QDecorationAction *decAction = static_cast<QDecorationAction *>(action);
    regionClicked(widget, decAction->reg);
}

/*!
    Performs the action for when \a region is clicked.
    Also used for items clicked in the system menu.
*/
void QDecoration::regionClicked(QWidget *widget, int reg)
{
    switch(reg)
    {
        case Move:
            startMove(widget);
            break;
        case Resize:
            startResize(widget);
            break;
        case Help:
            if (QWhatsThis::inWhatsThisMode())
                QWhatsThis::leaveWhatsThisMode();
            else
                QWhatsThis::enterWhatsThisMode();
            break;
        case Close:
            widget->close();
            break;
        case Maximize:
            if (widget->windowState() & Qt::WindowMaximized)
                widget->showNormal();
            else
                widget->showMaximized();
            break;
    }
}

void QDecoration::regionDoubleClicked(QWidget *widget, int reg)
{
    switch(reg)
    {
        case Title: {
            if (widget->windowState() & Qt::WindowMaximized)
                widget->showNormal();
            else
                widget->showMaximized();
            break;
        }
    }
}

/*!
    Initiates move mode for the TLW handled by QWSManager.
*/
void QDecoration::startMove(QWidget *widget)
{
    QWSManager *manager = widget->d_func()->topData()->qwsManager;
    if (manager)
        manager->startMove();
}

/*!
    Initiates resize mode for the TLW handled by QWSManager.
*/
void QDecoration::startResize(QWidget *widget)
{
    QWSManager *manager = widget->d_func()->topData()->qwsManager;
    if (manager)
        manager->startResize();
}

#endif
