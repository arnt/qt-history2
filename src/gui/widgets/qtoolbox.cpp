/****************************************************************************
** $Id$
**
** Implementation of QToolBox widget class
**
** Created : 961105
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtoolbox.h"

#ifndef QT_NO_TOOLBOX

#include <qbutton.h>
#include <qlayout.h>
#include <qscrollview.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlist.h>
#include <qtooltip.h>
#include <qeventloop.h>
#include <qdatetime.h>

class QToolBoxButton : public QAbstractButton
{
public:
    QToolBoxButton(QWidget *parent, const char *name)
        : QButton(parent, name), selected(false)
    {
        setBackgroundRole(QPalette::Background);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        setFocusPolicy(NoFocus);
    }

    inline void setSelected(bool b) { selected = b; update(); }
    inline void setTextLabel(const QString &text) { label = text; update(); }
    inline QString textLabel() const { return label; }
    inline void setIconSet(const QIconSet &is) { icon = is; update(); }
    inline QIconSet iconSet() const { return icon; }

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    bool selected;
    QString label;
    QIconSet icon;
};


class QToolBoxPrivate
{
public:
    struct Page
    {
        QToolBoxButton *button;
        QScrollView *sv;
        QWidget *widget;
        QString toolTip;

        inline void setTextLabel(const QString &text) { button->setTextLabel(text); }
        inline void setIconSet(const QIconSet &is) { button->setIconSet(is); }
        inline void setToolTip(const QString &tip)
        {
            toolTip = tip;
            QToolTip::remove(button);
            if (!tip.isNull())
                QToolTip::add(button, tip);
        }

        inline bool operator==(const Page& other) const
        {
            return widget == other.widget;
        }
    };
    typedef QList<Page> PageList;

    inline QToolBoxPrivate()
        : currentPage(0)
    {
    }

    Page *page(QWidget *widget);
    Page *page(int index);

    void updateTabs();

    PageList pageList;
    QVBoxLayout *layout;
    Page *currentPage;
};

QToolBoxPrivate::Page *QToolBoxPrivate::page(QWidget *widget)
{
    if (!widget)
        return 0;

    for (PageList::ConstIterator i = pageList.constBegin(); i != pageList.constEnd(); ++i)
        if ((*i).widget == widget)
            return (Page*) &(*i);
    return 0;
}

QToolBoxPrivate::Page *QToolBoxPrivate::page(int index)
{
    if (index >= 0 && index < pageList.size())
        return &pageList[index];
    return 0;
}

void QToolBoxPrivate::updateTabs()
{
    QToolBoxButton *lastButton = currentPage ? currentPage->button : 0;
    bool after = false;
    for (PageList::ConstIterator i = pageList.constBegin(); i != pageList.constEnd(); ++i) {
        QToolBoxButton *tB = (*i).button;
        QWidget *tW = (*i).widget;
        if (after) {
            QPalette p = tB->palette();
            p.setColor(tB->backgroundRole(), tW->palette().color(tW->backgroundRole()));
            tB->setPalette(p);
            tB->update();
        } else if (tB->backgroundRole() != QPalette::Background) {
            tB->setBackgroundRole(QPalette::Background);
            tB->update();
        }
        after = (*i).button == lastButton;
    }
}

QSize QToolBoxButton::sizeHint() const
{
    QSize iconSize(8, 8);
    if (!icon.isNull())
        iconSize += icon.pixmap(QIconSet::Small, QIconSet::Normal).size() + QSize(2, 0);
    QSize textSize = fontMetrics().size(Qt::ShowPrefix, label) + QSize(0, 8);

    QSize total(iconSize.width() + textSize.width(), qMax(iconSize.height(), textSize.height()));
    return total.expandedTo(QApplication::globalStrut());
}

QSize QToolBoxButton::minimumSizeHint() const
{
    if (icon.isNull())
        return QSize();
    return QSize(8, 8) + icon.pixmap(QIconSet::Small, QIconSet::Normal).size();
}

void QToolBoxButton::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    QPainter *p = &paint;
    QStyle::SFlags flags = QStyle::Style_Default;
    const QPalette &pal = palette();

    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (selected)
        flags |= QStyle::Style_Selected;
    if (hasFocus())
        flags |= QStyle::Style_HasFocus;
    if (isDown())
        flags |= QStyle::Style_Down;
    style().drawControl(QStyle::CE_ToolBoxTab, p, parentWidget(), rect(), pal, flags);

    QPixmap pm = icon.pixmap(QIconSet::Small, isEnabled() ? QIconSet::Normal : QIconSet::Disabled);

    QRect cr = style().subRect(QStyle::SR_ToolBoxTabContents, this);
    QRect tr, ir;
    int ih = 0;
    if (pm.isNull()) {
        tr = cr;
        tr.addCoords(4, 0, -8, 0);
    } else {
        int iw = pm.width() + 4;
        ih = pm.height();
        ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
        tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
    }

    if (selected && style().styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold)) {
        QFont f(p->font());
        f.setBold(true);
        p->setFont(f);
    }

    QString txt;
    if (p->fontMetrics().width(label) < tr.width()) {
        txt = label;
    } else {
        txt = label.left(1);
        int ew = p->fontMetrics().width("...");
        int i = 1;
        while (p->fontMetrics().width(txt) + ew +
                p->fontMetrics().width(label[i])  < tr.width())
            txt += label[i++];
        txt += "...";
    }

    if (ih)
        p->drawPixmap(ir.left(), (height() - ih) / 2, pm);

    QToolBox *tb = (QToolBox*)parentWidget();

    const QColor* fill = 0;
    if (selected &&
         style().styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold) &&
         !tb->testAttribute(WA_NoSystemBackground))
        fill = &pal.color(foregroundRole());

    int alignment = AlignLeft | AlignVCenter | ShowPrefix;
    if (!style().styleHint(QStyle::SH_UnderlineAccelerator, this))
        alignment |= NoAccel;
    style().drawItem(p, tr, alignment, pal,
                      isEnabled(), 0, txt, -1, fill);

    if (!txt.isEmpty() && hasFocus())
        style().drawPrimitive(QStyle::PE_FocusRect, p, tr, pal);
}

/*!
    \class QToolBox

    \brief The QToolBox class provides a column of tabbed widget
    items.

    \mainclass
    \ingroup advanced

    A toolbox is a widget that displays a column of tabs one above the
    other, with the current item displayed below the current tab.
    Every tab has an index position within the column of tabs. A tab's
    item is a QWidget.

    Each item has an itemLabel(), an optional icon, itemIconSet(), an
    optional itemToolTip(), and a \link item() widget\endlink. The
    item's attributes can be changed with setItemLabel(),
    setItemIconSet(), and setItemToolTip().

    Items are added using addItem(), or inserted at particular
    positions using insertItem(). The total number of items is given
    by count(). Items can be deleted with delete, or removed from the
    toolbox with removeItem(). Combining removeItem() and insertItem()
    allows you to move items to different positions.

    The index of the current item widget is returned by currentIndex(),
    and set with setCurrentIndex(). The index of a particular item can
    be found using indexOf(), and the item at a given index is returned
    by item().

    The currentChanged() signal is emitted when the current item is
    changed.

    \sa QTabWidget
*/

