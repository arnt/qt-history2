/****************************************************************************
 **
 ** Implementation of QTabBar class.
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is part of the $MODULE$ of the Qt Toolkit.
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
#include "qtabwidget.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include "qstylepainter.h"
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
         shape(QTabBar::RoundedNorth),
         layoutDirty(false), drawBase(true), scrollOffset(0){}

    int currentIndex;
    int pressedIndex;
    QTabBar::Shape shape;
    bool layoutDirty;
    bool drawBase;
    int scrollOffset;

    struct Tab {
        inline Tab():enabled(true), shortcutId(0){}
        inline Tab(const QIcon &ico,  const QString &txt):enabled(true), shortcutId(0), text(txt), icon(ico){}
        bool enabled;
        int shortcutId;
        QString text, toolTip;
        QIcon icon;
        QRect rect;
        QVariant data;
    };
    QList<Tab> tabList;

    void init();
    int extraWidth() const;

    Tab *at(int index);
    const Tab *at(int index) const;

    int indexAtPos(const QPoint &p) const;

    inline bool validIndex(int index) const { return index >= 0 && index < tabList.count(); }

    QToolButton* rightB; // right or bottom
    QToolButton* leftB; // left or top
    void scrollTabs(); // private slot
    QRect hoverRect;

    void refresh();
    void layoutTabs();

    void makeVisible(int index);
    QStyleOptionTab getStyleOption(int tab) const;
};


QStyleOptionTab QTabBarPrivate::getStyleOption(int tab) const
{
    Q_Q(const QTabBar);
    QStyleOptionTab opt;
    const QTabBarPrivate::Tab *ptab = &tabList.at(tab);
    opt.init(q);
    opt.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
    opt.rect = q->tabRect(tab);
    bool isCurrent = tab == currentIndex;
    opt.row = 0;
    if (tab == pressedIndex)
        opt.state |= QStyle::State_Sunken;
    if (isCurrent)
        opt.state |= QStyle::State_Selected;
    if (isCurrent && q->hasFocus())
        opt.state |= QStyle::State_HasFocus;
    if (q->isEnabled() && ptab->enabled)
        opt.state |= QStyle::State_Enabled;
    if (q->isActiveWindow())
        opt.state |= QStyle::State_Active;
    if (opt.rect == hoverRect)
        opt.state |= QStyle::State_MouseOver;
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
    if (const QTabWidget *tw = qobject_cast<const QTabWidget *>(q->parentWidget())) {
        if (tw->cornerWidget(Qt::TopLeftCorner) || tw->cornerWidget(Qt::BottomLeftCorner))
            opt.cornerWidgets |= QStyleOptionTab::LeftCornerWidget;
        if (tw->cornerWidget(Qt::TopRightCorner) || tw->cornerWidget(Qt::BottomRightCorner))
            opt.cornerWidgets |= QStyleOptionTab::RightCornerWidget;
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

    Each tab has a tabText(), an optional tabIcon(), an optional
    tabToolTip(), and optional tabData(). The tabs's attributes can be
    changed with setTabText(), setTabIcon(), setTabToolTip(), and
    setTabData(). Each tabs can be enabled or disabled individually
    with setTabEnabled().

    Tabs are added using addTab(), or inserted at particular positions
    using insertTab(). The total number of tabs is given by
    count(). Tabs can be removed from the tab bar with
    removeTab(). Combining removeTab() and insertTab() allows you to
    move tabs to different positions.

    The \l shape property defines the tabs' appearance. The choice of
    shape is a matter of taste, although tab dialogs (for preferences
    and similar) invariably use \c RoundedNorth.
    Tab controls in windows other than dialogs almost
    always use either \c RoundedSouth or \c TriangularSouth. Many
    spreadsheets and other tab controls in which all the pages are
    essentially similar use \c TriangularSouth, whereas \c
    RoundedSouth is used mostly when the pages are different (e.g. a
    multi-page tool palette). The default in QTabBar is \c
    RoundedNorth.

    The most important part of QTabBar's API is the currentChanged()
    signal.  This is emitted whenever the current tab changes (even at
    startup, when the current tab changes from 'none'). There is also
    a slot, setCurrentIndex(), which can be used to select a tab
    programmatically. The function currentIndex() returns the index of
    the current tab, \l count holds the number of tabs.

    QTabBar creates automatic mnemonic keys in the manner of QAbstractButton;
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

    This enum type lists the built-in shapes supported by QTabBar. Treat these
    as hints as some styles may not render some of the shapes. However,
    position should be honored.

    \value RoundedNorth  The normal rounded look above the pages

    \value RoundedSouth  The normal rounded look below the pages

    \value RoundedWest  The normal rounded look on the left side of the pages

    \value RoundedEast  The normal rounded look on the right side the pages

    \value TriangularNorth  Triangular tabs above the pages.

    \value TriangularSouth  Triangular tabs similar to those used in
    the Excel spreadsheet, for example

    \value TriangularWest  Triangular tabs on the left of the pages.

    \value TriangularEast  Triangular tabs on the right of the pages.
    \omitvalue RoundedAbove
    \omitvalue RoundedBelow
    \omitvalue TriangularAbove
    \omitvalue TriangularBelow
*/

