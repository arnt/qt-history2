/****************************************************************************
**
** Implementation of the abstract layout base class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlayout.h"

#ifndef QT_NO_LAYOUT
#include "qapplication.h"
#include "qlayoutengine_p.h"
#include "qmenubar.h"
#include "qevent.h"
#include "qtoolbar.h"
#include "qwidget_p.h"
#define d d_func()
#define q q_func()

static int menuBarHeightForWidth(QMenuBar *menubar, int w)
{
#ifndef QT_NO_MENUBAR
    if (menubar && !menubar->isHidden() && !menubar->isTopLevel())
        return menubar->heightForWidth(qMax(w, menubar->minimumWidth()));
    else
#endif
        return 0;
}

/*!
    \class QLayoutItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QLayoutItem class provides an abstract item that a
    QLayout manipulates.

    This is used by custom layouts.

    Pure virtual functions are provided to return information about
    the layout, including, sizeHint(), minimumSize(), maximumSize()
    and expanding().

    The layout's geometry can be set and retrieved with setGeometry()
    and geometry(), and its alignment with setAlignment() and
    alignment().

    isEmpty() returns whether the layout is empty. iterator() returns
    an iterator for the layout's children. If the concrete item is a
    QWidget, it can be retrieved using widget(). Similarly for
    layout() and spacerItem().

    \sa QLayout
*/

/*!
    \class QSpacerItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QSpacerItem class provides blank space in a layout.

    This class is used by custom layouts.

    \sa QLayout QLayout::spacerItem()
*/

/*!
    \class QWidgetItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QWidgetItem class is a layout item that represents a widget.

    This is used by custom layouts.

    \sa QLayout QLayout::widget()
*/

/*!
    \fn QLayoutItem::QLayoutItem(Qt::Alignment alignment)

    Constructs a layout item with an \a alignment that is a bitwise OR
    of the \l{Qt::AlignmentFlags}. Not all subclasses support
    alignment.
*/

/*!
    \fn Qt::Alignment QLayoutItem::alignment() const

    Returns the alignment of this item.
*/

/*!
    Sets the alignment of this item to \a alignment, which is a bitwise OR of
    the \l{Qt::AlignmentFlags}. Not all subclasses support alignment.
*/
void QLayoutItem::setAlignment(Qt::Alignment alignment)
{
    align = alignment;
}

/*!
    \fn QSize QLayoutItem::maximumSize() const

    Implemented in subclasses to return the maximum size of this item.
*/

/*!
    \fn QSize QLayoutItem::minimumSize() const

    Implemented in subclasses to return the minimum size of this item.
*/

/*!
    \fn QSize QLayoutItem::sizeHint() const

    Implemented in subclasses to return the preferred size of this item.
*/

/*!
    \fn QSizePolicy::ExpandData QLayoutItem::expanding() const

    Implemented in subclasses to return the direction(s) this item
    "wants" to expand in (if any).
*/

/*!
    \fn void QLayoutItem::setGeometry(const QRect &r)

    Implemented in subclasses to set this item's geometry to \a r.
*/

/*!
    \fn QRect QLayoutItem::geometry() const

    Returns the rectangle covered by this layout item.
*/

/*!
    \fn virtual bool QLayoutItem::isEmpty() const

    Implemented in subclasses to return whether this item is empty,
    i.e. whether it contains any widgets.
*/

/*!
    \fn QSpacerItem::QSpacerItem(int w, int h, QSizePolicy::SizeType hData, QSizePolicy::SizeType vData)

    Constructs a spacer item with preferred width \a w, preferred
    height \a h, horizontal size policy \a hData and vertical size
    policy \a vData.

    The default values provide a gap that is able to stretch if
    nothing else wants the space.
*/

/*!
    Changes this spacer item to have preferred width \a w, preferred
    height \a h, horizontal size policy \a hData and vertical size
    policy \a vData.

    The default values provide a gap that is able to stretch if
    nothing else wants the space.
*/
void QSpacerItem::changeSize(int w, int h, QSizePolicy::SizeType hData,
                              QSizePolicy::SizeType vData)
{
    width = w;
    height = h;
    sizeP = QSizePolicy(hData, vData);
}

/*!
    \fn QWidgetItem::QWidgetItem (QWidget * w)

    Creates an item containing widget \a w.
*/

/*!
    Destroys the QLayoutItem.
*/
QLayoutItem::~QLayoutItem()
{
}

/*!
    Invalidates any cached information in this layout item.
*/
void QLayoutItem::invalidate()
{
}

/*!
    If this item is a QLayout, it is returned as a QLayout; otherwise
    0 is returned. This function provides type-safe casting.
*/
QLayout * QLayoutItem::layout()
{
    return 0;
}

/*!
    If this item is a QSpacerItem, it is returned as a QSpacerItem;
    otherwise 0 is returned. This function provides type-safe casting.
*/
QSpacerItem * QLayoutItem::spacerItem()
{
    return 0;
}

/*!
    \reimp
*/
QLayout * QLayout::layout()
{
    return this;
}

/*!
    \reimp
*/
QSpacerItem * QSpacerItem::spacerItem()
{
    return this;
}

/*!
    If this item is a QWidget, it is returned as a QWidget; otherwise
    0 is returned. This function provides type-safe casting.
*/
QWidget * QLayoutItem::widget()
{
    return 0;
}

/*!
    Returns the widget managed by this item.
*/
QWidget * QWidgetItem::widget()
{
    return wid;
}

/*!
    Returns true if this layout's preferred height depends on its
    width; otherwise returns false. The default implementation returns
    false.

    Reimplement this function in layout managers that support height
    for width.

    \sa heightForWidth(), QWidget::heightForWidth()
*/
bool QLayoutItem::hasHeightForWidth() const
{
    return false;
}

