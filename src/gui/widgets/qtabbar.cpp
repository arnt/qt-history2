/****************************************************************************
 **
 ** Implementation of QTabBar class.
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

#include "qapplication.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qevent.h"
#include "qicon.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include <private/qinternal_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

#include "qdebug.h"
#include "private/qwidget_p.h"


class QTabBarPrivate  : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QTabBar)
public:
    QTabBarPrivate()
        :currentIndex(-1), pressedIndex(-1),
         shape(QTabBar::RoundedAbove), layoutDirty(false), scrollOffset(0){}

    int currentIndex;
    int pressedIndex;
    QTabBar::Shape shape;
    bool layoutDirty;
    int scrollOffset;

    struct Tab {
        inline Tab():enabled(true), shortcutId(0){}
        inline Tab(const QIcon &ico,  const QString &txt):enabled(true), shortcutId(0), text(txt), icon(ico){}
        bool enabled;
        int shortcutId;
        QString text, toolTip;
        QIcon icon;
        QRect rect;
    };
    QList<Tab> tabList;

    void init();
    int extraWidth() const;

    Tab *at(int index);
    const Tab *at(int index) const;

    int indexAtPos(const QPoint &p) const;

    inline bool validIndex(int index) const { return index >= 0 && index < tabList.count(); }

    QToolButton* rightB;
    QToolButton* leftB;
    void scrollTabs(); // private slot

    void refresh();
    void layoutTabs();

    void makeVisible(int index);
    QStyleOptionTab getStyleOption(int tab) const;
};


#define d d_func()
#define q q_func()
QStyleOptionTab QTabBarPrivate::getStyleOption(int tab) const
{
    QStyleOptionTab opt;
    const QTabBarPrivate::Tab *ptab = &tabList.at(tab);
    opt.rect = q->tabRect(tab);
    opt.palette = q->palette();
    opt.fontMetrics = q->fontMetrics();
    opt.direction = q->layoutDirection();
    bool isCurrent = tab == currentIndex;
    opt.state = QStyle::Style_None;
    opt.row = 0;
    if (tab == pressedIndex)
        opt.state |= QStyle::Style_Sunken;
    if (isCurrent)
        opt.state |= QStyle::Style_Selected;
    if (isCurrent && q->hasFocus())
        opt.state |= QStyle::Style_HasFocus;
    if (q->isEnabled() && ptab->enabled)
        opt.state |= QStyle::Style_Enabled;
    if (q->isActiveWindow())
        opt.state |= QStyle::Style_Active;
    if (opt.rect.contains(q->mapFromGlobal(QCursor::pos())))
        opt.state |= QStyle::Style_MouseOver;
    opt.shape = shape;
    opt.text = ptab->text;
    opt.icon = ptab->icon;

    int totalTabs = tabList.size();

    if (tab > 0 && tab - 1 == currentIndex)
        opt.selectedPosition = QStyleOptionTab::PreviousIsSelected;
    else if (tab < totalTabs - 1 && tab + 1 == currentIndex)
        opt.selectedPosition = QStyleOptionTab::NextIsSelected;
    else
        opt.selectedPosition = QStyleOptionTab::NotAdjacent;

    if (tab == 0) {
        if (totalTabs > 1)
            opt.position = QStyleOptionTab::Beginning;
        else
            opt.position = QStyleOptionTab::OnlyOneTab;
    } else if (tab == totalTabs - 1) {
        opt.position = QStyleOptionTab::End;
    } else {
        opt.position = QStyleOptionTab::Middle;
    }

    return opt;
}

/*!
    \class QTabBar
    \brief The QTabBar class provides a tab bar, e.g. for use in tabbed dialogs.

    \ingroup advanced
    \mainclass

    QTabBar is straightforward to use; it draws the tabs using one of
    the predefined \link QTabBar::Shape shapes\endlink, and emits a
    signal when a tab is selected. It can be subclassed to tailor the
    look and feel. Qt also provides a ready-made \l{QTabWidget}.

    Each tab has a tabText(), an optional tabIcon(), and an optional
    tabToolTip(). The tabs's attributes can be changed with
    setTabText(), setTabIcon(), and setTabToolTip(). Each tabs can be
    enabled or disabled individually with setTabEnabled().

    Tabs are added using addTab(), or inserted at particular positions
    using insertTab(). The total number of tabs is given by
    count(). Tabs can be removed from the tab bar with
    removeTab(). Combining removeTab() and insertTab() allows you to
    move tabs to different positions.

    The \l shape property defines the tabs' appearance. The choice of
    shape is a matter of taste, although tab dialogs (for preferences
    and similar) invariably use \c RoundedAbove; nobody uses \c
    TriangularAbove. Tab controls in windows other than dialogs almost
    always use either \c RoundedBelow or \c TriangularBelow. Many
    spreadsheets and other tab controls in which all the pages are
    essentially similar use \c TriangularBelow, whereas \c
    RoundedBelow is used mostly when the pages are different (e.g. a
    multi-page tool palette). The default in QTabBar is \c
    RoundedAbove.

    The most important part of QTabBar's API is the currentChanged()
    signal.  This is emitted whenever the current tab changes (even at
    startup, when the current tab changes from 'none'). There is also
    a slot, setCurrentIndex(), which can be used to select a tab
    programmatically. The function currentIndex() returns the index of
    the current tab, \l count holds the number of tabs.

    QTabBar creates automatic mnemonic keys in the manner of QButton;
    e.g. if a tab's label is "\&Graphics", Alt+G becomes a shortcut
    key for switching to that tab.

    The following virtual functions may need to be reimplemented in
    order to tailor the look and feel or store extra data with each
    tab:

    \list
    \i tabSizeHint() calcuates the size of a tab.
    \i tabInserted() notifies that a new tab was added.
    \i tabRemoved() notifies that a tab was removed.
    \i tabLayoutChange() notifies that the tabs have been re-laid out.
    \i paintEvent() paints all tabs.
    \endlist

    For subclasses, you might also need the tabRect() functions which
    returns the visual geometry of a single tab.

    \inlineimage qtabbar-m.png Screenshot in Motif style
    \inlineimage qtabbar-w.png Screenshot in Windows style
*/