/*!
    \fn void QToolBox::currentChanged(int index)

    This signal is emitted when the current item is changed. The new
    current item's index is passed in \a index, or -1 if there is no
    current item.
*/

/*!
    Constructs a toolbox called \a name with parent \a parent and flags \a f.
*/

QToolBox::QToolBox(QWidget *parent, const char *name, WFlags f)
    :  QFrame(parent, name, f)
{
    d = new QToolBoxPrivate;
    d->layout = new QVBoxLayout(this);
    setBackgroundRole(QPalette::Button);
}

/*! \reimp */

QToolBox::~QToolBox()
{
    delete d;
}

/*!
    \fn int QToolBox::addItem(QWidget *w, const QString &label)
    \overload

    Adds the widget \a w in a new tab at bottom of the toolbox. The
    new tab's label is set to \a label. Returns the new tab's index.
*/

/*!
    \fn int QToolBox::addItem(QWidget *widget, const QIconSet &iconSet,const QString &label)
    Adds the \a widget in a new tab at bottom of the toolbox. The
    new tab's label is set to \a label, and the \a iconSet is
    displayed to the left of the \a label.  Returns the new tab's index.
*/

/*!
    \fn int QToolBox::insertItem(int index, QWidget *widget, const QString &label)
    \overload

    Inserts the \a widget at position \a index, or at the bottom
    of the toolbox if \a index is out of range. The new item's label is
    set to \a label. Returns the new item's index.
*/

