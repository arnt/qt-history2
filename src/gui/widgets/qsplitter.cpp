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
#include "qvarlengtharray.h"
#include "qvector.h"
#include "private/qlayoutengine_p.h"
#include "private/qsplitter_p.h"
#define d d_func()
#define q q_func()

//#define QSPLITTER_DEBUG

static QSize verySmartMinSize(const QWidget *widget)
{
    QSize ret = qSmartMinSize(widget);
    if (ret.width() <= 1)
        ret.setWidth(0);
    if (ret.height() <= 1)
        ret.setHeight(0);
    return ret;
}

class QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle(Qt::Orientation o, QSplitter *parent);
    void setOrientation(Qt::Orientation o);
    Qt::Orientation orientation() const { return orient; }

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
    Qt::Orientation orient;
    bool opaq;
    int myId;

    QSplitter *s;
};

#include "qsplitter.moc"

const uint Default = 2;
static int mouseOffset;

QSplitterHandle::QSplitterHandle(Qt::Orientation o, QSplitter *parent)
    : QWidget(parent)
{
    s = parent;
    setOrientation(o);
}

QSize QSplitterHandle::sizeHint() const
{
    int hw = s->handleWidth();
    QStyleOption opt(0);
    opt.init(s);
    opt.state = QStyle::Style_None;
    return parentWidget()->style()->sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw), s)
        .expandedTo(QApplication::globalStrut());
}

void QSplitterHandle::setOrientation(Qt::Orientation o)
{
    orient = o;
#ifndef QT_NO_CURSOR
    setCursor(o == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
#endif
}

void QSplitterHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton))
        return;
    int pos = s->d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
                 - mouseOffset;
    if (opaque()) {
        s->moveSplitter(pos, id());
    } else {
        s->setRubberband(s->adjustPos(pos, id()));
    }
}

void QSplitterHandle::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        mouseOffset = s->d->pick(e->pos());
}

void QSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
    if (!opaque() && e->button() == Qt::LeftButton) {
        int pos = s->d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
                     - mouseOffset;
        s->setRubberband(-1);
        s->moveSplitter(pos, id());
    }
}

void QSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    if (orientation() == Qt::Horizontal)
        opt.state = QStyle::Style_Horizontal;
    else
        opt.state = QStyle::Style_None;
    parentWidget()->style()->drawControl(QStyle::CE_Splitter, &opt, &p, s);
}

class QSplitterLayoutStruct
{
public:
    QRect rect;
    int sizer;
    uint isHandle : 1;
    uint collapsed : 1;
    uint collapsible : 2;
    QWidget *wid;

    QSplitterLayoutStruct() : sizer(-1), isHandle(false), collapsed(false), collapsible(Default) {}
    int getSizer(Qt::Orientation orient);
    int pick(const QSize &size, Qt::Orientation orient)
    { return (orient == Qt::Horizontal) ? size.width() : size.height(); }
};

int QSplitterLayoutStruct::getSizer(Qt::Orientation orient)
{
    if (sizer == -1) {
        QSize s = wid->sizeHint();
        const int presizer = pick(s, orient);
        const int realsize = pick(wid->size(), orient);
        if (!s.isValid() || (wid->testAttribute(Qt::WA_Resized) && (realsize > presizer))) {
            sizer = pick(wid->size(), orient);
        } else {
            sizer = presizer;
        }
        QSizePolicy p = wid->sizePolicy();
        int sf = (orient == Qt::Horizontal) ? p.horizontalStretch() : p.verticalStretch();
        if (sf > 1)
            sizer *= sf;
    }
    return sizer;
}

