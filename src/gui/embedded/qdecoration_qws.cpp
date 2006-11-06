/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdecoration_qws.h"

#include "qapplication.h"
#include "qdrawutil.h"
#include "qpainter.h"
#include "qregion.h"
#include "qwhatsthis.h"

#include "qmenu.h"
#include "private/qwidget_p.h"
#include "qwsmanager_qws.h"


/*!
    \class QDecoration
    \ingroup qws

    \brief The QDecoration class allows the appearance of top level
    windows to be customized.

    Note that this class is non-portable, and that it is only
    available in \l {Qtopia Core}.

    \l {Qtopia Core} provides window management of top level
    windows. The appearance of the borders and buttons (the
    decoration) around these windows can be customized by deriving
    from the QDecoration class. Custom decorations can be added by
    subclassing the QDecorationPlugin class, using the
    QDecorationFactory class to dynamically load the decoration into
    the application. To actually apply a decoration to an
    application, use the QApplication::qwsSetDecoration() function.

    The QDecoration class provides the virtual paint() function that
    can be reimplemented to paint the border and title decoration
    around a specified widget using a given painter and decoration
    state.

    The DecorationRegion enum is used to specify which region to
    paint. This enum describes the various regions of the window
    decoration. The region() function is used to retrieve the actual
    regions the decoration occupy. It is also possible to determine
    the type of region containing a given point, using the regionAt()
    function. The DecorationState enum is used to specify a region's
    state, e.g. whether it is active or not.

    QDecoration also provides the possibility of building the system
    menu for a given top level widget with its virtual buildSysMenu()
    function. The menuTriggered() function is called whenever an
    action in such a widget's menu is triggered.

    The virtual regionClicked() and regionDoubleClicked() functions
    are provided to enable response to mouse clicks. The default
    implementations responds to (single) clicks on items in a widget's
    system menu and double clicks on a widget's title.

    Finally, the QDecoration class provides a couple of static
    functions, startMove() and startResize(), which start the move or
    resize action by making the appropriate decoration region active
    and grabbing the mouse input.

    \sa QDecorationFactory, QDecorationPlugin, QDirectPainter
*/

/*!
    \fn QDecoration::QDecoration()

    Constructs a decoration object.
*/

/*!
    \fn QDecoration::~QDecoration()

    Destroys this decoration object.
*/

/*!
    \enum QDecoration::DecorationRegion

    This enum describes the various regions of the window decoration.

    \value All The entire region used by the window decoration.

    \value Top    The top border used to vertically resize the window.
    \value Bottom The bottom border used to vertically resize the window.
    \value Left   The left border used to horizontally resize the window.
    \value Right  The right border used to horizontally resize the window.
    \value TopLeft    The top-left corner of the window used to resize the
                      window both horizontally and vertically.
    \value TopRight   The top-right corner of the window used to resize the
                      window both horizontally and vertically.
    \value BottomLeft The bottom-left corner of the window used to resize the
                      window both horizontally and vertically.
    \value BottomRight The bottom-right corner of the window used to resize the
                      window both horizontally and vertically.
    \value Borders    All the regions used to describe the window border.

    \value Title    The region containing the window title, used
                    to move the window by dragging with the mouse cursor.
    \value Close    The region occupied by the close button. Clicking in this
                    region closes the window.
    \value Minimize The region occupied by the minimize button. Clicking in
                    this region minimizes the window.
    \value Maximize The region occupied by the maximize button. Clicking in
                    this region maximizes the window.
    \value Normalize The region occupied by a button used to restore a window's
                     normal size. Clicking in this region restores a maximized
                     window to its previous size. The region used for this
                     button is often also the Maximize region.
    \value Menu     The region occupied by the window's menu button. Clicking
                    in this region opens the window operations (system) menu.
    \value Help     The region occupied by the window's help button. Clicking
                    in this region causes the context-sensitive help function
                    to be enabled.
    \value Resize   The region used to resize the window.
    \value Move     The region used to move the window.
    \value None      No region.

    \sa region(), regionAt(), DecorationState
*/

/*!
    \enum QDecoration::DecorationState

    This enum describes the various states of a decoration region.

    \value Normal The region is active
    \value Disabled The region is inactive.
    \value Hover The cursor is hovering over the region.
    \value Pressed The region is pressed.

    \sa paint(), DecorationRegion
*/

/*!
    \fn QRegion QDecoration::region(const QWidget *widget, const QRect & rectangle, int decorationRegion)

    Returns the region specified by \a decorationRegion, for the given
    top level \a widget.

    The \a rectangle parameter specifies the rectangle the decoration
    is wrapped around. The \a decorationRegion is a bitmask of the
    values described by the DecorationRegion enum.

    \sa regionAt()
*/

