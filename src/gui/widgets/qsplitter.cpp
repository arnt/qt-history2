/****************************************************************************
**
** Implementation of QSplitter class.
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

#include "qsplitter.h"
#ifndef QT_NO_SPLITTER

#include "qapplication.h"
#include "qcursor.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qlayout.h"
#include "qlist.h"
#include "qpainter.h"
#include "qrubberband.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtextstream.h"
#include "qvector.h"
#include "private/qlayoutengine_p.h"
#include "private/qsplitter_p.h"
#define d d_func()
#define q q_func()

static QSize verySmartMinSize(QWidget *widget)
{
    QSize ret = qSmartMinSize(widget);
    if (ret.width() == 1)
        ret.setWidth(0);
    if (ret.height() == 1)
        ret.setHeight(0);
    return ret;
}

class QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle(Orientation o,
                     QSplitter *parent, const char* name=0);
    void setOrientation(Orientation o);
    Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    QSize sizeHint() const;

    int id() const { return myId; } // d->list.at(id())->wid == this
    void setId(int i) { myId = i; }

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Orientation orient;
    bool opaq;
    int myId;

    QSplitter *s;
};

#include "qsplitter.moc"

const uint Default = 2;
static int mouseOffset;

QSplitterHandle::QSplitterHandle(Orientation o, QSplitter *parent,
                                  const char * name)
    : QWidget(parent, name)
{
    s = parent;
    setOrientation(o);
}

QSize QSplitterHandle::sizeHint() const
{
    int hw = s->handleWidth();
    Q4StyleOptionFrame opt(0);
    opt.rect = s->rect();
    opt.palette = s->palette();
    opt.state = QStyle::Style_Default;
    opt.lineWidth = 0;
    return parentWidget()->style().sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw),
                                                    s->fontMetrics(), s)
                                                    .expandedTo(QApplication::globalStrut());
}

void QSplitterHandle::setOrientation(Orientation o)
{
    orient = o;
#ifndef QT_NO_CURSOR
    setCursor(o == QSplitter::Horizontal ? splitHCursor : splitVCursor);
#endif
}

void QSplitterHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->state()&LeftButton))
        return;
    QCOORD pos = s->d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
                 - mouseOffset;
    if (opaque()) {
        s->moveSplitter(pos, id());
    } else {
        s->setRubberband(s->adjustPos(pos, id()));
    }
}

void QSplitterHandle::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == LeftButton)
        mouseOffset = s->d->pick(e->pos());
}

void QSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
    if (!opaque() && e->button() == LeftButton) {
        QCOORD pos = s->d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
                     - mouseOffset;
        s->setRubberband(-1);
        s->moveSplitter(pos, id());
    }
}

void QSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    Q4StyleOptionFrame opt(0);
    opt.rect = rect();
    opt.palette = palette();
    if (orientation() == Horizontal)
        opt.state = QStyle::Style_Horizontal;
    else
        opt.state = QStyle::Style_Default;
    opt.lineWidth = 0;
    parentWidget()->style().drawPrimitive(QStyle::PE_Splitter, &opt, &p, s);
}

class QSplitterLayoutStruct : public Qt
{
public:
    QRect rect;
    QCOORD sizer;
    uint isHandle : 1;
    uint collapsed : 1;
    uint collapsible : 2;
    uint resizeMode : 2;
    QWidget *wid;

    QSplitterLayoutStruct()
        : sizer(-1), isHandle(false), collapsed(false), collapsible(Default),
          resizeMode(QSplitter::Auto) { }
    QCOORD getSizer(Orientation orient);
    QCOORD pick(const QSize &size, Orientation orient)
    { return (orient == Horizontal) ? size.width() : size.height(); }
};

QCOORD QSplitterLayoutStruct::getSizer(Orientation orient)
{
    if (sizer == -1) {
        QSize s = wid->sizeHint();
        const int presizer = pick(s, orient);
        const int realsize = pick(wid->size(), orient);
        if (!s.isValid() || (wid->testAttribute(QWidget::WA_Resized) && (realsize > presizer))) {
            sizer = pick(wid->size(), orient);
        } else {
            sizer = presizer;
        }
        QSizePolicy p = wid->sizePolicy();
        int sizePolicyStretch = (orient == Horizontal) ? p.horStretch() : p.verStretch();
        if (sizePolicyStretch > 1)
            sizer *= sizePolicyStretch;
    }
    return sizer;
}

/*!
    \class QSplitter
    \brief The QSplitter class implements a splitter widget.

    \ingroup organizers
    \mainclass

    A splitter lets the user control the size of child widgets by
    dragging the boundary between the children. Any number of widgets
    may be controlled by a single splitter.

    To show a QListBox, a QListView and a QTextEdit side by side:
    \code
        QSplitter *split = new QSplitter(parent);
        QListBox *lb = new QListBox(split);
        QListView *lv = new QListView(split);
        QTextEdit *ed = new QTextEdit(split);
    \endcode

    QSplitter lays out its children horizontally (side by side); you
    can use setOrientation(QSplitter::Vertical) to lay out the
    children vertically.

    By default, all widgets can be as large or as small as the user
    wishes, between the \l minimumSizeHint() (or \l minimumSize())
    and \l maximumSize() of the widgets. Use setResizeMode() to
    specify that a widget should keep its size when the splitter is
    resized, or set the stretch component of the \l sizePolicy.

    Although QSplitter normally resizes the children only at the end
    of a resize operation, if you call setOpaqueResize(true) the
    widgets are resized as often as possible.

    The initial distribution of size between the widgets is determined
    by the initial size of each widget. You can also use setSizes() to
    set the sizes of all the widgets. The function sizes() returns the
    sizes set by the user.

    If you hide() a child its space will be distributed among the
    other children. It will be reinstated when you show() it again. It
    is also possible to reorder the widgets within the splitter using
    moveToFirst() and moveToLast().

    <img src=qsplitter-m.png> <img src=qsplitter-w.png>

    \sa QTabBar
*/


