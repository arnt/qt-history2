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

#include "qboxlayout.h"

#ifndef QT_NO_LAYOUT

#include "qapplication.h"
#include "qwidget.h"
#include "qlist.h"
#include "qsizepolicy.h"
#include "qvector.h"

#include "qlayoutengine_p.h"
#include "qlayout_p.h"
#define d d_func()
#define q q_func()


/*
  Returns true if the widget \a w can be added to the layout \a l;
  otherwise returns false.
*/
static bool checkWidget(QLayout *l, QWidget *w)
{
    if (!w) {
        qWarning("QLayout: Cannot add null widget to %s/%s", l->metaObject()->className(),
                  l->objectName().toLocal8Bit().data());
        return false;
    }
    return true;
}

struct QBoxLayoutItem
{
    QBoxLayoutItem(QLayoutItem *it, int stretch_ = 0)
        : item(it), stretch(stretch_), magic(false) { }
    ~QBoxLayoutItem() { delete item; }

    int hfw(int w) {
        if (item->hasHeightForWidth()) {
            return item->heightForWidth(w);
        } else {
            return item->sizeHint().height();
        }
    }
    int mhfw(int w) {
        if (item->hasHeightForWidth()) {
            return item->heightForWidth(w);
        } else {
            return item->minimumSize().height();
        }
    }
    int hStretch() {
        if (stretch == 0 && item->widget()) {
            return item->widget()->sizePolicy().horizontalStretch();
        } else {
            return stretch;
        }
    }
    int vStretch() {
        if (stretch == 0 && item->widget()) {
            return item->widget()->sizePolicy().verticalStretch();
        } else {
            return stretch;
        }
    }

    QLayoutItem *item;
    int stretch;
    bool magic;
};

class QBoxLayoutPrivate : public QLayoutPrivate
{
    Q_DECLARE_PUBLIC(QBoxLayout)
public:
    QBoxLayoutPrivate() : hfwWidth(-1), dirty(true) { }
    ~QBoxLayoutPrivate();

    void setDirty() {
        geomArray.clear();
        hfwWidth = -1;
        hfwHeight = -1;
        dirty = true;
    }

    QList<QBoxLayoutItem *> list;
    QVector<QLayoutStruct> geomArray;
    int hfwWidth;
    int hfwHeight;
    int hfwMinHeight;
    QSize sizeHint;
    QSize minSize;
    QSize maxSize;
    QSizePolicy::ExpandData expanding;
    uint hasHfw : 1;
    uint dirty : 1;
    QBoxLayout::Direction dir;

    inline void deleteAll() { while (!list.isEmpty()) delete list.takeFirst(); }

    void setupGeom();
    void calcHfw(int);

};

QBoxLayoutPrivate::~QBoxLayoutPrivate()
{
}

static inline bool horz(QBoxLayout::Direction dir)
{
    return dir == QBoxLayout::RightToLeft || dir == QBoxLayout::LeftToRight;
}

