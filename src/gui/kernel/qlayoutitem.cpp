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
#include "qvariant.h"

/*!
   Returns the size policy as a QVariant
*/
QSizePolicy::operator QVariant() const
{
    extern bool qRegisterGuiVariant();
    static const bool b = qRegisterGuiVariant();
    Q_UNUSED(b)
    return QVariant(QVariant::SizePolicy, this);
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

    isEmpty() returns whether the layout item is empty. If the
    concrete item is a QWidget, it can be retrieved using widget().
    Similarly for layout() and spacerItem().

    Some layouts have width and height interdependencies. These can
    be expressed using hasHeightForWidth(), heightForWidth(), and
    minimumHeightForWidth(). For more explanation see the \e{Qt
    Quarterly} article
    \l{http://doc.trolltech.com/qq/qq04-height-for-width.html}{Trading
    Height for Width}.

    \sa QLayout
*/

/*!
    \class QSpacerItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QSpacerItem class provides blank space in a layout.

    Normally, you don't need to use this class directly. Qt's
    built-in layout managers provide the following functions for
    manipulating empty space in layouts:

    \table
    \header \o Class
            \o Functions
    \row    \o QHBoxLayout
            \o \l{QBoxLayout::addSpacing()}{addSpacing()},
               \l{QBoxLayout::addStretch()}{addStretch()},
               \l{QBoxLayout::insertSpacing()}{insertSpacing()},
               \l{QBoxLayout::insertStretch()}{insertStretch()}
    \row    \o QGridLayout
            \o \l{QGridLayout::setRowMinimumHeight()}{setRowMinimumHeight()},
               \l{QGridLayout::setRowStretch()}{setRowStretch()},
               \l{QGridLayout::setColumnMinimumWidth()}{setColumnMinimumWidth()},
               \l{QGridLayout::setColumnStretch()}{setColumnStretch()}
    \endtable

    \sa QLayout, QWidgetItem, QLayoutItem::spacerItem()
*/

/*!
    \class QWidgetItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QWidgetItem class is a layout item that represents a widget.

    Normally, you don't need to use this class directly. Qt's
    built-in layout managers provide the following functions for
    manipulating widgets in layouts:

    \table
    \header \o Class
            \o Functions
    \row    \o QBoxLayout
            \o \l{QBoxLayout::addWidget()}{addWidget()},
               \l{QBoxLayout::insertWidget()}{insertWidget()},
               \l{QBoxLayout::setStretchFactor()}{setStretchFactor()}
    \row    \o QGridLayout
            \o \l{QGridLayout::addWidget()}{addWidget()}
    \row    \o QStackedLayout
            \o \l{QStackedLayout::addWidget()}{addWidget()},
               \l{QStackedLayout::insertWidget()}{insertWidget()},
               \l{QStackedLayout::currentWidget()}{currentWidget()},
               \l{QStackedLayout::setCurrentWidget()}{setCurrentWidget()},
               \l{QStackedLayout::widget()}{widget()}
    \endtable

    \sa QLayout, QSpacerItem, QLayoutItem::widget()
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
    \fn Qt::Orientations QLayoutItem::expandingDirections() const

    Implemented in subclasses to return the direction(s) this item
    "wants" to expand in (if any).
*/

/*!
    \fn void QLayoutItem::setGeometry(const QRect &r)

    Implemented in subclasses to set this item's geometry to \a r.

    \sa geometry()
*/

/*!
    \fn QRect QLayoutItem::geometry() const

    Returns the rectangle covered by this layout item.

    \sa setGeometry()
*/

/*!
    \fn virtual bool QLayoutItem::isEmpty() const

    Implemented in subclasses to return whether this item is empty,
    i.e. whether it contains any widgets.
*/

/*!
    \fn QSpacerItem::QSpacerItem(int w, int h, QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)

    Constructs a spacer item with preferred width \a w, preferred
    height \a h, horizontal size policy \a hPolicy and vertical size
    policy \a vPolicy.

    The default values provide a gap that is able to stretch if
    nothing else wants the space.
*/

/*!
    Changes this spacer item to have preferred width \a w, preferred
    height \a h, horizontal size policy \a hPolicy and vertical size
    policy \a vPolicy.

    The default values provide a gap that is able to stretch if
    nothing else wants the space.
*/
void QSpacerItem::changeSize(int w, int h, QSizePolicy::Policy hPolicy,
                             QSizePolicy::Policy vPolicy)
{
    width = w;
    height = h;
    sizeP = QSizePolicy(hPolicy, vPolicy);
}

/*!
    \fn QWidgetItem::QWidgetItem(QWidget *widget)

    Creates an item containing the given \a widget.
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
    Returns a pointer to this object.
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
QWidget *QWidgetItem::widget()
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
    \a w. The default implementation simply returns heightForWidth(\a
    w).
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
    \reimp
*/
void QSpacerItem::setGeometry(const QRect &r)
{
    rect = r;
}

/*!
    \reimp
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
    Qt::Alignment alignHoriz = QStyle::visualAlignment(wid->layoutDirection(), align);
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
    \reimp
*/
Qt::Orientations QSpacerItem::expandingDirections() const
{
    return sizeP.expandingDirections();
}

/*!
    \reimp
*/
Qt::Orientations QWidgetItem::expandingDirections() const
{
    if (isEmpty())
        return Qt::Orientations(0);

    Qt::Orientations e = wid->sizePolicy().expandingDirections();
    /*
      If the layout is expanding, we make the widget expanding, even if
      its own size policy isn't expanding. This behavior should be
      reconsidered in Qt 4.0. (###)
    */
    if (wid->layout()) {
        if (wid->sizePolicy().horizontalPolicy() & QSizePolicy::GrowFlag
                && (wid->layout()->expandingDirections() & Qt::Horizontal))
            e |= Qt::Horizontal;
        if (wid->sizePolicy().verticalPolicy() & QSizePolicy::GrowFlag
                && (wid->layout()->expandingDirections() & Qt::Vertical))
            e |= Qt::Vertical;
    }

    if (align & Qt::AlignHorizontal_Mask)
        e &= ~Qt::Horizontal;
    if (align & Qt::AlignVertical_Mask)
        e &= ~Qt::Vertical;
    return e;
}

/*!
    \reimp
*/
QSize QSpacerItem::minimumSize() const
{
    return QSize(sizeP.horizontalPolicy() & QSizePolicy::ShrinkFlag ? 0 : width,
                 sizeP.verticalPolicy() & QSizePolicy::ShrinkFlag ? 0 : height);
}

/*!
    \reimp
*/
QSize QWidgetItem::minimumSize() const
{
    if (isEmpty())
        return QSize(0, 0);
    return qSmartMinSize(this);
}

/*!
    \reimp
*/
QSize QSpacerItem::maximumSize() const
{
    return QSize(sizeP.horizontalPolicy() & QSizePolicy::GrowFlag ? QLAYOUTSIZE_MAX : width,
                 sizeP.verticalPolicy() & QSizePolicy::GrowFlag ? QLAYOUTSIZE_MAX : height);
}

/*!
    \reimp
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
    \reimp
*/
QSize QSpacerItem::sizeHint() const
{
    return QSize(width, height);
}

/*!
    \reimp
*/
QSize QWidgetItem::sizeHint() const
{
    QSize s;
    if (isEmpty()) {
        s = QSize(0, 0);
    } else {
        s = wid->sizeHint().expandedTo(wid->minimumSizeHint());
        if (wid->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored)
            s.setWidth(0);
        if (wid->sizePolicy().verticalPolicy() == QSizePolicy::Ignored)
            s.setHeight(0);
        s = s.boundedTo(wid->maximumSize())
            .expandedTo(wid->minimumSize());
    }
    return s;
}

/*!
    Returns true.
*/
bool QSpacerItem::isEmpty() const
{
    return true;
}

/*!
    Returns true if the widget is hidden; otherwise returns false.

    \sa QWidget::isHidden()
*/
bool QWidgetItem::isEmpty() const
{
    return wid->isHidden() || wid->isWindow();
}

#endif // QT_NO_LAYOUT
