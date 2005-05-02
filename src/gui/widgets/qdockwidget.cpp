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

#include "qdockwidget.h"

#include <qaction.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>

#include <private/qwidgetresizehandler_p.h>

#include "qdockwidget_p.h"
#include "qdockwidgetlayout_p.h"
#include "qmainwindowlayout_p.h"


inline bool hasFeature(QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{ return (dockwidget->features() & feature) == feature; }


/*
    A Dock Window:

    [+] is the float button
    [X] is the close button

    +-------------------------------+
    | Dock Window Title       [+][X]|
    +-------------------------------+
    |                               |
    | place to put the single       |
    | QDockWidget child (this space |
    | does not yet have a name)     |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    +-------------------------------+

*/




/*
  Tool window title
*/

class QDockWidgetTitle;

class QDockWidgetTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    QDockWidgetTitleButton(QDockWidgetTitle *title);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};

class QDockWidgetTitle : public QWidget
{
    Q_OBJECT

public:
    QDockWidgetTitle(QDockWidget *tw);

    void styleChange(QStyle &);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);

    void updateButtons();
    void updateWindowTitle();

    QDockWidget *dockwidget;

    QSpacerItem *spacer;
    QDockWidgetTitleButton *floatButton;
    QDockWidgetTitleButton *closeButton;

    QBoxLayout *box;

    struct DragState {
        QRubberBand *rubberband;
        QRect origin;   // starting position
        QRect current;  // current size of the dockwidget (can be either placed or floating)
        QRect floating; // size of the floating dockwidget
        QPoint offset;
        bool canDrop;
    };
    DragState *state;

public slots:
    void toggleTopLevel();
};

QDockWidgetTitleButton::QDockWidgetTitleButton(QDockWidgetTitle *title)
    : QAbstractButton(title)
{ setFocusPolicy(Qt::NoFocus); }

QSize QDockWidgetTitleButton::sizeHint() const
{
    ensurePolished();

    int dim = 0;
    if (!icon().isNull()) {
        const QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), QIcon::Normal);
        dim = qMax(pm.width(), pm.height());
    }

    return QSize(dim + 4, dim + 4);
}

void QDockWidgetTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void QDockWidgetTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void QDockWidgetTitleButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r = rect();
    QStyleOption opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

    r.adjust(2, 2, -2, -2);
    QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), isEnabled() ? QIcon::Normal : QIcon::Disabled);
    style()->drawItemPixmap(&p, r, Qt::AlignCenter, pm);
}

QDockWidgetTitle::QDockWidgetTitle(QDockWidget *tw)
    : QWidget(tw), dockwidget(tw), floatButton(0), closeButton(0), state(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

    box = new QBoxLayout(QBoxLayout::LeftToRight, this);
    box->setMargin(style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth));
    box->setSpacing(0);
    box->addItem(spacer);

    updateButtons();
    updateWindowTitle();
}

void QDockWidgetTitle::styleChange(QStyle &)
{
    box->setMargin(style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth));

    updateWindowTitle();

    if (floatButton)
        floatButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarMaxButton));

    if (closeButton)
        closeButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
}

