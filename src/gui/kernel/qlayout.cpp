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
#include "qwidget_p.h"
#include "qlayout_p.h"

static int menuBarHeightForWidth(QWidget *menubar, int w)
{
    if (menubar && !menubar->isExplicitlyHidden() && !menubar->isWindow())
        return menubar->heightForWidth(qMax(w, menubar->minimumWidth()));
    return 0;
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
    setSizeConstraint() or setMenuBar(). See \l{Layout Classes}
    for more information.

    To make your own layout manager, implement the functions
    addItem(), sizeHint(), setGeometry(), itemAt() and takeAt(). You
    should also implement minimumSize() to ensure your layout isn't
    resized to zero size if there is too little space. To support
    children whose heights depend on their widths, implement
    hasHeightForWidth() and heightForWidth(). See
    \l{Border Layout} and \l{Flow Layout} for examples of custom
    layout managers.

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
    Q_D(QLayout);
    if (parent) {
        if (parent->layout()) {
            qWarning("QLayout \"%s\" added to %s \"%s\", which already has a"
                     " layout", QObject::objectName().toLocal8Bit().data(), parent->metaObject()->className(),
                     parent->objectName().toLocal8Bit().data());
            parent->layout()->setParent(0);
        } else {
            d->topLevel = true;
            parent->d_func()->layout = this;
            invalidate();
        }
    }
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
   Q_D(QLayout);
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
            w->d_func()->layout = this;
            invalidate();
        }
    }
}

QLayoutPrivate::QLayoutPrivate()
    : QObjectPrivate(), insideSpacing(-1), outsideBorder(-1), topLevel(false), enabled(true),
      activated(true), autoNewChild(false), constraint(QLayout::SetDefaultConstraint)
#ifndef QT_NO_MENUBAR
      , menubar(0)
#endif
{
}




#ifdef QT3_SUPPORT
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
    Q_D(QLayout);
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
            parent->d_func()->layout = this;
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
    Q_D(QLayout);
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
    Q_D(QLayout);
    setObjectName(name);
    d->insideSpacing = spacing;
}

/*!
    Automatically adding widgets is deprecated. Use addWidget() or
    addLayout() instead.
*/
void QLayout::setAutoAdd(bool a) { Q_D(QLayout); d->autoNewChild = a; }

/*!
    Automatically adding widgets is deprecated. Use addWidget() or
    addLayout() instead.
*/
bool QLayout::autoAdd() const { Q_D(const QLayout); return d->autoNewChild; }
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
    Sets the alignment for widget \a w to \a alignment and returns
    true if \a w is found in this layout (not including child
    layouts); otherwise returns false.
*/
bool QLayout::setAlignment(QWidget *w, Qt::Alignment alignment)
{
    int i = 0;
    QLayoutItem *item = itemAt(i);
    while (item) {
        if (item->widget() == w) {
            item->setAlignment(alignment);
            invalidate();
            return true;
        }
        ++i;
        item = itemAt(i);
    }
    return false;
}

/*!
  \overload

  Sets the alignment for the layout \a l to \a alignment and
  returns true if \a l is found in this layout (not including child
  layouts); otherwise returns false.
*/
bool QLayout::setAlignment(QLayout *l, Qt::Alignment alignment)
{
    int i = 0;
    QLayoutItem *item = itemAt(i);
    while (item) {
        if (item->layout() == l) {
            item->setAlignment(alignment);
            invalidate();
            return true;
        }
        ++i;
        item = itemAt(i);
    }
    return false;
}