void QSplitterPrivate::init()
{
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
    if (orient == Qt::Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->clearWState(Qt::WState_OwnSizePolicy);
}

void QSplitterPrivate::recalc(bool update)
{
    int fi = 2 * q->frameWidth();
    int maxl = fi;
    int minl = fi;
    int maxt = QWIDGETSIZE_MAX;
    int mint = fi;
    int n = list.count();
    bool first = true;

    /*
      Splitter handles before the first visible widget or right
      before a hidden widget must be hidden.
    */
    for (int i = 0; i < n; i++) {
        QSplitterLayoutStruct *s = list.at(i);
        if (!s->isHandle) {
            QSplitterLayoutStruct *p = 0;
            if (i > 0)
                p = list.at(i - 1);

            // may trigger new recalc
            if (p && p->isHandle)
                p->wid->setHidden(first || s->wid->isHidden());

            if (!s->wid->isHidden())
                first = false;
        }
    }

    bool empty = true;
    for (int j = 0; j < n; j++) {
        QSplitterLayoutStruct *s = list.at(j);
        if (!s->wid->isHidden()) {
            empty = false;
            if (s->isHandle) {
                minl += s->getSizer(orient);
                maxl += s->getSizer(orient);
            } else {
                QSize minS = verySmartMinSize(s->wid);
                minl += pick(minS);
                maxl += pick(s->wid->maximumSize());
                mint = qMax(mint, d->trans(minS));
                int tm = trans(s->wid->maximumSize());
                if (tm > 0)
                    maxt = qMin(maxt, tm);
            }
        }
    }
    if (empty) {
        if (qt_cast<QSplitter *>(q->parentWidget())) {
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

    if (update) {
        if (orient == Qt::Horizontal) {
            q->setMaximumSize(maxl, maxt);
            if (q->isTopLevel())
                q->setMinimumSize(minl,mint);
        } else {
            q->setMaximumSize(maxt, maxl);
            if (q->isTopLevel())
                q->setMinimumSize(mint,minl);
        }
        doResize();
    } else {
        firstShow = true;
    }
}

void QSplitterPrivate::doResize()
{
    QRect r = q->contentsRect();
    int n = list.count();
    QVector<QLayoutStruct> a(n);
    int i;

    bool noStretchFactorsSet = true;
    for (i = 0; i < n; ++i) {
        QSizePolicy p = list.at(i)->wid->sizePolicy();
        int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
        if (sf != 0) {
            noStretchFactorsSet = false;
            break;
        }
    }

    for (i = 0; i < n; ++i) {
        a[i].init();
        QSplitterLayoutStruct *s = list.at(i);
#ifdef QSPLITTER_DEBUG
        qDebug("widget %d hidden: %d collapsed: %d handle: %d", i, s->wid->isHidden(), s->collapsed, s->isHandle);
#endif
        if (s->wid->isHidden() || s->collapsed) {
            a[i].maximumSize = 0;
        } else if (s->isHandle) {
            a[i].sizeHint = a[i].minimumSize = a[i].maximumSize = s->sizer;
            a[i].empty = false;
        } else {
            a[i].minimumSize = pick(verySmartMinSize(s->wid));
            a[i].maximumSize = pick(s->wid->maximumSize());
            a[i].empty = false;

            bool stretch = noStretchFactorsSet;
            if (!stretch) {
                QSizePolicy p = s->wid->sizePolicy();
                int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
                stretch = (sf != 0);
            }
            if (stretch) {
                a[i].stretch = s->getSizer(orient);
                a[i].sizeHint = a[i].minimumSize;
                a[i].expansive = true;
            } else {
                a[i].sizeHint = s->getSizer(orient);
            }
        }
    }

    qGeomCalc(a, 0, n, pick(r.topLeft()), pick(r.size()), 0);

#ifdef QSPLITTER_DEBUG
    for (i = 0; i < n; ++i) {
        qDebug("%*s%d: stretch %d, sh %d, minS %d, maxS %d, exp %d, emp %d -> %d, %d",
               i, "", i,
               a[i].stretch,
               a[i].sizeHint,
               a[i].minimumSize,
               a[i].maximumSize,
               a[i].expansive,
               a[i].empty,
               a[i].pos,
               a[i].size);
    }
#endif

    for (i = 0; i < n; ++i) {
        QSplitterLayoutStruct *s = list.at(i);
        setGeo(s, a[i].pos, a[i].size, false);
    }
}

void QSplitterPrivate::storeSizes()
{
    for (int i = 0; i < list.size(); ++i) {
        QSplitterLayoutStruct *sls = list.at(i);
        if (!sls->isHandle)
            sls->sizer = pick(sls->rect.size());
    }
}

void QSplitterPrivate::addContribution(int id, int *min, int *max, bool mayCollapse)
{
    QSplitterLayoutStruct *s = list.at(id);
    if (!s->wid->isHidden()) {
        if (s->isHandle) {
            *min += s->getSizer(orient);
            *max += s->getSizer(orient);
        } else {
            if (mayCollapse || !s->collapsed)
                *min += pick(verySmartMinSize(s->wid));
            *max += pick(s->wid->maximumSize());
        }
    }
}

int QSplitterPrivate::findWidgetJustBeforeOrJustAfter(int id, int delta, int &collapsibleSize)
{
    id += delta;
    do {
        QWidget *w = list.at(id)->wid;
        if (!w->isHidden()) {
            if (collapsible(list.at(id)))
                collapsibleSize = pick(verySmartMinSize(w));
            return id;
        }
        id += 2 * delta; // go to previous (or next) widget, skip the handle
    } while (id >= 0 && id < list.count());

    return -1;
}

void QSplitterPrivate::getRange(int id, int *farMin, int *min, int *max, int *farMax)
{
    int n = list.count();
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

    for (i = 0; i < id; ++i)
        addContribution(i, &minBefore, &maxBefore, i == idJustBefore);
    for (i = id; i < n; ++i)
        addContribution(i, &minAfter, &maxAfter, i == idJustAfter);

    QRect r = q->contentsRect();
    int farMinVal;
    int minVal;
    int maxVal;
    int farMaxVal;

    int smartMinBefore = qMax(minBefore, pick(r.size()) - maxAfter);
    int smartMaxBefore = qMin(maxBefore, pick(r.size()) - minAfter);

    if (orient == Qt::Vertical || QApplication::isLeftToRight()) {
        minVal = pick(r.topLeft()) + smartMinBefore;
        maxVal = pick(r.topLeft()) + smartMaxBefore;

        farMinVal = minVal;
        if (minBefore - collapsibleSizeBefore >= pick(r.size()) - maxAfter)
            farMinVal -= collapsibleSizeBefore;
        farMaxVal = maxVal;
        if (pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore)
            farMaxVal += collapsibleSizeAfter;
    } else {
        int hw = q->handleWidth();
        minVal = r.width() - smartMaxBefore - hw;
        maxVal = r.width() - smartMinBefore - hw;

        farMinVal = minVal;
        if (pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore)
            farMinVal -= collapsibleSizeAfter;
        farMaxVal = maxVal;
        if (minBefore - collapsibleSizeBefore >= pick(r.size()) - maxAfter)
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

int QSplitterPrivate::adjustPos(int pos, int id, int *farMin, int *min, int *max, int *farMax)
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

bool QSplitterPrivate::collapsible(QSplitterLayoutStruct *s)
{
    if (s->collapsible != Default) {
        return (bool)s->collapsible;
    } else {
        return childrenCollapsible;
    }
}

void QSplitterPrivate::updateHandles()
{
    int hw = q->handleWidth();
    for (int i = 0; i < list.size(); ++i) {
        QSplitterLayoutStruct *sl = list.at(i);
        if (sl->isHandle)
            sl->sizer = hw;
    }
    recalc(q->isVisible());
}

void QSplitterPrivate::setGeo(QSplitterLayoutStruct *sls, int p, int s, bool splitterMoved)
{
    QWidget *w = sls->wid;
    QRect r;
    QRect contents = q->contentsRect();
    if (orient == Qt::Horizontal) {
        if (q->isRightToLeft() && !splitterMoved)
            p = contents.width() - p - s;
        r.setRect(p, contents.y(), s, contents.height());
    } else {
        r.setRect(contents.x(), p, contents.width(), s);
    }
    sls->rect = r;
    sls->collapsed = false;

    /*
      Hide the child widget, but without calling hide() so that the
      splitter handle is still shown.
    */
    int minSize = pick(verySmartMinSize(w));
    if (!w->isHidden() && s <= 0 && minSize > 0) {
        sls->collapsed = minSize > 1;
        r.moveTopLeft(QPoint(-QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX));
    }
    w->setGeometry(r);
}

void QSplitterPrivate::doMove(bool backwards, int pos, int id, int delta, bool mayCollapse,
                              int *positions, int *widths)
{
    if (id < 0 || id >= list.count())
        return;

    QSplitterLayoutStruct *s = list.at(id);
    QWidget *w = s->wid;

    int nextId = backwards ? id - delta : id + delta;

    if (w->isHidden()) {
        doMove(backwards, pos, nextId, delta, true, positions, widths);
    } else {
        if (s->isHandle) {
            int dd = s->getSizer(d->orient);
            int nextPos = backwards ? pos - dd : pos + dd;
            positions[id] = backwards ? pos - dd : pos;
            widths[id] = dd;
            doMove(backwards, nextPos, nextId, delta, mayCollapse, positions, widths);
        } else {
            int dd = backwards ? pos - pick(s->rect.topLeft())
                               : pick(s->rect.bottomRight()) - pos + 1;
            if (dd > 0 || (!s->collapsed && !mayCollapse)) {
                dd = qMin(dd, pick(w->maximumSize()));
                dd = qMax(dd, pick(verySmartMinSize(w)));
            } else {
                dd = 0;
            }
            positions[id] = backwards ? pos - dd : pos;
            widths[id] = dd;
            doMove(backwards, backwards ? pos - dd : pos + dd, nextId, delta,
                   true, positions, widths);
        }
    }
}

void QSplitterPrivate::recalcId()
{
    int n = list.size();
    for (int i = 0; i < n; i++) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->isHandle)
            static_cast<QSplitterHandle *>(s->wid)->setId(i);
    }
}

QSplitterLayoutStruct *QSplitterPrivate::findWidget(QWidget *w)
{
    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i)->wid == w)
            return list.at(i);
    }
    return addWidget(w);
}