void QDockWidgetTitle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    // check if the tool window is movable... do nothing if it is not
    if (!::hasFeature(dockwidget, QDockWidget::DockWidgetMovable))
        return;

    QMainWindowLayout *layout =
        qobject_cast<QMainWindowLayout *>(dockwidget->parentWidget()->layout());
    if (!layout)
        return;
    layout->saveLayoutInfo();

    Q_ASSERT(!state);
    state = new DragState;

    const int screen_number = QApplication::desktop()->screenNumber(window());
    state->rubberband = new QRubberBand(QRubberBand::Rectangle,
                                        QApplication::desktop()->screen(screen_number));

    // the current location of the tool window in global coordinates
    state->origin = QRect(dockwidget->mapToGlobal(QPoint(0, 0)), dockwidget->size());
    state->current = state->origin;

    // like the above, except using the tool window's size hint
    state->floating = dockwidget->isWindow()
                      ? state->current
                      : QRect(state->current.topLeft(), dockwidget->sizeHint());

    const QPoint globalPos = event->globalPos();
    const int dl = globalPos.x() - state->current.left(),
              dr = state->current.right() - globalPos.x(),
       halfWidth = state->floating.width() / 2;
    state->offset = mapFrom(dockwidget,
                            (dl < dr)
                            ? QPoint(qMin(dl, halfWidth), 0)
                            : QPoint(state->floating.width() - qMin(dr, halfWidth), 0));
    state->offset = mapTo(dockwidget, QPoint(state->offset.x(), event->pos().y()));

    state->canDrop = true;

    state->rubberband->setGeometry(state->current);
    state->rubberband->show();
}

void QDockWidgetTitle::mouseMoveEvent(QMouseEvent *event)
{
    if (!state)
        return;

    QRect target;

    if (!(event->modifiers() & Qt::ControlModifier)) {
        // see if there is a main window under us, and ask it to place the tool window
        QWidget *widget = QApplication::widgetAt(event->globalPos());
        if (widget) {
            while (widget && !qobject_cast<QMainWindow *>(widget)) {
                if (widget->isWindow()) {
                    widget = 0;
                    break;
                }
                widget = widget->parentWidget();
            }

            if (widget) {
                QMainWindow *mainwindow = qobject_cast<QMainWindow *>(widget);
                if (mainwindow && mainwindow == dockwidget->parentWidget()) {
                    QMainWindowLayout *layout =
                        qobject_cast<QMainWindowLayout *>(dockwidget->parentWidget()->layout());
                    Q_ASSERT(layout != 0);
                    QRect request = state->origin;
                    request.moveTopLeft(event->globalPos() - state->offset);
                    target = layout->placeDockWidget(dockwidget, request, event->globalPos());
                    layout->resetLayoutInfo();
                }
            }
        }
    }

    state->canDrop = target.isValid();
    if (!state->canDrop) {
        if (hasFeature(dockwidget, QDockWidget::DockWidgetFloatable)) {
            /*
              main window refused to accept the tool window,
              recalculate absolute position as if the tool window
              was to be dropped to toplevel
            */
            target = state->floating;
            target.moveTopLeft(event->globalPos() - state->offset);
        } else {
            /*
              cannot float the window, so put it back into it's
              original position
            */
            target = state->origin;
        }
    }

    if (state->current == target)
        return;

    state->rubberband->setGeometry(target);
    state->current = target;
}

void QDockWidgetTitle::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    if (!state)
        return;

    QMainWindowLayout *layout =
        qobject_cast<QMainWindowLayout *>(dockwidget->parentWidget()->layout());
    if (!layout)
        return;
    layout->discardLayoutInfo();

    delete state->rubberband;

    QWidget *focus = qApp->focusWidget();

    // calculate absolute position if the tool window was to be
    // dropped to toplevel
    QRect target;
    bool dropped = false;
    if (!(event->modifiers() & Qt::ControlModifier)) {
        // see if there is a main window under us, and ask it to drop the tool window
        QWidget *widget = QApplication::widgetAt(event->globalPos());
        if (state->canDrop && widget) {
            while (widget && !qobject_cast<QMainWindow *>(widget)) {
                if (widget->isWindow()) {
                    widget = 0;
                    break;
                }
                widget = widget->parentWidget();
            }

            if (widget) {
                QMainWindow *mainwindow = qobject_cast<QMainWindow *>(widget);
                if (mainwindow && mainwindow == dockwidget->parentWidget()) {
                    QMainWindowLayout *layout =
                        qobject_cast<QMainWindowLayout *>(dockwidget->parentWidget()->layout());
                    Q_ASSERT(layout != 0);
                    QRect request = state->origin;
                    request.moveTopLeft(event->globalPos() - state->offset);
                    layout->dropDockWidget(dockwidget, request, event->globalPos());
                    dropped = true;
                }
            }
        }
    }

    if (!dropped && hasFeature(dockwidget, QDockWidget::DockWidgetFloatable)) {
        target = state->floating;
        target.moveTopLeft(event->globalPos() - state->offset);

        if (!dockwidget->isFloating()) {
            dockwidget->hide();
            dockwidget->setFloating(true);
            dockwidget->setGeometry(target);
            dockwidget->show();
        } else {
            // move to new location
            dockwidget->setGeometry(target);
        }
    }

    // restore focus
    if (focus)
        focus->setFocus();

    delete state;
    state = 0;
}

