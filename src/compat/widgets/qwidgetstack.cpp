/****************************************************************************
**
** Implementation of QWidgetStack class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwidgetstack.h"
#include "qlayout.h"
#include "qevent.h"
#include <private/qlayoutengine_p.h>
#ifndef QT_NO_WIDGETSTACK

#include "qbutton.h"
#include "qbuttongroup.h"
#include "qhash.h"

#include "qapplication.h"

class QWidgetStackPrivate {
public:
    class Invisible: public QWidget
    {
    public:
        Invisible(QWidgetStack * parent)
            : QWidget(parent, "qt_invisible_widgetstack")
        { setAttribute(WA_NoSystemBackground, true); }
        const char * className() const
        { return "QWidgetStackPrivate::Invisible"; }
    };
    QHash<int, QWidget *> hash;
    QWidget * topWidget;
    QWidget * invisible;
    int margin;
};


/*!
    \class QWidgetStack
    \brief The QWidgetStack class provides a stack of widgets of which
    only the top widget is user-visible.

    \ingroup organizers
    \mainclass

    The application programmer can move any widget to the top of the
    stack at any time using raiseWidget(), and add or remove widgets
    using addWidget() and removeWidget(). It is not sufficient to pass
    the widget stack as parent to a widget which should be inserted into
    the widgetstack.

    visibleWidget() is the \e get equivalent of raiseWidget(); it
    returns a pointer to the widget that is currently at the top of
    the stack.

    QWidgetStack also provides the ability to manipulate widgets
    through application-specified integer IDs. You can also translate
    from widget pointers to IDs using id() and from IDs to widget
    pointers using widget(). These numeric IDs are unique (per
    QWidgetStack, not globally), but QWidgetStack does not attach any
    additional meaning to them.

    The default widget stack is frameless, but you can use the usual
    QFrame functions (such as setFrameStyle()) to add a frame.

    QWidgetStack provides a signal, aboutToShow(), which is emitted
    just before a managed widget is shown.

    \sa QTabDialog QTabBar QFrame
*/


/*!
  Constructs an empty widget stack.

  The \a parent, \a name and \a f arguments are passed to the QFrame
  constructor.
*/
QWidgetStack::QWidgetStack(QWidget * parent, const char *name, WFlags f)
    : QFrame(parent, name, f)
{
   d = new QWidgetStackPrivate;
   d->topWidget = 0;
   d->margin = 0;
   d->invisible = new QWidgetStackPrivate::Invisible(this);
   d->invisible->hide();
}


/*!
    Destroys the object and frees any allocated resources.
*/

QWidgetStack::~QWidgetStack()
{
    delete d;
}


/*!
    Adds widget \a w to this stack of widgets, with ID \a id.

    If you pass an id \>= 0 this ID is used. If you pass an \a id of
    -1 (the default), the widgets will be numbered automatically. If
    you pass -2 a unique negative integer will be generated. No widget
    has an ID of -1. Returns the ID or -1 on failure (e.g. \w is 0).

    If you pass an id that is already used, then a unique negative
    integer will be generated to prevent two widgets having the same
    id.

    If \a w is not a child of this QWidgetStack moves it using
    reparent().
*/

int QWidgetStack::addWidget(QWidget * w, int id)
{
    static int nseq_no = -2;
    static int pseq_no = 0;

    if (!w || w == d->invisible)
        return -1;

    // prevent duplicates
    removeWidget(w);

    if (id >= 0 && d->hash.contains(id))
        id = -2;
    if (id < -1)
        id = nseq_no--;
    else if (id == -1)
        id = pseq_no++;
    else
        pseq_no = qMax(pseq_no, id + 1);
        // use id >= 0 as-is

    d->hash.insert(id, w);

    w->hide();
    if (w->parent() != this) {
        w->setParent(this);
        w->move(contentsRect().topLeft());
    }
    w->setGeometry(contentsRect());
    updateGeometry();
    return id;
}


/*!
    Removes widget \a w from this stack of widgets. Does not delete \a
    w. If \a w is the currently visible widget, no other widget is
    substituted.

    \sa visibleWidget() raiseWidget()
*/

void QWidgetStack::removeWidget(QWidget * w)
{
    if (!w)
        return;
    int i = id(w);
    if (i != -1)
        d->hash.remove(i);

    if (w == d->topWidget)
        d->topWidget = 0;
    if (d->hash.isEmpty())
        d->invisible->hide(); // let background shine through again
    updateGeometry();
}


/*!
    Raises the widget with ID \a id to the top of the widget stack.

    \sa visibleWidget()
*/