#ifdef QT_COMPAT
static void setStretch(QWidget *w, int sf)
{
    QSizePolicy sp = w->sizePolicy();
    sp.setHorizontalStretch(sf);
    sp.setVerticalStretch(sf);
    w->setSizePolicy(sp);
}

static int getStretch(const QWidget *w)
{
    QSizePolicy sp = w->sizePolicy();
    return qMax(sp.horizontalStretch(), sp.verticalStretch());
}

/*!
    This function tries to simulate the Qt 3.x ResizeMode behavior
    using QSizePolicy stretch factors. This isn't easy, because the
    default \c ResizeMode was \c Stretch, not \c KeepSize, whereas the
    default stetch factor is 0.

    So what we do is this: When the user calls setResizeMode() the first time,
    we iterate through all the child widgets and set their stretch factors to
    1. Later on, if children are added (using addWidget()), their stretch
    factors are also set to 1.

    There is just one problem left: Often, setResizeMode() is called
    \e{before} addWidget(), because addWidget() is called from the
    event loop. In that case, we use a special value, 243, instead of
    0 to prevent 0 from being overwritten with 1 in addWidget(). This
    is a wicked hack, but fortunately it only occurs as a result of
    calling a \c QT_COMPAT function.
*/
void QSplitter::setResizeMode(QWidget *w, ResizeMode mode)
{
    bool metWidget = false;
    if (!d->compatMode) {
        d->compatMode = true;
        for (int i = 0; i < d->list.size(); ++i) {
            QSplitterLayoutStruct *s = d->list.at(i);
            if (s->isHandle)
                continue;
            if (s->wid == w)
                metWidget = true;
            if (getStretch(s->wid) == 0)
                setStretch(s->wid, 1);
        }
    }
    int sf;
    if (mode == KeepSize)
        sf = metWidget ? 0 : 243;
    else
        sf = 1;
    setStretch(w, sf);
}
#endif