void QDockWidgetTitle::contextMenuEvent(QContextMenuEvent *event)
{
    if (state)
        event->accept();
    else
        QWidget::contextMenuEvent(event);
}

void QDockWidgetTitle::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QStyleOptionDockWidget opt;
    opt.rect = rect();
    opt.palette = palette();
    if (isEnabled()) {
        opt.state |= QStyle::State_Enabled;
        if (underMouse())
            opt.state |= QStyle::State_MouseOver;
    }
    opt.title = dockwidget->windowTitle();
    opt.closable = hasFeature(dockwidget, QDockWidget::DockWidgetClosable);
    opt.moveable = hasFeature(dockwidget, QDockWidget::DockWidgetMovable);
    opt.floatable = hasFeature(dockwidget, QDockWidget::DockWidgetFloatable);
    style()->drawControl(QStyle::CE_DockWidgetTitle, &opt, &p, this);
}

void QDockWidgetTitle::updateButtons()
{
    if (hasFeature(dockwidget, QDockWidget::DockWidgetFloatable)) {
        if (!floatButton) {
            floatButton = new QDockWidgetTitleButton(this);
            floatButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarMaxButton));
            connect(floatButton, SIGNAL(clicked()), SLOT(toggleTopLevel()));

            box->insertWidget(1, floatButton);

            if (!dockwidget->isHidden())
                floatButton->show();
        }
    } else {
        delete floatButton;
        floatButton = 0;
    }

    if (hasFeature(dockwidget, QDockWidget::DockWidgetClosable)) {
        if (!closeButton) {
            closeButton = new QDockWidgetTitleButton(this);
            closeButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
            connect(closeButton, SIGNAL(clicked()), dockwidget, SLOT(close()));

            box->insertWidget(2, closeButton);

            if (!dockwidget->isHidden())
                closeButton->show();
        }
    } else {
        delete closeButton;
        closeButton = 0;
    }
}

void QDockWidgetTitle::updateWindowTitle()
{
    const QFontMetrics fm(font());
    spacer->changeSize(fm.width(dockwidget->windowTitle()) + box->margin() * 2, fm.lineSpacing(),
                       QSizePolicy::Expanding, QSizePolicy::Fixed);

    update();
}

void QDockWidgetTitle::toggleTopLevel()
{
    QPoint p = dockwidget->mapToGlobal(QPoint(height(), height()));
    bool visible = dockwidget->isVisible();
    if (visible)
        dockwidget->hide();
    dockwidget->setFloating(!dockwidget->isFloating());
    if (dockwidget->isWindow())
        dockwidget->move(p);
    if (visible)
        dockwidget->show();
}





/*
  Private class
*/

void QDockWidgetPrivate::init() {
    Q_Q(QDockWidget);

    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth);
    top = new QVBoxLayout(q);
    top->setSpacing(fw);
    top->setMargin(qMax(fw, 3));

    title = new QDockWidgetTitle(q);
    top->insertWidget(0, title);

    box = new QVBoxLayout;
    top->addLayout(box);

    resizer = new QWidgetResizeHandler(q);
    resizer->setMovingEnabled(false);
    resizer->setActive(false);

    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    toggleViewAction->setText(q->windowTitle());
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(toggleView(bool)));
}