/*!
    Constructs a horizontal splitter with the \a parent and \a name
    arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter(QWidget *parent, const char *name)
    : QFrame(*new QSplitterPrivate, parent)
{
    setObjectName(name);
    d->orient = Horizontal;
    init();
}


/*!
    Constructs a splitter with orientation \a o with the \a parent and
    \a name arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter(Orientation o, QWidget *parent, const char *name)
    : QFrame(*new QSplitterPrivate, parent)
{
    setObjectName(name);
    d->orient = o;
    init();
}


/*!
    Destroys the splitter and any children.
*/

QSplitter::~QSplitter()
{
    while (!d->list.isEmpty())
        delete d->list.takeFirst();
}

void QSplitter::init()
{
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
    if (d->orient == Vertical)
        sp.transpose();
    setSizePolicy(sp);
    clearWState(WState_OwnSizePolicy);
}

/*!
    \fn void QSplitter::refresh()

    Updates the splitter's state. You should not need to call this
    function.
*/


/*!
    \property QSplitter::orientation
    \brief the orientation of the splitter

    By default the orientation is horizontal (the widgets are side by
    side). The possible orientations are \c Horizontal and
    \c Vertical.
*/

void QSplitter::setOrientation(Orientation o)
{
    if (d->orient == o)
        return;

    if (!testWState(WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);
        clearWState(WState_OwnSizePolicy);
    }

    d->orient = o;

    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->isHandle)
            static_cast<QSplitterHandle *>(s->wid)->setOrientation(o);
    }
    recalc(isVisible());
}

QSplitter::Orientation QSplitter::orientation() const
{
    return d->orient;
}

/*!
    \property QSplitter::childrenCollapsible
    \brief whether child widgets can be resized down to size 0 by the user

    By default, children are collapsible. It is possible to enable
    and disable the collapsing of individual children; see
    setCollapsible().
*/

void QSplitter::setChildrenCollapsible(bool collapse)
{
    d->childrenCollapsible = collapse;
}

bool QSplitter::childrenCollapsible() const
{
    return d->childrenCollapsible;
}

/*!
    Sets whether the child widget \a w is collapsible to \a collapse.

    By default, children are collapsible, meaning that the user can
    resize them down to size 0, even if they have a non-zero
    minimumSize() or minimumSizeHint(). This behavior can be changed
    on a per-widget basis by calling this function, or globally for
    all the widgets in the splitter by setting the \l
    childrenCollapsible property.

    \sa childrenCollapsible
*/

void QSplitter::setCollapsible(QWidget *w, bool collapse)
{
    findWidget(w)->collapsible = collapse ? 1 : 0;
}

/*!
    \reimp
*/
void QSplitter::resizeEvent(QResizeEvent *)
{
    doResize();
}

QSplitterLayoutStruct *QSplitter::findWidget(QWidget *w)
{
    processChildEvents();
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    while (it != d->list.end()) {
        if ((*it)->wid == w)
            return *it;
        ++it;
    }
    return addWidget(w);
}