/*!
    Inserts the \a widget at position \a index, or at the bottom
    of the toolbox if \a index is out of range. The new item's label
    is set to \a label, and the \a iconSet is displayed to the left of
    the \a label. Returns the new item's index.
*/

int QToolBox::insertItem(int index, QWidget *widget, const QIconSet &iconSet,
                           const QString &label)
{
    if (!widget)
        return -1;

    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));

    QToolBoxPrivate::Page c;
    c.widget = widget;
    c.button = new QToolBoxButton(this, label.latin1());
    connect(c.button, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    c.sv = new QScrollView(this);
    c.sv->hide();
    c.sv->setResizePolicy(QScrollView::AutoOneFit);
    c.sv->addChild(widget);
    c.sv->setFrameStyle(QFrame::NoFrame);

    c.setTextLabel(label);
    c.setIconSet(iconSet);

    if (index < 0 || index >= (int)d->pageList.count()) {
        index = d->pageList.count();
        d->pageList.append(c);
        d->layout->addWidget(c.button);
        d->layout->addWidget(c.sv);
        if (index == 0)
            setCurrentIndex(index);
    } else {
        d->pageList.insert(index, c);
        relayout();
        if (d->currentPage) {
            QWidget *current = d->currentPage->widget;
            int oldindex = indexOf(current);
            if (index <= oldindex) {
                d->currentPage = 0; // trigger change
                setCurrentIndex(oldindex);
            }
        }
    }

    c.button->show();

    d->updateTabs();
    itemInserted(index);
    return index;
}

void QToolBox::buttonClicked()
{
    QToolBoxButton *tb = ::qt_cast<QToolBoxButton*>(sender());
    QWidget* item = 0;
    for (QToolBoxPrivate::PageList::ConstIterator i = d->pageList.constBegin(); i != d->pageList.constEnd(); ++i)
        if ((*i).button == tb) {
            item = (*i).widget;
            break;
        }

    setCurrentIndex(indexOf(item));
}

/*!
    \property QToolBox::count
    \brief The number of items contained in the toolbox.
*/

int QToolBox::count() const
{
    return d->pageList.count();
}

void QToolBox::setCurrentIndex(int index)
{
    QToolBoxPrivate::Page *c = d->page(index);
    if (!c || d->currentPage == c)
        return;

    c->button->setSelected(true);
    if (d->currentPage) {
        d->currentPage->sv->hide();
        d->currentPage->button->setSelected(false);
    }
    d->currentPage = c;
    d->currentPage->sv->show();
    d->updateTabs();
    emit currentChanged(index);
}

void QToolBox::relayout()
{
    delete d->layout;
    d->layout = new QVBoxLayout(this);
    for (QToolBoxPrivate::PageList::ConstIterator i = d->pageList.constBegin(); i != d->pageList.constEnd(); ++i) {
        d->layout->addWidget((*i).button);
        d->layout->addWidget((*i).sv);
    }
}

void QToolBox::widgetDestroyed(QObject *object)
{
    // no verification - vtbl corrupted already
    QWidget *page = (QWidget*)object;

    QToolBoxPrivate::Page *c = d->page(page);
    if (!page || !c)
        return;

    d->layout->removeWidget(c->sv);
    d->layout->removeWidget(c->button);
    c->sv->deleteLater(); // page might still be a child of sv
    delete c->button;

    bool removeCurrent = c == d->currentPage;
    d->pageList.removeAll(*c);

    if (!d->pageList.count()) {
        d->currentPage = 0;
        emit currentChanged(-1);
    } else if (removeCurrent) {
        d->currentPage = 0;
        setCurrentIndex(0);
    }
}

/*!
    Removes the item at position \a index from the toolbox. Note that
    the widget is \e not deleted.
*/

void QToolBox::removeItem(int index)
{
    if (QWidget *w = widget(index)) {
        disconnect(w, SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyed(QObject*)));
        w->setParent(this);
        // destroy internal data
        widgetDestroyed(w);
        itemRemoved(index);
    }
}


