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

#include "qdockwidget.h"

#ifndef QT_NO_DOCKWIDGET
#include <qaction.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qmainwindow.h>
#include <qrubberband.h>
#include <qstylepainter.h>
#include <qtoolbutton.h>
#include <qdebug.h>

#include <private/qwidgetresizehandler_p.h>

#include "qdockwidget_p.h"
#include "qdockwidgetlayout_p.h"
#include "qmainwindowlayout_p.h"
#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif


static inline bool hasFeature(const QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
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




/******************************************************************************
** QDockWidgetTitleButton
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
{
    setFocusPolicy(Qt::NoFocus);
}

QSize QDockWidgetTitleButton::sizeHint() const
{
    ensurePolished();

    int size = 2*style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin);
    if (!icon().isNull()) {
        int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
        const QPixmap pm = icon().pixmap(iconSize);
        size += qMax(pm.width(), pm.height());
    }

    return QSize(size, size);
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

#ifdef Q_WS_MAC
    if (!qobject_cast<QMacStyle *>(style()))
#endif
    {
        if (isEnabled() && underMouse() && !isChecked() && !isDown())
            opt.state |= QStyle::State_Raised;
        if (isChecked())
            opt.state |= QStyle::State_On;
        if (isDown())
            opt.state |= QStyle::State_Sunken;
        style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
    }

    int shiftHorizontal = opt.state & QStyle::State_Sunken ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this) : 0;
    int shiftVertical = opt.state & QStyle::State_Sunken ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &opt, this) : 0;

    r.adjust(2, 2, -2, -2);
    r.translate(shiftHorizontal, shiftVertical);

    QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), isEnabled() ?
                                underMouse() ? QIcon::Active : QIcon::Normal
                                    : QIcon::Disabled,
                                isDown() ? QIcon::On : QIcon::Off);
    style()->drawItemPixmap(&p, r, Qt::AlignCenter, pm);
}

/******************************************************************************
** QDWLayout
*/

class QDWLayout : public QLayout
{
public:
    QDWLayout(QWidget *parent = 0);
    void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    QSize minimumSize() const;
    int count() const;

    enum Role { Content, CloseButton, FloatButton, RoleCount };
    QWidget *widget(Role r);
    void setWidget(Role r, QWidget *w);

    QRect titleArea() const { return _titleArea; }

private:
    QVector<QLayoutItem*> item_list;
    QRect _titleArea;

    int minimumTitleWidth() const;
    int titleHeight() const;
};

void QDWLayout::addItem(QLayoutItem*)
{
    qWarning() << "QDWLayout::addItem(): please use QDWLayout::setWidget()";
    return;
}

QLayoutItem *QDWLayout::itemAt(int index) const
{
    int cnt = 0;
    foreach (QLayoutItem *item, item_list) {
        if (item == 0)
            continue;
        if (index == cnt++)
            return item;
    }
    return 0;
}

QLayoutItem *QDWLayout::takeAt(int index)
{
    for (int i = 0; i < item_list.count(); ++i) {
        QLayoutItem *item = item_list.at(i);
        if (item == 0)
            continue;
        if (index == i) {
            item_list[i] = 0;
            return item;
        }
    }
    return 0;
}

int QDWLayout::count() const
{
    int result = 0;
    foreach (QLayoutItem *item, item_list) {
        if (item != 0)
            ++result;
    }
    return result;
}

QSize QDWLayout::sizeHint() const
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);

    QSize contentHint(0, 0);
    if (item_list[Content] != 0)
        contentHint = item_list[Content]->sizeHint();

    int w = qMax(contentHint.width(), minimumTitleWidth()) + 2*fw;
    int h = contentHint.height() + 2*fw + titleHeight();

    return QSize(w, h);
}

QSize QDWLayout::minimumSize() const
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);

    QSize contentMin(0, 0);
    if (item_list[Content] != 0)
        contentMin = item_list[Content]->minimumSize();

    int w = qMax(contentMin.width(), minimumTitleWidth()) + 2*fw;
    int h = contentMin.height() + 2*fw + titleHeight();

    return QSize(w, h);
}

QDWLayout::QDWLayout(QWidget *parent)
    : QLayout(parent), item_list(RoleCount, 0)
{
}

QWidget *QDWLayout::widget(Role r)
{
    QLayoutItem *item = item_list.at(r);
    return item == 0 ? 0 : item->widget();
}

void QDWLayout::setWidget(Role r, QWidget *w)
{
    QWidget *old = widget(r);
    if (old != 0)
        removeWidget(old);

    addChildWidget(w);

    item_list[r] = new QWidgetItem(w);
}