/*
    Inserts the widget \a w at the end (or at the beginning if \a
    prepend is true) of the splitter's list of widgets.

    It is the responsibility of the caller to make sure that \a w is
    not already in the splitter and to call recalcId() if needed. (If
    \a prepend is true, then recalcId() is very probably needed.)
*/

QSplitterLayoutStruct *QSplitter::addWidget(QWidget *w, bool prepend)
{
    QSplitterLayoutStruct *s;
    QSplitterHandle *newHandle = 0;
    if (d->list.count() > 0) {
        s = new QSplitterLayoutStruct;
        s->resizeMode = KeepSize;
        QString tmp = QLatin1String("qt_splithandle_");
        tmp += w->objectName();
        newHandle = new QSplitterHandle(orientation(), this);
        newHandle->setObjectName(tmp);
        s->wid = newHandle;
        newHandle->setId(d->list.count());
        s->isHandle = true;
        s->sizer = d->pick(newHandle->sizeHint());
        if (prepend)
            d->list.prepend(s);
        else
            d->list.append(s);
    }
    s = new QSplitterLayoutStruct;
    s->resizeMode = DefaultResizeMode;
    s->wid = w;
    s->isHandle = false;
    if (prepend)
        d->list.prepend(s);
    else
        d->list.append(s);
    if (newHandle && isVisible())
        newHandle->show(); // will trigger sending of post events
    return s;
}


/*!
    Tells the splitter that the child widget described by \a c has
    been inserted or removed.
*/

void QSplitter::childEvent(QChildEvent *c)
{
    if (c->type() == QEvent::ChildInserted) {
        if (!c->child()->isWidgetType())
            return;

        if (((QWidget*)c->child())->testWFlags(WType_TopLevel))
            return;

        QList<QSplitterLayoutStruct *>::iterator it = d->list.begin();
        while (it != d->list.end()) {
            if ((*it)->wid == c->child())
                return;
            ++it;
        }
        addWidget((QWidget*)c->child());
        recalc(isVisible());
    } else if (c->type() == QEvent::ChildRemoved) {
        QSplitterLayoutStruct *prev = 0;
        if (d->list.count() > 1)
            prev = d->list.at(1);  // yes, this is correct
        QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
        while (it != d->list.end()) {
            if ((*it)->wid == c->child()) {
                delete *it;
                d->list.erase(it);
                if (prev && prev->isHandle) {
                    QWidget *w = prev->wid;
                    d->list.removeAll(prev);
                    delete prev;
                    delete w; // will call childEvent()
                }
                recalcId();
                doResize();
                return;
            }
            prev = *it;
            ++it;
        }
    }
}


/*!
    Displays a rubber band at position \a p. If \a p is negative, the
    rubber band is removed.
*/

void QSplitter::setRubberband(int p)
{
    if(p < 0) {
        if(d->rubber)
            d->rubber->hide();
        return;
    }
    QRect r = contentsRect();
    const int rBord = 3; // customizable?
    int hw = handleWidth();
    if(!d->rubber)
        d->rubber = new QRubberBand(QRubberBand::Line, this);
    if(d->orient == Horizontal)
        d->rubber->setGeometry(QRect(mapToGlobal(QPoint(p + hw / 2 - rBord, r.y())), QSize(2 * rBord, r.height())));
    else
        d->rubber->setGeometry(QRect(mapToGlobal(QPoint(r.x(), p + hw / 2 - rBord)), QSize(r.width(), 2 * rBord)));
    if(!d->rubber->isVisible())
        d->rubber->show();
}


/*!
    \reimp
*/

bool QSplitter::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Show:
        if (!d->firstShow)
            break;
        d->firstShow = false;
        // fall through
    case QEvent::LayoutHint:
        recalc(isVisible());
        break;
    default:
        ;
    }
    return QWidget::event(e);
}


/*!
    Returns the ID of the widget to the right of or below the widget
    \a w, or 0 if there is no such widget (i.e. it is either not in
    this QSplitter or \a w is at the end).
*/

int QSplitter::idAfter(QWidget* w) const
{
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    bool seen_w = false;
    while (it != d->list.end()) {
        if ((*it)->isHandle && seen_w)
            return d->list.indexOf(*it);
        if (!(*it)->isHandle && (*it)->wid == w)
            seen_w = true;
        ++it;
    }
    return 0;
}


