/****************************************************************************
**
** Implementation of QTabWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtabwidget.h"
#ifndef QT_NO_TABWIDGET
#include "private/qwidget_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qstackedbox.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qtoolbutton.h"

/*!
    \class QTabWidget qtabwidget.h
    \brief The QTabWidget class provides a stack of tabbed widgets.

    \ingroup organizers
    \ingroup advanced
    \mainclass

    A tab widget provides a tab bar of tabs and a `page area' below
    (or above, see \l{TabPosition}) the tabs. Each tab is associated
    with a different widget (called a `page'). Only the current tab's
    widget is shown in the page area; all the other tabs' widgets are
    hidden. The user can show a different page by clicking on its tab
    or by pressing its Alt+\e{letter} shortcut if it has one.

    The normal way to use QTabWidget is to do the following in the
    constructor:
    \list 1
    \i Create a QTabWidget.
    \i Create a QWidget for each of the pages in the tab dialog,
    insert children into it, set up geometry management for it and use
    addTab() (or insertTab()) to set up a tab and keyboard shortcut
    for it.
    \i Connect to the signals and slots.
    \endlist

    The position of the tabs is defined by \l tabPosition, their shape
    by \l tabShape.

    The signal currentChanged() is emitted when the user selects a
    page.

    The current page index is available as currentIndex(), the current
    page widget with currentWidget().  You can retrieve a pointer to a
    page widget with a given index using widget(), and can find the
    index position of a widget with indexOf(). Use setCurrentIndex()
    to show a particular page.

    You can change a tab's text and icon using setTabText() or
    setTabIcon(). A tab can be removed with removeTab().

    Each tab is either enabled or disabled at any given time (see
    setTabEnabled()). If a tab is enabled, the tab text is drawn
    normally and the user can select that tab. If it is disabled, the
    tab is drawn in a different way and the user cannot select that
    tab. Note that even if a tab is disabled, the page can still be
    visible, for example if all of the tabs happen to be disabled.

    Tab widgets can be a very good way to split up a complex dialog.
    An alternative is to use a QStackedBox for which you provide some
    means of navigating between pages, for example, a QToolBar or a
    QListBox.

    Most of the functionality in QTabWidget is provided by a QTabBar
    (at the top, providing the tabs) and a QStackedBox (most of the
    area, organizing the individual pages).

    \inlineimage qtabwidget-m.png Screenshot in Motif style
    \inlineimage qtabwidget-w.png Screenshot in Windows style

    \sa QTabBar QStackedBox QToolBox
*/

/*!
  \enum Qt::Corner

  This enum type specifies a corner in a rectangle:

  \value TopLeftCorner     The top-left corner of the rectangle.
  \value TopRightCorner    The top-right corner of the rectangle.
  \value BottomLeftCorner  The bottom-left corner of the rectangle.
  \value BottomRightCorner The bottom-right corner of the rectangle.
*/

/*!
    \enum QTabWidget::TabPosition

    This enum type defines where QTabWidget draws the tab row:
    \value Top  above the pages
    \value Bottom  below the pages
*/

/*!
    \enum QTabWidget::TabShape

    This enum type defines the shape of the tabs:
    \value Rounded  rounded look (normal)
    \value Triangular  triangular look (very unusual, included for completeness)
*/

/* undocumented now
  \obsolete

  \fn void QTabWidget::selected(const QString &tabLabel);

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/


/*!
    \fn void QTabWidget::currentChanged(int);

    This signal is emitted whenever the current page index
    changes. The parameter is the new current page index.

    \sa currentWidget() currentIndex
*/


class QTabWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QTabWidget)
public:
    QTabWidgetPrivate();
    ~QTabWidgetPrivate();
    void showTab(int);
    void removeTab(int);
    void init();
    QTabBar* tabs;
    QStackedBox* stack;
    bool dirty;
    QTabWidget::TabPosition pos;
    QTabWidget::TabShape shape;
    int alignment;
    QWidget* leftCornerWidget;
    QWidget* rightCornerWidget;
    QRect paneRect;
};

#define d d_func()
#define q q_func()

QTabWidgetPrivate::QTabWidgetPrivate()
    : tabs(0), stack(0), dirty(true),
      pos(QTabWidget::Top), shape(QTabWidget::Rounded),
      leftCornerWidget(0), rightCornerWidget(0)
{}