/*!
    \enum QTabBar::Shape

    This enum type lists the built-in shapes supported by QTabBar:

    \value RoundedAbove  the normal rounded look above the pages

    \value RoundedBelow  the normal rounded look below the pages

    \value TriangularAbove  triangular tabs above the pages (very
    unusual; included for completeness)

    \value TriangularBelow  triangular tabs similar to those used in
    the Excel spreadsheet, for example
*/

/*!
    \fn void QTabBar::currentChanged(int index)

    This signal is emitted when the tab bar's current tab changes. The
    new current has the given \a index.
*/

int QTabBarPrivate::extraWidth() const
{
    return 2 * qMax(q->style()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, q),
                    QApplication::globalStrut().width());
}

void QTabBarPrivate::init()
{
    leftB = new QToolButton(Qt::LeftArrow, q);
    QObject::connect(leftB, SIGNAL(clicked()), q, SLOT(scrollTabs()));
    leftB->hide();
    rightB = new QToolButton(Qt::RightArrow, q);
    QObject::connect(rightB, SIGNAL(clicked()), q, SLOT(scrollTabs()));
    rightB->hide();
    q->setFocusPolicy(Qt::TabFocus);
    q->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QTabBarPrivate::Tab *QTabBarPrivate::at(int index)
{
    return validIndex(index)?&tabList[index]:0;
}

const QTabBarPrivate::Tab *QTabBarPrivate::at(int index) const
{
    return validIndex(index)?&tabList[index]:0;
}

int QTabBarPrivate::indexAtPos(const QPoint &p) const
{
    if (q->tabRect(currentIndex).contains(p))
        return currentIndex;
    for (int i = 0; i < tabList.count(); ++i)
        if (tabList.at(i).enabled && q->tabRect(i).contains(p))
            return i;
    return -1;
}


void QTabBarPrivate::layoutTabs()
{
    scrollOffset = 0;
    layoutDirty = false;
    int x = 0;
    for (int i = 0; i < tabList.count(); ++i) {
        QSize sz = q->tabSizeHint(i);
        tabList[i].rect = QRect(x, 0, sz.width(), sz.height());
        x += sz.width();
    }

    QSize size = q->size();
    if (tabList.count() && tabList.last().rect.right() > size.width()) {
        int extra = extraWidth();
        QRect arrows = QStyle::visualRect(q->layoutDirection(), q->rect(),  QRect(size.width() - extra, 0, extra, size.height()));
        d->leftB->setGeometry(arrows.left(), arrows.top(), extra/2, arrows.height());
        d->rightB->setGeometry(arrows.right() - extra/2 + 1, arrows.top(), extra/2, arrows.height());
        d->leftB->setEnabled(scrollOffset > 0);
        d->rightB->setEnabled(tabList.last().rect.right() - scrollOffset >= size.width() - extra);
        d->leftB->show();
        d->rightB->show();
    } else {
        rightB->hide();
        leftB->hide();
    }

    q->tabLayoutChange();
}

void QTabBarPrivate::makeVisible(int index)
{
    if (!validIndex(index) || leftB->isHidden())
        return;
    QRect tabRect = tabList.at(index).rect;

    int oldScrollOffset = scrollOffset;
    int availableWidth = q->width() - extraWidth();
    if (tabRect.left() < scrollOffset) // too far left
        scrollOffset = tabRect.left() - (index?8:0);
    else if (tabRect.right() > scrollOffset + availableWidth) // too far right
        scrollOffset = tabRect.right() - availableWidth + 1;

    if (scrollOffset && tabRect.right() < availableWidth)  // need scrolling at all?
        scrollOffset = 0;

    d->leftB->setEnabled(scrollOffset > 0);
    d->rightB->setEnabled(tabList.last().rect.right() - scrollOffset >= availableWidth);
    if (oldScrollOffset != scrollOffset)
        q->update();

}

void QTabBarPrivate::scrollTabs()
{
    const QObject *sender = q->sender();
    int i = -1;
    if (sender == leftB) {
        for (i = d->tabList.count() - 1; i >= 0; --i) {
            if (tabList.at(i).rect.left() - scrollOffset < 0) {
                makeVisible(i);
                return;
            }
        }
    } else if (sender == rightB) {
        int availableWidth = q->width() - extraWidth();
        for (i = 0; i < tabList.count(); ++i) {
            if (tabList.at(i).rect.right() - scrollOffset > availableWidth) {
                makeVisible(i);
                return;
            }
        }
    }
}

void QTabBarPrivate::refresh()
{
    if (!q->isVisible()) {
        layoutDirty = true;
    } else {
        layoutTabs();
        q->update();
        q->updateGeometry();
    }
}

/*!
    Creates a new tab bar with the given \a parent.
*/
QTabBar::QTabBar(QWidget* parent)
    :QWidget(*new QTabBarPrivate, parent, 0)
{
    d->init();
}


/*!
    Destroys the tab bar.
*/
QTabBar::~QTabBar()
{
}

/*!
    \property QTabBar::shape
    \brief the shape of the tabs in the tab bar

    The value of this property is one of the following: \c
    RoundedAbove (default), \c RoundedBelow, \c TriangularAbove or \c
    TriangularBelow.

    \sa Shape
*/


QTabBar::Shape QTabBar::shape() const
{
    return d->shape;
}

void QTabBar::setShape(Shape shape)
{
    if (d->shape == shape)
        return;
    d->shape = shape;
    d->refresh();
}

/*!
    Adds a new tab with text \a text. Returns the new
    tab's index.
*/
int QTabBar::addTab(const QString &text)
{
    return insertTab(-1, text);
}

/*!
    \overload

    Adds a new tab with icon \a icon and text \a
    text. Returns the new tab's index.
*/
int QTabBar::addTab(const QIcon& icon, const QString &text)
{
    return insertTab(-1, icon, text);
}

/*!
    Inserts a new tab with text \a text at position \a index. If \a
    index is out of range, the new tab is appened. Returns the new
    tab's index.
*/
int QTabBar::insertTab(int index, const QString &text)
{
    return insertTab(index, QIcon(), text);
}

/*!\overload

    Inserts a new tab with icon \a icon and text \a text at position
    \a index. If \a index is out of range, the new tab is
    appended. Returns the new tab's index.
*/
int QTabBar::insertTab(int index, const QIcon& icon, const QString &text)
{
    if (!d->validIndex(index)) {
        index = d->tabList.count();
        d->tabList.append(QTabBarPrivate::Tab(icon, text));
    } else {
        d->tabList.insert(index, QTabBarPrivate::Tab(icon, text));
    }
    d->tabList[index].shortcutId = grabShortcut(QKeySequence::mnemonic(text));
    d->refresh();
    tabInserted(index);
    return index;
}


/*!
    Removes the tab at position \a index.
 */
void QTabBar::removeTab(int index)
{
    if (d->validIndex(index)) {
        releaseShortcut(d->tabList.at(index).shortcutId);
        d->tabList.removeAt(index);
        if (index == d->currentIndex)
            setCurrentIndex(d->validIndex(index+1)?index+1:0);
        d->refresh();
        tabRemoved(index);
    }
}


/*!
    Returns true if the tab at position \a index is enabled; otherwise
    returns false.
*/
bool QTabBar::isTabEnabled(int index) const
{
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->enabled;
    return false;
}

/*!
    If \a enabled is true then the tab at position \a index is
    enabled; otherwise the item at position \a index is disabled.
*/
void QTabBar::setTabEnabled(int index, bool enabled)
{
    if (QTabBarPrivate::Tab *tab = d->at(index)) {
        tab->enabled = enabled;
        update();
        if (!enabled && index == d->currentIndex)
            setCurrentIndex(d->validIndex(index+1)?index+1:0);
        else if (enabled && !d->validIndex(d->currentIndex))
            setCurrentIndex(index);
    }
}


/*!
    Returns the text of the tab at position \a index, or an empty
    string if \a index is out of range.
*/
QString QTabBar::tabText(int index) const
{
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->text;
    return QString();
}

/*!
    Sets the text of the tab at position \a index to \a text.
*/
void QTabBar::setTabText(int index, const QString &text)
{
    if (QTabBarPrivate::Tab *tab = d->at(index)) {
        tab->text = text;
        releaseShortcut(tab->shortcutId);
        tab->shortcutId = grabShortcut(QKeySequence::mnemonic(text));
        d->refresh();
    }
}


/*!
    Returns the icon of the tab at position \a index, or a null icon
    if \a index is out of range.
*/
QIcon QTabBar::tabIcon(int index) const
{
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->icon;
    return QIcon();
}

/*!
    Sets the icon of the tab at position \a index to \a icon.
*/
void QTabBar::setTabIcon(int index, const QIcon & icon)
{
    if (QTabBarPrivate::Tab *tab = d->at(index))
        tab->icon = icon;
}


/*!
    Sets the tool tip of the tab at position \a index to \a tip.
*/
void QTabBar::setTabToolTip(int index, const QString & tip)
{
    if (QTabBarPrivate::Tab *tab = d->at(index))
        tab->toolTip = tip;
}

/*!
    Returns the tool tip of the tab at position \a index, or an empty
    string if \a index is out of range.
*/
QString QTabBar::tabToolTip(int index) const
{
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->toolTip;
    return QString();
}


/*!
    Returns the visual rectangle of the of the tab at position \a
    index, or a null rectangle if \a index is out of range.
*/
QRect QTabBar::tabRect(int index) const
{
    if (const QTabBarPrivate::Tab *tab = d->at(index)) {
        if (d->layoutDirty)
            const_cast<QTabBarPrivate*>(d)->layoutTabs();
        QRect r = tab->rect;
        r.translate(-d->scrollOffset, 0);
        return QStyle::visualRect(layoutDirection(), rect(), r);
    }
    return QRect();
}

/*!
    \property QTabBar::currentIndex
    \brief the index of the tab bar's visible tab
*/

int QTabBar::currentIndex() const
{
    if (d->validIndex(d->currentIndex))
        return d->currentIndex;
    return -1;
}


void QTabBar::setCurrentIndex(int index)
{
    if (d->validIndex(index)) {
        d->currentIndex = index;
        update();
        d->makeVisible(index);
#ifdef QT_COMPAT
        emit selected(index);
#endif
        emit currentChanged(index);
    }
}


/*!
    \property QTabBar::count
    \brief the number of tabs in the tab bar
*/

int QTabBar::count() const
{
    return d->tabList.count();
}


/*!\reimp
 */
QSize QTabBar::sizeHint() const
{
    if (d->layoutDirty)
        const_cast<QTabBarPrivate*>(d)->layoutTabs();
    QRect r;
    for (int i = 0; i < d->tabList.count(); ++i)
        r = r.unite(d->tabList.at(i).rect);
    return r.size().expandedTo(QApplication::globalStrut());
}

/*!\reimp
 */
QSize QTabBar::minimumSizeHint() const
{
    if (style()->styleHint(QStyle::SH_TabBar_PreferNoArrows, 0, this))
        return sizeHint();
    return QSize(d->rightB->sizeHint().width() * 2 + 75, sizeHint().height());
}

/*!
    Returns the size hint for the tab at position \a index.
*/
QSize QTabBar::tabSizeHint(int index) const
{
    if (const QTabBarPrivate::Tab *tab = d->at(index)) {
        QSize iconSize = tab->icon.isNull() ? QSize()
                                            : tab->icon.pixmapSize(Qt::SmallIconSize);
        QStyleOptionTab opt = d->getStyleOption(index);
        int hframe  = style()->pixelMetric(QStyle::PM_TabBarTabHSpace, &opt, this);
        int vframe  = style()->pixelMetric(QStyle::PM_TabBarTabVSpace, &opt, this);
        const QFontMetrics fm = fontMetrics();
        QSize csz(fm.size(Qt::TextShowMnemonic, tab->text).width() + iconSize.width() + hframe,
                  qMax(fm.height(), iconSize.height()) + vframe);
        return style()->sizeFromContents(QStyle::CT_TabBarTab, &opt, csz, this);
    }
    return QSize();
}

/*!
  This virtual handler is called after a new tab was added or
  inserted at position \a index.

  \sa tabRemoved()
 */
void QTabBar::tabInserted(int index)
{
    Q_UNUSED(index)
}

/*!
  This virtual handler is called after a tab was removed from
  position \a index.

  \sa tabInserted()
 */
void QTabBar::tabRemoved(int index)
{
    Q_UNUSED(index)
}

/*!
  This virtual handler is called whenever the tab layout changes.

  \sa tabRect()
 */
void QTabBar::tabLayoutChange()
{
}


/*!\reimp
 */
void QTabBar::showEvent(QShowEvent *)
{
    if (d->layoutDirty)
        d->layoutTabs();
    if (!d->validIndex(d->currentIndex))
        setCurrentIndex(0);

}


/*!\reimp
 */
bool QTabBar::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        if (const QTabBarPrivate::Tab *tab = d->at(d->indexAtPos(static_cast<QHelpEvent*>(e)->pos()))) {
            if (!tab->toolTip.isEmpty()) {
                QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(), tab->toolTip, this);
                return true;
            }
        }
    } else if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        for (int i = 0; i < d->tabList.count(); ++i) {
            const QTabBarPrivate::Tab *tab = &d->tabList.at(i);
            if (tab->shortcutId == se->shortcutId()) {
                setCurrentIndex(i);
                return true;
            }
        }
    }
    return QWidget::event(e);
}