/*!
    Moves the left/top edge of the splitter handle with ID \a id as
    close as possible to position \a p, which is the distance from the
    left (or top) edge of the widget.

    For Arabic, Hebrew and other right-to-left languages the layout is
    reversed.  \a p is then the distance from the right (or top) edge
    of the widget.

    \sa idAfter()
*/
void QSplitter::moveSplitter(QCOORD p, int id)
{
    QSplitterLayoutStruct *s = d->list.at(id);
    int farMin;
    int min;
    int max;
    int farMax;

    p = adjustPos(p, id, &farMin, &min, &max, &farMax);
    int oldP = d->pick(s->rect.topLeft());

    if (QApplication::reverseLayout() && d->orient == Horizontal) {
        int qs = p + s->rect.width();
        doMove(false, qs, id - 1, -1, (qs > oldP), (p > max));
        doMove(true, qs, id, -1, (qs > oldP), (p < min));
    } else {
        doMove(false, p, id, +1, (p < oldP), (p > max));
        doMove(true, p, id - 1, +1, (p < oldP), (p < min));
    }
    storeSizes();
}

void QSplitter::setGeo(QSplitterLayoutStruct *sls, int p, int s, bool splitterMoved)
{
    QWidget *w = sls->wid;
    QRect r;
    if (d->orient == Horizontal) {
        if (QApplication::reverseLayout() && !splitterMoved)
            p = contentsRect().width() - p - s;
        r.setRect(p, contentsRect().y(), s, contentsRect().height());
    } else {
        r.setRect(contentsRect().x(), p, contentsRect().width(), s);
    }
    sls->rect = r;
    sls->collapsed = false;

    /*
      Hide the child widget, but without calling hide() so that the
      splitter handle is still shown.
    */
    int minSize = d->pick(verySmartMinSize(w));
    if (!w->isHidden() && s <= 0 && minSize > 0) {
        sls->collapsed = (minSize > 1);
        r.moveTopLeft(QPoint(-QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX));
    }
    w->setGeometry(r);
}

void QSplitter::doMove(bool backwards, int pos, int id, int delta, bool upLeft, bool mayCollapse)
{
    if (id < 0 || id >= d->list.count())
        return;

    QSplitterLayoutStruct *s = d->list.at(id);
    QWidget *w = s->wid;

    int nextId = backwards ? id - delta : id + delta;

    if (w->isHidden()) {
        doMove(backwards, pos, nextId, delta, upLeft, true);
    } else {
        if (s->isHandle) {
            int dd = s->getSizer(d->orient);
            int nextPos = backwards ? pos - dd : pos + dd;
            int left = backwards ? pos - dd : pos;
            setGeo(s, left, dd, true);
            doMove(backwards, nextPos, nextId, delta, upLeft, mayCollapse);
        } else {
            int dd = backwards ? pos - d->pick(s->rect.topLeft())
                               : d->pick(s->rect.bottomRight()) - pos + 1;
            if (dd > 0 || (!s->collapsed && !mayCollapse)) {
                dd = qMin(dd, d->pick(w->maximumSize()));
                dd = qMax(dd, d->pick(verySmartMinSize(w)));
            } else {
                dd = 0;
            }
            setGeo(s, backwards ? pos - dd : pos, dd, true);
            doMove(backwards, backwards ? pos - dd : pos + dd, nextId, delta,
                    upLeft, true);
        }
    }
}

int QSplitter::findWidgetJustBeforeOrJustAfter(int id, int delta, int &collapsibleSize)
{
    id += delta;
    do {
        QWidget *w = d->list.at(id)->wid;
        if (!w->isHidden()) {
            if (collapsible(d->list.at(id)))
                collapsibleSize = d->pick(verySmartMinSize(w));
            return id;
        }
        id += 2 * delta; // go to previous (or next) widget, skip the handle
    } while (id >= 0 && id < d->list.count());

    return -1;
}

