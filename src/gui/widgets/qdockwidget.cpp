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

#ifndef QT_NO_DOCKWIDGET
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
#include <qdebug.h>

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

class QDockWidgetTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    QDockWidgetTitleButton(QDockWidget *dockWidget);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};


QDockWidgetTitleButton::QDockWidgetTitleButton(QDockWidget *dockWidget)
    : QAbstractButton(dockWidget)
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
    QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), isEnabled() ?
                                underMouse() ? QIcon::Active : QIcon::Normal
                                    : QIcon::Disabled,
                                isDown() ? QIcon::On : QIcon::Off);
    style()->drawItemPixmap(&p, r, Qt::AlignCenter, pm);
}


/*
  Private class
*/

void QDockWidgetPrivate::init() {
    Q_Q(QDockWidget);

    top = new QGridLayout(q);
    top->setMargin(0);
    top->setSpacing(0);
    top->setColumnStretch(1, 1);

    box = new QVBoxLayout(0, 0, 0);
    top->addLayout(box, 1, 1);

    leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Ignored);
    rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Ignored);
    topSpacer = new QSpacerItem(0, 20, QSizePolicy::Ignored, QSizePolicy::Fixed);
    bottomSpacer = new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Fixed);
    top->addItem(topSpacer, 0, 1);
    top->addItem(leftSpacer, 1, 0);
    top->addItem(rightSpacer, 1, 2);
    top->addItem(bottomSpacer, 2, 1);

    resizer = new QWidgetResizeHandler(q);
    resizer->setMovingEnabled(false);
    resizer->setActive(false);

#ifndef QT_NO_ACTION
    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    toggleViewAction->setText(q->windowTitle());
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(toggleView(bool)));
#endif

    updateButtons();
}

QStyleOptionDockWidget QDockWidgetPrivate::getStyleOption()
{
    Q_Q(QDockWidget);
    QStyleOptionDockWidget opt;
    opt.rect = titleArea;
    opt.palette = q->palette();
    if (q->isEnabled()) {
        opt.state |= QStyle::State_Enabled;
        if (q->underMouse())
            opt.state |= QStyle::State_MouseOver;
    }
    opt.title = q->windowTitle();
    opt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    opt.movable = hasFeature(q, QDockWidget::DockWidgetMovable);
    opt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);
    return opt;
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

void QDockWidgetPrivate::updateButtons()
{
    Q_Q(QDockWidget);
    if (hasFeature(q, QDockWidget::DockWidgetFloatable)) {
        if (!floatButton) {
            floatButton = new QDockWidgetTitleButton(q);
            QObject::connect(floatButton, SIGNAL(clicked()), q, SLOT(toggleTopLevel()));

            if (!q->isHidden())
                floatButton->show();
        }
    } else {
        delete floatButton;
        floatButton = 0;
    }

    if (hasFeature(q, QDockWidget::DockWidgetClosable)) {
        if (!closeButton) {
            closeButton = new QDockWidgetTitleButton(q);
            QObject::connect(closeButton, SIGNAL(clicked()), q, SLOT(close()));

            if (!q->isHidden())
                closeButton->show();
        }
    } else {
        delete closeButton;
        closeButton = 0;
    }

    bool anyButton = (floatButton || closeButton);
    if (anyButton) {
        QStyleOptionDockWidget opt = getStyleOption();
        if (floatButton)
            floatButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarMaxButton, &opt, q));
        if (closeButton)
            closeButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarCloseButton, &opt, q));
    }

    q->setAttribute(Qt::WA_ContentsPropagated, anyButton);
    relayout();
}

// ### Todo 4.1: Add subrects to style API, this will cover our styles for now
//               Also, add posibilty to get standardIcons
void QDockWidgetPrivate::relayout()
{
    Q_Q(QDockWidget);
    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth) : 0;
    QSize closeSize = closeButton ? closeButton->sizeHint() : QSize(0,0);
    QSize floatSize = floatButton ? floatButton->sizeHint() : QSize(0,0);

    int minWidth  = q->fontMetrics().width(q->windowTitle()) + 2 * fw;
    int minHeight = qMax(closeSize.width(), closeSize.height());
    minHeight = qMax(minHeight, qMax(floatSize.width(), floatSize.height()));
    minHeight += 2; // Allow 1px frame around title area with buttons inside
    minHeight = qMax(minHeight, q->fontMetrics().lineSpacing());
    titleArea = QRect(QPoint(fw, fw),
                      QSize(q->rect().width() - (fw * 2), minHeight));
    int posX = titleArea.right();

    if (closeButton) {
        closeButton->setGeometry(posX - closeSize.width(),
                                 titleArea.bottom() - closeSize.height(),
                                 closeSize.width(),
                                 closeSize.height());
        posX -= closeSize.width() + 1;
    }

    if (floatButton) {
        floatButton->setGeometry(posX - floatSize.width(),
                                 titleArea.bottom() - floatSize.height(),
                                 floatSize.width(),
                                 floatSize.height());
        posX -= floatSize.width() + 1;
    }

    leftSpacer->changeSize(fw, 0, QSizePolicy::Fixed, QSizePolicy::Ignored);
    rightSpacer->changeSize(fw, 0, QSizePolicy::Fixed, QSizePolicy::Ignored);
    topSpacer->changeSize(minWidth, fw + titleArea.height(), QSizePolicy::Expanding, QSizePolicy::Fixed);
    bottomSpacer->changeSize(0, fw, QSizePolicy::Ignored, QSizePolicy::Fixed);
    top->invalidate();
}