QTabWidgetPrivate::~QTabWidgetPrivate()
{}

void QTabWidgetPrivate::init()
{
    stack = new QStackedBox(q);
    QObject::connect(stack, SIGNAL(widgetRemoved(int)), q, SLOT(removeTab(int)));
    q->setTabBar(new QTabBar(q));

    stack->setFrameStyle(QFrame::Box | QFrame::Plain);
#ifdef Q_OS_TEMP
    pos = QTabWidget::Bottom;
#endif

    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    q->setFocusPolicy(Qt::TabFocus);
    q->setFocusProxy(tabs);
}

/*!
    Constructs a tabbed widget with parent \a parent.
*/
QTabWidget::QTabWidget(QWidget *parent)
    : QWidget(*new QTabWidgetPrivate, parent, 0)
{
    d->init();
}

#ifdef QT_COMPAT
QTabWidget::QTabWidget(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(*new QTabWidgetPrivate, parent, f)
{
    setObjectName(name);
    d->init();
}
#endif

/*!
    Destroys the tabbed widget.
*/
QTabWidget::~QTabWidget()
{
}

/*!
    Adds another tab and page to the tab view.

    The new page is \a child; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as a shortcut for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes a shortcut which will
    move the focus to this tab.

    \sa insertTab()
*/
int QTabWidget::addTab(QWidget *child, const QString &label)
{
    return insertTab(-1, child, label);
}


/*!
    \overload

    Adds another tab and page to the tab view.

    This function is the same as addTab(), but with an additional \a
    iconset.
*/
int QTabWidget::addTab(QWidget *child, const QIconSet& iconset, const QString &label)
{
    return insertTab(-1, child, iconset, label);
}


/*!
    Inserts another tab and page to the tab view.

    The new page is \a w; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as a shortcut for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes a shortcut which will
    move the focus to this tab.

    If \a index is out of range, the tab is simply appended.
    Otherwise it is inserted at the specified position.

    If you call insertTab() after show(), the screen will flicker and
    the user may be confused.

    \sa addTab()
*/
int QTabWidget::insertTab(int index, QWidget *w, const QString &label)
{
    return insertTab(index, w, QIconSet(), label);
}


/*!
    \overload

    Inserts another tab and page to the tab view.

    This function is the same as insertTab(), but with an additional
    \a icon.
*/
int QTabWidget::insertTab(int index, QWidget *w, const QIconSet& icon, const QString &label)
{
    index = d->tabs->insertTab(index, icon, label);
    d->stack->insertWidget(index, w);
    setUpLayout();
    tabInserted(index);
    return index;
}


/*!
    Defines a new \a label for the page at position \a index's tab.
*/
void QTabWidget::setTabText(int index, const QString &label)
{
    d->tabs->setTabText(index, label);
    setUpLayout();
}

/*!
    Returns the label text for the tab on the page at position \a index.
*/

QString QTabWidget::tabText(int index) const
{
    return d->tabs->tabText(index);
}

/*!
    \overload

    Defines a new \a iconset and a new \a label for the page at position \a index's tab.
*/
void QTabWidget::setTabIcon(int index, const QIconSet &icon)
{
    d->tabs->setTabIcon(index, icon);
    setUpLayout();
}

/*!
    Returns the label text for the tab on the page at position \a index.
*/

QIconSet QTabWidget::tabIcon(int index) const
{
    return d->tabs->tabIcon(index);
}

/*!
    Returns true if the the page at position \a index is enabled; otherwise returns false.

    \sa setTabEnabled(), QWidget::isEnabled()
*/

bool QTabWidget::isTabEnabled(int index) const
{
    return d->tabs->isTabEnabled(index);
}

/*!
    If \a enable is true, the page at position \a index is enabled; otherwise the page at position \a index is
    disabled. The page's tab is redrawn appropriately.

    QTabWidget uses QWidget::setEnabled() internally, rather than
    keeping a separate flag.

    Note that even a disabled tab/page may be visible. If the page is
    visible already, QTabWidget will not hide it; if all the pages are
    disabled, QTabWidget will show one of them.

    \sa isTabEnabled(), QWidget::setEnabled()
*/

void QTabWidget::setTabEnabled(int index, bool enable)
{
    d->tabs->setTabEnabled(index, enable);
}

/*!
  Sets widget \a w to be the shown in the specified \a corner of the
  tab widget.

  Only the horizontal element of the \a corner will be used.

  \sa cornerWidget(), setTabPosition()
*/
void QTabWidget::setCornerWidget(QWidget * w, Qt::Corner corner)
{
    if (!w)
        return;
    if ((uint)corner & 1)
        d->rightCornerWidget = w;
    else
        d->leftCornerWidget = w;
}

/*!
    Returns the widget shown in the \a corner of the tab widget or 0.
*/
QWidget * QTabWidget::cornerWidget(Qt::Corner corner) const
{
    if ((uint)corner & 1)
        return d->rightCornerWidget;
    return d->leftCornerWidget;
}

/*!
   Removes the page at position \a index from this stack of
   widgets. Does not delete the page widget.
*/
void QTabWidget::removeTab(int index)
{
    if (QWidget *w = d->stack->widget(index))
        d->stack->removeWidget(w);
}

/*!
    Returns a pointer to the page currently being displayed by the tab
    dialog. The tab dialog does its best to make sure that this value
    is never 0 (but if you try hard enough, it can be).
*/

QWidget * QTabWidget::currentWidget() const
{
    return d->stack->currentWidget();
}

/*!
    \property QTabWidget::autoMask
    \brief whether the tab widget is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QTabWidget::currentIndex
    \brief the index position of the current tab page

*/

int QTabWidget::currentIndex() const
{
    return d->tabs->currentIndex();
}

void QTabWidget::setCurrentIndex(int index)
{
    d->tabs->setCurrentIndex(index);
}


/*!
    Returns the index position of the page at position \a index, or -1 if the widget
    cannot be found.
*/
int QTabWidget::indexOf(QWidget* w) const
{
    return d->stack->indexOf(w);
}


/*!
    \reimp
*/
void QTabWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    setUpLayout();
}