/*
    Inserts the widget \a w at the end (or at the beginning if \a
    prepend is true) of the splitter's list of widgets.

    It is the responsibility of the caller to make sure that \a w is
    not already in the splitter and to call recalcId() if needed. (If
    \a prepend is true, then recalcId() is very probably needed.)
*/

QSplitterLayoutStruct *QSplitterPrivate::addWidget(QWidget *w, bool prepend)
{
    QSplitterLayoutStruct *s;
    QSplitterHandle *newHandle = 0;
    if (list.count() > 0) {
        s = new QSplitterLayoutStruct;
        QString tmp = QLatin1String("qt_splithandle_");
        tmp += w->objectName();
        newHandle = new QSplitterHandle(orient, q);
        newHandle->setObjectName(tmp);
        s->wid = newHandle;
        newHandle->setId(list.size());
        s->isHandle = true;
        s->sizer = d->pick(newHandle->sizeHint());
        if (prepend)
            list.prepend(s);
        else
            list.append(s);
    }
    s = new QSplitterLayoutStruct;
    s->wid = w;
    s->isHandle = false;
    if (prepend)
        list.prepend(s);
    else
        list.append(s);
    if (newHandle && q->isVisible())
        newHandle->show(); // will trigger sending of post events

#ifdef QT_COMPAT
    if (compatMode) {
        int sf = getStretch(s->wid);
        if (sf == 243)
            setStretch(s->wid, 0);
        else if (sf == 0)
            setStretch(s->wid, 1);
    }
#endif
    return s;
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
    can use setOrientation(Qt::Vertical) to lay out the
    children vertically.

    By default, all widgets can be as large or as small as the user
    wishes, between the \l minimumSizeHint() (or \l minimumSize())
    and \l maximumSize() of the widgets. Use setResizeMode() to
    specify that a widget should keep its size when the splitter is
    resized, or set the stretch component of the \l sizePolicy.

    By default, QSplitter resizes the childre dynamically. If you
    would rather have QSplitter resize the children only at the end of
    a resize operation, call setOpaqueResize(false).

    The initial distribution of size between the widgets is determined
    by the initial size of each widget. You can also use setSizes() to
    set the sizes of all the widgets. The function sizes() returns the
    sizes set by the user.

    If you hide() a child its space will be distributed among the
    other children. It will be reinstated when you show() it again. It
    is also possible to reorder the widgets within the splitter using
    moveToFirst() and moveToLast().

    \inlineimage qsplitter-m.png Screenshot in Motif style
    \inlineimage qsplitter-w.png Screenshot in Windows style

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
    d->orient = Qt::Horizontal;
    d->init();
}


