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

#include "qlayout.h"

#ifndef QT_NO_LAYOUT
#include "qapplication.h"
#include "qlayoutengine_p.h"
#include "qmenubar.h"
#include "qtoolbar.h"
#include "qevent.h"
#include "qstyle.h"
#include "qwidget_p.h"
#include "qlayout_p.h"
#define d d_func()
#define q q_func()

static int menuBarHeightForWidth(QWidget *menubar, int w)
{
    if (menubar && !menubar->isHidden() && !menubar->isTopLevel())
        return menubar->heightForWidth(qMax(w, menubar->minimumWidth()));
    return 0;
}

/*!
    \class QLayoutItem
    \brief The QLayoutItem class provides an abstract item that a
    QLayout manipulates.

    \ingroup appearance
    \ingroup geomanagement

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

    Some layouts have width and height interdependencies. These can be
    expressed using hasHeightForWidth(), heightForWidth(), and
    minimumHeightForWidth(). For more explanation see the \link
    http://doc.trolltech.com/qq/ Qt Quarterly\endlink article, \link
    http://doc.trolltech.com/qq/qq04-height-for-width.html Trading
    Height for Width\endlink.

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

    Constructs a layout item with an \a alignment.
    Not all subclasses support alignment.
*/

/*!
    \fn Qt::Alignment QLayoutItem::alignment() const

    Returns the alignment of this item.
*/

/*!
    Sets the alignment of this item to \a alignment.
    Not all subclasses support alignment.
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
    Qt::Alignment alignHoriz = QStyle::visualAlignment(wid->layoutDirection(),  align);
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
    return d->rect;
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
    if (hfw < 0)
        hfw = 0;
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
    sizeHint(). A value of \c Qt::Vertical or \c Qt::Horizontal means that it wants
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
        if (wid->sizePolicy().horizontalData() == QSizePolicy::Ignored)
            s.setWidth(1);
        if (wid->sizePolicy().verticalData() == QSizePolicy::Ignored)
            s.setHeight(1);
        s = s.boundedTo(wid->maximumSize())
            .expandedTo(wid->minimumSize());
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
    Constructs a new top-level QLayout, with parent \a parent.
    \a parent may not be 0.

    There can be only one top-level layout for a widget. It is
    returned by QWidget::layout()
*/
QLayout::QLayout(QWidget *parent)
    : QObject(*new QLayoutPrivate, parent)
{
    if (parent) {
        if (parent->layout()) {
            qWarning("QLayout \"%s\" added to %s \"%s\", which already has a"
                     " layout", QObject::objectName().toLocal8Bit().data(), parent->metaObject()->className(),
                     parent->objectName().toLocal8Bit().data());
            parent->layout()->setParent(0);
        } else {
            d->topLevel = true;
            parent->d->layout = this;
            invalidate();
        }
    }
}


/*!
    Constructs a new child QLayout, and places it inside \a
    parentLayout by using the default placement defined by addItem().
*/
QLayout::QLayout(QLayout *parentLayout)
    : QObject(*new QLayoutPrivate,parentLayout)
{
    if (parentLayout)
        parentLayout->addItem(this);
}

/*!
    Constructs a new child QLayout.

    This layout has to be inserted into another layout before geometry
    management will work.
*/
QLayout::QLayout()
    : QObject(*new QLayoutPrivate, 0)
{
}


/*! \internal
 */
QLayout::QLayout(QLayoutPrivate &dd, QLayout *lay, QWidget *w)
    : QObject(dd, lay ? static_cast<QObject*>(lay) : static_cast<QObject*>(w))
{
    if (lay) {
        lay->addItem(this);
    } else if (w) {
        if (w->layout()) {
            qWarning("QLayout \"%s\" added to %s \"%s\", which already has a"
                     " layout", QObject::objectName().toLocal8Bit().data(), w->metaObject()->className(),
                     w->objectName().toLocal8Bit().data());
            w->layout()->setParent(0);
        } else {
            d->topLevel = true;
            w->d->layout = this;
            invalidate();
        }
    }
}

QLayoutPrivate::QLayoutPrivate()
    : QObjectPrivate(), insideSpacing(-1), outsideBorder(-1), topLevel(false), enabled(true), frozen(false),
      activated(true), autoMinimum(false), autoResizeMode(true), autoNewChild(false)