int QDWLayout::minimumTitleWidth() const
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    QSize closeSize(0, 0);
    QSize floatSize(0, 0);
    if (QLayoutItem *item = item_list[CloseButton])
        closeSize = item->widget()->sizeHint();
    if (QLayoutItem *item = item_list[FloatButton])
        floatSize = item->widget()->sizeHint();

    int titleHeight = this->titleHeight();

    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);
    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    return closeSize.width() + floatSize.width() + titleHeight
            + 2*fw + 3*mw;
}

int QDWLayout::titleHeight() const
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    QSize closeSize(0, 0);
    QSize floatSize(0, 0);
    if (QLayoutItem *item = item_list[CloseButton])
        closeSize = item->widget()->sizeHint();
    if (QLayoutItem *item = item_list[FloatButton])
        floatSize = item->widget()->sizeHint();

    int buttonHeight = qMax(closeSize.height(), floatSize.height());

    QFontMetrics titleFontMetrics = q->fontMetrics();
#ifdef Q_WS_MAC
    if (qobject_cast<QMacStyle *>(q->style())) {
        extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp
        QFont font = qt_app_fonts_hash()->value("QToolButton", q->font());
        titleFontMetrics = QFontMetrics(font);
    }
#endif

    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    return qMax(buttonHeight + 2, titleFontMetrics.lineSpacing() + 2*mw);
}

void QDWLayout::setGeometry(const QRect &geometry)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = 0;
    if (q->isFloating())
        fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);
    int titleHeight = this->titleHeight();

    _titleArea = QRect(QPoint(fw, fw),
                        QSize(geometry.width() - (fw * 2), titleHeight));

#if 0
    QPoint buttonOffset(0, 0);
#ifdef Q_OS_WIN
    // ### Qt 4.3: Fix this properly
    if (q->style()->inherits("QWindowsXPStyle")) {
        if(q->isFloating())
            buttonOffset = QPoint(2, -1);
        else
            buttonOffset = QPoint(0, 1);
    }
#endif
#endif

    QStyleOptionDockWidget opt;
    q->initStyleOption(&opt);

    if (QLayoutItem *item = item_list[CloseButton]) {
        QRect r = q->style()->subElementRect(QStyle::SE_DockWidgetCloseButton, &opt, q);
        if (!r.isNull())
            item->setGeometry(r);
    }

    if (QLayoutItem *item = item_list[FloatButton]) {
        QRect r = q->style()->subElementRect(QStyle::SE_DockWidgetFloatButton, &opt, q);
        if (!r.isNull())
            item->setGeometry(r);
    }

    if (QLayoutItem *item = item_list[Content]) {
        QRect r = geometry;
        r.setTop(_titleArea.bottom() + 1);
        r.adjust(fw, 0, -fw, -fw);
        item->setGeometry(r);
    }
}

/******************************************************************************
** QDockWidgetPrivate
*/

void QDockWidgetPrivate::init()
{
    Q_Q(QDockWidget);

    QDWLayout *layout = new QDWLayout(q);

    QAbstractButton *button = new QDockWidgetTitleButton(q);
    QObject::connect(button, SIGNAL(clicked()), q, SLOT(_q_toggleTopLevel()));
    layout->setWidget(QDWLayout::FloatButton, button);

    button = new QDockWidgetTitleButton(q);
    QObject::connect(button, SIGNAL(clicked()), q, SLOT(close()));
    layout->setWidget(QDWLayout::CloseButton, button);

    resizer = new QWidgetResizeHandler(q);
    resizer->setMovingEnabled(false);
    resizer->setActive(false);

#ifndef QT_NO_ACTION
    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    toggleViewAction->setText(q->windowTitle());
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)),
                        q, SLOT(_q_toggleView(bool)));
#endif

    updateButtons();
}

/*!
    Initialize \a option with the values from this QDockWidget. This method
    is useful for subclasses when they need a QStyleOptionDockWidget, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom()
*/
void QDockWidget::initStyleOption(QStyleOptionDockWidget *option) const
{
    if (!option)
        return;
    QDWLayout *dwlayout = qobject_cast<QDWLayout*>(layout());

    option->initFrom(this);
    option->rect = dwlayout->titleArea();
    option->title = windowTitle();
    option->closable = hasFeature(this, QDockWidget::DockWidgetClosable);
    option->movable = hasFeature(this, QDockWidget::DockWidgetMovable);
    option->floatable = hasFeature(this, QDockWidget::DockWidgetFloatable);
}