/*!
    Constructs a splitter with orientation \a o with the \a parent and
    \a name arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter(Qt::Orientation o, QWidget *parent, const char *name)
    : QFrame(*new QSplitterPrivate, parent)
{
    setObjectName(name);
    d->orient = o;
    d->init();
}


/*!
    Destroys the splitter and any children.
*/

QSplitter::~QSplitter()
{
    while (!d->list.isEmpty())
        delete d->list.takeFirst();
}

/*!
    Updates the splitter's state. You should not need to call this
    function.
*/
void QSplitter::refresh()
{
    d->recalc(true);
}

/*!
    \property QSplitter::orientation
    \brief the orientation of the splitter

    By default the orientation is horizontal (the widgets are side by
    side). The possible orientations are \c Qt::Horizontal and
    \c Qt::Vertical.
*/

void QSplitter::setOrientation(Qt::Orientation o)
{
    if (d->orient == o)
        return;

    if (!testWState(Qt::WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);
        clearWState(Qt::WState_OwnSizePolicy);
    }

    d->orient = o;

    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->isHandle)
            static_cast<QSplitterHandle *>(s->wid)->setOrientation(o);
    }
    d->recalc(isVisible());
}

Qt::Orientation QSplitter::orientation() const
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
    d->findWidget(w)->collapsible = collapse ? 1 : 0;
}

/*!
    \reimp
*/
void QSplitter::resizeEvent(QResizeEvent *)
{
    d->doResize();
}

/*!
    Tells the splitter that the child widget described by \a c has
    been inserted or removed.
*/

