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

#include "qstatusbar.h"
#ifndef QT_NO_STATUSBAR

#include "qlist.h"
#include "qevent.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qsizegrip.h"
#include <private/qlayoutengine_p.h>

#include <private/qwidget_p.h>

class QStatusBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QStatusBar)
public:
    QStatusBarPrivate() {}

    struct SBItem {
        SBItem(QWidget* widget, int stretch, bool permanent)
            : s(stretch), w(widget), p(permanent) {}
        int s;
        QWidget * w;
        bool p;
    };

    QList<SBItem *> items;
    QString tempItem;

    QBoxLayout * box;
    QTimer * timer;

#ifndef QT_NO_SIZEGRIP
    QSizeGrip * resizer;
#endif

    int savedStrut;

    int indexToLastNonPermanentWidget() const
    {
        int i = items.size() - 1;
        for (; i >= 0; --i) {
            SBItem *item = items.at(i);
            if (!(item && item->p))
                break;
        }
        return i;
    }
};

/*!
    \class QStatusBar
    \brief The QStatusBar class provides a horizontal bar suitable for
    presenting status information.

    \ingroup application
    \ingroup helpsystem
    \mainclass

    Each status indicator falls into one of three categories:

    \list
    \o \e Temporary - briefly occupies most of the status bar. Used
        to explain tool tip texts or menu entries, for example.
    \o \e Normal - occupies part of the status bar and may be hidden
        by temporary messages. Used to display the page and line
        number in a word processor, for example.
    \o \e Permanent - is never hidden. Used for important mode
        indications, for example, some applications put a Caps Lock
        indicator in the status bar.
    \endlist

    QStatusBar lets you display all three types of indicators.

    Typically, a request for the status bar functionality occurs in
    relation to a QMainWindow object. QMainWindow provides a main
    application window, with a menu bar, tool bars, dock widgets \e
    and a status bar around a large central widget. The status bar can
    be retrieved using the QMainWindow::statusBar() function, and
    replaced using the QMainWindow::setStatusBar() function.

    Use the showMessage() slot to display a \e temporary message:

    \quotefromfile mainwindows/dockwidgets/mainwindow.cpp
    \skipto MainWindow::createStatusBar()
    \printuntil /^\}/

    To remove a temporary message, use the clearMessage() slot, or set
    a time limit when calling showMessage(). For example:

    \quotefromfile mainwindows/dockwidgets/mainwindow.cpp
    \skipto void MainWindow::print()
    \printuntil /^\}/

    Use the currentMessage() function to retrieve the temporary
    message currently shown. The QStatusBar class also provide the
    messageChanged() signal which is emitted whenever the temporary
    status message changes.

    \e Normal and \e Permanent messages are displayed by creating a
    small widget (QLabel, QProgressBar or even QToolButton) and then
    adding it to the status bar using the addWidget() or the
    addPermanentWidget() function. Use the removeWidget() function to
    remove such messages from the status bar.

    \code
        statusBar()->addWidget(new MyReadWriteIndication);
    \endcode

    By default QStatusBar provides a QSizeGrip in the lower-right
    corner. You can disable it using the setSizeGripEnabled()
    function. Use the isSizeGripEnabled() function to determine the
    current status of the size grip.

    \image plastique-statusbar.png A status bar shown in the Plastique widget style

    \sa QMainWindow, QStatusTipEvent, {fowler}{GUI Design Handbook:
    Status Bar}, {Application Example}
*/

#ifdef QT3_SUPPORT
/*!
    Constructs a status bar with a size grip and the given \a parent
    and object \a name.

    Use the QStatusBar() constructor and the QObject::setObjectName()
    function instead.

    \oldcode
        QStatusBar *myStatusBar = new QStatusBar(parent, name);
    \newcode
        QStatusBar *myStatusBar = new QStatusBar(parent);
        myStatusBar->setObjectName(name);
    \endcode
*/
QStatusBar::QStatusBar(QWidget * parent, const char *name)
    : QWidget(*new QStatusBarPrivate, parent, 0)
{
    Q_D(QStatusBar);
    setObjectName(QString::fromAscii(name));
    d->box = 0;
    d->timer = 0;

#ifndef QT_NO_SIZEGRIP
    d->resizer = 0;
    setSizeGripEnabled(true); // causes reformat()
#else
    reformat();
#endif
}