void QWidgetStack::raiseWidget(int id)
{
    if (id == -1)
        return;
    QWidget * w = d->hash.value(id, 0);
    if (w)
        raiseWidget(w);
}

static bool isChildOf(QWidget* child, QWidget *parent)
{
    if (!child)
        return false;
    QObjectList list = parent->children();
    for (int i = 0; i < list.size(); ++i) {
        QObject *obj = list.at(i);
        if (!obj->isWidgetType())
            continue;
        QWidget *widget = static_cast<QWidget *>(obj);
        if (!widget->isTopLevel())
            continue;
        if (widget == child || isChildOf(child, widget))
            return true;
    }
    return false;
}

// ### fix include
#include <private/qwidget_p.h>
/*!
    \overload

    Raises widget \a w to the top of the widget stack.
*/

void QWidgetStack::raiseWidget(QWidget *w)
{
    if (!w || w == d->invisible || w->parent() != this || w == d->topWidget)
        return;

    if (id(w) == -1)
        addWidget(w);
    if (!isVisible()) {
        d->topWidget = w;
        return;
    }

    if (w->maximumSize().width() < d->invisible->width()
        || w->maximumSize().height() < d->invisible->height())
        d->invisible->setAttribute(WA_NoSystemBackground, false);
    else if (d->invisible->testAttribute(WA_NoSystemBackground))
        d->invisible->setAttribute(WA_NoSystemBackground, true);

    if (d->invisible->isHidden()) {
        d->invisible->setGeometry(contentsRect());
        d->invisible->lower();
        d->invisible->show();
        QApplication::sendPostedEvents(d->invisible, QEvent::ShowWindowRequest);
    }

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.
    if (d->topWidget) {
        QWidget * fw = topLevelWidget()->focusWidget();
        if (d->topWidget->isAncestorOf(fw)) { // focus was on old page
            // look for the best focus widget we can find
            QWidget *p = w->focusWidget();
            if (!p) {
                // second best == first child widget in the focus chain
                QWidget *i = fw;
                while ((i = i->nextInFocusChain()) != fw) {
                    if (((i->focusPolicy() & TabFocus) == TabFocus)
                        && !i->focusProxy() && i->isVisibleTo(w) && i->isEnabled()
                        && w->isAncestorOf(i)) {
                        p = i;
                        break;
                    }
                }
            }
            if (p)
                p->setFocus();
        } else {
            // the focus wasn't on the old page, so we have to ensure focus doesn't go to
            // the widget in the page that last had focus when we show the page again.
            QWidget *oldfw = d->topWidget->focusWidget();
            if (oldfw)
                oldfw->clearFocus();
            // rather hacky, but achieves the same without function calls.....
//             static_cast<QWidgetPrivate*>(((QWidgetStack *)d->topWidget)->d_ptr)->focus_child = 0;
        }
    }

    if (isVisible()) {
        emit aboutToShow(w);
        int i = id(w);
        if (i != -1)
            emit aboutToShow(i);
    }

    d->topWidget = w;

    QObjectList c = children();
    for (int i = 0; i < c.size(); ++i) {
        QObject * o = c.at(i);
        if (o->isWidgetType() && o != w && o != d->invisible)
            static_cast<QWidget *>(o)->hide();
    }

    w->setGeometry(d->invisible->geometry());
    w->show();
}


/*!
    Fixes up the children's geometries.
*/

void QWidgetStack::setChildGeometries()
{
    d->invisible->setGeometry(contentsRect());
    if (d->topWidget)
        d->topWidget->setGeometry(d->invisible->geometry());
}


/*!
    \reimp
*/
void QWidgetStack::show()
{
    //  Reimplemented in order to set the children's geometries
    //  appropriately and to pick the first widget as d->topWidget if no
    //  topwidget was defined
    QObjectList c = children();
    if (!isVisible() && !c.isEmpty()) {
        for (int i = 0; i < c.size(); ++i) {
            QObject * o = c.at(i);
            if (o->isWidgetType()) {
                if (!d->topWidget && o != d->invisible)
                    d->topWidget = static_cast<QWidget*>(o);
                if (o == d->topWidget)
                    static_cast<QWidget *>(o)->show();
                else
                    static_cast<QWidget *>(o)->hide();
            }
        }
        setChildGeometries();
    }
    QFrame::show();
}


/*!
    Returns the widget with ID \a id. Returns 0 if this widget stack
    does not manage a widget with ID \a id.

    \sa id() addWidget()
*/

QWidget * QWidgetStack::widget(int id) const
{
    return d->hash.value(id, 0);
}