/*
    Initializes the data structure needed by qGeomCalc and
    recalculates max/min and size hint.
*/
void QBoxLayoutPrivate::setupGeom()
{
    if (!dirty)
        return;

    int maxw = horz(dir) ? 0 : QLAYOUTSIZE_MAX;
    int maxh = horz(dir) ? QLAYOUTSIZE_MAX : 0;
    int minw = 0;
    int minh = 0;
    int hintw = 0;
    int hinth = 0;

    bool horexp = false;
    bool verexp = false;

    hasHfw = false;

    int n = list.count();
    geomArray.clear();
    geomArray.resize(n);
    QVector<QLayoutStruct> &a = geomArray;

    bool first = true;
    for (int i = 0; i < n; i++) {
        QBoxLayoutItem *box = list.at(i);
        QSize max = box->item->maximumSize();
        QSize min = box->item->minimumSize();
        QSize hint = box->item->sizeHint();
        QSizePolicy::ExpandData exp = box->item->expanding();
        bool empty = box->item->isEmpty();
        // space before non-empties, except the first:
        int space = (empty || first) ? 0 : q->spacing();
        bool ignore = empty && box->item->widget(); // ignore hidden widgets

        if (horz(dir)) {
            bool expand = exp & QSizePolicy::Horizontally || box->stretch > 0;
            horexp = horexp || expand;
            maxw += max.width() + space;
            minw += min.width() + space;
            hintw += hint.width() + space;
            if (!ignore)
                qMaxExpCalc(maxh, verexp,
                             max.height(), exp & QSizePolicy::Vertically);
            minh = qMax(minh, min.height());
            hinth = qMax(hinth, hint.height());

            a[i].sizeHint = hint.width();
            a[i].maximumSize = max.width();
            a[i].minimumSize = min.width();
            a[i].expansive = expand;
            a[i].stretch = box->stretch ? box->stretch : box->hStretch();
        } else {
            bool expand = (exp & QSizePolicy::Vertically || box->stretch > 0);
            verexp = verexp || expand;
            maxh += max.height() + space;
            minh += min.height() + space;
            hinth += hint.height() + space;
            if (!ignore)
                qMaxExpCalc(maxw, horexp,
                             max.width(), exp & QSizePolicy::Horizontally);
            minw = qMax(minw, min.width());
            hintw = qMax(hintw, hint.width());

            a[i].sizeHint = hint.height();
            a[i].maximumSize = max.height();
            a[i].minimumSize = min.height();
            a[i].expansive = expand;
            a[i].stretch = box->stretch ? box->stretch : box->vStretch();
        }

        a[i].empty = empty;
        hasHfw = hasHfw || box->item->hasHeightForWidth();
        first = first && empty;
    }

    expanding = (QSizePolicy::ExpandData)
                       ((horexp ? QSizePolicy::Horizontally : 0)
                         | (verexp ? QSizePolicy::Vertically : 0));

    minSize = QSize(minw, minh);
    maxSize = QSize(maxw, maxh).expandedTo(minSize);
    sizeHint = QSize(hintw, hinth)
                     .expandedTo(minSize)
                     .boundedTo(maxSize);

    dirty = false;
}

/*
  Calculates and stores the preferred height given the width \a w.
*/
void QBoxLayoutPrivate::calcHfw(int w)
{
    int h = 0;
    int mh = 0;

    if (horz(dir)) {
        QVector<QLayoutStruct> &a = geomArray;
        int n = a.count();
        qGeomCalc(a, 0, n, 0, w, q->spacing());
        for (int i = 0; i < n; i++) {
            QBoxLayoutItem *box = list.at(i);
            h = qMax(h, box->hfw(a[i].size));
            mh = qMax(mh, box->mhfw(a[i].size));
        }
    } else {
        bool first = true;
        for (int i = 0; i < list.size(); ++i) {
            QBoxLayoutItem *box = list.at(i);
            bool empty = box->item->isEmpty();
            h += box->hfw(w);
            mh += box->mhfw(w);
            if (!first && !empty) {
                h += q->spacing();
                mh += q->spacing();
            }
            first = first && empty;
        }
    }
    hfwWidth = w;
    hfwHeight = h;
    hfwMinHeight = mh;
}


/*!
    \class QBoxLayout

    \brief The QBoxLayout class lines up child widgets horizontally or
    vertically.

    \ingroup geomanagement
    \ingroup appearance

    QBoxLayout takes the space it gets (from its parent layout or from
    the parentWidget()), divides it up into a row of boxes, and makes
    each managed widget fill one box.

    \img qhbox-m.png Qt::Horizontal box with five child widgets

    If the QBoxLayout's orientation is \c Qt::Horizontal the boxes are
    placed in a row, with suitable sizes. Each widget (or other box)
    will get at least its minimum size and at most its maximum size.
    Any excess space is shared according to the stretch factors (more
    about that below).

    \img qvbox-m.png Qt::Vertical box with five child widgets

    If the QBoxLayout's orientation is \c Qt::Vertical, the boxes are
    placed in a column, again with suitable sizes.

    The easiest way to create a QBoxLayout is to use one of the
    convenience classes, e.g. QHBoxLayout (for \c Qt::Horizontal boxes) or
    QVBoxLayout (for \c Qt::Vertical boxes). You can also use the
    QBoxLayout constructor directly, specifying its direction as \c
    LeftToRight, \c Down, \c RightToLeft or \c Up.

    If the QBoxLayout is not the top-level layout (i.e. it is not
    managing all of the widget's area and children), you must add it
    to its parent layout before you can do anything with it. The
    normal way to add a layout is by calling
    parentLayout-\>addLayout().

    Once you have done this, you can add boxes to the QBoxLayout using
    one of four functions:

    \list
    \i addWidget() to add a widget to the QBoxLayout and set the
    widget's stretch factor. (The stretch factor is along the row of
    boxes.)

    \i addSpacing() to create an empty box; this is one of the
    functions you use to create nice and spacious dialogs. See below
    for ways to set margins.

    \i addStretch() to create an empty, stretchable box.

    \i addLayout() to add a box containing another QLayout to the row
    and set that layout's stretch factor.
    \endlist

    Use insertWidget(), insertSpacing(), insertStretch() or
    insertLayout() to insert a box at a specified position in the
    layout.

    QBoxLayout also includes two margin widths:

    \list
    \i setMargin() sets the width of the outer border. This is the width
       of the reserved space along each of the QBoxLayout's four sides.
    \i setSpacing() sets the width between neighboring boxes. (You
       can use addSpacing() to get more space at a particular spot.)
    \endlist

    The margin defaults to 0. The spacing defaults to the same as the
    margin width for a top-level layout, or to the same as the parent
    layout. Both are parameters to the constructor.

    To remove a widget from a layout, call remove(). Calling
    QWidget::hide() on a widget also effectively removes the widget
    from the layout until QWidget::show() is called.

    You will almost always want to use QVBoxLayout and QHBoxLayout
    rather than QBoxLayout because of their convenient constructors.

    \sa QGrid \link layout.html Layout Overview \endlink
*/