#ifndef QT_NO_MENUBAR
      , menubar(0)
#endif
{
}




#ifdef QT_COMPAT
/*!
    Constructs a new top-level QLayout called \a name, with parent
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
    : QObject(*new QLayoutPrivate,parent)
{
    setObjectName(name);
    d->outsideBorder = margin;
    if (spacing < 0)
        d->insideSpacing = margin;
    else
        d->insideSpacing = spacing;
    if (parent) {
        if (parent->layout()) {
            qWarning("QLayout \"%s\" added to %s \"%s\", which already has a"
                      " layout", QObject::objectName().toLocal8Bit().data(), parent->metaObject()->className(),
                      parent->objectName().toLocal8Bit().data());
            parent->layout()->setParent(0);
        } else {
            d->topLevel = true;
            parent->d->layout = this;
            invalidate();
        }
    }
}

/*!
    Constructs a new child QLayout called \a name, and places it
    inside \a parentLayout by using the default placement defined by
    addItem().

    If \a spacing is -1, this QLayout inherits \a parentLayout's
    spacing(), otherwise the value of \a spacing is used.
*/
QLayout::QLayout(QLayout *parentLayout, int spacing, const char *name)
    : QObject(*new QLayoutPrivate,parentLayout)

{
    setObjectName(name);
    d->insideSpacing = spacing;
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
    : QObject(*new QLayoutPrivate, 0)
{
    setObjectName(name);
    d->insideSpacing = spacing;
}

/*!
    Automatically adding widgets is deprecated. Use addWidget() or
    addLayout() instead.
*/
void QLayout::setAutoAdd(bool a) { d->autoNewChild = a; }

/*!
    Automatically adding widgets is deprecated. Use addWidget() or
    addLayout() instead.
*/
bool QLayout::autoAdd() const { return d->autoNewChild; }
#endif


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
    \fn bool QLayout::isTopLevel () const

    Returns true if this layout is a top-level layout, i.e. not a
    child of another layout; otherwise returns false.
*/

/*!
    \property QLayout::margin
    \brief the width of the outside border of the layout

    \sa spacing
*/

/*!
    \property QLayout::spacing
    \brief the spacing between widgets inside the layout

    The default value is -1, which signifies that the layout's spacing
    is inherited from the parent layout, or from the style settings for the parent widget.

    \sa margin
*/


int QLayout::margin() const
{
    if ( d->outsideBorder >= 0 )
        return d->outsideBorder;
    if (!d->topLevel)
        return 0;
    QWidget *pw = parentWidget();
    if (pw)
        return pw->style()->pixelMetric(
            (pw->isTopLevel() || pw->testWFlags(Qt::WSubWindow))
            ? QStyle::PM_DefaultToplevelMargin
            : QStyle::PM_DefaultChildMargin
            );
    return 0;
}


int QLayout::spacing() const
{
    if (d->insideSpacing >=0) {
        return d->insideSpacing;
    } else if (d->topLevel) {
        QWidget *pw = parentWidget();
        if (pw)
            return pw->style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
        else
            return QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    } else if (parent()) {
        return static_cast<QLayout*>(parent())->spacing();
    } else {
        return -1; //this is a layout that hasn't been inserted yet
    }
}

bool QLayout::isTopLevel() const
{
    return d->topLevel;
}

void QLayout::setMargin(int margin)
{
    d->outsideBorder = margin;
    invalidate();
}


void QLayout::setSpacing(int spacing)
{
    d->insideSpacing = spacing;
    invalidate();
}

/*!
    Returns the parent widget of this layout, or 0 if
    this layout is a sub-layout that is not yet inserted.
*/
QWidget *QLayout::parentWidget() const
{
    if (!d->topLevel) {
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
    d->rect = r;
}

/*!
    Invalidates cached information. Reimplementations must call this.
*/
void QLayout::invalidate()
{
    d->rect = QRect();
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
    if (!d->enabled)
        return;

    switch (e->type()) {
    case QEvent::Resize:
        if (d->activated) {
            QResizeEvent *r = (QResizeEvent *)e;
            int mbh = menuBarHeightForWidth(d->menubar, r->size().width());
            QWidget *mw = parentWidget();
            QRect rect = mw->testAttribute(Qt::WA_LayoutOnEntireRect)?mw->rect():mw->contentsRect();
            rect.setTop(rect.top() + mbh); //goes away when menubar isn't magic anymore
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
                if (w == d->menubar)
                    d->menubar = 0;
#endif
                removeWidgetRecursively(this, w);
            }
        }
        break;
#ifdef QT_COMPAT
    case QEvent::ChildInserted:
        if (d->topLevel && d->autoNewChild) {
            QChildEvent *c = (QChildEvent *)e;
            if (c->child()->isWidgetType()) {
                QWidget *w = (QWidget *)c->child();
                if (!w->isTopLevel()) {
#if !defined(QT_NO_MENUBAR) && !defined(QT_NO_TOOLBAR)
                    if (qt_cast<QMenuBar*>(w) && !::qt_cast<QToolBar*>(w->parentWidget())) {
                        d->menubar = (QMenuBar *)w;
                        invalidate();
                    } else
#endif
                        addItem(new QWidgetItem(w));
                }
            }
        }
        break;
    case QEvent::LayoutHint:
        d->activated = false;
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
    if (!d->enabled)
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
  Also takes contentsMargins and menu bar into account.
*/
int QLayout::totalHeightForWidth(int w) const
{
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *parent = parentWidget();
        parent->ensurePolished();
        QWidgetPrivate *wd = parent->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }
    int h = heightForWidth(w - side) + top;
#ifndef QT_NO_MENUBAR
    h += menuBarHeightForWidth(d->menubar, w);
#endif
    return h;
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
QSize QLayout::totalMinimumSize() const
{
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *pw = parentWidget();
        pw->ensurePolished();
        QWidgetPrivate *wd = pw->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = minimumSize();
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(d->menubar, s.width() + side);
#endif
    return s + QSize(side, top);
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
QSize QLayout::totalSizeHint() const
{
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *pw = parentWidget();
        pw->ensurePolished();
        QWidgetPrivate *wd = pw->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = sizeHint();
    if (hasHeightForWidth())
        s.setHeight(heightForWidth(s.width() + side));
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(d->menubar, s.width());
#endif
    return s + QSize(side, top);
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
QSize QLayout::totalMaximumSize() const
{
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *pw = parentWidget();
        pw->ensurePolished();
        QWidgetPrivate *wd = pw->d;
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = maximumSize();
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(d->menubar, s.width());
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
}

/*!
    This function is called from addWidget() functions in subclasses
    to add \a w as a child widget.

    If \a w is already in a layout, this function will give a warning
    and remove \a w from the layout. This function must therefore be
    called before adding \a w to the layout's data structure.
*/
void QLayout::addChildWidget(QWidget *w)
{
    QWidget *mw = parentWidget();
    QWidget *pw = w->parentWidget();

    //Qt::WA_LaidOut is never reset. It only means that the widget at some point has
    //been in a layout.
    if (w->testAttribute(Qt::WA_LaidOut)) {
        QLayout *l = pw->layout();
        if (l && removeWidgetRecursively(l, w))
            qWarning("QLayout::addChildWidget: %s is already in a layout; moved to new layout", w->metaObject()->className());
    }
    if (!pw && !mw) {
        qWarning("QLayout::addChildWidget: add layout to parent before adding children to layout.");
    } else if (pw && mw && pw != mw) {
        qWarning("QLayout::addChildWidget: %s in wrong parent; moved to correct parent", w->metaObject()->className());
        pw = 0;
    }
    if (!pw && mw)
        w->setParent(mw);
    w->setAttribute(Qt::WA_LaidOut);
}


/*!

  Sets this layout's parent widget to a fixed size with width \a w and
  height \a h, stopping the user form resizing it, and also prevents the
  layout from resizing it, even if the layout's size hint should
  change. Does nothing if this is not a toplevel layout (isTopLevel()
  returns true).

  As a special case, if both \a w and \a h are 0, then the layout's
  current sizeHint() is used.

  Use \c setResizeMode(Fixed) to stop the widget from being resized by
  the user, while still allowing the layout to resize it when the sizeHint() changes.

  Use \c setResizeMode(FreeResize) to allow the user to resize the
  widget, while preventing the layout from resizing it.

*/
void QLayout::freeze(int w, int h)
{
    if (!d->topLevel)
        return;
    if (w <= 0 || h <= 0) {
        QSize s = totalSizeHint();
        w = s.width();
        h = s.height();
    }
    setResizeMode(FreeResize); // layout will not change min/max size
    QWidget *parent = parentWidget();
    if (parent)
        parent->setFixedSize(w, h);
}

/*!
    Makes the geometry manager take account of the menu bar \a w. All
    child widgets are placed below the bottom edge of the menu bar.

    A menu bar does its own geometry management: never do addWidget()
    on a QMenuBar.
*/
void QLayout::setMenuBar(QWidget *w)
{
    d->menubar = w;
}

/*!
    Returns the menu bar set for this layout, or 0 if no menu bar is
    set.
*/

QWidget *QLayout::menuBar() const
{
    return d->menubar;
}


/*!
    Returns the minimum size of this layout. This is the smallest size
    that the layout can have while still respecting the
    specifications. Does not include what's needed by QWidget::setContentsMargins() or
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
    specifications. Does not include what's needed by QWidget::setContentsMargins() or
    menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::maximumSize() const
{
    return QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX);
}

/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Qt::Vertical or \c Qt::Horizontal means that it wants
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
        layout->d->activated = true;
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
    while (layout && layout->d->activated) {
        layout->d->activated = false;
        if (layout->d->topLevel) {
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
    if (!d->topLevel)
        return static_cast<QLayout*>(parent())->activate();
    if (d->activated)
        return false;
    QWidget *mw = static_cast<QWidget*>(parent());
    if (mw == 0) {
        qWarning("QLayout::activate: %s \"%s\" does not have a main widget",
                  QObject::metaObject()->className(), QObject::objectName().toLocal8Bit().data());
        return false;
    }
    activateRecursiveHelper(this);
    QSize s = mw->size();
    QSize ms;
    int mbh = 0;
#ifndef QT_NO_MENUBAR
    mbh = menuBarHeightForWidth(d->menubar, s.width());
#endif
    QRect rect = mw->testAttribute(Qt::WA_LayoutOnEntireRect)?mw->rect():mw->contentsRect();
    rect.setTop(rect.top() + mbh);
    setGeometry(rect);
    if (d->frozen) {
        // will trigger resize
        mw->setFixedSize(totalSizeHint());
    } else if (d->autoMinimum) {
        ms = totalMinimumSize();
    } else if (d->autoResizeMode && mw->isTopLevel()) {
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
    prefer when being laid out.

    QSizePolicy contains two independent SizeType objects and two
    stretch factors; one describes the widgets's horizontal size
    policy, and the other describes its vertical size policy. It also
    contains a flag to indicate whether the height and width of its
    preferred size are related.

    The horizontal and vertical \l{SizeType}s are set in the usual constructor
    and can be queried using a variety of functions.

    The hasHeightForWidth() flag indicates whether the widget's sizeHint()
    is width-dependent (such as a word-wrapping label) or not.

    \sa QSizePolicy::SizeType
*/

/*!
    \enum QSizePolicy::SizeTypeFlag

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

    \omitvalue Horizontal
    \omitvalue Vertical
*/

/*!
    \fn QSizePolicy::QSizePolicy()

    Constructs a minimally initialized QSizePolicy.
*/

/*!
    \fn QSizePolicy::QSizePolicy(SizeType hor, SizeType ver)

    It constructs a QSizePolicy with independent horizontal and
    vertical sizing types, \a hor and \a ver respectively. These \link
    QSizePolicy::SizeType sizing types\endlink affect how the widget
    is treated by the \link QLayout layout engine\endlink.

    Use setHeightForWidth() if the preferred height of the widget is dependent
    on the width of the widget (for example, a QLabel with line
    wrapping).

    \sa horizontalData() verticalData() setHeightForWidth()
*/


/*!
    \fn QSizePolicy::SizeType QSizePolicy::horizontalData() const

    Returns the horizontal component of the size policy.

    \sa setHorizontalData() verticalData() horizontalStretch()
*/

/*!
    \fn QSizePolicy::SizeType QSizePolicy::verticalData() const

    Returns the vertical component of the size policy.

    \sa setVerticalData() horizontalData() verticalStretch()
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
    sizeHint(). A value of \c Qt::Vertical or \c Qt::Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.

    \sa mayShrinkHorizontally() mayGrowHorizontally()
        mayShrinkVertically() mayGrowVertically()
*/

/*!
    \fn void QSizePolicy::setHorizontalData(SizeType d)

    Sets the horizontal component of the size policy to size type \a
    d.

    \sa horizontalData() setVerticalData()
*/

/*!
    \fn void QSizePolicy::setVerticalData(SizeType d)

    Sets the vertical component of the size policy to size type \a d.

    \sa verticalData() setHorizontalData()
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
    \fn uint QSizePolicy::horizontalStretch() const

    Returns the horizontal stretch factor of the size policy.

    \sa setHorizontalStretch() verticalStretch()
*/

/*!
    \fn uint QSizePolicy::verticalStretch() const

    Returns the vertical stretch factor of the size policy.

    \sa setVerticalStretch() horizontalStretch()
*/

/*!
    \fn void QSizePolicy::setHorizontalStretch(uchar sf)

    Sets the horizontal stretch factor of the size policy to \a sf.

    \sa horizontalStretch() setVerticalStretch()
*/

/*!
    \fn void QSizePolicy::setVerticalStretch(uchar sf)

    Sets the vertical stretch factor of the size policy to \a sf.

    \sa verticalStretch() setHorizontalStretch()
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
        d->frozen = false;
        d->autoMinimum = false;
        d->autoResizeMode = true;
        break;
    case Fixed:
        d->frozen = true;
        d->autoMinimum = false;
        d->autoResizeMode = false;
        break;
    case FreeResize:
        d->frozen = false;
        d->autoMinimum = false;
        d->autoResizeMode = false;
        break;
    case Minimum:
        d->frozen = false;
        d->autoMinimum = true;
        d->autoResizeMode = false;
    }
    invalidate();
}

QLayout::ResizeMode QLayout::resizeMode() const
{
    return (d->autoResizeMode ? Auto :
             (d->frozen ? Fixed : (d->autoMinimum ? Minimum : FreeResize)));
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

    QWidget *parent = parentWidget();
    a = QStyle::visualAlignment(parent ? parent->layoutDirection() : QApplication::layoutDirection(), a);
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
    d->enabled = enable;
}

/*!
    Returns true if the layout is enabled; otherwise returns false.

    \sa setEnabled()
*/
bool QLayout::isEnabled() const
{
    return d->enabled;
}

/*!
    Returns a size that satisfies all size constraints on \a w, including heightForWidth()
    and that is as close as possible to \a s.
*/

QSize QLayout::closestAcceptableSize(const QWidget *w, QSize s)
{
    QSize result = s.boundedTo(qSmartMaxSize(w));
    result = result.expandedTo(qSmartMinSize(w));
    QLayout *l = w->layout();
    if (l && l->hasHeightForWidth() && result.height() < l->minimumHeightForWidth(result.width()) ) {
        QSize current = w->size();
        int currentHfw =  l->minimumHeightForWidth(current.width());
        int newHfw = l->minimumHeightForWidth(result.width());
        if (current.height() < currentHfw || currentHfw == newHfw) {
            //handle the constant hfw case and the vertical-only case, as well as the
            // current-size-is-not-correct case
            result.setHeight(newHfw);
        } else {
            // binary search; assume hfw is decreasing ###

            int maxw = qMax(w->width(),result.width());
            int maxh = qMax(w->height(), result.height());
            int minw = qMin(w->width(),result.width());
            int minh = qMin(w->height(), result.height());

            int minhfw = l->minimumHeightForWidth(minw);
            int maxhfw = l->minimumHeightForWidth(maxw);
            while (minw < maxw) {
                if (minhfw > maxh) { //assume decreasing
                    minw = maxw - (maxw-minw)/2;
                    minhfw = l->minimumHeightForWidth(minw);
                } else if (maxhfw < minh ) { //assume decreasing
                    maxw = minw + (maxw-minw)/2;
                    maxhfw = l->minimumHeightForWidth(maxw);
                } else  {
                    break;
                }
            }
            result = result.expandedTo(QSize(minw, minhfw));
        }
    }
    return result;
}

#endif // QT_NO_LAYOUT