/*!
    Returns the ID of the \a widget. Returns -1 if \a widget is 0 or
    is not being managed by this widget stack.

    \sa widget() addWidget()
*/

int QWidgetStack::id(QWidget * widget) const
{
    if (!widget)
        return -1;

    QHash<int, QWidget *>::Iterator it = d->hash.begin();
    for (; it != d->hash.end(); ++it)
        if (*it == widget)
            return it.key();
    return -1;
}


/*!
    Returns the currently visible widget (the one at the top of the
    stack), or 0 if nothing is currently being shown.

    \sa aboutToShow() id() raiseWidget()
*/

QWidget * QWidgetStack::visibleWidget() const
{
    return d->topWidget;
}


/*!
    \fn void QWidgetStack::aboutToShow(int)

    This signal is emitted just before a managed widget is shown if
    that managed widget has an ID != -1. The argument is the numeric
    ID of the widget.

    If you call visibleWidget() in a slot connected to aboutToShow(),
    the widget it returns is the one that is currently visible, not
    the one that is about to be shown.
*/


/*!
    \fn void QWidgetStack::aboutToShow(QWidget *)

    \overload

    This signal is emitted just before a managed widget is shown. The
    argument is a pointer to the widget.

    If you call visibleWidget() in a slot connected to aboutToShow(),
    the widget returned is the one that is currently visible, not the
    one that is about to be shown.
*/


/*!
    \reimp
*/

void QWidgetStack::resizeEvent(QResizeEvent * /* e */)
{
    setChildGeometries();
}


/*!
    \reimp
*/

QSize QWidgetStack::sizeHint() const
{
    ensurePolished();

    QSize size(0, 0);

    QHash<int, QWidget *>::Iterator it = d->hash.begin();
    for (; it != d->hash.end(); ++it) {
        QWidget *w = *it;
        QSize sh = w->sizeHint();
        if (w->sizePolicy().horData() == QSizePolicy::Ignored)
            sh.rwidth() = 0;
        if (w->sizePolicy().verData() == QSizePolicy::Ignored)
            sh.rheight() = 0;
#ifndef QT_NO_LAYOUT
        size = size.expandedTo(sh).expandedTo(qSmartMinSize(w));
#endif
    }
    if (size.isNull())
        size = QSize(128, 64);
    size += QSize(2*frameWidth() + 2*d->margin, 2*frameWidth() + 2*d->margin);
    return size;
}


/*!
    \reimp
*/
QSize QWidgetStack::minimumSizeHint() const
{
    ensurePolished();

    QSize size(0, 0);

    QHash<int, QWidget *>::Iterator it = d->hash.begin();
    for (; it != d->hash.end(); ++it) {
        QWidget *w = *it;
        QSize sh = w->minimumSizeHint();
        if (w->sizePolicy().horData() == QSizePolicy::Ignored)
            sh.rwidth() = 0;
        if (w->sizePolicy().verData() == QSizePolicy::Ignored)
            sh.rheight() = 0;
#ifndef QT_NO_LAYOUT
        size = size.expandedTo(sh).expandedTo(w->minimumSize());
#endif
    }
    if (size.isNull())
        size = QSize(64, 32);
    size += QSize(2*frameWidth() + 2*d->margin, 2*frameWidth() + 2*d->margin);
    return size;
}

/*!
    \reimp
*/
void QWidgetStack::childEvent(QChildEvent * e)
{
    if (e->child()->isWidgetType() && e->removed())
        removeWidget((QWidget*) e->child());
}

/*! \reimp
 */
bool QWidgetStack::event(QEvent* e)
{
    if (e->type() == QEvent::LayoutRequest
#ifdef QT_COMPAT
        || e->type() == QEvent::LayoutHint
#endif
       )
        updateGeometry(); // propgate layout hints to parent
    return QFrame::event(e);
}

/*!
    Sets the widget stack's margin (how much space to leave around
    the visible widget) to \a margin.

    \sa margin()
*/
void QWidgetStack::setMargin(int margin)
{
    if (d->margin == margin)
        return;
    d->margin = margin;
    updateGeometry();
    setChildGeometries();
}

/*!
    Returns the widget stack's margin.

    \sa setMargin()
*/
int QWidgetStack::margin() const
{
    return d->margin;
}

/*!
    Returns the widget stack's contents rectangle.
*/
QRect QWidgetStack::contentsRect() const
{
    QRect cr(QFrame::contentsRect());
    cr.addCoords(d->margin, d->margin, -d->margin, -d->margin);
    return cr;
}

#endif