/*!
    \enum QBoxLayout::Direction

    This type is used to determine the direction of a box layout.

    \value LeftToRight  Qt::Horizontal, from left to right
    \value RightToLeft  Qt::Horizontal, from right to left
    \value TopToBottom  Qt::Vertical, from top to bottom
    \value Down  The same as \c TopToBottom
    \value BottomToTop  Qt::Vertical, from bottom to top
    \value Up  The same as \c BottomToTop
*/

/*!
    Constructs a new QBoxLayout with direction \a dir and parent widget \a
    parent. \a parent may not be 0.

    \sa direction()
*/
QBoxLayout::QBoxLayout(Direction dir, QWidget *parent)
    : QLayout(*new QBoxLayoutPrivate, 0, parent)
{
    d->dir = dir;
}



/*!
    \obsolete

    Constructs a new QBoxLayout, with direction \a dir, and inserts it
    into \a parentLayout.

*/
QBoxLayout::QBoxLayout(Direction dir, QLayout *parentLayout)
    : QLayout(*new QBoxLayoutPrivate, parentLayout, 0)
{
    d->dir = dir;
}

/*!
    Constructs a new QBoxLayout with direction \a dir.

    You must insert this box into another layout.
*/
QBoxLayout::QBoxLayout(Direction dir)
    : QLayout(*new QBoxLayoutPrivate, 0, 0)
{
    d->dir = dir;
}


#ifdef QT3_SUPPORT
/*!
  \obsolete
    Constructs a new QBoxLayout with direction \a dir and main widget \a
    parent. \a parent may not be 0.

    The \a margin is the number of pixels between the edge of the
    widget and its managed children. The \a spacing is the default
    number of pixels between neighboring children. If \a spacing is -1
    the value of \a margin is used for \a spacing.

    \a name is the internal object name.

    \sa direction()
*/
QBoxLayout::QBoxLayout(QWidget *parent, Direction dir,
                        int margin, int spacing, const char *name)
    : QLayout(*new QBoxLayoutPrivate, 0, parent)
{
    d->dir = dir;
    setMargin(margin);
    setObjectName(name);
    setSpacing(spacing<0 ? margin : spacing);
}

/*!
  \obsolete
    Constructs a new QBoxLayout called \a name, with direction \a dir,
    and inserts it into \a parentLayout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, the layout will inherit its
    parent's spacing().
*/
QBoxLayout::QBoxLayout(QLayout *parentLayout, Direction dir, int spacing,
                        const char *name)
    : QLayout(*new QBoxLayoutPrivate, parentLayout, 0)
{
    d->dir = dir;
    setObjectName(name);
    setSpacing(spacing);
}

/*!
  \obsolete
    Constructs a new QBoxLayout called \a name, with direction \a dir.

    If \a spacing is -1, the layout will inherit its parent's
    spacing(); otherwise \a spacing is used.

    You must insert this box into another layout.
*/
QBoxLayout::QBoxLayout(Direction dir, int spacing, const char *name)
    : QLayout(*new QBoxLayoutPrivate,0, 0)
{
    d->dir = dir;
    setObjectName(name);
    setSpacing(spacing);
}
#endif //QT3_SUPPORT