void QSplitter::getRange(int id, int *farMin, int *min, int *max, int *farMax)
{
    int n = d->list.count();
    if (id <= 0 || id >= n - 1)
        return;

    int collapsibleSizeBefore = 0;
    int idJustBefore = findWidgetJustBeforeOrJustAfter(id, -1, collapsibleSizeBefore);

    int collapsibleSizeAfter = 0;
    int idJustAfter = findWidgetJustBeforeOrJustAfter(id, +1, collapsibleSizeAfter);

    int minBefore = 0;
    int minAfter = 0;
    int maxBefore = 0;
    int maxAfter = 0;
    int i;

    for (i = 0; i < id; i++)
        addContribution(i, &minBefore, &maxBefore, i == idJustBefore);
    for (i = id; i < n; i++)
        addContribution(i, &minAfter, &maxAfter, i == idJustAfter);

    QRect r = contentsRect();
    int farMinVal;
    int minVal;
    int maxVal;
    int farMaxVal;

    int smartMinBefore = qMax(minBefore, d->pick(r.size()) - maxAfter);
    int smartMaxBefore = qMin(maxBefore, d->pick(r.size()) - minAfter);

    if (d->orient == Vertical || !QApplication::reverseLayout()) {
        minVal = d->pick(r.topLeft()) + smartMinBefore;
        maxVal = d->pick(r.topLeft()) + smartMaxBefore;

        farMinVal = minVal;
        if (minBefore - collapsibleSizeBefore >= d->pick(r.size()) - maxAfter)
            farMinVal -= collapsibleSizeBefore;
        farMaxVal = maxVal;
        if (d->pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore)
            farMaxVal += collapsibleSizeAfter;
    } else {
        int hw = handleWidth();
        minVal = r.width() - smartMaxBefore - hw;
        maxVal = r.width() - smartMinBefore - hw;

        farMinVal = minVal;
        if (d->pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore)
            farMinVal -= collapsibleSizeAfter;
        farMaxVal = maxVal;
        if (minBefore - collapsibleSizeBefore >= d->pick(r.size()) - maxAfter)
            farMaxVal += collapsibleSizeBefore;
    }

    if (farMin)
        *farMin = farMinVal;
    if (min)
        *min = minVal;
    if (max)
        *max = maxVal;
    if (farMax)
        *farMax = farMaxVal;
}

/*!
    Returns the valid range of the splitter with ID \a id in \a *min
    and \a *max if \a min and \a max are not 0.

    \sa idAfter()
*/

void QSplitter::getRange(int id, int *min, int *max)
{
    getRange(id, min, 0, 0, max);
}


/*!
    Returns the closest legal position to \a pos of the widget with ID
    \a id.

    \sa idAfter()
*/

int QSplitter::adjustPos(int pos, int id)
{
    int x, i, n, u;
    return adjustPos(pos, id, &u, &n, &i, &x);
}

int QSplitter::adjustPos(int pos, int id, int *farMin, int *min, int *max,
                          int *farMax)
{
    const int Threshold = 40;

    getRange(id, farMin, min, max, farMax);

    if (pos >= *min) {
        if (pos <= *max) {
            return pos;
        } else {
            int delta = pos - *max;
            int width = *farMax - *max;

            if (delta > width / 2 && delta >= qMin(Threshold, width)) {
                return *farMax;
            } else {
                return *max;
            }
        }
    } else {
        int delta = *min - pos;
        int width = *min - *farMin;

        if (delta > width / 2 && delta >= qMin(Threshold, width)) {
            return *farMin;
        } else {
            return *min;
        }
    }
}

bool QSplitter::collapsible(QSplitterLayoutStruct *s)
{
    if (s->collapsible != Default) {
        return (bool)s->collapsible;
    } else {
        return d->childrenCollapsible;
    }
}

void QSplitter::doResize()
{
    QRect r = contentsRect();
    int n = d->list.count();
    QVector<QLayoutStruct> a(n);
    int numAutoWithStretch = 0;
    int numAutoWithoutStretch = 0;
    int i;

    for (int pass = 0; pass < 2; pass++) {
        for (i = 0; i < n; ++i) {
            a[i].init();
            QSplitterLayoutStruct *s = d->list.at(i);
            if (s->wid->isHidden() || s->collapsed) {
                a[i].maximumSize = 0;
            } else if (s->isHandle) {
                a[i].sizeHint = a[i].minimumSize = a[i].maximumSize = s->sizer;
                a[i].empty = false;
            } else {
                int mode = s->resizeMode;

                if (mode == DefaultResizeMode) {
                    QSizePolicy p = s->wid->sizePolicy();
                    int sizePolicyStretch =
                            d->pick(QSize(p.horStretch(), p.verStretch()));
                    if (sizePolicyStretch > 0) {
                        mode = Stretch;
                        ++numAutoWithStretch;
                    } else {
                        /*
                          Do things differently on the second pass,
                          if there's one. A second pass is necessary
                          if it was found out during the first pass
                          that all DefaultResizeMode items are
                          KeepSize items. In that case, we make them
                          all Stretch items instead, for a more Qt
                          3.0-compatible behavior.
                        */
                        mode = (pass == 0) ? KeepSize : Stretch;
                        ++numAutoWithoutStretch;
                    }
                }

                a[i].minimumSize = d->pick(verySmartMinSize(s->wid));
                a[i].maximumSize = d->pick(s->wid->maximumSize());
                a[i].empty = false;

                if (mode == Stretch) {
                    a[i].stretch = s->getSizer(d->orient);
                    a[i].sizeHint = a[i].minimumSize;
                } else if (mode == KeepSize) {
                    a[i].sizeHint = s->getSizer(d->orient);
                } else { // mode == FollowSizeHint
                    a[i].sizeHint = d->pick(s->wid->sizeHint());
                }
            }
        }

        // a second pass would yield the same results
        if (numAutoWithStretch > 0 || numAutoWithoutStretch == 0)
            break;
    }

    Q_LLONG total = 0;
    for (i = 0; i < n; ++i) {
        if (a[i].stretch == 0) {
            total += a[i].sizeHint;
        } else {
            total += a[i].maximumSize;
        }
    }
    if (total < (Q_LLONG)d->pick(r.size())) {
        for (i = 0; i < n; i += 2) {
            if (a[i].stretch == 0)
                a[i].stretch = 1;
        }
    }

    qGeomCalc(a, 0, n, d->pick(r.topLeft()), d->pick(r.size()), 0);

    for (i = 0; i < n; ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        setGeo(s, a[i].pos, a[i].size, false);
    }
}