/*!
    \property QToolBox::currentIndex
    \brief The index of the current item, or -1 if the toolbox is empty.
    \sa indexOf(), item()
*/


int QToolBox::currentIndex() const
{
    return d->currentPage ? indexOf(d->currentPage->widget) : -1;
}

/*!
    Returns the widget at position \a index, or 0 if there is no such
    item.
*/

QWidget *QToolBox::widget(int index) const
{
    if (index < 0 || index >= (int) d->pageList.size())
        return 0;
    return d->pageList.at(index).widget;
}

/*!
    Returns the index of \a widget, or -1 if the item does not
    exist.
*/

int QToolBox::indexOf(QWidget *widget) const
{
    QToolBoxPrivate::Page *c = d->page(widget);
    return c ? d->pageList.indexOf(*c) : -1;
}

/*!
    If \a enabled is true then the item at position \a index is enabled; otherwise
    the item at position \a index is disabled.
*/

void QToolBox::setItemEnabled(int index, bool enabled)
{
    QToolBoxPrivate::Page *c = d->page(index);
    if (!c)
        return;

    c->button->setEnabled(enabled);
    if (!enabled && c == d->currentPage) {
        int curIndexUp = index;
        int curIndexDown = curIndexUp;
        const int count = d->pageList.count();
        while (curIndexUp > 0 || curIndexDown < count-1) {
            if (curIndexDown < count-1) {
                if (d->page(++curIndexDown)->button->isEnabled()) {
                    index = curIndexDown;
                    break;
                }
            }
            if (curIndexUp > 0) {
                if (d->page(--curIndexUp)->button->isEnabled()) {
                    index = curIndexUp;
                    break;
                }
            }
        }
        setCurrentIndex(index);
    }
}


/*!
    Sets the label of the item at position \a index to \a label.
*/

void QToolBox::setItemLabel(int index, const QString &label)
{
    QToolBoxPrivate::Page *c = d->page(index);
    if (c)
        c->setTextLabel(label);
}

/*!
    Sets the icon of the item at position \a index to \a iconSet.
*/

void QToolBox::setItemIconSet(int index, const QIconSet &iconSet)
{
    QToolBoxPrivate::Page *c = d->page(index);
    if (c)
        c->setIconSet(iconSet);
}

/*!
    Sets the tooltip of the item at position \a index to \a toolTip.
*/

void QToolBox::setItemToolTip(int index, const QString &toolTip)
{
    QToolBoxPrivate::Page *c = d->page(index);
    if (c)
        c->setToolTip(toolTip);
}

/*!
    Returns true if the item at position \a index is enabled; otherwise returns false.
*/

bool QToolBox::isItemEnabled(int index) const
{
    QToolBoxPrivate::Page *c = d->page(index);
    return c && c->button->isEnabled();
}

/*!
    Returns the label of the item at position \a index, or an empty string if
    \a index is out of range.
*/

QString QToolBox::itemLabel(int index) const
{
    QToolBoxPrivate::Page *c = d->page(index);
    return (c ? c->button->textLabel() : QString());
}

/*!
    Returns the icon of the item at position \a index, or a null
    icon if \a index is out of range.
*/

QIconSet QToolBox::itemIconSet(int index) const
{
    QToolBoxPrivate::Page *c = d->page(index);
    return (c ? c->button->iconSet() : QIconSet());
}

/*!
    Returns the tooltip of the item at position \a index, or an
    empty string if \a index is out of range.
*/

QString QToolBox::itemToolTip(int index) const
{
    QToolBoxPrivate::Page *c = d->page(index);
    return (c ? c->toolTip : QString());
}

/*! \reimp */
void QToolBox::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
}

/*! \reimp */
void QToolBox::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        d->updateTabs();
    QFrame::changeEvent(ev);
}

/*!
  This virtual handler is called after a new item was added or
  inserted at position \a index.
 */
void QToolBox::itemInserted(int index)
{
    Q_UNUSED(index)
}

/*!
  This virtual handler is called after an item was removed from
  position \a index.
 */
void QToolBox::itemRemoved(int index)
{
    Q_UNUSED(index)
}

#endif //QT_NO_TOOLBOX