/*!
    Destroys this box layout.

    The layout's widgets aren't destroyed.
*/
QBoxLayout::~QBoxLayout()
{
    d->deleteAll(); //must do it before QObject deletes children, so can't be in ~QBoxLayoutPrivate
}

/*!
    Returns the preferred size of this box layout.
*/
QSize QBoxLayout::sizeHint() const
{
    if (d->dirty)
        const_cast<QBoxLayout*>(this)->d->setupGeom();
    int m = margin();
    return d->sizeHint + QSize(2 * m, 2 * m);
}

/*!
    Returns the minimum size needed by this box layout.
*/
QSize QBoxLayout::minimumSize() const
{
    if (d->dirty)
        const_cast<QBoxLayout*>(this)->d->setupGeom();
    int m = margin();
    return d->minSize + QSize(2 * m, 2 * m);
}

/*!
    Returns the maximum size needed by this box layout.
*/
QSize QBoxLayout::maximumSize() const
{
    if (d->dirty)
        const_cast<QBoxLayout*>(this)->d->setupGeom();
    int m = margin();
    QSize s = (d->maxSize + QSize(2 * m, 2 * m))
              .boundedTo(QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX));
    if (alignment() & Qt::AlignHorizontal_Mask)
        s.setWidth(QLAYOUTSIZE_MAX);
    if (alignment() & Qt::AlignVertical_Mask)
        s.setHeight(QLAYOUTSIZE_MAX);
    return s;
}

/*!
  Returns true if this layout's preferred height depends on its width;
  otherwise returns false.
*/
bool QBoxLayout::hasHeightForWidth() const
{
    if (d->dirty)
        const_cast<QBoxLayout*>(this)->d->setupGeom();
    return d->hasHfw;
}

/*!
    Returns the layout's preferred height when it is \a w pixels wide.
*/
int QBoxLayout::heightForWidth(int w) const
{
    if (!hasHeightForWidth())
        return -1;
    int m = margin();
    w -= 2 * m;
    if (w != d->hfwWidth)
        const_cast<QBoxLayout*>(this)->d->calcHfw(w);

    return d->hfwHeight + 2 * m;
}

/*! \internal */
int QBoxLayout::minimumHeightForWidth(int w) const
{
    (void) heightForWidth(w);
    return d->hasHfw ? (d->hfwMinHeight + 2 * margin()) : -1;
}

/*!
    Resets cached information.
*/
void QBoxLayout::invalidate()
{
    d->setDirty();
    QLayout::invalidate();
}

/*!
    \reimp
*/
QLayoutItem *QBoxLayout::itemAt(int index) const
{
    return index >= 0 && index < d->list.count() ? d->list.at(index)->item : 0;
}


/*!
    \reimp
*/
QLayoutItem *QBoxLayout::takeAt(int index)
{
    if (index >= d->list.count())
        return 0;
    QBoxLayoutItem *b = d->list.takeAt(index);
    QLayoutItem *item = b->item;
    b->item = 0;
    delete b;
    return item;
}


/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Qt::Vertical or \c Qt::Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.
*/
QSizePolicy::ExpandData QBoxLayout::expanding() const
{
    if (d->dirty)
        const_cast<QBoxLayout*>(this)->d->setupGeom();
    return d->expanding;
}

/*!
    Resizes managed widgets within the rectangle \a r.
*/
void QBoxLayout::setGeometry(const QRect &r)
{
    if (d->dirty || r != geometry()) {
        QLayout::setGeometry(r);
        if (d->dirty)
            d->setupGeom();
        QRect cr = alignment() ? alignmentRect(r) : r;
        int m = margin();
        QRect s(cr.x() + m, cr.y() + m,
                 cr.width() - 2 * m, cr.height() - 2 * m);

        QVector<QLayoutStruct> a = d->geomArray;
        int pos = horz(d->dir) ? s.x() : s.y();
        int space = horz(d->dir) ? s.width() : s.height();
        int n = a.count();
        if (d->hasHfw && !horz(d->dir)) {
            for (int i = 0; i < n; i++) {
                QBoxLayoutItem *box = d->list.at(i);
                if (box->item->hasHeightForWidth())
                    a[i].sizeHint = a[i].minimumSize =
                                    box->item->heightForWidth(s.width());
            }
        }

        Direction visualDir = d->dir;
        QWidget *parent = parentWidget();
        if (parent && parent->isRightToLeft()) {
            if (d->dir == LeftToRight)
                visualDir = RightToLeft;
            else if (d->dir == RightToLeft)
                visualDir = LeftToRight;
        }

        qGeomCalc(a, 0, n, pos, space, spacing());
        for (int i = 0; i < n; i++) {
            QBoxLayoutItem *box = d->list.at(i);

            switch (visualDir) {
            case LeftToRight:
                box->item->setGeometry(QRect(a[i].pos, s.y(),
                                              a[i].size, s.height()));
                break;
            case RightToLeft:
                box->item->setGeometry(QRect(s.left() + s.right()
                                              - a[i].pos - a[i].size + 1, s.y(),
                                              a[i].size, s.height()));
                break;
            case TopToBottom:
                box->item->setGeometry(QRect(s.x(), a[i].pos,
                                              s.width(), a[i].size));
                break;
            case BottomToTop:
                box->item->setGeometry(QRect(s.x(), s.top() + s.bottom()
                                              - a[i].pos - a[i].size + 1,
                                              s.width(), a[i].size));
            }
        }
    }
}