/*!
    Returns the minimum height this widget needs for the given width,
    \a w. The default implementation simply returns heightForWidth(w).
*/
int QLayoutItem::minimumHeightForWidth(int w) const
{
    return heightForWidth(w);
}


/*!
    Returns the preferred height for this layout item, given the width
    \a w.

    The default implementation returns -1, indicating that the
    preferred height is independent of the width of the item. Using
    the function hasHeightForWidth() will typically be much faster
    than calling this function and testing for -1.

    Reimplement this function in layout managers that support height
    for width. A typical implementation will look like this:
    \code
        int MyLayout::heightForWidth(int w) const
        {
            if (cache_dirty || cached_width != w) {
                // not all C++ compilers support "mutable"
                MyLayout *that = (MyLayout*)this;
                int h = calculateHeightForWidth(w);
                that->cached_hfw = h;
                return h;
            }
            return cached_hfw;
        }
    \endcode

    Caching is strongly recommended; without it layout will take
    exponential time.

    \sa hasHeightForWidth()
*/
int QLayoutItem::heightForWidth(int /* w */) const
{
    return -1;
}

/*!
    Stores the spacer item's rect \a r so that it can be returned by
    geometry().
*/
void QSpacerItem::setGeometry(const QRect &r)
{
    rect = r;
}

/*!
    Sets the geometry of this item's widget to be contained within
    rect \a r, taking alignment and maximum size into account.
*/
void QWidgetItem::setGeometry(const QRect &r)
{
    if (isEmpty())
        return;
    QSize s = r.size().boundedTo(qSmartMaxSize(this));
    int x = r.x();
    int y = r.y();
    if (align & (Qt::AlignHorizontal_Mask | Qt::AlignVertical_Mask)) {
        QSize pref = wid->sizeHint().expandedTo(wid->minimumSize()); //###
        if (align & Qt::AlignHorizontal_Mask)
            s.setWidth(qMin(s.width(), pref.width()));
        if (align & Qt::AlignVertical_Mask) {
            if (hasHeightForWidth())
                s.setHeight(qMin(s.height(), heightForWidth(s.width())));
            else
                s.setHeight(qMin(s.height(), pref.height()));
        }
    }
    Qt::Alignment alignHoriz = QApplication::horizontalAlignment(align);
    if (alignHoriz & Qt::AlignRight)
        x = x + (r.width() - s.width());
    else if (!(alignHoriz & Qt::AlignLeft))
        x = x + (r.width() - s.width()) / 2;

    if (align & Qt::AlignBottom)
        y = y + (r.height() - s.height());
    else if (!(align & Qt::AlignTop))
        y = y + (r.height() - s.height()) / 2;

    wid->setGeometry(x, y, s.width(), s.height());
}

/*!
    \reimp
*/
QRect QSpacerItem::geometry() const
{
    return rect;
}

/*!
    \reimp
*/
QRect QWidgetItem::geometry() const
{
    return wid->geometry();
}

/*!
    \reimp
*/
QRect QLayout::geometry() const
{
    return rect;
}

/*!
    \reimp
*/
bool QWidgetItem::hasHeightForWidth() const
{
    if (isEmpty())
        return false;
    if (wid->layout())
        return wid->layout()->hasHeightForWidth();
    return wid->sizePolicy().hasHeightForWidth();
}

/*!
    \reimp
*/
int QWidgetItem::heightForWidth(int w) const
{
    if (isEmpty())
        return -1;
    int hfw;
    if (wid->layout())
        hfw = wid->layout()->totalHeightForWidth(w);
    else
        hfw = wid->heightForWidth(w);

    if (hfw > wid->maximumHeight())
        hfw = wid->maximumHeight();
    if (hfw < wid->minimumHeight())
        hfw = wid->minimumHeight();
    if (hfw < 1)
        hfw = 1;
    return hfw;
}

/*!
    Returns the direction in which this spacer item will expand.

    \sa QSizePolicy::ExpandData
*/
QSizePolicy::ExpandData QSpacerItem::expanding() const
{
    return sizeP.expanding();
}

/*!
    Returns whether this item's widget can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions and \c NoDirection means that
    it doesn't want to grow at all.
*/
QSizePolicy::ExpandData QWidgetItem::expanding() const
{
    if (isEmpty())
        return QSizePolicy::NoDirection;

    int e = wid->sizePolicy().expanding();
    /*
      If the layout is expanding, we make the widget expanding, even if
      its own size policy isn't expanding. This behavior should be
      reconsidered in Qt 4.0. (###)
    */
    if (wid->layout()) {
        if (wid->sizePolicy().mayGrowHorizontally()
                && (wid->layout()->expanding() & QSizePolicy::Horizontally))
            e |= QSizePolicy::Horizontally;
        if (wid->sizePolicy().mayGrowVertically()
                && (wid->layout()->expanding() & QSizePolicy::Vertically))
            e |= QSizePolicy::Vertically;
    }

    if (align & Qt::AlignHorizontal_Mask)
        e &= ~QSizePolicy::Horizontally;
    if (align & Qt::AlignVertical_Mask)
        e &= ~QSizePolicy::Vertically;
    return (QSizePolicy::ExpandData)e;
}

/*!
    Returns the minimum size of this spacer item.
*/
QSize QSpacerItem::minimumSize() const
{
    return QSize(sizeP.mayShrinkHorizontally() ? 0 : width,
                  sizeP.mayShrinkVertically() ? 0 : height);
}