void QSplitter::childEvent(QChildEvent *c)
{
    if (c->type() == QEvent::ChildPolished) {
        if (!c->child()->isWidgetType())
            return;

        if (static_cast<QWidget *>(c->child())->testWFlags(Qt::WType_TopLevel))
            return;

        QList<QSplitterLayoutStruct *>::iterator it = d->list.begin();
        while (it != d->list.end()) {
            if ((*it)->wid == c->child())
                return;
            ++it;
        }
        d->addWidget(static_cast<QWidget *>(c->child()));
        d->recalc(isVisible());
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
                d->recalcId();
                d->doResize();
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
    if (p < 0) {
        if (d->rubberBand)
            d->rubberBand->hide();
        return;
    }
    QRect r = contentsRect();
    const int rBord = 3; // customizable?
    int hw = handleWidth();
    if (!d->rubberBand)
        d->rubberBand = new QRubberBand(QRubberBand::Line, this);
    if (d->orient == Qt::Horizontal)
        d->rubberBand->setGeometry(QRect(mapToGlobal(QPoint(p + hw / 2 - rBord, r.y())),
                                   QSize(2 * rBord, r.height())));
    else
        d->rubberBand->setGeometry(QRect(mapToGlobal(QPoint(r.x(), p + hw / 2 - rBord)),
                                   QSize(r.width(), 2 * rBord)));
    if (!d->rubberBand->isVisible())
        d->rubberBand->show();
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
    case QEvent::LayoutRequest:
#ifdef QT_COMPAT
    case QEvent::LayoutHint:
#endif
        d->recalc(isVisible());
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
void QSplitter::moveSplitter(int p, int id)
{
    QSplitterLayoutStruct *s = d->list.at(id);
    int farMin;
    int min;
    int max;
    int farMax;

    p = d->adjustPos(p, id, &farMin, &min, &max, &farMax);
    int oldP = d->pick(s->rect.topLeft());

    QVarLengthArray<int, 32> poss(d->list.count());
    QVarLengthArray<int, 32> ws(d->list.count());
    bool upLeft;

    if (isRightToLeft() && d->orient == Qt::Horizontal) {
        int qs = p + s->rect.width();
        d->doMove(false, qs, id - 1, -1, (p > max), poss.data(), ws.data());
        d->doMove(true, qs, id, -1, (p < min), poss.data(), ws.data());
        upLeft = (qs > oldP);
    } else {
        d->doMove(false, p, id, +1, (p > max), poss.data(), ws.data());
        d->doMove(true, p, id - 1, +1, (p < min), poss.data(), ws.data());
        upLeft = (p < oldP);
    }

    int wid, delta, count = d->list.count();
    if (upLeft) {
        wid = 0;
        delta = 1;
    } else {
        wid = count - 1;
        delta = -1;
    }
    for (; wid >= 0 && wid < count; wid += delta) {
        QSplitterLayoutStruct *sls = d->list.at( wid );
        if (!sls->wid->isHidden())
            d->setGeo(sls, poss[wid], ws[wid], true);
    }
    d->storeSizes();
}

/*!
    Returns the valid range of the splitter with ID \a id in
    \c{*}\a{min} and \c{*}\a{max} if \a min and \a max are not 0.

    \sa idAfter()
*/

void QSplitter::getRange(int id, int *min, int *max)
{
    d->getRange(id, min, 0, 0, max);
}


/*!
    Returns the closest legal position to \a pos of the widget with ID
    \a id.

    \sa idAfter()
*/

int QSplitter::adjustPos(int pos, int id)
{
    int x, i, n, u;
    return d->adjustPos(pos, id, &u, &n, &i, &x);
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
        d->addWidget(w, true);
    d->recalcId();
}


/*!
    Moves widget \a w to the rightmost/bottom position.
*/

void QSplitter::moveToLast(QWidget *w)
{
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
        d->addWidget(w);
    d->recalcId();
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
    return orientation() == Qt::Horizontal ? QSize(l, t) : QSize(t, l);
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
    return orientation() == Qt::Horizontal ? QSize(l, t) : QSize(t, l);
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

    The values in \a list should be the height or width (depending on
    orientation()) that the widgets should be resized to.

    \sa sizes()
*/

void QSplitter::setSizes(const QList<int> &list)
{
    int j = 0;

    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->isHandle)
            continue;

        s->collapsed = false;
        s->sizer = qMax(list.value(j++), 0);
        int smartMinSize = d->pick(verySmartMinSize(s->wid));

        // Make sure that we reset the collapsed state.
        if (s->sizer == 0) {
            if (d->collapsible(s) && smartMinSize > 0) {
                s->collapsed = true;
            } else {
                s->sizer = smartMinSize;
            }
        } else {
            if (s->sizer < smartMinSize)
                s->sizer = smartMinSize;
        }
    }
    d->doResize();
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
        return style()->pixelMetric(QStyle::PM_SplitterWidth, 0, this);
    }
}

void QSplitter::setHandleWidth(int width)
{
    d->handleWidth = width;
    d->updateHandles();
}

/*!
    \reimp
*/
void QSplitter::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        d->updateHandles();
    QFrame::changeEvent(ev);
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
                    splitter.d->setGeo(*it, 0, 0, false);
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
    splitter.d->doResize();
    return ts;
}
#endif

#endif

/*!
    \enum QSplitter::ResizeMode

    \compat

    \value Stretch
    \value KeepSize
    \value FollowSizeHint
    \value Auto

*/