/*!
    Adds \a item to the end of this box layout.
*/
void QBoxLayout::addItem(QLayoutItem *item)
{
    QBoxLayoutItem *it = new QBoxLayoutItem(item);
    d->list.append(it);
    invalidate();
}

/*!
    Inserts \a item into this box layout at position \a index. If \a
    index is negative, the item is added at the end.

    \warning Does not call QLayout::insertChildLayout() if \a item is
    a QLayout.

    \sa addItem(), findWidget()
*/
void QBoxLayout::insertItem(int index, QLayoutItem *item)
{
    if (index < 0)                                // append
        index = d->list.count();

    QBoxLayoutItem *it = new QBoxLayoutItem(item);
    d->list.insert(index, it);
    invalidate();
}

/*!
    Inserts a non-stretchable space at position \a index, with size \a
    size. If \a index is negative the space is added at the end.

    The box layout has default margin and spacing. This function adds
    additional space.

    \sa insertStretch()
*/
void QBoxLayout::insertSpacing(int index, int size)
{
    if (index < 0)                                // append
        index = d->list.count();

    QLayoutItem *b;
    if (horz(d->dir))
        b = new QSpacerItem(size, 0, QSizePolicy::Fixed,
                             QSizePolicy::Minimum);
    else
        b = new QSpacerItem(0, size, QSizePolicy::Minimum,
                             QSizePolicy::Fixed);

    QBoxLayoutItem *it = new QBoxLayoutItem(b);
    it->magic = true;
    d->list.insert(index, it);
    invalidate();
}

/*!
    Inserts a stretchable space at position \a index, with zero
    minimum size and stretch factor \a stretch. If \a index is
    negative the space is added at the end.

    \sa insertSpacing()
*/
void QBoxLayout::insertStretch(int index, int stretch)
{
    if (index < 0)                                // append
        index = d->list.count();

    QLayoutItem *b;
    if (horz(d->dir))
        b = new QSpacerItem(0, 0, QSizePolicy::Expanding,
                             QSizePolicy::Minimum);
    else
        b = new QSpacerItem(0, 0,  QSizePolicy::Minimum,
                             QSizePolicy::Expanding);

    QBoxLayoutItem *it = new QBoxLayoutItem(b, stretch);
    it->magic = true;
    d->list.insert(index, it);
    invalidate();
}

/*!
    Inserts \a layout at position \a index, with stretch factor \a
    stretch. If \a index is negative, the layout is added at the end.

    \a layout becomes a child of the box layout.

    \sa insertWidget(), insertSpacing()
*/
void QBoxLayout::insertLayout(int index, QLayout *layout, int stretch)
{
    addChildLayout(layout);
    if (index < 0)                                // append
        index = d->list.count();
    QBoxLayoutItem *it = new QBoxLayoutItem(layout, stretch);
    d->list.insert(index, it);
    invalidate();
}