void QSplitter::recalc(bool update)
{
    int fi = 2 * frameWidth();
    int maxl = fi;
    int minl = fi;
    int maxt = QWIDGETSIZE_MAX;
    int mint = fi;
    int n = d->list.count();
    bool first = true;

    /*
      Splitter handles before the first visible widget or right
      before a hidden widget must be hidden.
    */
    for (int i = 0; i < n; i++) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (!s->isHandle) {
            QSplitterLayoutStruct *p = 0;
            if (i > 0)
                p = d->list.at(i - 1);

            // may trigger new recalc
            if (p && p->isHandle)
                p->wid->setHidden(first || s->wid->isHidden());

            if (!s->wid->isHidden())
                first = false;
        }
    }

    bool empty = true;
    for (int j = 0; j < n; j++) {
        QSplitterLayoutStruct *s = d->list.at(j);
        if (!s->wid->isHidden()) {
            empty = false;
            if (s->isHandle) {
                minl += s->getSizer(d->orient);
                maxl += s->getSizer(d->orient);
            } else {
                QSize minS = verySmartMinSize(s->wid);
                minl += d->pick(minS);
                maxl += d->pick(s->wid->maximumSize());
                mint = qMax(mint, d->trans(minS));
                int tm = d->trans(s->wid->maximumSize());
                if (tm > 0)
                    maxt = qMin(maxt, tm);
            }
        }
    }
    if (empty) {
        if (qt_cast<QSplitter*>(parentWidget())) {
            // nested splitters; be nice
            maxl = maxt = 0;
        } else {
            // QSplitter with no children yet
            maxl = QWIDGETSIZE_MAX;
        }
    } else {
        maxl = qMin(maxl, QWIDGETSIZE_MAX);
    }
    if (maxt < mint)
        maxt = mint;

    if (d->orient == Horizontal) {
        setMaximumSize(maxl, maxt);
        setMinimumSize(minl, mint);
    } else {
        setMaximumSize(maxt, maxl);
        setMinimumSize(mint, minl);
    }
    if (update)
        doResize();
    else
        d->firstShow = true;
}

/*!
    \enum QSplitter::ResizeMode

    This enum type describes how QSplitter will resize each of its
    child widgets.

    \value Auto  The widget will be resized according to the stretch
    factors set in its sizePolicy().

    \value Stretch  The widget will be resized when the splitter
    itself is resized.

    \value KeepSize  QSplitter will try to keep the widget's size
    unchanged.

    \value FollowSizeHint  QSplitter will resize the widget when the
    widget's size hint changes.
*/

/*!
    Sets resize mode of widget \a w to \a mode. (The default is \c
    Auto.)
*/

void QSplitter::setResizeMode(QWidget *w, ResizeMode mode)
{
    findWidget(w)->resizeMode = mode;
}


/*!
    \property QSplitter::opaqueResize
    \brief whether resizing is opaque

    Opaque resizing is off by default.
*/

bool QSplitter::opaqueResize() const
{
    return d->opaque;
}


void QSplitter::setOpaqueResize(bool on)
{
    d->opaque = on;
}


/*!
    Moves widget \a w to the leftmost/top position.
*/