/*!
    \fn QRegion QDecoration::region(const QWidget *widget, int decorationRegion)
    \overload

    Returns the region specified by \a decorationRegion containing the
    given \a widget. The \a decorationRegion is a bitmask of the
    values described by the DecorationRegion enum.
*/

/*!
    \fn void QDecoration::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                                DecorationState state)

    This virtual function allows subclasses of QDecoration to paint
    the border and title decoration for the specified top level \a
    widget using the given \a painter and \a state.

    The specified \a decorationRegion is a bitmask of the values
    described by the DecorationRegion enum.
*/

/*!
    \fn int QDecoration::regionAt(const QWidget *widget, const QPoint &point)

    Returns the type of the first region of the specified top level \a
    widget containing the given \a point.

    The return value is one of the DecorationRegion enum's values. Use
    the region() function to retrieve the actual region. If none of
    the widget's regions contain the point, this function returns \l
    None.

    \sa region()
*/
int QDecoration::regionAt(const QWidget *w, const QPoint &point)
{
    int regions[] = {
        TopLeft, Top, TopRight, Left, Right, BottomLeft, Bottom, BottomRight, // Borders first
        Menu, Title, Help, Minimize, Normalize, Maximize, Close,                         // then buttons
        None
    };

//     char *regions_str[] = {
//         "TopLeft", "Top", "TopRight", "Left", "Right", "BottomLeft", "Bottom", "BottomRight",
//         "Menu", "Title", "Help", "Minimize", "Normalize", "Maximize", "Close",
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
    Builds the system \a menu for the given top level \a widget.

    \sa menuTriggered()
*/
void QDecoration::buildSysMenu(QWidget *widget, QMenu *menu)
{
    QDecorationAction *act = new QDecorationAction(QLatin1String("Restore"),
                                                   menu, Maximize);
    act->setEnabled(widget->windowState() & Qt::WindowMaximized);
    menu->addAction(act);
    menu->addAction(new QDecorationAction(QLatin1String("Move"), menu, Move));
    menu->addAction(new QDecorationAction(QLatin1String("Size"), menu, Resize));
    act = new QDecorationAction(QLatin1String("Minimize"), menu, Minimize);
    menu->addAction(act);
    act = new QDecorationAction(QLatin1String("Maximize"), menu, Maximize);
    act->setDisabled(widget->windowState() & Qt::WindowMaximized);
    menu->addAction(act);
    menu->addSeparator();
    menu->addAction(new QDecorationAction(QLatin1String("Close"), menu, Close));
}

/*!
    This function is called whenever an action in a top level widget's
    menu is triggered. Pointers to the \a widget and \a action are
    passed as arguments.

    \sa buildSysMenu()
*/
void QDecoration::menuTriggered(QWidget *widget, QAction *action)
{
    QDecorationAction *decAction = static_cast<QDecorationAction *>(action);
    regionClicked(widget, decAction->reg);
}
#endif // QT_NO_MENU

/*!
    \fn void QDecoration::regionClicked(QWidget *widget, int region)

    This function is called whenever a region in a top level widget is
    clicked. The parameters specifies the \a widget as well as the \a
    region.  Note that the \a region parameter is one of the
    DecorationRegion values.

    The default implementation responds to clicks on items in the
    system menu, performing the requested actions.

    \sa regionDoubleClicked()
*/
void QDecoration::regionClicked(QWidget *widget, int reg)
{
    switch(reg) {
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
    case Normalize:
        widget->showNormal();
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
    \fn void QDecoration::regionDoubleClicked(QWidget *widget, int region)

    This function is called whenever a region in a top level widget is
    double clicked. The parameters specifies the \a widget as well as
    the \a region. Note that the \a region parameter is one of the
    DecorationRegion values.

    The default implementation responds to a double click on the widget's
    title, toggling its size between the maximum and its normal size.

    \sa regionClicked()
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
    Starts to move the given \a widget by making its \l Title region
    active and grabbing the mouse input. The \a widget must be a top
    level widget.

    \sa DecorationRegion
*/
void QDecoration::startMove(QWidget *widget)
{
#ifdef QT_NO_QWS_MANAGER
    Q_UNUSED(widget);
#else
    QWSManager *manager = widget->d_func()->topData()->qwsManager;
    if (manager)
        manager->startMove();
#endif
}

/*!
    Starts to resize the given \a widget by making its \l BottomRight
    region active and grabbing the mouse input. The \a widget must be
    a top level widget.

    \sa DecorationRegion
*/
void QDecoration::startResize(QWidget *widget)
{
#ifdef QT_NO_QWS_MANAGER
    Q_UNUSED(widget);
#else
    QWSManager *manager = widget->d_func()->topData()->qwsManager;
    if (manager)
        manager->startResize();
#endif
}