/*!
    Replaces the dialog's QTabBar heading with the tab bar \a tb. Note
    that this must be called \e before any tabs have been added, or
    the behavior is undefined.

    \sa tabBar()
*/
void QTabWidget::setTabBar(QTabBar* tb)
{
    if (tb->parentWidget() != this) {
        tb->setParent(this);
        tb->show();
    }
    delete d->tabs;
    d->tabs = tb;
    setFocusProxy(d->tabs);
    connect(d->tabs, SIGNAL(currentChanged(int)),
             this,    SLOT(showTab(int)));
    setUpLayout();
}


/*!
    Returns the current QTabBar.

    \sa setTabBar()
*/
QTabBar* QTabWidget::tabBar() const
{
    return d->tabs;
}

/*!
    Ensures that the selected tab's page is visible and appropriately
    sized.
*/

void QTabWidgetPrivate::showTab(int index)
{
    if (index < stack->count() && index >= 0) {
        stack->setCurrentIndex(index);
        emit q->currentChanged(index);
#ifdef QT_COMPAT
        emit q->currentChanged(stack->widget(index));
#endif
    }
}

void QTabWidgetPrivate::removeTab(int index)
{
    tabs->removeTab(index);
    q->setUpLayout();
    q->tabRemoved(index);
}

/*
    Set up the layout.
*/
void QTabWidget::setUpLayout(bool onlyCheck)
{
    if (onlyCheck && !d->dirty)
        return; // nothing to do

    if (!isVisible()) {
        d->dirty = true;
        return; // we'll do it later
    }

    QSize t(0, d->stack->frameWidth());
    if (d->tabs->isVisibleTo(this))
        t = d->tabs->sizeHint();
    int lcw = 0;
    if (d->leftCornerWidget && d->leftCornerWidget->isVisible() ) {
        QSize sz = d->leftCornerWidget->sizeHint();
        d->leftCornerWidget->resize(sz);
        lcw = sz.width();
        if (t.height() > lcw)
            lcw = t.height();
     }
    int rcw = 0;
    if (d->rightCornerWidget && d->rightCornerWidget->isVisible()) {
        QSize sz = d->rightCornerWidget->sizeHint();
        d->rightCornerWidget->resize(sz);
        rcw = sz.width();
        if (t.height() > rcw)
            rcw = t.height();
    }
    int tw = width() - lcw - rcw;
    if (t.width() > tw)
        t.setWidth(tw);
    int lw = d->stack->lineWidth();
    bool reverse = QApplication::reverseLayout();
    int tabx, taby, stacky, exty, exth, overlap;

    exth = style().pixelMetric(QStyle::PM_TabBarBaseHeight, this);
    overlap = style().pixelMetric(QStyle::PM_TabBarBaseOverlap, this);

    if (reverse)
        tabx = qMin(width() - t.width(), width() - t.width() - lw + 2) - lcw;
    else
        tabx = qMax(0, lw - 2) + lcw;
    if (d->pos == Bottom) {
        taby = height() - t.height() - lw;
        stacky = 0;
        exty = taby - (exth - overlap);
    } else { // Top
        taby = 0;
        stacky = t.height()-lw + (exth - overlap);
        exty = taby + t.height() - overlap;
    }

    // do alignment
    int alignment = style().styleHint(QStyle::SH_TabBar_Alignment, this);
    if (alignment != Qt::AlignLeft && t.width() < width()) {
        if (alignment == Qt::AlignHCenter)
            tabx += (width()-lcw-rcw)/2 - t.width()/2;
        else if (alignment == Qt::AlignRight)
            tabx += width() - t.width() - rcw;
    }
    d->paneRect.setRect(0, exty, width(), exth);
    d->tabs->setGeometry(tabx, taby, t.width(), t.height());

    d->stack->setGeometry(0, stacky, width(), height() - (exth-overlap) -
                           t.height()+qMax(0, lw-2));

    d->dirty = false;

    // move cornerwidgets
    if (d->leftCornerWidget) {
        int y = (t.height() / 2) - (d->leftCornerWidget->height() / 2);
        int x = (reverse ? width() - lcw + y : y);
        d->leftCornerWidget->move(x, y + taby);
    }
    if (d->rightCornerWidget) {
        int y = (t.height() / 2) - (d->rightCornerWidget->height() / 2);
        int x = (reverse ? y : width() - rcw + y);
        d->rightCornerWidget->move(x, y + taby);
    }
    if (!onlyCheck)
        update();
    updateGeometry();
    if (autoMask())
        updateMask();
}