/*!
    \fn bool QLayout::isTopLevel() const

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
    Q_D(const QLayout);
    if ( d->outsideBorder >= 0 )
        return d->outsideBorder;
    if (!d->topLevel)
        return 0;
    QWidget *pw = parentWidget();
    if (pw)
        return pw->style()->pixelMetric(
            (pw->isWindow() || (pw->windowType() == Qt::SubWindow))
            ? QStyle::PM_DefaultToplevelMargin
            : QStyle::PM_DefaultChildMargin
            );
    return 0;
}


int QLayout::spacing() const
{
    Q_D(const QLayout);
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

#ifdef QT3_SUPPORT
bool QLayout::isTopLevel() const
{
    Q_D(const QLayout);
    return d->topLevel;
}
#endif

void QLayout::setMargin(int margin)
{
    Q_D(QLayout);
    d->outsideBorder = margin;
    invalidate();
}


void QLayout::setSpacing(int spacing)
{
    Q_D(QLayout);
    d->insideSpacing = spacing;
    invalidate();
}

/*!
    Returns the parent widget of this layout, or 0 if
    this layout is a sub-layout that is not yet inserted.
*/
QWidget *QLayout::parentWidget() const
{
    Q_D(const QLayout);
    if (!d->topLevel) {
        if (parent()) {
            QLayout *parentLayout = ::qobject_cast<QLayout*>(parent());
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
    iterates over all items and returns true if all child items are empty.
*/
bool QLayout::isEmpty() const
{
    int i = 0;
    QLayoutItem *item = itemAt(i);
    while (item) {
        if (!item->isEmpty())
            return false;
        ++i;
        item = itemAt(i);
    }
    return true;
}

/*!
    This function is reimplemented in subclasses to perform layout.

    The default implementation maintains the geometry() information
    given by rect \a r. Reimplementors must call this function.
*/
void QLayout::setGeometry(const QRect &r)
{
    Q_D(QLayout);
    d->rect = r;
}

/*!
    \reimp
*/
QRect QLayout::geometry() const
{
    Q_D(const QLayout);
    return d->rect;
}

/*!
    Invalidates cached information. Reimplementations must call this.
*/
void QLayout::invalidate()
{
    Q_D(QLayout);
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


void QLayoutPrivate::doResize(const QSize &r)
{
    Q_Q(QLayout);
    int mbh = menuBarHeightForWidth(menubar, r.width());
    QWidget *mw = q->parentWidget();
    QRect rect = mw->testAttribute(Qt::WA_LayoutOnEntireRect) ? mw->rect() : mw->contentsRect();
    rect.setTop(rect.top() + mbh);
    q->setGeometry(rect);
#ifndef QT_NO_MENUBAR
    if (menubar)
        menubar->setGeometry(0,0,r.width(), mbh);
#endif
}


/*!
    \internal
    Performs child widget layout when the parent widget is
    resized.  Also handles removal of widgets. \a e is the
    event
*/
void QLayout::widgetEvent(QEvent *e)
{
    Q_D(QLayout);
    if (!d->enabled)
        return;

    switch (e->type()) {
    case QEvent::Resize:
        if (d->activated) {
            QResizeEvent *r = (QResizeEvent *)e;
            d->doResize(r->size());
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
#ifdef QT3_SUPPORT
    case QEvent::ChildInserted:
        if (d->topLevel && d->autoNewChild) {
            QChildEvent *c = (QChildEvent *)e;
            if (c->child()->isWidgetType()) {
                QWidget *w = (QWidget *)c->child();
                if (!w->isWindow()) {
#if !defined(QT_NO_MENUBAR) && !defined(QT_NO_TOOLBAR)
                    if (qobject_cast<QMenuBar*>(w) && !::qobject_cast<QToolBar*>(w->parentWidget())) {
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
    Q_D(QLayout);
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
    Q_D(const QLayout);
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *parent = parentWidget();
        parent->ensurePolished();
        QWidgetPrivate *wd = parent->d_func();
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
    Q_D(const QLayout);
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *pw = parentWidget();
        pw->ensurePolished();
        QWidgetPrivate *wd = pw->d_func();
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
    Q_D(const QLayout);
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *pw = parentWidget();
        pw->ensurePolished();
        QWidgetPrivate *wd = pw->d_func();
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
    Q_D(const QLayout);
    int side=0, top=0;
    if (d->topLevel) {
        QWidget *pw = parentWidget();
        pw->ensurePolished();
        QWidgetPrivate *wd = pw->d_func();
        side += wd->leftmargin + wd->rightmargin;
        top += wd->topmargin + wd->bottommargin;
    }

    QSize s = maximumSize();
#ifndef QT_NO_MENUBAR
    top += menuBarHeightForWidth(d->menubar, s.width());
#endif

    if (d->topLevel)
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
    Q_D(QLayout);
    /*
      This function may be called during the QObject destructor,
      when the parent no longer is a QWidget.
    */
    if (d->topLevel && parent() && parent()->isWidgetType() &&
         ((QWidget*)parent())->layout() == this)
        ((QWidget*)parent())->d_func()->layout = 0;
}

#ifdef QT3_SUPPORT
/*!
    Removes and deletes all items in this layout.
*/
void QLayout::deleteAllItems()
{
    QLayoutItem *l;
    while ((l = takeAt(0)))
        delete l;
}
#endif

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

    if (QWidget *mw = parentWidget()) {
        l->d_func()->reparentChildWidgets(mw);
    }

}

void QLayoutPrivate::reparentChildWidgets(QWidget *mw)
{
    Q_Q(QLayout);
    int n =  q->count();
    for (int i = 0; i < n; ++i) {
        QLayoutItem *item = q->itemAt(i);
        if (QWidget *w = item->widget()) {
            QWidget *pw = w->parentWidget();
#ifdef QT_DEBUG
            if (pw && pw != mw) {
                qWarning("QLayout::addChildLayout: widget %s in wrong parent; moved to correct parent", w->metaObject()->className());
            }
#endif
            if (pw != mw)
                w->setParent(mw);
        } else if (QLayout *l = item->layout()) {
            l->d_func()->reparentChildWidgets(mw);
        }
    }
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
    if (pw && w->testAttribute(Qt::WA_LaidOut)) {
        QLayout *l = pw->layout();
        if (l && removeWidgetRecursively(l, w))
            qWarning("QLayout::addChildWidget: %s is already in a layout; moved to new layout", w->metaObject()->className());
    }
    if (pw && mw && pw != mw) {
        qWarning("QLayout::addChildWidget: %s in wrong parent; moved to correct parent", w->metaObject()->className());
        pw = 0;
    }
    if (!pw && mw)
        w->setParent(mw);
    w->setAttribute(Qt::WA_LaidOut);
}

#ifdef QT3_SUPPORT
/*!
  \compat

  Sets this layout's parent widget to a fixed size with width \a w and
  height \a h, stopping the user form resizing it, and also prevents the
  layout from resizing it, even if the layout's size hint should
  change. Does nothing if this is not a toplevel layout (if parent()->isWidgetType()).

  As a special case, if both \a w and \a h are 0, then the layout's
  current sizeHint() is used.

  Use \c setResizeMode(Fixed) to stop the widget from being resized by
  the user, while still allowing the layout to resize it when the sizeHint() changes.

  Use \c setResizeMode(FreeResize) to allow the user to resize the
  widget, while preventing the layout from resizing it.

*/
void QLayout::freeze(int w, int h)
{
    Q_D(QLayout);
    if (!d->topLevel)
        return;
    if (w <= 0 || h <= 0) {
        QSize s = totalSizeHint();
        w = s.width();
        h = s.height();
    }
    setSizeConstraint(SetNoConstraint); // layout will not change min/max size
    QWidget *parent = parentWidget();
    if (parent)
        parent->setFixedSize(w, h);
}

#endif







/*!
    Tells the geometry manager to place the menu bar \a w at the top
    of parentWidget(), outside QWidget::contentsMargins(). All child
    widgets are placed below the bottom edge of the menu bar.
*/
void QLayout::setMenuBar(QWidget *w)
{
    Q_D(QLayout);
    if (w)
        addChildWidget(w);
    d->menubar = w;
}

/*!
    Returns the menu bar set for this layout, or 0 if no menu bar is
    set.
*/

QWidget *QLayout::menuBar() const
{
    Q_D(const QLayout);
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
    to grow in only one dimension, whereas \c { Qt::Vertical | Qt::Horizontal }  means that
    it wants to grow in both dimensions.

    The default implementation returns  \c { Qt::Vertical | Qt::Horizontal }
*/
Qt::Orientations QLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
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
        layout->d_func()->activated = true;
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
    while (layout && layout->d_func()->activated) {
        layout->d_func()->activated = false;
        if (layout->d_func()->topLevel) {
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
    Q_D(QLayout);
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
    d->doResize(mw->size());

    switch (d->constraint) {
    case SetFixedSize:
        // will trigger resize
        mw->setFixedSize(totalSizeHint());
        break;
    case SetMinimumSize:
        mw->setMinimumSize(totalMinimumSize());
        break;
    case SetMaximumSize:
        mw->setMaximumSize(totalMaximumSize());
        break;
    case SetMinAndMaxSize:
        mw->setMinimumSize(totalMinimumSize());
        mw->setMaximumSize(totalMaximumSize());
        break;
    case SetDefaultConstraint: {
        QSize ms = totalMinimumSize();
        if (hasHeightForWidth()) {
            int h = minimumHeightForWidth(ms.width());
            if (h > ms.height())
                ms = QSize(0, 0);
        }
        mw->setMinimumSize(ms);
        break;
    }
    case SetNoConstraint:
        break;
    }

    // ideally only if sizeHint() or sizePolicy() has changed
    mw->updateGeometry();
    return true;
}

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
 \fn int *QLayout::count() const

  Must be implemented in subclasses to return the number of items in the layout.

*/

/*!
      Searches for widget \a w in this layout (not including child
    layouts).

    Returns the index of \a w, or -1 if \a w is not found.

    The default implementation iterates over all items using itemAt()
*/
int QLayout::indexOf(QWidget *w) const
{
    int i = 0;
    QLayoutItem *item = itemAt(i);
    while (item) {
        if (item->widget() == w)
            return i;
        ++i;
        item = itemAt(i);
    }
    return -1;
}


/*!
    \enum QLayout::SizeConstraint

    The possible values are:

    \value SetDefaultConstraint  If the main widget is a top-level widget with no
                 height-for-width (hasHeightForWidth()), this is
                 the same as \c SetMinimumSize; otherwise, this is the
                 same as \c SetNoConstraint.
    \value SetFixedSize The main widget's size is set to sizeHint(); it
                  cannot be resized at all.
    \value SetMinimumSize  The main widget's minimum size is set to
                    minimumSize(); it cannot be smaller.
    \value SetNoConstraint  The widget is not constrained.
*/

/*!
    \property QLayout::sizeConstraint
    \brief the resize mode of the layout

    The default mode is \c SetDefaultConstraint.

    \sa QLayout::SizeConstraint
*/

void QLayout::setSizeConstraint(SizeConstraint constraint)
{
    Q_D(QLayout);
    if (constraint == d->constraint)
        return;

    d->constraint = constraint;
    invalidate();
}

QLayout::SizeConstraint QLayout::sizeConstraint() const
{
    Q_D(const QLayout);
    return d->constraint;
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
    QLayout *that = const_cast<QLayout *>(this);
    that->setAlignment(0);
    QSize ms = that->maximumSize();
    that->setAlignment(a);

    if ((expandingDirections() & Qt::Horizontal) ||
         !(a & Qt::AlignHorizontal_Mask)) {
        s.setWidth(qMin(r.width(), ms.width()));
    }
    if ((expandingDirections() & Qt::Vertical) ||
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
    Q_D(QLayout);
    d->enabled = enable;
}

/*!
    Returns true if the layout is enabled; otherwise returns false.

    \sa setEnabled()
*/
bool QLayout::isEnabled() const
{
    Q_D(const QLayout);
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