void QDockWidgetPrivate::toggleView(bool b)
{
    Q_Q(QDockWidget);
    if (b == q->isHidden()) {
        if (b)
            q->show();
        else
            q->close();
    }
}



/*!
    \class QDockWidget

    \brief The QDockWidget class provides a widget that can be docked
    inside a QMainWindow or floated as a top-level window on the
    desktop.

    \ingroup application

    QDockWidget provides the concept of dock widgets, also know as
    tool palettes or utility windows.  Dock windows are secondary
    windows placed in the \e {dock widget area} around the
    \l{QMainWindow::centralWidget()}{central widget} in a
    QMainWindow.

    \image mainwindow-docks.png

    Dock windows can be moved inside their current area, moved into
    new areas and floated (e.g. undocked) by the end-user.  The
    QDockWidget API allows the programmer to restrict the dock widgets
    ability to move, float and close, as well as the areas in which
    they can be placed.

    \section1 Appearance

    A QDockWidget consists of a title bar and the content area.  The
    titlebar displays the dock widgets \link QWidget::windowTitle()
    window title\endlink, a \e float button and a \e close button.
    Depending on the state of the QDockWidget, the \e float and \e
    close buttons may be either disabled or not shown at all.

    The visual appearance of the title bar and buttons is dependent
    on the \l{QStyle}{style} in use.

    ### \e {screenshot of QDockWidget in a few styles should go here}

    \section1 Behavior

    \list

    \i behaviour of QDockWidget while docked in QMainWindow:

    \i behaviour of QDockWidget while floated:

    \i behaviour while dragging QDockWidget:

        \list

        \i to an unoccupied dock widget area:

        \i to an occupied dock widget area:

        \i to top-level:

        \endlist

    \endlist

    \sa QMainWindow
*/

/*!
    \enum QDockWidget::DockWidgetFeature

    \value DockWidgetClosable   The dock widget can be closed.
    \value DockWidgetMovable    The dock widget can be moved between docks
                                by the user.
    \value DockWidgetFloatable  The dock widget can be detached from the
                                main window, and floated as an independent
                                window.

    \value AllDockWidgetFeatures  The dock widget can be closed, moved,
                                  and floated.
    \value NoDockWidgetFeatures   The dock widget cannot be closed, moved,
                                  or floated.

    \omitvalue DockWidgetFeatureMask
    \omitvalue Reserved
*/

/*!
    Constructs a QDockWidget with parent \a parent and window flags \a
    flags. The dock widget will be placed in the left dock widget
    area.
*/
QDockWidget::QDockWidget(QWidget *parent, Qt::WFlags flags)
    : QWidget(*new QDockWidgetPrivate, parent, flags)
{
    Q_D(QDockWidget);
    d->init();
}

/*!
    Constructs a QDockWidget with parent \a parent and window flags \a
    flags. The dock widget will be placed in the left dock widget
    area.

    The window title is set to \a title. This title is used when the
    QDockWidget is docked and undocked. It is also used in the context
    menu provided by QMainWindow.

    \sa setWindowTitle()
*/
QDockWidget::QDockWidget(const QString &title, QWidget *parent, Qt::WFlags flags)
    : QWidget(*new QDockWidgetPrivate, parent, flags)
{
    Q_D(QDockWidget);
    d->init();
    setWindowTitle(title);
}

/*!
    Destroys the dock widget.
*/
QDockWidget::~QDockWidget()
{ }

/*!
    Returns the widget for the dock widget. This function returns zero
    if the widget has not been set.

    \sa setWidget()
*/
QWidget *QDockWidget::widget() const
{
    Q_D(const QDockWidget);
    return d->widget;
}