/*!
    \fn void QStatusBar::addWidget(QWidget * widget, int stretch, bool permanent)

   Use addWidget() or addPermanentWidget() instead, depending on the
   value of the \a permanent parameter.

    \oldcode
        QStatusBar *myStatusBar;
        myStatusBar->addWidget(widget, stretch, permanent);  // permanent == true
    \newcode
        QStatusBar *myStatusBar;
        myStatusBar->addPermanentWidget(widget, stretch);
    \endcode
 */

#endif

/*!
    Constructs a status bar with a size grip and the given \a parent.

    \sa setSizeGripEnabled()
*/
QStatusBar::QStatusBar(QWidget * parent)
    : QWidget(*new QStatusBarPrivate, parent, 0)
{
    Q_D(QStatusBar);
    d->box = 0;
    d->timer = 0;

#ifndef QT_NO_SIZEGRIP
    d->resizer = 0;
    setSizeGripEnabled(true); // causes reformat()
#else
    reformat();
#endif
}

/*!
    Destroys this status bar and frees any allocated resources and
    child widgets.
*/
QStatusBar::~QStatusBar()
{
    Q_D(QStatusBar);
    while (!d->items.isEmpty())
        delete d->items.takeFirst();
}


/*!
    Adds the given \a widget to this status bar, reparenting the
    widget if it isn't already a child of this QStatusBar object. The
    \a stretch parameter is used to compute a suitable size for the
    given \a widget as the status bar grows and shrinks. The default
    stretch factor is 0, i.e giving the widget a minimum of space.

    The widget is located to the far left of the first permanent
    widget (see addPermanentWidget()) and may be obscured by temporary
    messages.

    Note that this function may cause some flicker.

    \sa insertWidget(), removeWidget(), addPermanentWidget()
*/