void QSplitter::moveToFirst(QWidget *w)
{
    // ### Jasmin 4.0: use QList::move() instead of that stuff, and do the same with moveToLast()
    processChildEvents();
    bool found = false;
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    while (it != d->list.end()) {
        if ((*it)->wid == w) {
            found = true;
            QSplitterLayoutStruct *s = *it;
            QSplitterLayoutStruct *p = *(--it);
            if (it != d->list.begin()) { // not already at first place
                d->list.removeAll(p);
                d->list.removeAll(s);
                d->list.prepend(p);
                d->list.prepend(s);
            }
            break;
        }
        ++it;
    }
    if (!found)
        addWidget(w, true);
    recalcId();
}


/*!
    Moves widget \a w to the rightmost/bottom position.
*/

void QSplitter::moveToLast(QWidget *w)
{
    processChildEvents();
    bool found = false;
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    while (it != d->list.end()) {
        if ((*it)->wid == w) {
            found = true;
            QSplitterLayoutStruct *s = *it;
            QSplitterLayoutStruct *p = *(++it);
            if (it != d->list.end()) { // the splitter handle after s
                d->list.removeAll(p);
                d->list.append(p);
            }
            d->list.removeAll(s);
            d->list.append(s);
            break;
        }
        ++it;
    }
    if (!found)
        addWidget(w);
    recalcId();
}


void QSplitter::recalcId()
{
    int n = d->list.count();
    for (int i = 0; i < n; i++) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->isHandle)
            ((QSplitterHandle*)s->wid)->setId(i);
    }
}


/*!
    \reimp
*/
QSize QSplitter::sizeHint() const
{
    ensurePolished();
    int l = 0;
    int t = 0;
    QObjectList childs = children();
    for (int i = 0; i < childs.size(); ++i) {
        QObject * o = childs.at(i);
        if (o->isWidgetType() && !static_cast<QWidget*>(o)->isHidden()) {
            QSize s = static_cast<QWidget*>(o)->sizeHint();
            if (s.isValid()) {
                l += d->pick(s);
                t = qMax(t, d->trans(s));
            }
        }
    }
    return orientation() == Horizontal ? QSize(l, t) : QSize(t, l);
}


/*!
    \reimp
*/

QSize QSplitter::minimumSizeHint() const
{
    ensurePolished();
    int l = 0;
    int t = 0;
    QObjectList childs = children();
    for (int i = 0; i < childs.size(); ++i) {
        QObject * o = childs.at(i);
        if (o->isWidgetType() && !static_cast<QWidget *>(o)->isHidden()) {
            QSize s = verySmartMinSize(static_cast<QWidget *>(o));
            if (s.isValid()) {
                l += d->pick(s);
                t = qMax(t, d->trans(s));
            }
        }
    }
    return orientation() == Horizontal ? QSize(l, t) : QSize(t, l);
}


void QSplitter::storeSizes()
{
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    while (it != d->list.end()) {
        if (!(*it)->isHandle)
            (*it)->sizer = d->pick((*it)->rect.size());
        ++it;
    }
}


void QSplitter::addContribution(int id, int *min, int *max,
                                 bool mayCollapse)
{
    QSplitterLayoutStruct *s = d->list.at(id);
    if (!s->wid->isHidden()) {
        if (s->isHandle) {
            *min += s->getSizer(d->orient);
            *max += s->getSizer(d->orient);
        } else {
            if (mayCollapse || !s->collapsed)
                *min += d->pick(verySmartMinSize(s->wid));
            *max += d->pick(s->wid->maximumSize());
        }
    }
}


/*!
    Returns a list of the size parameters of all the widgets in this
    splitter.

    If the splitter's orientation is horizontal, the list is a list of
    widget widths; if the orientation is vertical, the list is a list
    of widget heights.

    Giving the values to another splitter's setSizes() function will
    produce a splitter with the same layout as this one.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<int> list = mySplitter.sizes();
    QList<int>::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

    \sa setSizes()
*/

QList<int> QSplitter::sizes() const
{
    ensurePolished();

    QList<int> list;
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    while (it != d->list.end()) {
        if (!(*it)->isHandle)
            list.append(d->pick((*it)->rect.size()));
        ++it;
    }
    return list;
}

/*!
    Sets the size parameters to the values given in the \a list. If
    the splitter is horizontal, the values set the widths of each
    widget going from left to right. If the splitter is vertical, the
    values set the heights of each widget going from top to bottom.
    Extra values in the \a list are ignored.

    If \a list contains too few values, the result is undefined but
    the program will still be well-behaved.

    Note that the values in \a list should be the height/width that
    the widgets should be resized to.

    \sa sizes()
*/