/*!
    \fn void QTabBar::currentChanged(int index)

    This signal is emitted when the tab bar's current tab changes. The
    new current has the given \a index.
*/

int QTabBarPrivate::extraWidth() const
{
    Q_Q(const QTabBar);
    return 2 * qMax(q->style()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, q),
                    QApplication::globalStrut().width());
}

void QTabBarPrivate::init()
{
    Q_Q(QTabBar);
    leftB = new QToolButton(q);
    QObject::connect(leftB, SIGNAL(clicked()), q, SLOT(scrollTabs()));
    leftB->hide();
    rightB = new QToolButton(q);
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
    Q_Q(const QTabBar);
    if (q->tabRect(currentIndex).contains(p))
        return currentIndex;
    for (int i = 0; i < tabList.count(); ++i)
        if (tabList.at(i).enabled && q->tabRect(i).contains(p))
            return i;
    return -1;
}

inline static bool verticalTabs(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest
           || shape == QTabBar::RoundedEast
           || shape == QTabBar::TriangularWest
           || shape == QTabBar::TriangularEast;
}


void QTabBarPrivate::layoutTabs()
{
    Q_Q(QTabBar);
    scrollOffset = 0;
    layoutDirty = false;
    QSize size = q->size();
    int last, available;
    bool vertTabs = verticalTabs(shape);
    if (!vertTabs) {
        int x = 0;
        int maxHeight = 0;
        int i;
        for (i = 0; i < tabList.count(); ++i) {
            QSize sz = q->tabSizeHint(i);
            tabList[i].rect = QRect(x, 0, sz.width(), sz.height());
            maxHeight = qMax(maxHeight, sz.height());
            x += sz.width();
        }

        // Go through the list again and make sure we have a consistent height
        for (i = 0; i < tabList.count(); ++i)
            tabList[i].rect.setHeight(maxHeight);

        last = x;
        available = size.width();
    } else {
        int y = 0;
        int i;
        int maxWidth = 0;
        for (i = 0; i < tabList.count(); ++i) {
            QSize sz = q->tabSizeHint(i);
            tabList[i].rect = QRect(0, y, sz.width(), sz.height());
            maxWidth = qMax(0, sz.width());
            y += sz.height();
        }

        // Consistent width
        for (i = 0; i < tabList.count(); ++i)
            tabList[i].rect.setWidth(maxWidth);

        last = y;
        available = size.height();
    }

    if (tabList.count() && last > available) {
        int extra = extraWidth();
        if (!vertTabs) {
            Qt::LayoutDirection ld = q->layoutDirection();
            QRect arrows = QStyle::visualRect(ld, q->rect(),
                                              QRect(available - extra, 0, extra, size.height()));
            if (ld == Qt::LeftToRight) {
                leftB->setGeometry(arrows.left(), arrows.top(), extra/2, arrows.height());
                rightB->setGeometry(arrows.right() - extra/2 + 1, arrows.top(),
                                    extra/2, arrows.height());
                leftB->setArrowType(Qt::LeftArrow);
                rightB->setArrowType(Qt::RightArrow);
            } else {
                rightB->setGeometry(arrows.left(), arrows.top(), extra/2, arrows.height());
                leftB->setGeometry(arrows.right() - extra/2 + 1, arrows.top(),
                                    extra/2, arrows.height());
                rightB->setArrowType(Qt::LeftArrow);
                leftB->setArrowType(Qt::RightArrow);
            }
        } else {
            QRect arrows = QRect(0, available - extra, size.width(), extra );
            leftB->setGeometry(arrows.left(), arrows.top(), arrows.width(), extra/2);
            leftB->setArrowType(Qt::UpArrow);
            rightB->setGeometry(arrows.left(), arrows.bottom() - extra/2 + 1,
                                arrows.width(), extra/2);
            rightB->setArrowType(Qt::DownArrow);
        }
        leftB->setEnabled(scrollOffset > 0);
        rightB->setEnabled(last - scrollOffset >= available - extra);
        leftB->show();
        rightB->show();
    } else {
        rightB->hide();
        leftB->hide();
    }

    q->tabLayoutChange();
}