void QStatusBar::addWidget(QWidget * widget, int stretch)
{
    if (!widget)
        return;

    Q_D(QStatusBar);
    QStatusBarPrivate::SBItem* item = new QStatusBarPrivate::SBItem(widget, stretch, false);

    int i = d->indexToLastNonPermanentWidget();
    d->items.insert(i >= 0 ? i + 1 : 0, item);

    if (!d->tempItem.isEmpty())
        widget->hide();

    reformat();
    if (!widget->isHidden() || !widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
        widget->show();
}

/*!
    Inserts the given \a widget at the given \a index to this status bar,
    reparenting the widget if it isn't already a child of this
    QStatusBar object. If \a index is out of range, the widget is appended
    (in which case it is the actual index of the widget that is returned).

    The widget is located to the far left of the first permanent
    widget (see addPermanentWidget()) and may be obscured by temporary
    messages.

    Note that this function may cause some flicker.

    \sa addWidget(), removeWidget(), addPermanentWidget()
*/
int QStatusBar::insertWidget(int index, QWidget *widget, int stretch)
{
    if (!widget)
        return -1;

    Q_D(QStatusBar);
    QStatusBarPrivate::SBItem* item = new QStatusBarPrivate::SBItem(widget, stretch, false);

    int idx = d->indexToLastNonPermanentWidget();
    if (index < 0 || index > d->items.size() || (idx >= 0 && index > idx + 1)) {
        qWarning("QStatusBar::insertWidget: Index out of range (%d), appending widget", index);
        addWidget(widget);
        return idx + 1;
    }
    d->items.insert(index, item);

    if (!d->tempItem.isEmpty())
        widget->hide();

    reformat();
    if (!widget->isHidden() || !widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
        widget->show();

    return index;
}

/*!
    Adds the given \a widget permanently to this status bar,
    reparenting the widget if it isn't already a child of this
    QStatusBar object. The \a stretch parameter is used to compute a
    suitable size for the given \a widget as the status bar grows and
    shrinks. The default stretch factor is 0, i.e giving the widget a
    minimum of space.

    Permanently means that the widget may not be obscured by temporary
    messages. It is is located at the far right of the status bar.

    Note that this function may cause some flicker.

    \sa insertPermanentWidget, removeWidget(), addWidget()
*/

void QStatusBar::addPermanentWidget(QWidget * widget, int stretch)
{
    if (!widget)
        return;

    Q_D(QStatusBar);
    QStatusBarPrivate::SBItem* item = new QStatusBarPrivate::SBItem(widget, stretch, true);

    int i = d->items.size() - 1;
    d->items.insert(i >= 0 ? i + 1 : 0, item);

    reformat();
    if (!widget->isHidden() || !widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
        widget->show();
}


/*!
    Inserts the given \a widget at the given \a index permanently to this status bar,
    reparenting the widget if it isn't already a child of this
    QStatusBar object. If \a index is out of range, the widget is appended
    (in which case it is the actual index of the widget that is returned).

    The \a stretch parameter is used to compute a
    suitable size for the given \a widget as the status bar grows and
    shrinks. The default stretch factor is 0, i.e giving the widget a
    minimum of space.

    Permanently means that the widget may not be obscured by temporary
    messages. It is is located at the far right of the status bar.

    Note that this function may cause some flicker.

    \sa addPermanentWidget, removeWidget(), addWidget()
*/
int QStatusBar::insertPermanentWidget(int index, QWidget *widget, int stretch)
{
    if (!widget)
        return -1;

    Q_D(QStatusBar);
    QStatusBarPrivate::SBItem* item = new QStatusBarPrivate::SBItem(widget, stretch, true);

    int idx = d->indexToLastNonPermanentWidget();
    if (index < 0 || index > d->items.size() || (idx >= 0 && index <= idx)) {
        qWarning("QStatusBar::insertPermanentWidget: Index out of range (%d), appending widget", index);
        addPermanentWidget(widget);
        return d->items.size() - 1;
    }
    d->items.insert(index, item);

    reformat();
    if (!widget->isHidden() || !widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
        widget->show();

    return index;
}

/*!
    Removes the specifed \a widget from the status bar (without
    deleting it).

    This function may cause some flicker.

    \sa addWidget(), addPermanentWidget(), clearMessage()
*/

void QStatusBar::removeWidget(QWidget* widget)
{
    if (!widget)
        return;

    Q_D(QStatusBar);
    bool found = false;
    QStatusBarPrivate::SBItem* item;
    for (int i=0; i<d->items.size(); ++i) {
        item = d->items.at(i);
        if (!item)
            break;
        if (item->w == widget) {
            d->items.removeAt(i);
            item->w->hide();
            delete item;
            found = true;
            break;
        }
    }

    if (found)
        reformat();
#if defined(QT_DEBUG)
    else
        qDebug("QStatusBar::removeWidget(): Widget not found.");
#endif
}

/*!
    \property QStatusBar::sizeGripEnabled

    \brief whether the QSizeGrip in the bottom-right corner of the
    status bar is enabled

    The size grip is enabled by default.
*/

bool QStatusBar::isSizeGripEnabled() const
{
#ifdef QT_NO_SIZEGRIP
    return false;
#else
    Q_D(const QStatusBar);
    return !!d->resizer;
#endif
}

void QStatusBar::setSizeGripEnabled(bool enabled)
{
#ifdef QT_NO_SIZEGRIP
    Q_UNUSED(enabled);
#else
    Q_D(QStatusBar);
    if (!enabled != !d->resizer) {
        if (enabled) {
            d->resizer = new QSizeGrip(this);
        } else {
            delete d->resizer;
            d->resizer = 0;
        }
        reformat();
        if (d->resizer && isVisible())
            d->resizer->show();
    }
#endif
}


/*!
    Changes the status bar's appearance to account for item changes.

    Special subclasses may need this function, but geometry management
    will usually take care of any necessary rearrangements.
*/
void QStatusBar::reformat()
{
    Q_D(QStatusBar);
    if (d->box)
        delete d->box;

    QBoxLayout *vbox;
#ifndef QT_NO_SIZEGRIP
    if (d->resizer) {
        d->box = new QHBoxLayout(this);
        d->box->setMargin(0);
        vbox = new QVBoxLayout;
        d->box->addLayout(vbox);
    } else
#endif
    {
        vbox = d->box = new QVBoxLayout(this);
        d->box->setMargin(0);
    }
    vbox->addSpacing(3);
    QBoxLayout* l = new QHBoxLayout;
    vbox->addLayout(l);
    l->addSpacing(3);
    l->setSpacing(4);

    int maxH = fontMetrics().height();

    int i;
    QStatusBarPrivate::SBItem* item;
    for (i=0,item=0; i<d->items.size(); ++i) {
        item = d->items.at(i);
        if (!item || item->p)
            break;
        l->addWidget(item->w, item->s);
        int itemH = qMin(qSmartMinSize(item->w).height(), item->w->maximumHeight());
        maxH = qMax(maxH, itemH);
    }

    l->addStretch(0);

    for (item=0; i<d->items.size(); ++i) {
        item = d->items.at(i);
        if (!item)
            break;
        l->addWidget(item->w, item->s);
        int itemH = qMin(qSmartMinSize(item->w).height(), item->w->maximumHeight());
        maxH = qMax(maxH, itemH);
    }
    l->addSpacing(4);
#ifndef QT_NO_SIZEGRIP
    if (d->resizer) {
        maxH = qMax(maxH, d->resizer->sizeHint().height());
        d->box->addSpacing(1);
        d->box->addWidget(d->resizer, 0, Qt::AlignBottom);
    }
#endif
    l->addStrut(maxH);
    d->savedStrut = maxH;
    vbox->addSpacing(2);
    d->box->activate();
    repaint();
}

/*!
    Hides the normal status indications and displays the given \a
    message for the specified \a timeout milli-seconds (if non-zero),
    or until clearMessage() or another showMessage() is called,
    whichever occurs first.

    \sa messageChanged(), currentMessage(), clearMessage()
*/
void QStatusBar::showMessage(const QString &message, int timeout)
{
    Q_D(QStatusBar);
    if (d->tempItem == message)
        return;

    d->tempItem = message;

    if (timeout > 0) {
        if (!d->timer) {
            d->timer = new QTimer(this);
            connect(d->timer, SIGNAL(timeout()), this, SLOT(clearMessage()));
        }
        d->timer->start(timeout);
    } else if (d->timer) {
        delete d->timer;
        d->timer = 0;
    }

    hideOrShow();
}

/*!
    Removes any temporary message being shown.

    \sa currentMessage(), showMessage(), removeWidget()
*/

void QStatusBar::clearMessage()
{
    Q_D(QStatusBar);
    if (d->tempItem.isEmpty())
        return;
    if (d->timer) {
        delete d->timer;
        d->timer = 0;
    }
    d->tempItem.clear();
    hideOrShow();
}

/*!
    Returns the temporary message currently shown,
    or an empty string if there is no such message.

    \sa showMessage()
*/
QString QStatusBar::currentMessage() const
{
    Q_D(const QStatusBar);
    return d->tempItem;
}

/*!
    \fn void QStatusBar::message(const QString &message, int timeout)

    Use the showMessage() function instead.
*/

/*!
    \fn void QStatusBar::clear()

    Use the clearMessage() function instead.
*/

/*!
    \fn QStatusBar::messageChanged(const QString &message)

    This signal is emitted whenever the temporary status message
    changes. The new temporary message is passed in the \a message
    parameter which is a null-string when the message has been
    removed.

    \sa showMessage(), clearMessage()
*/

/*!
    Ensures that the right widgets are visible.

    Used by the showMessage() and clearMessage() functions.
*/
void QStatusBar::hideOrShow()
{
    Q_D(QStatusBar);
    bool haveMessage = !d->tempItem.isEmpty();

    QStatusBarPrivate::SBItem* item = 0;
    for (int i=0; i<d->items.size(); ++i) {
        item = d->items.at(i);
        if (!item || item->p)
            break;
        if (haveMessage)
            item->w->hide();
        else
            item->w->show();
    }

    emit messageChanged(d->tempItem);
    repaint();
}


/*!
    \reimp
    \fn void QStatusBar::paintEvent(QPaintEvent *event)

    Shows the temporary message, if appropriate, in response to the
    paint \a event.
*/
void QStatusBar::paintEvent(QPaintEvent *)
{
    Q_D(QStatusBar);
    bool haveMessage = !d->tempItem.isEmpty();

    QPainter p(this);
    QStatusBarPrivate::SBItem* item = 0;

    bool rtl = layoutDirection() == Qt::RightToLeft;

    int left = 6;
    int right = width() - 12;

#ifndef QT_NO_SIZEGRIP
    if (d->resizer && d->resizer->isVisible()) {
        if (rtl)
            left = d->resizer->x() + d->resizer->width();
        else
            right = d->resizer->x();
    }
#endif

    for (int i=0; i<d->items.size(); ++i) {
        item = d->items.at(i);
        if (!item)
            break;
        if (!haveMessage || item->p)
            if (item->w->isVisible()) {
                if (item->p) {
                    if (rtl)
                        left = qMax(left, item->w->x() + item->w->width() + 2);
                    else
                        right = qMin(right, item->w->x()-1);
                }
                QStyleOption opt(0);
                opt.rect.setRect(item->w->x() - 1, item->w->y() - 1,
                                 item->w->width() + 2, item->w->height() + 2);
                opt.palette = palette();
                opt.state = QStyle::State_None;
                style()->drawPrimitive(QStyle::PE_FrameStatusBar, &opt, &p, item->w);
            }
    }
    if (haveMessage) {
        p.setPen(palette().foreground().color());
        p.drawText(left, 0, right-left, height(), Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, d->tempItem);
    }
}

/*!
    \reimp
*/
void QStatusBar::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);
}

/*!
    \reimp
*/

bool QStatusBar::event(QEvent *e)
{
    Q_D(QStatusBar);

    if (e->type() == QEvent::LayoutRequest
#ifdef QT3_SUPPORT
        || e->type() == QEvent::LayoutHint
#endif
        ) {
        // Calculate new strut height and call reformat() if it has changed
        int maxH = fontMetrics().height();

        QStatusBarPrivate::SBItem* item = 0;
        for (int i=0; i<d->items.size(); ++i) {
            item = d->items.at(i);
            if (!item)
                break;
            int itemH = qMin(qSmartMinSize(item->w).height(), item->w->maximumHeight());
            maxH = qMax(maxH, itemH);
        }

#ifndef QT_NO_SIZEGRIP
        if (d->resizer)
            maxH = qMax(maxH, d->resizer->sizeHint().height());
#endif

        if (maxH != d->savedStrut)
            reformat();
        else
            update();
    }
    if (e->type() == QEvent::ChildRemoved) {
        QStatusBarPrivate::SBItem* item = 0;
        for (int i=0; i<d->items.size(); ++i) {
            item = d->items.at(i);
            if (!item)
                break;
            if (item->w == ((QChildEvent*)e)->child()) {
                d->items.removeAt(i);
                delete item;
            }
        }
    }
    return QWidget::event(e);
}

#endif