void QDockWidgetPrivate::_q_toggleView(bool b)
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
    QDWLayout *layout = qobject_cast<QDWLayout*>(q->layout());

    QStyleOptionDockWidget opt;
    q->initStyleOption(&opt);

    bool canClose = hasFeature(q, QDockWidget::DockWidgetClosable);
    bool canFloat = hasFeature(q, QDockWidget::DockWidgetFloatable);

    QAbstractButton *button
        = qobject_cast<QAbstractButton*>(layout->widget(QDWLayout::FloatButton));
    button->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarNormalButton,
                                                &opt, q));
    button->setVisible(canFloat);

    button
        = qobject_cast <QAbstractButton*>(layout->widget(QDWLayout::CloseButton));
    button->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarCloseButton,
                                                &opt, q));
    button->setVisible(canClose);

    q->setAttribute(Qt::WA_ContentsPropagated, canFloat || canClose);
    layout->update();
}

void QDockWidgetPrivate::_q_toggleTopLevel()
{
    Q_Q(QDockWidget);
    QDWLayout *layout = qobject_cast<QDWLayout*>(q->layout());
    QRect titleArea = layout->titleArea();

    QPoint p = q->mapToGlobal(QPoint(titleArea.height(), titleArea.height()));
    bool visible = q->isVisible();
    if (visible)
        q->hide();
    q->setFloating(!q->isFloating());
    if (q->isWindow())
        q->move(p);
    if (visible)
        q->show();
}

QMainWindow *QDockWidgetPrivate::findMainWindow(QWidget *widget) const
{
    Q_Q(const QDockWidget);
    QMainWindow *mainwindow = 0;
    // look for our QMainWindow
    while (widget && !mainwindow) {
        if (widget == q->parentWidget()) {
            // found our parent widget, has to be the mainwindow we're looking for
            mainwindow = qobject_cast<QMainWindow *>(widget);
            break;
        } else if (widget->isWindow()) {
            // found a window that isn't our QMainWindow, stop looking
            widget = 0;
        } else {
            widget = widget->parentWidget();
        }
    }
    return mainwindow;
}

void QDockWidgetPrivate::mousePressEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_Q(QDockWidget);

    QDWLayout *layout = qobject_cast<QDWLayout*>(q->layout());
    QRect titleArea = layout->titleArea();

    if (event->button() != Qt::LeftButton)
        return;

    if (!titleArea.contains(event->pos()))
        return;
    // check if the tool window is movable... do nothing if it is not
    if (!::hasFeature(q, QDockWidget::DockWidgetMovable))
        return;

    if (!::hasFeature(q, QDockWidget::DockWidgetFloatable))
        return;

    if (qobject_cast<QMainWindow*>(q->parentWidget()) == 0)
        return;

    if (state != 0)
        return;

    state = new QDockWidgetPrivate::DragState;

    state->pressPos = event->pos();
    state->dragging = false;
    state->widgetItem = 0;
    state->ownWidgetItem = false;
#endif // !defined(QT_NO_MAINWINDOW)
}

void QDockWidgetPrivate::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_Q(QDockWidget);

    QDWLayout *layout = qobject_cast<QDWLayout*>(q->layout());
    QRect titleArea = layout->titleArea();

    if (event->button() != Qt::LeftButton)
        return;
    if (!titleArea.contains(event->pos()))
        return;
    if (!::hasFeature(q, QDockWidget::DockWidgetFloatable))
        return;
    _q_toggleTopLevel();
}

void QDockWidgetPrivate::mouseMoveEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_Q(QDockWidget);

    if (!state)
        return;

    QMainWindowLayout *layout
        = qobject_cast<QMainWindowLayout *>(q->parentWidget()->layout());
    Q_ASSERT(layout != 0);

    if (!state->dragging
        && layout->pluggingWidget == 0
        && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {
        state->widgetItem = layout->unplug(q);
        if (state->widgetItem == 0) {
            /* I have a QMainWindow parent, but I was never inserted with QMainWindow::addDockWidget,
               so the QMainWindowLayout has no widget item for me. :(
               I have to create it myself, and then delete it if I don't get dropped into a
               dock area. */
            state->widgetItem = new QWidgetItem(q);
            state->ownWidgetItem = true;
        }
        q->grabMouse();
        state->dragging = true;
    }

    if (state->dragging) {
        q->move(event->globalPos() - state->pressPos);

        if (!(event->modifiers() & Qt::ControlModifier))
            state->pathToGap = layout->hover(state->widgetItem, event->globalPos());
    }
#endif // !defined(QT_NO_MAINWINDOW)
}

void QDockWidgetPrivate::mouseReleaseEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_Q(QDockWidget);
    if (event->button() != Qt::LeftButton)
        return;
    if (!state)
        return;

    if (state->dragging) {
        QMainWindowLayout *layout =
            qobject_cast<QMainWindowLayout *>(q->parentWidget()->layout());
        Q_ASSERT(layout != 0);

        q->releaseMouse();

        if (!(event->modifiers() & Qt::ControlModifier) && !state->pathToGap.isEmpty()) {
            layout->plug(state->widgetItem, state->pathToGap);
        } else {
            layout->restore();

            if (state->ownWidgetItem)
                delete state->widgetItem;

            q->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
            updateButtons();
            resizer->setActive(QWidgetResizeHandler::Resize, true);
            q->show();
        }
    }

    delete state;
    state = 0;