void QTabBarPrivate::makeVisible(int index)
{
    Q_Q(QTabBar);
    if (!validIndex(index) || leftB->isHidden())
        return;
    const QRect tabRect = tabList.at(index).rect;

    const int oldScrollOffset = scrollOffset;
    const bool horiz = !verticalTabs(shape);
    const int available = (horiz ? q->width() : q->height()) - extraWidth();
    const int start = horiz ? tabRect.left() : tabRect.top();
    const int end = horiz ? tabRect.right() : tabRect.bottom();
    if (start < scrollOffset) // too far left
        scrollOffset = start - (index?8:0);
    else if (end > scrollOffset + available) // too far right
        scrollOffset = end - available + 1;

    if (scrollOffset && end < available)  // need scrolling at all?
        scrollOffset = 0;

    leftB->setEnabled(scrollOffset > 0);
    const int last = horiz ? tabList.last().rect.right() : tabList.last().rect.bottom();
    rightB->setEnabled(last - scrollOffset >= available);
    if (oldScrollOffset != scrollOffset)
        q->update();

}

void QTabBarPrivate::scrollTabs()
{
    Q_Q(QTabBar);
    const QObject *sender = q->sender();
    int i = -1;
    if (!verticalTabs(shape)) {
        if (sender == leftB) {
            for (i = tabList.count() - 1; i >= 0; --i) {
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
    } else { // vertical
        if (sender == leftB) {
            for (i = tabList.count() - 1; i >= 0; --i) {
                if (tabList.at(i).rect.top() - scrollOffset < 0) {
                    makeVisible(i);
                    return;
                }
            }
        } else if (sender == rightB) {
            int available = q->height() - extraWidth();
            for (i = 0; i < tabList.count(); ++i) {
                if (tabList.at(i).rect.bottom() - scrollOffset > available) {
                    makeVisible(i);
                    return;
                }
            }
        }
    }
}

void QTabBarPrivate::refresh()
{
    Q_Q(QTabBar);
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
    Q_D(QTabBar);
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
    RoundedNorth (default), \c RoundedSouth, \c TriangularNorth or \c
    TriangularBelow.

    \sa Shape
*/


QTabBar::Shape QTabBar::shape() const
{
    Q_D(const QTabBar);
    return d->shape;
}

void QTabBar::setShape(Shape shape)
{
    Q_D(QTabBar);
    if (d->shape == shape)
        return;
    d->shape = shape;
    d->refresh();
}


/*!
    \property QTabBar::drawBase
    \brief defines whether or not tabbar should draw it's base.

    If true then QTabBar draws a base in relation to the styles overlab.
    Otherwise only the tabs are drawn.

    \sa QStyle::pixelMetric() QStyle::PM_TabBarBaseOverlap
*/

void QTabBar::setDrawBase(bool drawBase)
{
    Q_D(QTabBar);
    d->drawBase = drawBase;
}

bool QTabBar::drawBase() const
{
    Q_D(const QTabBar);
    return d->drawBase;
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
    Q_D(QTabBar);
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
    Q_D(QTabBar);
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
    Q_D(const QTabBar);
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
    Q_D(QTabBar);
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
    Q_D(const QTabBar);
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->text;
    return QString();
}

/*!
    Sets the text of the tab at position \a index to \a text.
*/
void QTabBar::setTabText(int index, const QString &text)
{
    Q_D(QTabBar);
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
    Q_D(const QTabBar);
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->icon;
    return QIcon();
}

/*!
    Sets the icon of the tab at position \a index to \a icon.
*/
void QTabBar::setTabIcon(int index, const QIcon & icon)
{
    Q_D(QTabBar);
    if (QTabBarPrivate::Tab *tab = d->at(index)) {
        tab->icon = icon;
        d->refresh();
    }
}


/*!
    Sets the tool tip of the tab at position \a index to \a tip.
*/
void QTabBar::setTabToolTip(int index, const QString & tip)
{
    Q_D(QTabBar);
    if (QTabBarPrivate::Tab *tab = d->at(index))
        tab->toolTip = tip;
}

/*!
    Returns the tool tip of the tab at position \a index, or an empty
    string if \a index is out of range.
*/
QString QTabBar::tabToolTip(int index) const
{
    Q_D(const QTabBar);
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->toolTip;
    return QString();
}

/*!
    Sets the data of the tab at position \a index to \a data.
*/
void QTabBar::setTabData(int index, const QVariant & data)
{
    Q_D(QTabBar);
    if (QTabBarPrivate::Tab *tab = d->at(index))
        tab->data = data;
}

/*!
    Returns the datad of the tab at position \a index, or a null
    variant if \a index is out of range.
*/
QVariant QTabBar::tabData(int index) const
{
    Q_D(const QTabBar);
    if (const QTabBarPrivate::Tab *tab = d->at(index))
        return tab->data;
    return QVariant();
}

/*!
    Returns the visual rectangle of the of the tab at position \a
    index, or a null rectangle if \a index is out of range.
*/
QRect QTabBar::tabRect(int index) const
{
    Q_D(const QTabBar);
    if (const QTabBarPrivate::Tab *tab = d->at(index)) {
        if (d->layoutDirty)
            const_cast<QTabBarPrivate*>(d)->layoutTabs();
        QRect r = tab->rect;
        if (verticalTabs(d->shape))
            r.translate(0, -d->scrollOffset);
        else
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
    Q_D(const QTabBar);
    if (d->validIndex(d->currentIndex))
        return d->currentIndex;
    return -1;
}


void QTabBar::setCurrentIndex(int index)
{
    Q_D(QTabBar);
    if (d->validIndex(index)) {
        d->currentIndex = index;
        update();
        d->makeVisible(index);
#ifdef QT3_SUPPORT
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
    Q_D(const QTabBar);
    return d->tabList.count();
}


/*!\reimp
 */
QSize QTabBar::sizeHint() const
{
    Q_D(const QTabBar);
    if (d->layoutDirty)
        const_cast<QTabBarPrivate*>(d)->layoutTabs();
    QRect r;
    for (int i = 0; i < d->tabList.count(); ++i)
        r = r.unite(d->tabList.at(i).rect);
    QSize sz = QApplication::globalStrut();
    return r.size().expandedTo(sz);
}

/*!\reimp
 */
QSize QTabBar::minimumSizeHint() const
{
    Q_D(const QTabBar);
    if (style()->styleHint(QStyle::SH_TabBar_PreferNoArrows, 0, this))
        return sizeHint();
    if (verticalTabs(d->shape))
        return QSize(sizeHint().width(), d->rightB->sizeHint().height() * 2 + 75);
    else
        return QSize(d->rightB->sizeHint().width() * 2 + 75, sizeHint().height());
}

/*!
    Returns the size hint for the tab at position \a index.
*/
QSize QTabBar::tabSizeHint(int index) const
{
    Q_D(const QTabBar);
    if (const QTabBarPrivate::Tab *tab = d->at(index)) {
        QStyleOptionTab opt = d->getStyleOption(index);
        int iconExtent = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
        QSize iconSize = tab->icon.isNull() ? QSize() : QSize(iconExtent, iconExtent);
        int hframe  = style()->pixelMetric(QStyle::PM_TabBarTabHSpace, &opt, this);
        int vframe  = style()->pixelMetric(QStyle::PM_TabBarTabVSpace, &opt, this);
        const QFontMetrics fm = fontMetrics();
        QSize csz(fm.size(Qt::TextShowMnemonic, tab->text).width() + iconSize.width() + hframe,
                  qMax(fm.height(), iconSize.height()) + vframe);
        if (verticalTabs(d->shape))
            csz.transpose();

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
    Q_D(QTabBar);
    if (d->layoutDirty)
        d->layoutTabs();
    if (!d->validIndex(d->currentIndex))
        setCurrentIndex(0);

}


/*!\reimp
 */
bool QTabBar::event(QEvent *e)
{
    Q_D(QTabBar);
    if (e->type() == QEvent::HoverMove
        || e->type() == QEvent::HoverEnter) {
        QHoverEvent *he = static_cast<QHoverEvent *>(e);
        if (!d->hoverRect.contains(he->pos())) {
            QRect oldHoverRect = d->hoverRect;
            for (int i = 0; i < d->tabList.count(); ++i) {
                QRect area = tabRect(i);
                if (area.contains(he->pos())) {
                    d->hoverRect = area;
                    break;
                }
            }
            if (he->oldPos() != QPoint(-1, -1))
                update(oldHoverRect);
            update(d->hoverRect);
        }
    } else if (e->type() == QEvent::HoverLeave ) {
        QRect oldHoverRect = d->hoverRect;
        d->hoverRect = QRect();
        update(oldHoverRect);
    } else if (e->type() == QEvent::ToolTip) {
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
    Q_D(QTabBar);
    d->layoutTabs();
    d->makeVisible(d->currentIndex);
}

/*!\reimp
 */
void QTabBar::paintEvent(QPaintEvent *)
{
    Q_D(QTabBar);
    QStyleOptionTab tabOverlap;
    tabOverlap.shape = d->shape;
    int overlap = style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, this);
    QWidget *theParent = parentWidget();
    QStyleOptionTabBarBase optTabBase;
    optTabBase.init(this);
    optTabBase.shape = d->shape;
    if (theParent && overlap > 0) {
        QPainter::setRedirected(theParent, this, pos());
        QRect rect;
        switch (tabOverlap.shape) {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
            rect.setRect(0, height()-overlap, width(), overlap);
            break;
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
            rect.setRect(0, 0, width(), overlap);
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
            rect.setRect(0, 0, overlap, height());
            break;
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            rect.setRect(width()-overlap, 0, overlap, height());
            break;
        }
        optTabBase.rect = rect;
        QPaintEvent e(rect);
        QApplication::sendEvent(theParent, &e);
        QPainter::restoreRedirected(theParent);
    }
    QStylePainter p(this);
    int selected = -1;
    int cut = -1;
    bool rtl = optTabBase.direction == Qt::RightToLeft;
    bool verticalTabs = (d->shape == QTabBar::RoundedWest || d->shape == QTabBar::RoundedEast
                         || d->shape == QTabBar::TriangularWest
                         || d->shape == QTabBar::TriangularEast);
    QStyleOptionTab cutTab;
    QStyleOptionTab selectedTab;
    for (int i = 0; i < d->tabList.count(); ++i) {
        QStyleOptionTab tab = d->getStyleOption(i);
        // If this tab is partially obscured, make a note of it so that we can pass the information
        // along when we draw the tear.
        if ((!verticalTabs && (!rtl && tab.rect.left() < 0) || (rtl && tab.rect.right() > width()))
            || (verticalTabs && tab.rect.top() < 0)) {
            cut = i;
            cutTab = tab;
        }
        // Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
        if ((!verticalTabs && (tab.rect.right() < 0 || tab.rect.left() > width()))
            || (verticalTabs && (tab.rect.bottom() < 0 || tab.rect.top() > height())))
            continue;

        optTabBase.tabBarRect |= tab.rect;
        if (i == d->currentIndex) {
            selected = i;
            selectedTab = tab;
            optTabBase.selectedTabRect = tab.rect;
            continue;
        }
        p.drawControl(QStyle::CE_TabBarTab, tab);
    }

    // Draw the selected tab last to get it "on top"
    if (selected >= 0) {
        QStyleOptionTab tab = d->getStyleOption(selected);
        p.drawControl(QStyle::CE_TabBarTab, tab);
    }
    if (d->drawBase)
        p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);

    // Only draw the tear indicator if necessary. Most of the time we don't need too.
    if (d->leftB->isVisible() && cut >= 0) {
        cutTab.rect = rect();
        cutTab.rect = style()->subElementRect(QStyle::SE_TabBarTearIndicator, &cutTab, this);
        p.drawPrimitive(QStyle::PE_IndicatorTabTear, cutTab);
    }
}

/*!\reimp
 */
void QTabBar::mousePressEvent (QMouseEvent *e)
{
    Q_D(QTabBar);
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
    Q_D(QTabBar);
    if (e->buttons() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    if (style()->styleHint(QStyle::SH_TabBar_SelectMouseType, 0, this)
            == QEvent::MouseButtonRelease) {
        int i = d->indexAtPos(e->pos());
        if (i != d->pressedIndex) {
            int oldIndex = d->pressedIndex;
            d->pressedIndex = -1;
            if (oldIndex >= 0)
                repaint(tabRect(oldIndex));
            if ((d->pressedIndex = i) >= 0)
                repaint(tabRect(i));
        }
    }
}

/*!\reimp
 */
void QTabBar::mouseReleaseEvent (QMouseEvent *e)
{
    Q_D(QTabBar);
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
    Q_D(QTabBar);
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
    Q_D(QTabBar);
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