/*!
    Returns the minimum size of this item.
*/
QSize QWidgetItem::minimumSize() const
{
    if (isEmpty())
        return QSize(0, 0);
    return qSmartMinSize(this);
}

/*!
    Returns the maximum size of this spacer item.
*/
QSize QSpacerItem::maximumSize() const
{
    return QSize(sizeP.mayGrowHorizontally() ? QLAYOUTSIZE_MAX : width,
                  sizeP.mayGrowVertically() ? QLAYOUTSIZE_MAX : height);
}

/*!
    Returns the maximum size of this item.
*/
QSize QWidgetItem::maximumSize() const
{
    if (isEmpty()) {
        return QSize(0, 0);
    } else {
        return qSmartMaxSize(this, align);
    }
}

/*!
    Returns the preferred size of this spacer item.
*/
QSize QSpacerItem::sizeHint() const
{
    return QSize(width, height);
}

/*!
    Returns the preferred size of this item.
*/
QSize QWidgetItem::sizeHint() const
{
    QSize s;
    if (isEmpty()) {
        s = QSize(0, 0);
    } else {
        s = wid->sizeHint();
        if (wid->sizePolicy().horData() == QSizePolicy::Ignored)
            s.setWidth(1);
        if (wid->sizePolicy().verData() == QSizePolicy::Ignored)
            s.setHeight(1);
        s = s.boundedTo(wid->maximumSize())
            .expandedTo(wid->minimumSize()).expandedTo(QSize(1, 1));
    }
    return s;
}

/*!
    Returns true because a spacer item never contains widgets.
*/
bool QSpacerItem::isEmpty() const
{
    return true;
}

/*!
    Returns true if the widget has been hidden; otherwise returns
    false.
*/
bool QWidgetItem::isEmpty() const
{
    return wid->isHidden() || wid->isTopLevel();
}

/*!
    \class QLayout
    \brief The QLayout class is the base class of geometry managers.

    \ingroup appearance
    \ingroup geomanagement

    This is an abstract base class inherited by the concrete classes,
    QBoxLayout and QGridLayout.

    For users of QLayout subclasses or of QMainWindow there is seldom
    any need to use the basic functions provided by QLayout, such as
    \l setResizeMode() or setMenuBar(). See the \link layout.html layout
    overview page \endlink for more information.

    To make your own layout manager, implement the functions
    addItem(), sizeHint(), setGeometry(), itemAt() and takeAt(). You
    should also implement minimumSize() to ensure your layout isn't
    resized to zero size if there is too little space. To support
    children whose heights depend on their widths, implement
    hasHeightForWidth() and heightForWidth(). See the \link
    customlayout.html custom layout page \endlink for an in-depth
    description.

    Geometry management stops when the layout manager is deleted.
*/

/*!
    Constructs a new top-level QLayout called \a name, with main
    widget \a parent. \a parent may not be 0.

    The \a margin is the number of pixels between the edge of the
    widget and the managed children. The \a spacing sets the value of
    spacing(), which gives the spacing between the managed widgets. If
    \a spacing is -1 (the default), spacing is set to the value of \a
    margin.

    There can be only one top-level layout for a widget. It is
    returned by QWidget::layout()
*/
QLayout::QLayout(QWidget *parent, int margin, int spacing, const char *name)
    : QObject(parent, name)
{
    init();
    outsideBorder = margin;
    if (spacing < 0)
        insideSpacing = margin;
    else
        insideSpacing = spacing;
    if (parent) {
        if (parent->layout()) {
            qWarning("QLayout \"%s\" added to %s \"%s\", which already has a"
                      " layout", QObject::objectName(), parent->className(),
                      parent->objectName());
            parent->layout()->setParent(0);
        } else {
            topLevel = true;
            parent->d->layout = this;
            invalidate();
        }
    }
}

void QLayout::init()
{
    insideSpacing = 0;
    outsideBorder = 0;
    topLevel = false;
    enabled = true;
    autoNewChild = false;
    frozen = false;
    activated = true;
    marginImpl = false;
    autoMinimum = false;
    autoResizeMode = true;
    extraData = 0;
#ifndef QT_NO_MENUBAR
    menubar = 0;
#endif
}

/*!
    Constructs a new child QLayout called \a name, and places it
    inside \a parentLayout by using the default placement defined by
    addItem().

    If \a spacing is -1, this QLayout inherits \a parentLayout's
    spacing(), otherwise the value of \a spacing is used.
*/
QLayout::QLayout(QLayout *parentLayout, int spacing, const char *name)
    : QObject(parentLayout, name)

{
    init();
    insideSpacing = spacing < 0 ? parentLayout->insideSpacing : spacing;
    parentLayout->addItem(this);
}

/*!
    Constructs a new child QLayout called \a name. If \a spacing is
    -1, this QLayout inherits its parent's spacing(); otherwise the
    value of \a spacing is used.

    This layout has to be inserted into another layout before geometry
    management will work.
*/
QLayout::QLayout(int spacing, const char *name)
    : QObject(0, name)
{
    init();
    insideSpacing = spacing;
}

/*!
    \fn void QLayout::addItem(QLayoutItem *item)

    Implemented in subclasses to add an \a item. How it is added is
    specific to each subclass.

    The ownership of \a item is transferred to the layout, and it's
    the layout's responsibility to delete it.
*/

/*!
    Adds widget \a w to this layout in a manner specific to the
    layout. This function uses addItem().
*/
void QLayout::addWidget(QWidget *w)
{
    addChildWidget(w);
    addItem(new QWidgetItem(w));
}
/*!
    \fn QMenuBar* QLayout::menuBar () const

    Returns the menu bar set for this layout, or 0 if no menu bar is
    set.
*/