/*!
    Inserts \a widget at position \a index, with stretch factor \a
    stretch and alignment \a alignment. If \a index is negative, the
    widget is added at the end.

    The stretch factor applies only in the \link direction() direction
    \endlink of the QBoxLayout, and is relative to the other boxes and
    widgets in this QBoxLayout. Widgets and boxes with higher stretch
    factors grow more.

    If the stretch factor is 0 and nothing else in the QBoxLayout has
    a stretch factor greater than zero, the space is distributed
    according to the QWidget:sizePolicy() of each widget that's
    involved.

    The alignment is specified by \a alignment. The default alignment
    is 0, which means that the widget fills the entire cell.

    From Qt 3.0, the \a alignment parameter is interpreted more
    aggressively than in previous versions of Qt. A non-default
    alignment now indicates that the widget should not grow to fill
    the available space, but should be sized according to sizeHint().

    \sa insertLayout(), insertSpacing()
*/
void QBoxLayout::insertWidget(int index, QWidget *widget, int stretch,
                               Qt::Alignment alignment)
{
    if (!checkWidget(this, widget))
         return;
    addChildWidget(widget);
    if (index < 0)                                // append
        index = d->list.count();
    QWidgetItem *b = new QWidgetItem(widget);
    b->setAlignment(alignment);
    QBoxLayoutItem *it = new QBoxLayoutItem(b, stretch);
    d->list.insert(index, it);
    invalidate();
}

/*!
    Adds a non-stretchable space with size \a size to the end of this
    box layout. QBoxLayout provides default margin and spacing. This
    function adds additional space.

    \sa insertSpacing(), addStretch()
*/
void QBoxLayout::addSpacing(int size)
{
    insertSpacing(-1, size);
}

/*!
    Adds a stretchable space with zero minimum size and stretch factor
    \a stretch to the end of this box layout.

    \sa addSpacing()
*/
void QBoxLayout::addStretch(int stretch)
{
    insertStretch(-1, stretch);
}

/*!
    Adds \a widget to the end of this box layout, with a stretch
    factor of \a stretch and alignment \a alignment.

    The stretch factor applies only in the \link direction() direction
    \endlink of the QBoxLayout, and is relative to the other boxes and
    widgets in this QBoxLayout. Widgets and boxes with higher stretch
    factors grow more.

    If the stretch factor is 0 and nothing else in the QBoxLayout has
    a stretch factor greater than zero, the space is distributed
    according to the QWidget:sizePolicy() of each widget that's
    involved.

    The alignment is specified by \a alignment. The default
    alignment is 0, which means that the widget fills the entire cell.

    From Qt 3.0, the \a alignment parameter is interpreted more
    aggressively than in previous versions of Qt. A non-default
    alignment now indicates that the widget should not grow to fill
    the available space, but should be sized according to sizeHint().

    \sa insertWidget(), addLayout(), addSpacing()
*/
void QBoxLayout::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    insertWidget(-1, widget, stretch, alignment);
}

/*!
    Adds \a layout to the end of the box, with serial stretch factor
    \a stretch.

    \sa insertLayout(), addWidget(), addSpacing()
*/
void QBoxLayout::addLayout(QLayout *layout, int stretch)
{
    insertLayout(-1, layout, stretch);
}

/*!
    Limits the perpendicular dimension of the box (e.g. height if the
    box is LeftToRight) to a minimum of \a size. Other constraints may
    increase the limit.
*/
void QBoxLayout::addStrut(int size)
{
    QLayoutItem *b;
    if (horz(d->dir))
        b = new QSpacerItem(0, size, QSizePolicy::Fixed,
                             QSizePolicy::Minimum);
    else
        b = new QSpacerItem(size, 0, QSizePolicy::Minimum,
                             QSizePolicy::Fixed);

    QBoxLayoutItem *it = new QBoxLayoutItem(b);
    it->magic = true;
    d->list.append(it);
    invalidate();
}

/*!
    Searches for widget \a w in this layout (not including child
    layouts).

    Returns the index of \a w, or -1 if \a w is not found.
*/
int QBoxLayout::findWidget(QWidget* w)
{
    const int n = d->list.count();
    for (int i = 0; i < n; i++) {
        if (d->list.at(i)->item->widget() == w)
            return i;
    }
    return -1;
}