/*!\reimp
 */
void QTabBar::resizeEvent(QResizeEvent *)
{
    d->layoutTabs();
    d->makeVisible(d->currentIndex);
}


/*!\reimp
 */
void QTabBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int selected = -1;
    for (int i = 0; i < d->tabList.count(); ++i) {
        if (i == d->currentIndex) {
            selected = i;
            continue;
        }
        QStyleOptionTab opt = d->getStyleOption(i);
        style()->drawControl(QStyle::CE_TabBarTab, &opt, &p, this);
        style()->drawControl(QStyle::CE_TabBarLabel, &opt, &p, this);
    }

    // Draw the selected tab last to get it "on top"
    if (selected >= 0) {
        QStyleOptionTab opt = d->getStyleOption(selected);
        style()->drawControl(QStyle::CE_TabBarTab, &opt, &p, this);
        style()->drawControl(QStyle::CE_TabBarLabel, &opt, &p, this);
    }

}

/*!\reimp
 */
void QTabBar::mousePressEvent (QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    d->pressedIndex = d->indexAtPos(e->pos());
    if (d->pressedIndex >= 0) {
        if (e->type() == style()->styleHint(QStyle::SH_TabBar_SelectMouseType, 0, this))
            setCurrentIndex(d->pressedIndex);
        else
            repaint(tabRect(d->pressedIndex));
    }
}

