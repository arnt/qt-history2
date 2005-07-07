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
    \brief The QDecoration class allows the appearance of the Qtopia Core Window
    Manager to be customized.

    \ingroup qws

    Qtopia Core provides window management to top level windows. The
    appearance of the borders and buttons (the decoration) around the
    managed windows can be customized by creating your own class
    derived from QDecoration and overriding a few methods.

    This class is non-portable. It is available \e only in Qtopia Core.

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

    \value All The entire region used by the window decoration.

    \value Top    The top border is used to vertically resize the window.
    \value Bottom The bottom border is used to vertically resize the window.
    \value Left   The left border is used to horizontally resize the window.
    \value Right  The right border is used to horizontally resize the window.
    \value TopLeft    The top-left corner of the window is used to resize the
                      window both horizontally and vertically.
    \value TopRight   The top-right corner of the window is used to resize the
                      window both horizontally and vertically.
    \value BottomLeft The bottom-left corner of the window is used to resize the
                      window both horizontally and vertically.
    \value BottomRight The bottom-right corner of the window is used to resize the
                      window both horizontally and vertically.
    \value Borders    All of the regions used to describe the window border.

    \value Title    The region containing the window title that can be used
                    to move the window by dragging with the mouse cursor.
    \value Close    The region occupied by the close button. Clicking in this
                    region closes the window.
    \value Minimize The region occupied by the minimize button. Clicking in
                    this region minimizes the window.
    \value Maximize The region occupied by the maximize button. Clicking in
                    this region maximizes the window.
    \value Normalize The region occupied by a button used to restore a window's
                     normal size. Clicking in this region restores a maximized
                     window to its previous size. This region used for this
                     button is often also the maximize region.
    \value Menu     The region occupied by the window's menu button. Clicking
                    in this region opens the window operations (system) menu.
    \value Help     The region occupied by the window's help button. Clicking
                    in this region causes the context-sensitive help function
                    to be enabled.
    \value Resize   The region that the user can click to resize the window.
    \value Move     The region that the user can click to move the window.
    \omitvalue None
*/

/*!
    \enum QDecoration::DecorationState

    \value Normal
    \value Disabled
    \value Hover
    \value Pressed
*/

/*!
    \fn QRegion QDecoration::region(const QWidget *widget, int decorationRegion)

    Returns the requested region \a decorationRegion which will
    contain \a widget.
*/

/*!
    \fn void QDecoration::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                                DecorationState state)

    Override to paint the border and title decoration around \a
    widget using \a painter. \a decorationRegion is the number of the
    decoration region. \a state specifies how to render the region.
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

#ifndef QT_NO_MENU
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

/*!
    This function is called whenever an \a action in a menu is triggered
    for a top level \a widget.
*/
void QDecoration::menuTriggered(QWidget *widget, QAction *action)
{
    QDecorationAction *decAction = static_cast<QDecorationAction *>(action);
    regionClicked(widget, decAction->reg);
}
#endif // QT_NO_MENU

/*!
    This function is called whenever a region in a top level widget is
    clicked. The widget and region are specified by \a widget and \a reg.

    The default implementation performs the action for when a region is
    clicked, and it is used to handle clicks on items in the system menu.
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
#ifndef QT_NO_WHATSTHIS
            if (QWhatsThis::inWhatsThisMode())
                QWhatsThis::leaveWhatsThisMode();
            else
                QWhatsThis::enterWhatsThisMode();
#endif
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

/*!
    This function is called whenever a region in a top level widget is
    double clicked. The widget and region are specified by \a widget and
    \a reg.

    The default implementation responds to a double click on the widget's
    title, toggling its size between the maximum and its normal size.
*/
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
    Initiates move mode for the top level widget specified by \a widget
    that is TLW handled by QWSManager.
*/
void QDecoration::startMove(QWidget *widget)
{
    QWSManager *manager = widget->d_func()->topData()->qwsManager;
    if (manager)
        manager->startMove();
}

/*!
    Initiates resize mode for the top level widget specified by \a widget
    that is handled by QWSManager.
*/
void QDecoration::startResize(QWidget *widget)
{
    QWSManager *manager = widget->d_func()->topData()->qwsManager;
    if (manager)
        manager->startResize();
}

#endif