/*!
    \reimp
*/
QSize QTabWidget::sizeHint() const
{
    QSize lc(0, 0), rc(0, 0);

    if (d->leftCornerWidget)
        lc = d->leftCornerWidget->sizeHint();
    if(d->rightCornerWidget)
        rc = d->rightCornerWidget->sizeHint();
    if (!d->dirty) {
        QTabWidget *that = (QTabWidget*)this;
        that->setUpLayout(true);
    }
    QSize s(d->stack->sizeHint());
    QSize t(d->tabs->sizeHint());
    if(!style().styleHint(QStyle::SH_TabBar_PreferNoArrows, d->tabs))
        t = t.boundedTo(QSize(200,200));
    else
        t = t.boundedTo(QApplication::desktop()->size());
    QSize sz(qMax(s.width(), t.width() + rc.width() + lc.width()),
              s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))));
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::Style_Default;
    return style().sizeFromContents(QStyle::CT_TabWidget, &opt, sz, fontMetrics(), this)
                    .expandedTo(QApplication::globalStrut());
}


/*!
    \reimp

    Returns a suitable minimum size for the tab widget.
*/
QSize QTabWidget::minimumSizeHint() const
{
    QSize lc(0, 0), rc(0, 0);

    if(d->leftCornerWidget)
        lc = d->leftCornerWidget->minimumSizeHint();
    if(d->rightCornerWidget)
        rc = d->rightCornerWidget->minimumSizeHint();
    if (!d->dirty) {
        QTabWidget *that = (QTabWidget*)this;
        that->setUpLayout(true);
    }
    QSize s(d->stack->minimumSizeHint());
    QSize t(d->tabs->minimumSizeHint());

    QSize sz(qMax(s.width(), t.width() + rc.width() + lc.width()),
              s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))));
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::Style_Default;
    return style().sizeFromContents(QStyle::CT_TabWidget, &opt, sz, fontMetrics(), this)
                    .expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
 */
void QTabWidget::showEvent(QShowEvent *)
{
    setUpLayout();
}


/*!
    \property QTabWidget::tabPosition
    \brief the position of the tabs in this tab widget

    Possible values for this property are \c QTabWidget::Top and \c
    QTabWidget::Bottom.

    \sa TabPosition
*/
QTabWidget::TabPosition QTabWidget::tabPosition() const
{
    return d->pos;
}