void QDockWidgetPrivate::toggleTopLevel()
{
    Q_Q(QDockWidget);
    QPoint p = q->mapToGlobal(QPoint(q->height(), q->height()));
    bool visible = q->isVisible();
    if (visible)
        q->hide();
    q->setFloating(!q->isFloating());
    if (q->isWindow())
        q->move(p);
    if (visible)
        q->show();
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
    d->updateButtons();
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

    d->updateButtons();
#ifndef QT_NO_MAINWINDOW
    if (floating) {
        if (QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(parentWidget()->layout()))
            layout->invalidate();
    }
#endif

    d->resizer->setActive(floating);

    if (visible)
        show();

    emit topLevelChanged(isWindow());
}

/*!
    \property QDockWidget::allowedAreas
    \brief areas where the dock widget may be placed

    The default is Qt::AllDockWidgetAreas.

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
        update(d->titleArea);
#ifndef QT_NO_ACTION
        d->toggleViewAction->setText(windowTitle());
#endif
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
    QPainter p(this);
    // ### Add PixelMetric to change spacers, so style may show border
    // when not floating.
    if (isFloating()) {
        QStyleOptionFrame framOpt;
        framOpt.init(this);
        style()->drawPrimitive(QStyle::PE_FrameDockWidget, &framOpt, &p, this);
    }

    // Title must be painted after the frame, since the areas overlap, and
    // the title may wish to extend out to all sides (eg. XP style)
    Q_D(QDockWidget);
    QStyleOptionDockWidget titleOpt = d->getStyleOption();
    style()->drawControl(QStyle::CE_DockWidgetTitle, &titleOpt, &p, this);
}

/*! \reimp */
void QDockWidget::contextMenuEvent(QContextMenuEvent *event)
{
    Q_D(QDockWidget);
    qDebug() << "contextmenu event";
    if (d->state) {
        qDebug() << "eat contextmenu";
        event->accept();
    }
    else {
        qDebug() << "propagate contextmenu";
        QWidget::contextMenuEvent(event);
    }
}

/*! \reimp */
void QDockWidget::mousePressEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_D(QDockWidget);
    if (event->button() != Qt::LeftButton) return;

    // check if the tool window is movable... do nothing if it is not
    if (!::hasFeature(this, QDockWidget::DockWidgetMovable))
        return;

    QMainWindowLayout *layout =
        qobject_cast<QMainWindowLayout *>(parentWidget()->layout());
    if (!layout)
        return;

    if (!d->titleArea.contains(event->pos()))
        return;

    layout->saveLayoutInfo();

    Q_ASSERT(!d->state);
    d->state = new QDockWidgetPrivate::DragState;

    d->state->rubberband = 0;

    // the current location of the tool window in global coordinates
    d->state->origin = QRect(mapToGlobal(QPoint(0, 0)), size());
    d->state->current = d->state->origin;

    // like the above, except using the tool window's size hint
    d->state->floating = isWindow()
                      ? d->state->current
                      : QRect(d->state->current.topLeft(), sizeHint());

    const QPoint globalPos = event->globalPos();
    const int dl = globalPos.x() - d->state->current.left(),
              dr = d->state->current.right() - globalPos.x(),
       halfWidth = d->state->floating.width() / 2;
    d->state->offset = mapFrom(this,
                            (dl < dr)
                            ? QPoint(qMin(dl, halfWidth), 0)
                            : QPoint(d->state->floating.width() - qMin(dr, halfWidth) - 1, 0));
    d->state->offset = mapTo(this, QPoint(d->state->offset.x(), event->pos().y()));

    d->state->canDrop = true;

#ifdef Q_WS_WIN
    /* Work around windows expose bug when windows are partially covered by
     * a top level transparent object.
     */
    update();
    QWidgetList children = qFindChildren<QWidget *>(this);
    for (int i=0; i<children.size(); ++i)
        children.at(i)->update();
#endif
#endif // !defined(QT_NO_MAINWINDOW)
}

/*! \reimp */
void QDockWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(QDockWidget);
    if (event->button() != Qt::LeftButton)
        return;
    if (!d->titleArea.contains(event->pos()))
        return;
    d->toggleTopLevel();
}