#endif // !defined(QT_NO_MAINWINDOW)
}

void QDockWidgetPrivate::unplug(const QRect &rect)
{
    Q_Q(QDockWidget);
    QRect r = rect;
    r.moveTopLeft(q->mapToGlobal(r.topLeft()));
    q->hide();
    q->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::X11BypassWindowManagerHint);
    resizer->setActive(QWidgetResizeHandler::Resize, false);
    q->setGeometry(r);
    q->show();
    updateButtons();
    emit q->topLevelChanged(q->isWindow());
}

void QDockWidgetPrivate::plug(const QRect &rect)
{
    Q_Q(QDockWidget);
    q->hide();

    q->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);

    updateButtons();
    resizer->setActive(QWidgetResizeHandler::Resize, false);
    q->setGeometry(rect);
    q->show();

    emit q->topLevelChanged(false);
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

    \sa QMainWindow, {Dock Widgets Example}
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
    \property QDockWidget::windowTitle
    \internal
*/

/*!
    Constructs a QDockWidget with parent \a parent and window flags \a
    flags. The dock widget will be placed in the left dock widget
    area.
*/
QDockWidget::QDockWidget(QWidget *parent, Qt::WindowFlags flags)
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
QDockWidget::QDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
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
    QDWLayout *layout = qobject_cast<QDWLayout*>(this->layout());
    return layout->widget(QDWLayout::Content);
}

/*!
    Sets the widget for the dock widget to \a widget.

    \sa widget()
*/
void QDockWidget::setWidget(QWidget *widget)
{
    QDWLayout *layout = qobject_cast<QDWLayout*>(this->layout());
    layout->setWidget(QDWLayout::Content, widget);
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
    d->toggleViewAction->setEnabled((d->features & DockWidgetClosable) == DockWidgetClosable);
    emit featuresChanged(d->features);
    update();
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

    if (!floating && parentWidget() != 0) {
        QMainWindowLayout *layout
            = qobject_cast<QMainWindowLayout*>(parentWidget()->layout());
        if (layout != 0)
            layout->keepSize(this);
    }

    setWindowFlags(Qt::FramelessWindowHint | (floating ? Qt::Tool : Qt::Widget));

    d->updateButtons();
    d->resizer->setActive(QWidgetResizeHandler::Resize, floating);

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
    QDWLayout *layout = qobject_cast<QDWLayout*>(this->layout());

    switch (event->type()) {
    case QEvent::WindowTitleChange:
        update(layout->titleArea());
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
    QStylePainter p(this);
    // ### Add PixelMetric to change spacers, so style may show border
    // when not floating.
    if (isFloating()) {
        QStyleOptionFrame framOpt;
        framOpt.init(this);
        p.drawPrimitive(QStyle::PE_FrameDockWidget, framOpt);
    }

    // Title must be painted after the frame, since the areas overlap, and
    // the title may wish to extend out to all sides (eg. XP style)
    QStyleOptionDockWidget titleOpt;
    initStyleOption(&titleOpt);
    p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
}

/*! \reimp */
bool QDockWidget::event(QEvent *event)
{
    Q_D(QDockWidget);

    QMainWindow *win = qobject_cast<QMainWindow*>(parentWidget());
    QMainWindowLayout *layout = 0;
    if (win != 0)
        layout = qobject_cast<QMainWindowLayout*>(win->layout());

    switch (event->type()) {
#ifndef QT_NO_ACTION
    case QEvent::Hide:
        if (layout != 0)
            layout->keepSize(this);
        if (!isHidden())
            break;
        // fallthrough intended
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
#endif
    case QEvent::ApplicationLayoutDirectionChange:
    case QEvent::LayoutDirectionChange:
    case QEvent::StyleChange:
        d->updateButtons();
        break;
    case QEvent::ZOrderChange: {
        if (isFloating())
            break;
        if (layout == 0)
            break;
        layout->raise(this);
        break;
    }
    case QEvent::ContextMenu:
        if (d->state) {
            event->accept();
            return true;
        }
        break;
        // return true after calling the handler since we don't want
        // them to be passed onto the default handlers
    case QEvent::MouseButtonPress:
        d->mousePressEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseButtonDblClick:
        d->mouseDoubleClickEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseMove:
        d->mouseMoveEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseButtonRelease:
        d->mouseReleaseEvent(static_cast<QMouseEvent *>(event));
        return true;
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
    \fn void QDockWidget::featuresChanged(QDockWidget::DockWidgetFeatures features)

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