/*!
    \fn bool QLayout::isTopLevel () const

    Returns true if this layout is a top-level layout, i.e. not a
    child of another layout; otherwise returns false.
*/

/*!
    \property QLayout::margin
    \brief the width of the outside border of the layout

    For some layout classes this property has an effect only on
    top-level layouts; QBoxLayout and QGridLayout support margins for
    child layouts. The default value is 0.

    \sa spacing
*/

/*!
    \property QLayout::spacing
    \brief the spacing between widgets inside the layout

    The default value is -1, which signifies that the layout's spacing
    should not override the widget's spacing.

    \sa margin
*/
void QLayout::setMargin(int margin)
{
    outsideBorder = margin;
    invalidate();
}

void QLayout::setSpacing(int spacing)
{
    insideSpacing = spacing;
    if (spacing >= 0)
        propagateSpacing(this);
    invalidate();
}

/*!
    Returns the parent widget of this layout, or 0 if
    this layout is a sub-layout that is not yet inserted.
*/
QWidget *QLayout::parentWidget() const
{
    if (!topLevel) {
        if (parent()) {
            QLayout *parentLayout = ::qt_cast<QLayout*>(parent());
            Q_ASSERT(parentLayout);
            return parentLayout->parentWidget();
        } else {
            return 0;
        }
    } else {
        Q_ASSERT(parent() && parent()->isWidgetType());
        return static_cast<QWidget *>(parent());
    }
}

/*!
    Returns true if this layout is empty. The default implementation
    returns false.
*/
bool QLayout::isEmpty() const
{
    return false; //### should check
}

/*!
    This function is reimplemented in subclasses to perform layout.

    The default implementation maintains the geometry() information
    given by rect \a r. Reimplementors must call this function.
*/
void QLayout::setGeometry(const QRect &r)
{
    rect = r;
}

/*!
    Invalidates cached information. Reimplementations must call this.
*/
void QLayout::invalidate()
{
    rect = QRect();
    update();
}

static bool removeWidgetRecursively(QLayoutItem *li, QWidget *w)
{
    QLayout *lay = li->layout();
    if (!lay)
        return false;
    int i = 0;
    QLayoutItem *child;
    while ((child = lay->itemAt(i))) {
        if (child->widget() == w) {
            delete lay->takeAt(i);
            lay->invalidate();
            return true;
        } else if (removeWidgetRecursively(child, w)) {
            return true;
        } else {
            ++i;
        }
    }
    return false;
}

/*!
    \internal
    Performs child widget layout when the parent widget is
    resized.  Also handles removal of widgets. \a e is the
    event
*/
void QLayout::widgetEvent(QEvent *e)
{
    if (!enabled)
        return;

    switch (e->type()) {
    case QEvent::Resize:
        if (activated) {
            QResizeEvent *r = (QResizeEvent *)e;
            int mbh = 0;
#ifndef QT_NO_MENUBAR
            mbh = menuBarHeightForWidth(menubar, r->size().width());
#endif
            int b = marginImpl ? 0 : outsideBorder;
            QWidget *mw = parentWidget();
            QRect rect = mw->testAttribute(QWidget::WA_LayoutOnEntireRect)?mw->rect():mw->contentsRect();
            rect.addCoords(b, mbh + b, -b, -b);
            setGeometry(rect);
        } else {
            activate();
        }
        break;
    case QEvent::ChildRemoved:
        {
            QChildEvent *c = (QChildEvent *)e;
            if (c->child()->isWidgetType()) {
                QWidget *w = (QWidget *)c->child();
#ifndef QT_NO_MENUBAR
                if (w == menubar)
                    menubar = 0;
#endif
                removeWidgetRecursively(this, w);
            }
        }
        break;
#ifdef QT_COMPAT
    case QEvent::ChildInserted:
        if (topLevel && autoNewChild) {
            QChildEvent *c = (QChildEvent *)e;
            if (c->child()->isWidgetType()) {
                QWidget *w = (QWidget *)c->child();
                if (!w->isTopLevel()) {
#if !defined(QT_NO_MENUBAR) && !defined(QT_NO_TOOLBAR)
                    if (qt_cast<QMenuBar*>(w) && !::qt_cast<QToolBar*>(w->parentWidget())) {
                        menubar = (QMenuBar *)w;
                        invalidate();
                    } else
#endif
                        addItem(new QWidgetItem(w));
                }
            }
        }
        break;
    case QEvent::LayoutHint:
        activated = false;
        // fall through
#endif
    case QEvent::LayoutRequest:
        activate();
        break;
    default:
        break;
    }
}