/*! \reimp */
void QDockWidget::mouseMoveEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_D(QDockWidget);
    if (!d->state)
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
                if (mainwindow && mainwindow == parentWidget()) {
                    QMainWindowLayout *layout =
                        qobject_cast<QMainWindowLayout *>(parentWidget()->layout());
                    Q_ASSERT(layout != 0);
                    QRect request = d->state->origin;
                    request.moveTopLeft(event->globalPos() - d->state->offset);
                    target = layout->placeDockWidget(this, request, event->globalPos());
                    layout->resetLayoutInfo();
                }
            }
        }
    }

    d->state->canDrop = target.isValid();
    if (!d->state->canDrop) {
        if (hasFeature(this, QDockWidget::DockWidgetFloatable)) {
            /*
              main window refused to accept the tool window,
              recalculate absolute position as if the tool window
              was to be dropped to toplevel
            */
            target = d->state->floating;
            target.moveTopLeft(event->globalPos() - d->state->offset);
        } else {
            /*
              cannot float the window, so put it back into it's
              original position
            */
            target = d->state->origin;
        }
    }

    if (d->state->current == target)
        return;

    if (!d->state->rubberband) {
        const int screen_number = QApplication::desktop()->screenNumber(window());
        d->state->rubberband = new QRubberBand(QRubberBand::Rectangle,
                                               QApplication::desktop()->screen(screen_number));
        d->state->rubberband->setGeometry(target);
        d->state->rubberband->show();
    } else {
        d->state->rubberband->setGeometry(target);
    }
    d->state->current = target;
#endif // !defined(QT_NO_MAINWINDOW)
}

/*! \reimp */
void QDockWidget::mouseReleaseEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_D(QDockWidget);
    if (event->button() != Qt::LeftButton)
        return;

    if (!d->state)
        return;

    QMainWindowLayout *layout =
        qobject_cast<QMainWindowLayout *>(parentWidget()->layout());
    if (!layout)
        return;
    layout->discardLayoutInfo();

    delete d->state->rubberband;

    QWidget *focus = qApp->focusWidget();

    // calculate absolute position if the tool window was to be
    // dropped to toplevel
    QRect target;
    bool dropped = false;
    if (!(event->modifiers() & Qt::ControlModifier)) {
        // see if there is a main window under us, and ask it to drop the tool window
        QWidget *widget = QApplication::widgetAt(event->globalPos());
        if (d->state->canDrop && widget) {
            while (widget && !qobject_cast<QMainWindow *>(widget)) {
                if (widget->isWindow()) {
                    widget = 0;
                    break;
                }
                widget = widget->parentWidget();
            }

            if (widget) {
                QMainWindow *mainwindow = qobject_cast<QMainWindow *>(widget);
                if (mainwindow && mainwindow == parentWidget()) {
                    QMainWindowLayout *layout =
                        qobject_cast<QMainWindowLayout *>(parentWidget()->layout());
                    Q_ASSERT(layout != 0);
                    QRect request = d->state->origin;
                    request.moveTopLeft(event->globalPos() - d->state->offset);
                    layout->dropDockWidget(this, request, event->globalPos());
                    dropped = true;
                }
            }
        }
    }

    if (!dropped && hasFeature(this, QDockWidget::DockWidgetFloatable)) {
        target = d->state->floating;
        target.moveTopLeft(event->globalPos() - d->state->offset);

        if (!isFloating()) {
            hide();
            setFloating(true);
            setGeometry(target);
            show();
        } else {
            // move to new location
            setGeometry(target);
        }
    }

    // restore focus
    if (focus)
        focus->setFocus();

    delete d->state;
    d->state = 0;
#endif // !defined(QT_NO_MAINWINDOW)
}

/*! \reimp */
void QDockWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    Q_D(QDockWidget);
    d->relayout();
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
#ifndef QT_NO_ACTION
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
#endif
    case QEvent::StyleChange:
        d->updateButtons();
        break;

    default:
        break;
    }
    return QWidget::event(event);
}

#ifndef QT_NO_ACTION
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
#endif // QT_NO_ACTION

/*!
    \fn void QDockWidget::featuresChanged(DockWidgetFeatures features)

    This signal is emitted when the \l features property changes. The
    \a features parameter gives the new value of the property.
*/

/*!
    \fn void QDockWidget::topLevelChanged(bool topLevel)

    This signal is emitted when the \l floating property changes.
    The \a topLevel parameter is true if the dock widget is now floating;
    otherwise it is false.

    \sa isWindow()
*/

/*!
    \fn void QDockWidget::allowedAreasChanged(Qt::DockWidgetAreas allowedAreas)

    This signal is emitted when the \l allowedAreas property changes. The
    \a allowedAreas parameter gives the new value of the property.
*/

#include "qdockwidget.moc"
#include "moc_qdockwidget.cpp"
#endif // QT_NO_DOCKWIDGET