void QTabWidget::setTabPosition(TabPosition pos)
{
    if (d->pos == pos)
        return;
    d->pos = pos;
    if (d->tabs->shape() == QTabBar::TriangularAbove || d->tabs->shape() == QTabBar::TriangularBelow) {
        if (pos == Bottom)
            d->tabs->setShape(QTabBar::TriangularBelow);
        else
            d->tabs->setShape(QTabBar::TriangularAbove);
    }
    else {
        if (pos == Bottom)
            d->tabs->setShape(QTabBar::RoundedBelow);
        else
            d->tabs->setShape(QTabBar::RoundedAbove);
    }
    setUpLayout();
}

/*!
    \property QTabWidget::tabShape
    \brief the shape of the tabs in this tab widget

    Possible values for this property are \c QTabWidget::Rounded
    (default) or \c QTabWidget::Triangular.

    \sa TabShape
*/

QTabWidget::TabShape QTabWidget::tabShape() const
{
    return d->shape;
}

void QTabWidget::setTabShape(TabShape s)
{
    if (d->shape == s)
        return;
    d->shape = s;
    if (d->pos == Top) {
        if (s == Rounded)
            d->tabs->setShape(QTabBar::RoundedAbove);
        else
            d->tabs->setShape(QTabBar::TriangularAbove);
    } else {
        if (s == Rounded)
            d->tabs->setShape(QTabBar::RoundedBelow);
        else
            d->tabs->setShape(QTabBar::TriangularBelow);
    }
    setUpLayout();
}



/*!
    \reimp
 */
void QTabWidget::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        setUpLayout();
    QWidget::changeEvent(ev);
}


/*!
    \reimp
 */
void QTabWidget::updateMask()
{
    if (!autoMask())
        return;

    QRect r;
    QRegion reg(r);
    reg += QRegion(d->tabs->geometry());
    reg += QRegion(d->stack->geometry());
    setMask(reg);
}


/*!
    \reimp
 */
void QTabWidget::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) &&
        count() > 1 &&
        e->state() & Qt::ControlButton) {
        int page = currentIndex();
        if (e->key() == Qt::Key_Backtab || e->state() & Qt::ShiftButton) {
            page--;
            if (page < 0)
                page = count() - 1;
        } else {
            page++;
            if (page >= count())
                page = 0;
        }
        setCurrentIndex(page);
        if (!qApp->focusWidget())
            d->tabs->setFocus();
    }
}

/*!
    Returns the tab page at index position \a index or 0 if the \a
    index is out of range.
*/
QWidget *QTabWidget::widget(int index) const
{
    return d->stack->widget(index);
}

/*!
    \property QTabWidget::count
    \brief the number of tabs in the tab bar
*/
int QTabWidget::count() const
{
    return d->tabs->count();
}


/*!
    Sets the tab tool tip for the page at position \a index to \a tip.

    \sa  tabToolTip()
*/
void QTabWidget::setTabToolTip(int index, const QString & tip)
{
    d->tabs->setTabToolTip(index, tip);
}

/*!
    Returns the tab tool tip for the page at position \a index or
    QString::null if no tool tip has been set.

    \sa setTabToolTip()
*/
QString QTabWidget::tabToolTip(int index) const
{
    return d->tabs->tabToolTip(index);
}

/*!
  This virtual handler is called after a new tab was added or
  inserted at position \a index.

  \sa tabRemoved()
 */
void QTabWidget::tabInserted(int index)
{
    Q_UNUSED(index)
}

/*!
  This virtual handler is called after a tab was removed from
  position \a index.

  \sa tabInserted()
 */
void QTabWidget::tabRemoved(int index)
{
    Q_UNUSED(index)
}

void QTabWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt(0);
    opt.rect = d->paneRect;
    opt.palette = palette();
    opt.state = QStyle::Style_Default;
    if (isEnabled())
        opt.state |= QStyle::Style_Enabled;
    if (tabPosition() == QTabWidget::Top)
        opt.state |= QStyle::Style_Top;
    else if (tabPosition() == QTabWidget::Bottom)
        opt.state |= QStyle::Style_Bottom;
    style().drawPrimitive(QStyle::PE_TabBarBase, &opt, &p, this);
}

#include "moc_qtabwidget.cpp"
#endif