void QSplitter::setSizes(QList<int> list)
{
    processChildEvents();
    QList<int>::Iterator it = list.begin();
    QList<QSplitterLayoutStruct*>::iterator it2 = d->list.begin();
    while (it2 != d->list.end() && it != list.end()) {
        if (!(*it2)->isHandle) {
            (*it2)->collapsed = false;
            (*it2)->sizer = qMax(*it, 0);
            int smartMinSize = d->pick(verySmartMinSize((*it2)->wid));
            // Make sure that we reset the collapsed state.
            if ((*it2)->sizer == 0) {
                if (collapsible(*it2) && smartMinSize > 0) {
                    (*it2)->collapsed = true;
                } else {
                    (*it2)->sizer = smartMinSize;
                }
            } else {
                if ((*it2)->sizer < smartMinSize)
                    (*it2)->sizer = smartMinSize;
            }
            ++it;
        }
        ++it2;
    }
    doResize();
}

/*!
    \property QSplitter::handleWidth
    \brief the width of the splitter handle
*/

int QSplitter::handleWidth() const
{
    if (d->handleWidth > 0) {
        return d->handleWidth;
    } else {
        return style().pixelMetric(QStyle::PM_SplitterWidth, this);
    }
}

void QSplitter::setHandleWidth(int width)
{
    d->handleWidth = width;
    updateHandles();
}

/*!
    Processes all posted child events, ensuring that the internal state of
    the splitter is kept consistent.
*/

void QSplitter::processChildEvents()
{
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
}

/*!
    \reimp
*/
void QSplitter::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        updateHandles();
    QFrame::changeEvent(ev);
}

void QSplitter::updateHandles()
{
    int hw = handleWidth();
    QList<QSplitterLayoutStruct*>::iterator it = d->list.begin();
    while (it != d->list.end()) {
        if ((*it)->isHandle)
            (*it)->sizer = hw;
        ++it;
    }
    recalc(isVisible());
}

#ifndef QT_NO_TEXTSTREAM
/*!
    \relates QSplitter

    Writes the sizes and the hidden state of the widgets in the
    splitter \a splitter to the text stream \a ts.

    \sa operator>>(), sizes(), QWidget::isHidden()
*/

QTextStream& operator<<(QTextStream& ts, const QSplitter& splitter)
{
    QList<QSplitterLayoutStruct*>::iterator it = splitter.d->list.begin();
    bool first = true;
    ts << "[";

    while (it != splitter.d->list.end()) {
        if (!(*it)->isHandle) {
            if (!first)
                ts << ",";

            if ((*it)->wid->isHidden()) {
                ts << "H";
            } else {
                ts << splitter.d->pick((*it)->rect.size());
            }
            first = false;
        }
        ++it;;
    }
    ts << "]" << endl;
    return ts;
}

/*!
    \relates QSplitter

    Reads the sizes and the hidden state of the widgets in the
    splitter \a splitter from the text stream \a ts. The sizes must
    have been previously written by the operator<<() function.

    \sa operator<<(), setSizes(), QWidget::hide()
*/

QTextStream& operator>>(QTextStream& ts, QSplitter& splitter)
{
#undef SKIP_SPACES
#define SKIP_SPACES() \
    while (line[i].isSpace()) \
        i++

    splitter.processChildEvents();
    QList<QSplitterLayoutStruct*>::iterator it = splitter.d->list.begin();
    QString line = ts.readLine();
    int i = 0;

    SKIP_SPACES();
    if (line[i] == '[') {
        i++;
        SKIP_SPACES();
        while (line[i] != ']') {
            while ((*it) != 0 && (*it)->isHandle)
                ++it;
            if (it == splitter.d->list.end())
                break;

            if (line[i].toUpper() == 'H') {
                (*it)->wid->hide();
                i++;
            } else {
                (*it)->wid->show();
                int dim = 0;
                while (line[i].digitValue() >= 0) {
                    dim *= 10;
                    dim += line[i].digitValue();
                    i++;
                }
                (*it)->sizer = dim;
                if (dim == 0)
                    splitter.setGeo(*it, 0, 0, false);
            }
            SKIP_SPACES();
            if (line[i] == ',') {
                i++;
            } else {
                break;
            }
            SKIP_SPACES();
            ++it;
        }
    }
    splitter.doResize();
    return ts;
}
#endif

#endif