/*!
    Sets the widget for the dock widget to \a widget.

    \sa widget()
*/
void QDockWidget::setWidget(QWidget *widget)
{
    Q_D(QDockWidget);
    if (d->widget)
        d->box->removeWidget(d->widget);
    d->widget = widget;
    if (d->widget)
        d->box->insertWidget(1, d->widget);
}

/*!
    \property QDockWidget::features
    \brief whether the dock widget is movable, closable, and floatable

    \sa DockWidgetFeature
*/

void QDockWidget::setFeatures(QDockWidget::DockWidgetFeatures features)
{
    Q_D(QDockWidget);
    features &= DockWidgetFeatureMask;
    if (d->features == features)
        return;
    d->features = features;
    d->title->updateButtons();
    d->title->update();
    emit featuresChanged(d->features);
}

QDockWidget::DockWidgetFeatures QDockWidget::features() const
{
    Q_D(const QDockWidget);
    return d->features;
}

/*!
    \property QDockWidget::floating
    \brief whether the dock widget is floating

    A floating dock widget is presented to the user as an independent
    window "on top" of its parent QMainWindow, instead of being
    docked in the QMainWindow.

    \sa isWindow()
*/
void QDockWidget::setFloating(bool floating)
{
    Q_D(QDockWidget);
    if (floating == isFloating())
        return;

    const bool visible = isVisible();

    setWindowFlags(Qt::FramelessWindowHint | (floating ? Qt::Tool : Qt::Widget));

    if (floating) {
        if (QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(parentWidget()->layout()))
            layout->invalidate();
    }

    d->resizer->setActive(floating);

    if (visible)
        show();

    emit topLevelChanged(isWindow());
}

/*!
    \property QDockWidget::allowedAreas
    \brief areas where the dock widget may be placed

    The default is \c Qt::AllDockWidgetAreas.

    \sa Qt::DockWidgetArea
*/

void QDockWidget::setAllowedAreas(Qt::DockWidgetAreas areas)
{
    Q_D(QDockWidget);
    areas &= Qt::DockWidgetArea_Mask;
    if (areas == d->allowedAreas)
        return;
    d->allowedAreas = areas;
    emit allowedAreasChanged(d->allowedAreas);
}

Qt::DockWidgetAreas QDockWidget::allowedAreas() const
{
    Q_D(const QDockWidget);
    return d->allowedAreas;
}

/*!
    \fn bool QDockWidget::isAreaAllowed(Qt::DockWidgetArea area) const

    Returns true if this dock widget can be placed in the given \a area;
    otherwise returns false.
*/

/*! \reimp */
void QDockWidget::changeEvent(QEvent *event)
{
    Q_D(QDockWidget);
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        d->title->updateWindowTitle();
        d->toggleViewAction->setText(windowTitle());
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

/*! \reimp */
void QDockWidget::closeEvent(QCloseEvent *event)
{
    Q_D(QDockWidget);
    if (!(d->features & DockWidgetClosable))
        event->ignore();
}

/*! \reimp */
void QDockWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOptionFrame opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_FrameDockWidget, &opt, &p, this);
}

/*! \reimp */
bool QDockWidget::event(QEvent *event)
{
    Q_D(QDockWidget);
    switch (event->type()) {
    case QEvent::Hide:
        if (!isHidden())
            break;
        // fallthrough intended
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
    case QEvent::StyleChange: {
        int fw = style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth);
        d->top->setSpacing(fw);
        d->top->setMargin(qMax(fw, 3));
    } break;
    default:
        break;
    }
    return QWidget::event(event);
}

/*!
  Returns a checkable action that can be used to show or close this
  dock widget.

  The action's text is set to the dock widget's window title.

  \sa QAction::text QWidget::windowTitle
 */
QAction * QDockWidget::toggleViewAction() const
{
    Q_D(const QDockWidget);
    return d->toggleViewAction;
}

#include "qdockwidget.moc"
#include "moc_qdockwidget.cpp"