/*!
    \reimp
*/
void QLayout::childEvent(QChildEvent *e)
{
    if (!enabled)
        return;

    if (e->type() == QEvent::ChildRemoved) {
        QChildEvent *c = (QChildEvent*)e;
        int i = 0;

        QLayoutItem *item;
        while ((item = itemAt(i))) {
            if (item == static_cast<QLayout*>(c->child())) {
                takeAt(i);
                invalidate();
                break;
            } else {
                ++i;
            }
        }
    }
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
int QLayout::totalHeightForWidth(int w) const
{
    if (topLevel)
        parent()->ensurePolished();
    int b = (topLevel && !marginImpl) ? 2 * outsideBorder : 0;
    int side=b, top=b;
    if (topLevel) {
        QWidgetPrivate *wd = parentWidget()->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }
    int h = heightForWidth(w - side) + top;
#ifndef QT_NO_MENUBAR
    h += menuBarHeightForWidth(menubar, w);
#endif
    return h;
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
QSize QLayout::totalMinimumSize() const
{
    if (topLevel)
        parent()->ensurePolished();
    int b = (topLevel && !marginImpl) ? 2 * outsideBorder : 0;

    int side=b, top=b;
    if (topLevel) {
        QWidgetPrivate *wd = parentWidget()->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = minimumSize();
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(menubar, s.width() + side);
#endif
    return s + QSize(side, top);
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
QSize QLayout::totalSizeHint() const
{
    if (topLevel)
        parent()->ensurePolished();
    int b = (topLevel && !marginImpl) ? 2 * outsideBorder : 0;
    int side=b, top=b;
    if (topLevel) {
        QWidgetPrivate *wd = parentWidget()->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = sizeHint();
    if (hasHeightForWidth())
        s.setHeight(heightForWidth(s.width() + side));
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(menubar, s.width());
#endif
    return s + QSize(side, top);
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
QSize QLayout::totalMaximumSize() const
{
    if (topLevel)
        parent()->ensurePolished();
    int b = (topLevel && !marginImpl) ? 2 * outsideBorder : 0;
    int side=b, top=b;
    if (topLevel) {
        QWidgetPrivate *wd = parentWidget()->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = maximumSize();
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(menubar, s.width());
#endif

    if (isTopLevel())
        s = QSize(qMin(s.width() + side, QLAYOUTSIZE_MAX),
                   qMin(s.height() + top, QLAYOUTSIZE_MAX));
    return s;
}

/*!
  \internal
  Destroys the layout, deleting all child layouts.
  Geometry management stops when a top-level layout is deleted.

  The layout classes will probably be fatally confused if you delete
  a sublayout.
*/
QLayout::~QLayout()
{
    /*
      This function may be called during the QObject destructor,
      when the parent no longer is a QWidget.
    */
    if (isTopLevel() && parent() && parent()->isWidgetType() &&
         ((QWidget*)parent())->layout() == this)
        ((QWidget*)parent())->d->layout = 0;
}

/*!
    Removes and deletes all items in this layout.
*/
void QLayout::deleteAllItems()
{
    QLayoutItem *l;
    while ((l = takeAt(0)))
        delete l;
}

/*!
    This function is called from addLayout() functions in subclasses
    to add layout \a l as a sub-layout.
*/
void QLayout::addChildLayout(QLayout *l)
{
    if (l->parent()) {
        qWarning("QLayout::addChildLayout: layout already has a parent");
        return;
    }
    l->setParent(this);
    if (l->insideSpacing < 0) {
        l->insideSpacing = insideSpacing;
        propagateSpacing(l);
    }
}

/*!
    This function is called from addWidget() functions in subclasses
    to add \a w as a child widget.
*/
void QLayout::addChildWidget(QWidget *w)
{
    QWidget *mw = parentWidget();
    QWidget *pw = w->parentWidget();

    //WA_Layouted is never reset. It only means that the widget at some point has
    //been in a layout.
    if (w->testAttribute(QWidget::WA_Layouted)) {
        QLayout *l = pw->layout();
        if (l && removeWidgetRecursively(l, w))
            qWarning("QLayout::addChildWidget: widget is already in a layout; moved to new layout");
    }
    if (!pw && !mw) {
        qWarning("QLayout::addChildWidget: add layout to parent before adding children to layout.");
    } else if (pw && mw && pw != mw) {
        qWarning("QLayout::addChildWidget: widget in wrong parent; moved to correct parent");
        pw = 0;
    }
    if (!pw && mw)
        w->setParent(mw);
    w->setAttribute(QWidget::WA_Layouted);
}


/*! \fn int QLayout::defaultBorder() const

  \internal
*/

/*! \fn void QLayout::freeze()

  \internal
*/

/*!
  \internal
  Fixes the size of the main widget and distributes the available
  space to the child widgets. For widgets which should not be
  resizable, but where a QLayout subclass is used to set up the initial
  geometry.

  As a special case, freeze(0, 0) is equivalent to setResizeMode(Fixed).
*/
void QLayout::freeze(int w, int h)
{
    if (w <= 0 || h <= 0) {
        setResizeMode(Fixed);
    } else {
        setResizeMode(FreeResize); // layout will not change min/max size
        parentWidget()->setFixedSize(w, h);
    }
}

#ifndef QT_NO_MENUBAR

/*!
    Makes the geometry manager take account of the menu bar \a w. All
    child widgets are placed below the bottom edge of the menu bar.

    A menu bar does its own geometry management: never do addWidget()
    on a QMenuBar.
*/
void QLayout::setMenuBar(QMenuBar *w)
{
    menubar = w;
}

#endif

/*!
    Returns the minimum size of this layout. This is the smallest size
    that the layout can have while still respecting the
    specifications. Does not include what's needed by margin() or
    menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::minimumSize() const
{
    return QSize(0, 0);
}

/*!
    Returns the maximum size of this layout. This is the largest size
    that the layout can have while still respecting the
    specifications. Does not include what's needed by margin() or
    menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::maximumSize() const
{
    return QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX);
}

/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.

    The default implementation returns \c BothDirections.
*/
QSizePolicy::ExpandData QLayout::expanding() const
{
    return QSizePolicy::BothDirections;
}

void QLayout::activateRecursiveHelper(QLayoutItem *item)
{
    item->invalidate();
    QLayout *layout = item->layout();
    if (layout) {
        QLayoutItem *child;
        int i=0;
        while ((child = layout->itemAt(i++))) 
            activateRecursiveHelper(child);
        layout->activated = true;
    }
}

/*!
  Updates the layout for parentWidget().

  You should generally not need to call this because it is
  automatically called at the most appropriate times.

  \sa activate(), invalidate()
*/

void QLayout::update()
{
    QLayout *layout = this;
    while (layout && layout->activated) {
        layout->activated = false;
        if (layout->topLevel) {
            Q_ASSERT(layout->parent()->isWidgetType());
            QWidget *mw = static_cast<QWidget*>(layout->parent());
            if (mw->isVisible())
                QApplication::postEvent(mw, new QEvent(QEvent::LayoutRequest));
            break;
        }
        layout = static_cast<QLayout*>(layout->parent());
    }
}

/*!
    Redoes the layout for parentWidget() if necessary.

    You should generally not need to call this because it is
    automatically called at the most appropriate times.

    \sa update(), QWidget::updateGeometry()
*/
bool QLayout::activate()
{
    if (!parent())
        return false;
    if (!topLevel)
        return static_cast<QLayout*>(parent())->activate();
    if (activated)
        return false;
    QWidget *mw = static_cast<QWidget*>(parent());
    if (mw == 0) {
        qWarning("QLayout::activate: %s \"%s\" does not have a main widget",
                  QObject::className(), QObject::objectName());
        return false;
    }
    activateRecursiveHelper(this);
    QSize s = mw->size();
    QSize ms;
    int mbh = 0;
#ifndef QT_NO_MENUBAR
    mbh = menuBarHeightForWidth(menubar, s.width());
#endif
    int b = marginImpl ? 0 : outsideBorder;
    QRect rect = mw->testAttribute(QWidget::WA_LayoutOnEntireRect)?mw->rect():mw->contentsRect();
    rect.addCoords(b, mbh + b, -b, -b);
    setGeometry(rect);
    if (frozen) {
        // will trigger resize
        mw->setFixedSize(totalSizeHint());
    } else if (autoMinimum) {
        ms = totalMinimumSize();
    } else if (autoResizeMode && mw->isTopLevel()) {
        ms = totalMinimumSize();
        if (hasHeightForWidth()) {
            int h = minimumHeightForWidth(ms.width());
            if (h > ms.height())
                ms = QSize(0, 0);
        }
    }

    if (ms.isValid())
        mw->setMinimumSize(ms);

    // ideally only if sizeHint() or sizePolicy() has changed
    mw->updateGeometry();
    return true;
}

/*!
    \class QSizePolicy
    \brief The QSizePolicy class is a layout attribute describing horizontal
    and vertical resizing policy.

    \ingroup appearance
    \ingroup geomanagement

    The size policy of a widget is an expression of its willingness to
    be resized in various ways.

    Widgets that reimplement QWidget::sizePolicy() return a QSizePolicy
    that describes the horizontal and vertical resizing policy they
    prefer when being laid out. Only \link #interesting one of the
    constructors\endlink is of interest in most applications.

    QSizePolicy contains two independent SizeType objects; one describes
    the widgets's horizontal size policy, and the other describes its
    vertical size policy. It also contains a flag to indicate whether the
    height and width of its preferred size are related.

    The horizontal and vertical \l{SizeType}s are set in the usual constructor
    and can be queried using a variety of functions.

    The hasHeightForWidth() flag indicates whether the widget's sizeHint()
    is width-dependent (such as a word-wrapping label) or not.

    \sa QSizePolicy::SizeType
*/

/*!
    \enum QSizePolicy::SizeType

    The per-dimension sizing types used when constructing a
    QSizePolicy are:

    \value Fixed  the QWidget::sizeHint() is the only acceptable
    alternative, so the widget can never grow or shrink (e.g. the
    vertical direction of a push button).

    \value Minimum  the sizeHint() is minimal, and sufficient. The
    widget can be expanded, but there is no advantage to it being
    larger (e.g. the horizontal direction of a push button).

    \value Maximum  the sizeHint() is a maximum. The widget can be
    shrunk any amount without detriment if other widgets need the
    space (e.g. a separator line).

    \value Preferred  the sizeHint() is best, but the widget can be
    shrunk and still be useful. The widget can be expanded, but there
    is no advantage to it being larger than sizeHint() (the default
    QWidget policy).

    \value Expanding  the sizeHint() is a sensible size, but the
    widget can be shrunk and still be useful. The widget can make use
    of extra space, so it should get as much space as possible (e.g.
    the horizontal direction of a slider).

    \value MinimumExpanding the sizeHint() is minimal, and sufficient.
    The widget can make use of extra space, so it should get as much
    space as possible (e.g. the horizontal direction of a slider).

    \value Ignored the sizeHint() is ignored. The widget will get as
    much space as possible.
*/

/*!
    \enum QSizePolicy::ExpandData

    This enum type describes in which directions a widget can make use
    of extra space. There are four possible values:

    \value NoDirection  the widget cannot make use of extra space in
    any direction.

    \value Horizontally  the widget can usefully be wider than the
    sizeHint().

    \value Vertically  the widget can usefully be taller than the
    sizeHint().

    \value BothDirections  the widget can usefully be both wider and
    taller than the sizeHint().
*/

/*!
    \fn QSizePolicy::QSizePolicy()

    Constructs a minimally initialized QSizePolicy.
*/

/*!
    \fn QSizePolicy::QSizePolicy(SizeType hor, SizeType ver, bool hfw)

    \target interesting
    This is the constructor normally used to return a value in the
    overridden \l QWidget::sizePolicy() function of a QWidget
    subclass.

    It constructs a QSizePolicy with independent horizontal and
    vertical sizing types, \a hor and \a ver respectively. These \link
    QSizePolicy::SizeType sizing types\endlink affect how the widget
    is treated by the \link QLayout layout engine\endlink.

    If \a hfw is true, the preferred height of the widget is dependent
    on the width of the widget (for example, a QLabel with line
    wrapping).

    \sa horData() verData() hasHeightForWidth()
*/

/*!
    \fn QSizePolicy::QSizePolicy(SizeType hor, SizeType ver, uchar horStretch, uchar verStretch, bool hfw)

    Constructs a QSizePolicy with independent horizontal and vertical
    sizing types \a hor and \a ver, and stretch factors \a horStretch
    and \a verStretch.

    If \a hfw is true, the preferred height of the widget is dependent on the
    width of the widget.

    \sa horStretch() verStretch()
*/

/*!
    \fn QSizePolicy::SizeType QSizePolicy::horData() const

    Returns the horizontal component of the size policy.

    \sa setHorData() verData() horStretch()
*/

/*!
    \fn QSizePolicy::SizeType QSizePolicy::verData() const

    Returns the vertical component of the size policy.

    \sa setVerData() horData() verStretch()
*/

/*!
    \fn bool QSizePolicy::mayShrinkHorizontally() const

    Returns true if the widget can sensibly be narrower than its
    sizeHint(); otherwise returns false.

    \sa mayShrinkVertically() mayGrowHorizontally()
*/

/*!
    \fn bool QSizePolicy::mayShrinkVertically() const

    Returns true if the widget can sensibly be shorter than its
    sizeHint(); otherwise returns false.

    \sa mayShrinkHorizontally() mayGrowVertically()
*/

/*!
    \fn bool QSizePolicy::mayGrowHorizontally() const

    Returns true if the widget can sensibly be wider than its
    sizeHint(); otherwise returns false.

    \sa mayGrowVertically() mayShrinkHorizontally()
*/

/*!
    \fn bool QSizePolicy::mayGrowVertically() const

    Returns true if the widget can sensibly be taller than its
    sizeHint(); otherwise returns false.

    \sa mayGrowHorizontally() mayShrinkVertically()
*/

/*!
    \fn QSizePolicy::ExpandData QSizePolicy::expanding() const

    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.

    \sa mayShrinkHorizontally() mayGrowHorizontally()
        mayShrinkVertically() mayGrowVertically()
*/

/*!
    \fn void QSizePolicy::setHorData(SizeType d)

    Sets the horizontal component of the size policy to size type \a
    d.

    \sa horData() setVerData()
*/

/*!
    \fn void QSizePolicy::setVerData(SizeType d)

    Sets the vertical component of the size policy to size type \a d.

    \sa verData() setHorData()
*/

/*!
    \fn bool QSizePolicy::hasHeightForWidth() const

    Returns true if the widget's preferred height depends on its
    width; otherwise returns false.

    \sa setHeightForWidth()
*/

/*!
    \fn void QSizePolicy::setHeightForWidth(bool b)

    Sets the hasHeightForWidth() flag to \a b.

    \sa hasHeightForWidth()
*/

/*!
    \fn uint QSizePolicy::horStretch() const

    Returns the horizontal stretch factor of the size policy.

    \sa setHorStretch() verStretch()
*/

/*!
    \fn uint QSizePolicy::verStretch() const

    Returns the vertical stretch factor of the size policy.

    \sa setVerStretch() horStretch()
*/

/*!
    \fn void QSizePolicy::setHorStretch(uchar sf)

    Sets the horizontal stretch factor of the size policy to \a sf.

    \sa horStretch() setVerStretch()
*/

/*!
    \fn void QSizePolicy::setVerStretch(uchar sf)

    Sets the vertical stretch factor of the size policy to \a sf.

    \sa verStretch() setHorStretch()
*/

/*!
    \fn void QSizePolicy::transpose()

    Swaps the horizontal and vertical policies and stretches.
*/


/*!
    \fn bool QSizePolicy::operator==(const QSizePolicy &s) const

    Returns true if this policy is equal to \a s; otherwise returns
    false.

    \sa operator!=()
*/

/*!
    \fn bool QSizePolicy::operator!=(const QSizePolicy &s) const

    Returns true if this policy is different from \a s; otherwise
    returns false.

    \sa operator==()
*/

/*!
  \fn QLayoutItem *QLayout::itemAt(int index) const

  Must be implemented in subclasses to return the layout item at \a
  index. If there is no such item, the function must return 0.
  Items are numbered consecutively from 0. If an item is deleted, other items will be renumbered.

  This function can be used to iterate over a layout. The following
  code will draw a rectangle for each layout item in the layout structure of the widget.
    \code
    static void paintLayout(QPainter *p, QLayoutItem *item)
    {
        QLayout *layout = item->layout();
        if (layout) {
            QLayoutItem *child;
            int i = 0;
            while ((child = layout->itemAt(i)) != 0) {
                paintLayout(p, child);
                ++i;
            }
        }
        p->drawRect(lay->geometry());
    }
    void ExampleWidget::paintEvent(QPaintEvent *)
    {
        QPainter p(this);
        if (layout())
            paintLayout(&p, layout());
    }
    \endcode

    \sa QLayout::takeAt()
*/

/*!
 \fn QLayoutItem *QLayout::takeAt(int index)

  Must be implemented in subclasses to remove the layout item at \a
  index from the layout, and return the item. If there is no such
  item, the function must do nothing and return 0.  Items are numbered
  consecutively from 0. If an item is deleted, other items will be
  renumbered.

  The following code fragment shows a safe way to remove all items from a layout:
  \code
  
  QLayoutItem *child;
  while((child = layout->takeAt(0)) != 0) {
     //process child...
  }
  \endcode
  
    \sa QLayout::itemAt()
*/



/*!
    \enum QLayout::ResizeMode

    The possible values are:

    \value Auto  If the main widget is a top-level widget with no
                 height-for-width (hasHeightForWidth()), this is
                 the same as \c Minimium; otherwise, this is the
                 same as \c FreeResize.
    \value Fixed  The main widget's size is set to sizeHint(); it
                  cannot be resized at all.
    \value Minimum  The main widget's minimum size is set to
                    minimumSize(); it cannot be smaller.
    \value FreeResize  The widget is not constrained.
*/

/*!
    \property QLayout::resizeMode
    \brief the resize mode of the layout

    The default mode is \c Auto.

    \sa QLayout::ResizeMode
*/

void QLayout::setResizeMode(ResizeMode mode)
{
    if (mode == resizeMode())
        return;

    switch (mode) {
    case Auto:
        frozen = false;
        autoMinimum = false;
        autoResizeMode = true;
        break;
    case Fixed:
        frozen = true;
        autoMinimum = false;
        autoResizeMode = false;
        break;
    case FreeResize:
        frozen = false;
        autoMinimum = false;
        autoResizeMode = false;
        break;
    case Minimum:
        frozen = false;
        autoMinimum = true;
        autoResizeMode = false;
    }
    invalidate();
}

QLayout::ResizeMode QLayout::resizeMode() const
{
    return (autoResizeMode ? Auto :
             (frozen ? Fixed : (autoMinimum ? Minimum : FreeResize)));
}

/*!
    \fn  bool QLayout::supportsMargin() const

    Returns true if this layout supports \l QLayout::margin on
    non-top-level layouts; otherwise returns false.

    \sa margin
*/

/*!
    Sets the value returned by supportsMargin(). If \a b is true,
    margin() handling is implemented by the subclass. If \a b is
    false (the default), QLayout will add margin() around top-level
    layouts.

    If \a b is true, margin handling needs to be implemented in
    setGeometry(), maximumSize(), minimumSize(), sizeHint() and
    heightForWidth().

    \sa supportsMargin()
*/
void QLayout::setSupportsMargin(bool b)
{
    marginImpl = b;
}

/*!
    Returns the rectangle that should be covered when the geometry of
    this layout is set to \a r, provided that this layout supports
    setAlignment().

    The result is derived from sizeHint() and expanding(). It is never
    larger than \a r.
*/
QRect QLayout::alignmentRect(const QRect &r) const
{
    QSize s = sizeHint();
    Qt::Alignment a = alignment();

    /*
      This is a hack to obtain the real maximum size, not
      QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX), the value consistently
      returned by QLayoutItems that have an alignment.
    */
    QLayout *that = (QLayout *) this;
    that->setAlignment(0);
    QSize ms = maximumSize();
    that->setAlignment(a);

    if ((expanding() & QSizePolicy::Horizontally) ||
         !(a & Qt::AlignHorizontal_Mask)) {
        s.setWidth(qMin(r.width(), ms.width()));
    }
    if ((expanding() & QSizePolicy::Vertically) ||
         !(a & Qt::AlignVertical_Mask)) {
        s.setHeight(qMin(r.height(), ms.height()));
    } else if (hasHeightForWidth()) {
        int hfw = heightForWidth(s.width());
        if (hfw < s.height())
            s.setHeight(qMin(hfw, ms.height()));
    }

    int x = r.x();
    int y = r.y();

    if (a & Qt::AlignBottom)
        y += (r.height() - s.height());
    else if (!(a & Qt::AlignTop))
        y += (r.height() - s.height()) / 2;

    a = QApplication::horizontalAlignment(a);
    if (a & Qt::AlignRight)
        x += (r.width() - s.width());
    else if (!(a & Qt::AlignLeft))
        x += (r.width() - s.width()) / 2;

    return QRect(x, y, s.width(), s.height());
}

/*!
    Removes the widget \a widget from the layout. After this call, it
    is the caller's responsibility to give the widget a reasonable
    geometry or to put the widget back into a layout.

    \sa removeItem(), QWidget::setGeometry(), addWidget()
*/
void QLayout::removeWidget(QWidget *widget)
{
    int i = 0;
    QLayoutItem *child;
    while ((child = itemAt(i))) {
        if (child->widget() == widget) {
            delete takeAt(i);
            invalidate();
        } else {
            ++i;
        }
    }
}

/*!
    Removes the layout item \a item from the layout. It is the
    caller's responsibility to delete the item.

    Notice that \a item can be a layout (since QLayout inherits
    QLayoutItem).

    \sa removeWidget(), addItem()
*/
void QLayout::removeItem(QLayoutItem *item)
{
    int i = 0;
    QLayoutItem *child;
    while ((child = itemAt(i))) {
        if (child == item) {
            takeAt(i);
            invalidate();
        } else {
            ++i;
        }
    }
}

/*!
    Enables this layout if \a enable is true, otherwise disables it.

    An enabled layout adjusts dynamically to changes; a disabled
    layout acts as if it did not exist.

    By default all layouts are enabled.

    \sa isEnabled()
*/
void QLayout::setEnabled(bool enable)
{
    enabled = enable;
}

/*!
    Returns true if the layout is enabled; otherwise returns false.

    \sa setEnabled()
*/
bool QLayout::isEnabled() const
{
    return enabled;
}

void QLayout::propagateSpacing(QLayout *parent)
{
    int i = 0;
    QLayoutItem *child;
    while ((child = parent->itemAt(i))) {
        QLayout *childLayout = child->layout();
        if (childLayout && childLayout->insideSpacing < 0) {
            childLayout->insideSpacing = parent->insideSpacing;
            propagateSpacing(childLayout);
        }
        ++i;
    }
}

#endif // QT_NO_LAYOUT