/*!\reimp
 */
void QTabBar::mouseMoveEvent (QMouseEvent *e)
{
    if (e->buttons() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    if (style()->styleHint(QStyle::SH_TabBar_SelectMouseType, 0, this)
            == QEvent::MouseButtonRelease) {
        int i = d->indexAtPos(e->pos());
        if (i != d->pressedIndex) {
            if (d->pressedIndex >= 0)
                repaint(tabRect(d->pressedIndex));
            if ((d->pressedIndex = i) >= 0)
                repaint(tabRect(i));
        }
    }
}

/*!\reimp
 */
void QTabBar::mouseReleaseEvent (QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        e->ignore();

    int i = d->indexAtPos(e->pos()) == d->pressedIndex ? d->pressedIndex : -1;
    d->pressedIndex = -1;
    if (e->type() == style()->styleHint(QStyle::SH_TabBar_SelectMouseType, 0, this))
        setCurrentIndex(i);
}

/*!\reimp
 */
void QTabBar::keyPressEvent(QKeyEvent *e)
{
    if (e->key() != Qt::Key_Left && e->key() != Qt::Key_Right) {
        e->ignore();
        return;
    }
    int dx = e->key() == (isRightToLeft() ? Qt::Key_Right : Qt::Key_Left) ? -1 : 1;
    for (int index = d->currentIndex + dx; d->validIndex(index); index += dx) {
        if (d->tabList.at(index).enabled) {
            setCurrentIndex(index);
            break;
        }
    }
}

/*!\reimp
 */
void QTabBar::changeEvent(QEvent *e)
{
    d->refresh();
    QWidget::changeEvent(e);
}


/*!
    \fn void QTabBar::setCurrentTab(int index)

    Use setCurrentIndex() instead.
*/

/*!
    \fn void QTabBar::selected(int index);

    Use currentChanged() instead.
*/


#include "moc_qtabbar.cpp"