/*!
    Sets the stretch factor for widget \a w to \a stretch and returns
    true if \a w is found in this layout (not including child
    layouts); otherwise returns false.

    \sa setAlignment()
*/
bool QBoxLayout::setStretchFactor(QWidget *w, int stretch)
{
    for (int i = 0; i < d->list.size(); ++i) {
        QBoxLayoutItem *box = d->list.at(i);
        if (box->item->widget() == w) {
            box->stretch = stretch;
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
  \overload

  Sets the stretch factor for the layout \a l to \a stretch and
  returns true if \a l is found in this layout (not including child
  layouts); otherwise returns false.
*/
bool QBoxLayout::setStretchFactor(QLayout *l, int stretch)
{
    for (int i = 0; i < d->list.size(); ++i) {
        QBoxLayoutItem *box = d->list.at(i);
        if (box->item->layout() == l) {
            box->stretch = stretch;
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
    Sets the alignment for widget \a w to \a alignment and returns
    true if \a w is found in this layout (not including child
    layouts); otherwise returns false.

    \sa setStretchFactor()
*/
bool QBoxLayout::setAlignment(QWidget *w, Qt::Alignment alignment)
{
    for (int i = 0; i < d->list.size(); ++i) {
        QBoxLayoutItem *box = d->list.at(i);
        if (box->item->widget() == w) {
            box->item->setAlignment(alignment);
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
  \overload

  Sets the alignment for the layout \a l to \a alignment and
  returns true if \a l is found in this layout (not including child
  layouts); otherwise returns false.
*/
bool QBoxLayout::setAlignment(QLayout *l, Qt::Alignment alignment)
{
    for (int i = 0; i < d->list.size(); ++i) {
        QBoxLayoutItem *box = d->list.at(i);
        if (box->item->layout() == l) {
            box->item->setAlignment(alignment);
            invalidate();
            return true;
        }
    }
    return false;
}

/*!
    Sets the direction of this layout to \a direction.
*/
void QBoxLayout::setDirection(Direction direction)
{
    if (d->dir == direction)
        return;
    if (horz(d->dir) != horz(direction)) {
        //swap around the spacers (the "magic" bits)
        //#### a bit yucky, knows too much.
        //#### probably best to add access functions to spacerItem
        //#### or even a QSpacerItem::flip()
        for (int i = 0; i < d->list.size(); ++i) {
            QBoxLayoutItem *box = d->list.at(i);
            if (box->magic) {
                QSpacerItem *sp = box->item->spacerItem();
                if (sp) {
                    if (sp->expanding() == QSizePolicy::NoDirection) {
                        //spacing or strut
                        QSize s = sp->sizeHint();
                        sp->changeSize(s.height(), s.width(),
                            horz(direction) ? QSizePolicy::Fixed:QSizePolicy::Minimum,
                            horz(direction) ? QSizePolicy::Minimum:QSizePolicy::Fixed);

                    } else {
                        //stretch
                        if (horz(direction))
                            sp->changeSize(0, 0, QSizePolicy::Expanding,
                                            QSizePolicy::Minimum);
                        else
                            sp->changeSize(0, 0, QSizePolicy::Minimum,
                                            QSizePolicy::Expanding);
                    }
                }
            }
        }
    }
    d->dir = direction;
    invalidate();
}

/*!
    \fn QBoxLayout::Direction QBoxLayout::direction() const

    Returns the direction of the box. addWidget() and addSpacing()
    work in this direction; the stretch stretches in this direction.

    \sa QBoxLayout::Direction addWidget() addSpacing()
*/

QBoxLayout::Direction QBoxLayout::direction() const
{
    return d->dir;
}

/*!
    \class QHBoxLayout
    \brief The QHBoxLayout class lines up widgets horizontally.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    This class is used to construct horizontal box layout objects. See
    \l QBoxLayout for more details.

    The simplest use of the class is like this:
    \code
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->addWidget(existingChildOfWidget);
        layout->addWidget(anotherChildOfWidget);
    \endcode

    \img qhboxlayout.png QHBox

    \sa QVBoxLayout QGridLayout
        \link layout.html the Layout overview \endlink
*/


/*!
    Constructs a new top-level horizontal box with
    parent \a parent.
*/
QHBoxLayout::QHBoxLayout(QWidget *parent)
    : QBoxLayout(LeftToRight, parent)
{
}

/*!
    Constructs a new horizontal box and adds it to
    \a parentLayout.
*/
QHBoxLayout::QHBoxLayout(QLayout *parentLayout)
    : QBoxLayout(LeftToRight, parentLayout)
{
}

/*!
    Constructs a new horizontal box. You must add
    it to another layout.
*/
QHBoxLayout::QHBoxLayout()
    : QBoxLayout(LeftToRight)
{
}



#ifdef QT3_SUPPORT
/*!
  \obsolete
    Constructs a new top-level horizontal box called \a name, with
    parent \a parent.

    The \a margin is the number of pixels between the edge of the
    widget and its managed children. The \a spacing is the default
    number of pixels between neighboring children. If \a spacing is -1
    the value of \a margin is used for \a spacing.
*/
QHBoxLayout::QHBoxLayout(QWidget *parent, int margin,
                          int spacing, const char *name)
    : QBoxLayout(LeftToRight, parent)
{
       setMargin(margin);
       setSpacing(spacing<0 ? margin : spacing);
       setObjectName(name);
}

/*!
  \obsolete
    Constructs a new horizontal box called name \a name and adds it to
    \a parentLayout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QHBoxLayout will inherit its
    parent's spacing().
*/
QHBoxLayout::QHBoxLayout(QLayout *parentLayout, int spacing,
                          const char *name)
    : QBoxLayout(LeftToRight, parentLayout)
{
       setSpacing(spacing);
       setObjectName(name);
}

/*!
  \obsolete
    Constructs a new horizontal box called name \a name. You must add
    it to another layout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QHBoxLayout will inherit its
    parent's spacing().
*/
QHBoxLayout::QHBoxLayout(int spacing, const char *name)
    : QBoxLayout(LeftToRight)
{
    setSpacing(spacing);
    setObjectName(name);
}
#endif


/*!
    Destroys this box layout.

    The layout's widgets aren't destroyed.
*/
QHBoxLayout::~QHBoxLayout()
{
}

/*!
    \class QVBoxLayout

    \brief The QVBoxLayout class lines up widgets vertically.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    This class is used to construct vertical box layout objects. See
    QBoxLayout for more details.

    The simplest use of the class is like this:
    \code
        QVBoxLayout *layout = new QVBoxLayout(widget);
        layout->addWidget(aWidget);
        layout->addWidget(anotherWidget);
    \endcode

    \img qvboxlayout.png QVBox

    \sa QHBoxLayout QGridLayout \link layout.html the Layout overview \endlink
*/

/*!
    Constructs a new top-level vertical box with
    parent \a parent.
*/
QVBoxLayout::QVBoxLayout(QWidget *parent)
    : QBoxLayout(TopToBottom, parent)
{
}

/*!
    Constructs a new vertical box and adds it to
    \a parentLayout.
*/
QVBoxLayout::QVBoxLayout(QLayout *parentLayout)
    : QBoxLayout(TopToBottom, parentLayout)
{
}

/*!
    Constructs a new vertical box. You must add
    it to another layout.

*/
QVBoxLayout::QVBoxLayout()
    : QBoxLayout(TopToBottom)
{
}

#ifdef QT3_SUPPORT
/*!
  \obsolete
    Constructs a new top-level vertical box called \a name, with
    parent \a parent.

    The \a margin is the number of pixels between the edge of the
    widget and its managed children. The \a spacing is the default
    number of pixels between neighboring children. If \a spacing is -1
    the value of \a margin is used for \a spacing.
*/
QVBoxLayout::QVBoxLayout(QWidget *parent, int margin, int spacing,
                          const char *name)
    : QBoxLayout(TopToBottom, parent)
{
    setMargin(margin);
    setSpacing(spacing<0 ? margin : spacing);
    setObjectName(name);
}

/*!
  \obsolete
    Constructs a new vertical box called name \a name and adds it to
    \a parentLayout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QVBoxLayout will inherit its
    parent's spacing().
*/
QVBoxLayout::QVBoxLayout(QLayout *parentLayout, int spacing,
                          const char *name)
    : QBoxLayout(TopToBottom, parentLayout)
{
    setSpacing(spacing);
    setObjectName(name);
}

/*!
  \obsolete
    Constructs a new vertical box called name \a name. You must add
    it to another layout.

    The \a spacing is the default number of pixels between neighboring
    children. If \a spacing is -1, this QVBoxLayout will inherit its
    parent's spacing().
*/
QVBoxLayout::QVBoxLayout(int spacing, const char *name)
    : QBoxLayout(TopToBottom)
{
    setSpacing(spacing);
    setObjectName(name);
}


#endif

/*!
    Destroys this box layout.

    The layout's widgets aren't destroyed.
*/
QVBoxLayout::~QVBoxLayout()
{
}

/*!
    \fn QWidget *QLayout::mainWidget() const

    Use parentWidget() instead.
*/

/*!
    \fn void QLayout::remove(QWidget *w)

    Use removeWidget() instead.
*/

/*!
    \fn void QLayout::add(QWidget *w)

    Use addWidget() instead.
*/

/*!
    \fn QLayoutIterator QLayout::iterator()

    Use a QLayoutIterator() constructor instead.
*/

/*!
    \fn int QLayout::defaultBorder() const

    Use spacing() instead.
*/
#endif // QT_NO_LAYOUT
