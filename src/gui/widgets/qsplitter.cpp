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
#include "private/qframe_p.h"
#include "private/qlayoutengine_p.h"

#include <ctype.h>

#define d d_func()
#define q q_func()

#include "qdebug.h"
//#define QSPLITTER_DEBUG


const uint Default = 2;
static int mouseOffset;

class QSplitterLayoutStruct
{
public:
    QRect rect;
    int sizer;
    uint collapsed : 1;
    uint collapsible : 2;
    QWidget *widget;
    QSplitterHandle *handle;

    QSplitterLayoutStruct() : sizer(-1), collapsed(false), collapsible(Default), widget(0), handle(0) {}
    ~QSplitterLayoutStruct() { delete handle; }
    int getWidgetSize(Qt::Orientation orient);
    int getHandleSize(Qt::Orientation orient);
    int pick(const QSize &size, Qt::Orientation orient)
    { return (orient == Qt::Horizontal) ? size.width() : size.height(); }
};

class QSplitterPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QSplitter)
public:
    QSplitterPrivate() : rubberBand(0), opaque(true), firstShow(true),
                         childrenCollapsible(true), compatMode(false), handleWidth(0), blockChildAdd(false) {}

    QPointer<QRubberBand> rubberBand;
    mutable QList<QSplitterLayoutStruct *> list;
    Qt::Orientation orient;
    bool opaque : 8;
    bool firstShow : 8;
    bool childrenCollapsible : 8;
    bool compatMode : 8;
    int handleWidth;
    bool blockChildAdd;

    inline int pick(const QPoint &p) const
    { return orient == Qt::Horizontal ? p.x() : p.y(); }
    inline int pick(const QSize &s) const
    { return orient == Qt::Horizontal ? s.width() : s.height(); }

    inline int trans(const QPoint &p) const
    { return orient == Qt::Vertical ? p.x() : p.y(); }
    inline int trans(const QSize &s) const
    { return orient == Qt::Vertical ? s.width() : s.height(); }

    void init();
    void recalc(bool update = false);
    void doResize();
    void storeSizes();
    void getRange(int index, int *, int *, int *, int *) const;
    void addContribution(int, int *, int *, bool) const;
    int adjustPos(int, int, int *, int *, int *, int *) const;
    bool collapsible(QSplitterLayoutStruct *) const;
    QSplitterLayoutStruct *findWidget(QWidget *) const;
    QSplitterLayoutStruct *insertWidget(int index, QWidget *);
    void doMove(bool backwards, int pos, int index, int delta,
                bool mayCollapse, int *positions, int *widths);
    void setGeo(QSplitterLayoutStruct *s, int pos, int size);
    int findWidgetJustBeforeOrJustAfter(int index, int delta, int &collapsibleSize) const;
    void updateHandles();

};

class QSplitterHandlePrivate : QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QSplitterHandle)
public:
    QSplitterHandlePrivate() : orient(Qt::Horizontal), opaq(false), s(0) {}

    inline int pick(const QPoint &p) const
    { return orient == Qt::Horizontal ? p.x() : p.y(); }

    Qt::Orientation orient;
    bool opaq;
    QSplitter *s;
};

/*!
    \class QSplitterHandle
    \brief The QSplitterHandle class provides handle functionality of the splitter.

    \ingroup organizers
    \mainclass

    QSplitterHandle is typically what people think about when they think about
    a splitter. It is the handle that is used to resize the widgets.

    A typical developer using QSplitter will never have to worry about QSplitterHandle.
    It is provided for developers who want splitter handles that do more. The typical
    way one would create splitter handles is to subclass QSplitter and then
    reimplement QSplitter::createHandle() to instatiate the custom splitter
    handle.

    Most of the functions inside QSplitterHandle are forwards to QSplitter or
    like orientation() and opaqueResize(), controlled by the QSplitter.
*/

/*!
    Create a QSplitter handle with orientation \a o and QSplitter \a parent for its parent.
*/
QSplitterHandle::QSplitterHandle(Qt::Orientation o, QSplitter *parent)
    : QWidget(*new QSplitterHandlePrivate, parent, 0)
{
    d->s = parent;
    setOrientation(o);
}

/*!
    Set the orientation of the splitter handle to \a o. This is usually
    propogated from the QSplitter.

    \sa QSplitter::setOrientation()
*/
void QSplitterHandle::setOrientation(Qt::Orientation o)
{
    d->orient = o;
#ifndef QT_NO_CURSOR
    setCursor(o == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
#endif
}

/*!
   Return the handle's orientation. This is usually propagated from the QSplitter.

   \sa QSplitter::orientation()
*/
Qt::Orientation QSplitterHandle::orientation() const
{
    return d->orient;
}


/*!

    Returns true if widgets are resized dynamically (opaquely), otherwise
    returns false. This value is controlled by the QSplitter

    \sa QSplitter::opaqueResize()

*/
bool QSplitterHandle::opaqueResize() const
{
    return d->s->opaqueResize();
}


/*!
  Returns the splitter associated with this splitter handle.

  \sa QSplitter
*/
QSplitter *QSplitterHandle::splitter() const
{
    return d->s;
}

/*!
    Tells the splitter to move this handle to position /a p, which is
    the distance from the left (or top) edge of the widget.

    Note that \a p is also measured from the left (or top) for
    right-to-left languages. This function will map \a p to the
    appropriate position before calling QSplitter::moveSplitter().

  \sa QSplitter::moveSplitter() closestLegalPosition()
*/
void QSplitterHandle::moveSplitter(int p)
{
    if (d->s->isRightToLeft() && d->orient == Qt::Horizontal)
        p = d->s->contentsRect().width() - p;
    d->s->moveSplitter(p, d->s->indexOfHandle(this));
}

/*!
   Returns the closest legal position to \a p of the splitter handle. The positions are
   measured from the left (or top) edge of the splitter, even for right-to-left languages.

   \sa QSplitter::closestLegalPosition(), moveSplitter()
*/

int QSplitterHandle::closestLegalPosition(int p)
{
    QSplitter *s = d->s;
    if (s->isRightToLeft() && d->orient == Qt::Horizontal) {
        int w = s->contentsRect().width();
        return w - s->closestLegalPosition(w - p, s->indexOfHandle(this));
    }
    return s->closestLegalPosition(p, s->indexOfHandle(this));
}

/*!
    \reimp
*/
QSize QSplitterHandle::sizeHint() const
{
    int hw = d->s->handleWidth();
    QStyleOption opt(0);
    opt.init(d->s);
    opt.state = QStyle::State_None;
    return parentWidget()->style()->sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw), d->s)
        .expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/
void QSplitterHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton))
        return;
    int pos = d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
                 - mouseOffset;
    if (opaqueResize()) {
        moveSplitter(pos);
    } else {
        d->s->setRubberband(closestLegalPosition(pos));
    }
}

/*!
   \reimp
*/
void QSplitterHandle::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        mouseOffset = d->pick(e->pos());
}

/*!
   \reimp
*/
void QSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
    if (!opaqueResize() && e->button() == Qt::LeftButton) {
        int pos = d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
                     - mouseOffset;
        d->s->setRubberband(-1);
        moveSplitter(pos);
    }
}

/*!
   \reimp
*/
void QSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    if (orientation() == Qt::Horizontal)
        opt.state = QStyle::State_Horizontal;
    else
        opt.state = QStyle::State_None;
    parentWidget()->style()->drawControl(QStyle::CE_Splitter, &opt, &p, d->s);
}


int QSplitterLayoutStruct::getWidgetSize(Qt::Orientation orient)
{
    if (sizer == -1) {
        QSize s = widget->sizeHint();
        const int presizer = pick(s, orient);
        const int realsize = pick(widget->size(), orient);
        if (!s.isValid() || (widget->testAttribute(Qt::WA_Resized) && (realsize > presizer))) {
            sizer = pick(widget->size(), orient);
        } else {
            sizer = presizer;
        }
        QSizePolicy p = widget->sizePolicy();
        int sf = (orient == Qt::Horizontal) ? p.horizontalStretch() : p.verticalStretch();
        if (sf > 1)
            sizer *= sf;
    }
    return sizer;
}

int QSplitterLayoutStruct::getHandleSize(Qt::Orientation orient)
{
    return pick(handle->sizeHint(), orient);
}

void QSplitterPrivate::init()
{
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
    if (orient == Qt::Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

void QSplitterPrivate::recalc(bool update)
{
    int n = list.count();
    /*
      Splitter handles before the first visible widget or right
      before a hidden widget must be hidden.
    */
    bool first = true;
    for (int i = 0; i < n ; ++i) {
        QSplitterLayoutStruct *s = list.at(i);
        s->handle->setHidden(first || s->widget->isHidden());
        if (!s->widget->isHidden())
            first = false;
    }

    int fi = 2 * q->frameWidth();
    int maxl = fi;
    int minl = fi;
    int maxt = QWIDGETSIZE_MAX;
    int mint = fi;
    /*
      calculate min/max sizes for the whole splitter
    */
    bool empty = true;
    for (int j = 0; j < n; j++) {
        QSplitterLayoutStruct *s = list.at(j);

        if (!s->widget->isHidden()) {
            empty = false;
            if (!s->handle->isHidden()) {
                minl += s->getHandleSize(orient);
                maxl += s->getHandleSize(orient);
            }

            QSize minS = qSmartMinSize(s->widget);
            minl += pick(minS);
            maxl += pick(s->widget->maximumSize());
            mint = qMax(mint, d->trans(minS));
            int tm = trans(s->widget->maximumSize());
            if (tm > 0)
                maxt = qMin(maxt, tm);
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
    QVector<QLayoutStruct> a(n*2);
    int i;

    bool noStretchFactorsSet = true;
    for (i = 0; i < n; ++i) {
        QSizePolicy p = list.at(i)->widget->sizePolicy();
        int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
        if (sf != 0) {
            noStretchFactorsSet = false;
            break;
        }
    }

    int j=0;
    for (i = 0; i < n; ++i) {
        QSplitterLayoutStruct *s = list.at(i);
#ifdef QSPLITTER_DEBUG
        qDebug("widget %d hidden: %d collapsed: %d handle hidden: %d", i, s->widget->isHidden(),
               s->collapsed, s->handle->isHidden());
#endif

        a[j].init();
        if (s->handle->isHidden()) {
            a[j].maximumSize = 0;
        } else {
            a[j].sizeHint = a[j].minimumSize = a[j].maximumSize = s->getHandleSize(orient);
            a[j].empty = false;
        }
        ++j;

        a[j].init();
        if (s->widget->isHidden() || s->collapsed) {
            a[j].maximumSize = 0;
        } else {
            a[j].minimumSize = pick(qSmartMinSize(s->widget));
            a[j].maximumSize = pick(s->widget->maximumSize());
            a[j].empty = false;

            bool stretch = noStretchFactorsSet;
            if (!stretch) {
                QSizePolicy p = s->widget->sizePolicy();
                int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
                stretch = (sf != 0);
            }
            if (stretch) {
                a[j].stretch = s->getWidgetSize(orient);
                a[j].sizeHint = a[i].minimumSize;
                a[j].expansive = true;
            } else {
                a[j].sizeHint = s->getWidgetSize(orient);
            }
        }
        ++j;
    }

    qGeomCalc(a, 0, n*2, pick(r.topLeft()), pick(r.size()), 0);

#ifdef QSPLITTER_DEBUG
    for (i = 0; i < n*2; ++i) {
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
        setGeo(s, a[i*2+1].pos, a[i*2+1].size);
    }
}

void QSplitterPrivate::storeSizes()
{
    for (int i = 0; i < list.size(); ++i) {
        QSplitterLayoutStruct *sls = list.at(i);
        sls->sizer = pick(sls->rect.size());
    }
}

void QSplitterPrivate::addContribution(int index, int *min, int *max, bool mayCollapse) const
{
    QSplitterLayoutStruct *s = list.at(index);
    if (!s->widget->isHidden()) {
        if (!s->handle->isHidden()) {
            *min += s->getHandleSize(orient);
            *max += s->getHandleSize(orient);
        }
        if (mayCollapse || !s->collapsed)
            *min += pick(qSmartMinSize(s->widget));

        *max += pick(s->widget->maximumSize());
    }
}

int QSplitterPrivate::findWidgetJustBeforeOrJustAfter(int index, int delta, int &collapsibleSize) const
{
    if (delta < 0)
        index += delta;
    do {
        QWidget *w = list.at(index)->widget;
        if (!w->isHidden()) {
            if (collapsible(list.at(index)))
                collapsibleSize = pick(qSmartMinSize(w));
            return index;
        }
        index += delta;
    } while (index >= 0 && index < list.count());

    return -1;
}

/*
  For the splitter handle with index \a index, \a min and \a max give the range without collapsing any widgets,
  and \a farMin and farMax give the range with collapsing included.
*/
void QSplitterPrivate::getRange(int index, int *farMin, int *min, int *max, int *farMax) const
{
    int n = list.count();
    if (index <= 0 || index >= n)
        return;

    int collapsibleSizeBefore = 0;
    int idJustBefore = findWidgetJustBeforeOrJustAfter(index, -1, collapsibleSizeBefore);

    int collapsibleSizeAfter = 0;
    int idJustAfter = findWidgetJustBeforeOrJustAfter(index, +1, collapsibleSizeAfter);

    int minBefore = 0;
    int minAfter = 0;
    int maxBefore = 0;
    int maxAfter = 0;
    int i;

    for (i = 0; i < index; ++i)
        addContribution(i, &minBefore, &maxBefore, i == idJustBefore);
    for (i = index; i < n; ++i)
        addContribution(i, &minAfter, &maxAfter, i == idJustAfter);

    QRect r = q->contentsRect();
    int farMinVal;
    int minVal;
    int maxVal;
    int farMaxVal;

    int smartMinBefore = qMax(minBefore, pick(r.size()) - maxAfter);
    int smartMaxBefore = qMin(maxBefore, pick(r.size()) - minAfter);

    minVal = pick(r.topLeft()) + smartMinBefore;
    maxVal = pick(r.topLeft()) + smartMaxBefore;

    farMinVal = minVal;
    if (minBefore - collapsibleSizeBefore >= pick(r.size()) - maxAfter)
        farMinVal -= collapsibleSizeBefore;
    farMaxVal = maxVal;
    if (pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore)
        farMaxVal += collapsibleSizeAfter;

    if (farMin)
        *farMin = farMinVal;
    if (min)
        *min = minVal;
    if (max)
        *max = maxVal;
    if (farMax)
        *farMax = farMaxVal;
}

int QSplitterPrivate::adjustPos(int pos, int index, int *farMin, int *min, int *max, int *farMax) const
{
    const int Threshold = 40;

    getRange(index, farMin, min, max, farMax);

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

bool QSplitterPrivate::collapsible(QSplitterLayoutStruct *s) const
{
    if (s->collapsible != Default) {
        return (bool)s->collapsible;
    } else {
        return childrenCollapsible;
    }
}

void QSplitterPrivate::updateHandles()
{
    recalc(q->isVisible());
}

void QSplitterPrivate::setGeo(QSplitterLayoutStruct *sls, int p, int s)
{
    QWidget *w = sls->widget;
    QRect r;
    QRect contents = q->contentsRect();
    if (orient == Qt::Horizontal) {
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
    int minSize = pick(qSmartMinSize(w));
    if (!w->isHidden() && s <= 0 && minSize > 0) {
        sls->collapsed = minSize > 1;
        r.moveTopLeft(QPoint(-QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX));
    }


    if (orient == Qt::Horizontal && q->isRightToLeft())
        r.moveRight(contents.width() - r.left());
    w->setGeometry(r);

    if (!sls->handle->isHidden()) {
        QSplitterHandle *h = sls->handle;
        QSize hs = h->sizeHint();
        if (orient==Qt::Horizontal) {
            if (q->isRightToLeft())
                p = contents.width() - p + hs.width();
            h->setGeometry(p-hs.width(), contents.y(), hs.width(), contents.height());
        } else {
            h->setGeometry(contents.x(), p-hs.height(), contents.width(), hs.height());
        }
    }
}

void QSplitterPrivate::doMove(bool backwards, int hPos, int index, int delta, bool mayCollapse,
                              int *positions, int *widths)
{
    if (index < 0 || index >= list.count())
        return;


#ifdef QSPLITTER_DEBUG
    qDebug() << "QSplitterPrivate::doMove" << backwards << hPos << index << delta << mayCollapse;
#endif



    QSplitterLayoutStruct *s = list.at(index);
    QWidget *w = s->widget;

    int nextId = backwards ? index - delta : index + delta;

    if (w->isHidden()) {
        doMove(backwards, hPos, nextId, delta, true, positions, widths);
    } else {
        int hs =s->handle->isHidden() ? 0 : s->getHandleSize(d->orient);

        int  ws = backwards ? hPos - pick(s->rect.topLeft())
                 : pick(s->rect.bottomRight()) - hPos -hs + 1;
        if (ws > 0 || (!s->collapsed && !mayCollapse)) {
            ws = qMin(ws, pick(w->maximumSize()));
            ws = qMax(ws, pick(qSmartMinSize(w)));
        } else {
            ws = 0;
        }
        positions[index] = backwards ? hPos - ws : hPos + hs;
        widths[index] = ws;
        doMove(backwards, backwards ? hPos - ws - hs : hPos + hs + ws, nextId, delta,
               true, positions, widths);
    }

}


QSplitterLayoutStruct *QSplitterPrivate::findWidget(QWidget *w) const
{
    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i)->widget == w)
            return list.at(i);
    }
    return 0;
}

#ifdef QT3_SUPPORT
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
    calling a \c QT3_SUPPORT function.
*/
void QSplitter::setResizeMode(QWidget *w, ResizeMode mode)
{
    bool metWidget = false;
    if (!d->compatMode) {
        d->compatMode = true;
        for (int i = 0; i < d->list.size(); ++i) {
            QSplitterLayoutStruct *s = d->list.at(i);
            if (s->widget == w)
                metWidget = true;
            if (getStretch(s->widget) == 0)
                setStretch(s->widget, 1);
        }
    }
    int sf;
    if (mode == KeepSize)
        sf = metWidget ? 0 : 243;
    else
        sf = 1;
    setStretch(w, sf);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSplitter::QSplitter(QWidget *parent, const char *name)
    : QFrame(*new QSplitterPrivate, parent)
{
    setObjectName(name);
    d->orient = Qt::Horizontal;
    d->init();
}


/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSplitter::QSplitter(Qt::Orientation o, QWidget *parent, const char *name)
    : QFrame(*new QSplitterPrivate, parent)
{
    setObjectName(name);
    d->orient = o;
    d->init();
}


#endif

/*
    Inserts the widget \a w at position \a index in the splitter's list of widgets.

    If \a w is already in the splitter, it will be moved to the new position.
*/

QSplitterLayoutStruct *QSplitterPrivate::insertWidget(int index, QWidget *w)
{
    QSplitterLayoutStruct *sls = 0;
    int i;
    for (i = 0; i < list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->widget == w) {
            sls = s;
            break;
        }
    }
    if (sls) {
        list.move(i,index);
    } else {
        QSplitterHandle *newHandle = 0;
        sls = new QSplitterLayoutStruct;
        QString tmp = QLatin1String("qt_splithandle_");
        tmp += w->objectName();
        newHandle = q->createHandle();
        newHandle->setObjectName(tmp);
        sls->handle = newHandle;
        sls->widget = w;
        list.insert(index,sls);

        if (newHandle && q->isVisible())
            newHandle->show(); // will trigger sending of post events

#ifdef QT3_SUPPORT
        if (compatMode) {
            int sf = getStretch(sls->widget);
            if (sf == 243)
                setStretch(sls->widget, 0);
            else if (sf == 0)
                setStretch(sls->widget, 1);
        }
#endif
    }
    return sls;
}

/*!
    \class QSplitter
    \brief The QSplitter class implements a splitter widget.

    \ingroup organizers
    \mainclass

    A splitter lets the user control the size of child widgets by dragging the
    boundary between the children. Any number of widgets may be controlled by a
    single splitter. The typical use of a QSplitter is to create several
    widgets and add them using insertWidget() or addWidget().

    The following example will show a QListView, QTreeView, and QTextEdit side by side:

    \quotefromfile ../../../doc/src/snippets/splitter/splitter.cpp
    \skipto  QSplitter
    \printuntil splitter->addWidget(textedit);

    If a widget is already inside a QSplitter when insertWidget() or
    addWidget() is called, it will move to the new position. This can be used
    to reorder widgets in the splitter later. You can use indexOf(),
    widget(), and count() to get access to the widgets inside the splitter.

    A default QSplitter lays out its children horizontally (side by side); you
    can use setOrientation(Qt::Vertical) to lay its
    children out vertically.

    By default, all widgets can be as large or as small as the user
    wishes, between the \l minimumSizeHint() (or \l minimumSize())
    and \l maximumSize() of the widgets.

    QSplitter resizes its children dynamically by default. If you
    would rather have QSplitter resize the children only at the end of
    a resize operation, call setOpaqueResize(false).

    The initial distribution of size between the widgets is determined by the
    initial size of each widget. You can also use setSizes() to set the sizes
    of all the widgets. The function sizes() returns the sizes set by the user.
    Alternatively, you can save and restore the sizes of the widgets from a
    QByteArray using saveState() and restoreState() respectively.

    When you hide() a child its space will be distributed among the
    other children. It will be reinstated when you show() it again.

    \inlineimage qsplitter-m.png Screenshot in Motif style
    \inlineimage qsplitter-w.png Screenshot in Windows style
    \inlineimage qsplitter-mac.png Screenshot in Mac style

    \sa QTabBar QSplitterHandle
*/


/*!
    Constructs a horizontal splitter with the \a parent
    arguments is passed on to the QFrame constructor.
*/
QSplitter::QSplitter(QWidget *parent)
    : QFrame(*new QSplitterPrivate, parent)
{
    d->orient = Qt::Horizontal;
    d->init();
}


/*!
    Constructs a splitter with orientation \a o. The \a parent argument is
    passed on to the QFrame constructor.
*/
QSplitter::QSplitter(Qt::Orientation o, QWidget *parent)
    : QFrame(*new QSplitterPrivate, parent)
{
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

    \sa QSplitterHandle::orientation()
*/

void QSplitter::setOrientation(Qt::Orientation o)
{
    if (d->orient == o)
        return;

    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);
        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }

    d->orient = o;

    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        s->handle->setOrientation(o);
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
    QSplitterLayoutStruct *s = d->findWidget(w);
    if (s)
        s->collapsible = collapse ? 1 : 0;
    else
        qWarning() << "QSplitter::setCollapsible() cannot find" << w << "in" << this;
}

/*!
    \reimp
*/
void QSplitter::resizeEvent(QResizeEvent *)
{
    d->doResize();
}

/*!
    Adds the given \a widget to the splitter's layout after all the other
    items.

    If \a widget is already in the splitter, it will be moved to the new position.

    \sa insertWidget() widget() indexOf()
*/
void QSplitter::addWidget(QWidget *widget)
{
    insertWidget(d->list.count(), widget);
}

/*!
    Inserts the \a widget specified into the splitter's layout at the
    given \a index.

    If \a widget is already in the splitter, it will be moved to the new position.

    \sa addWidget() indexOf() widget()
*/
void QSplitter::insertWidget(int index, QWidget *widget)
{
    QBoolBlocker b(d->blockChildAdd);
    if (widget->parentWidget() != this)
        widget->setParent(this);
    if (isVisible())
        widget->show();
    d->insertWidget(index, widget);
    d->recalc(isVisible());
}

/*!
    \fn int QSplitter::indexOf(QWidget *widget) const
    Returns the index in the splitter's layout of the specified \a widget.

    \sa widget()
*/
int QSplitter::indexOf(QWidget *w) const
{
    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->widget == w)
            return i;
    }
    return -1;
}

/*!
  Returns a new splitter handle as a child widget of this splitter.
  This function can be reimplemented in subclasses to provide support
  for custom handles.

  \sa QSplitterHandle
*/
QSplitterHandle *QSplitter::createHandle()
{
    return new QSplitterHandle(d->orient, this);
}

/*!
    Returns the index in the splitter's layout for the given \a handle.

    \sa handle()
*/
int QSplitter::indexOfHandle(QSplitterHandle *handle) const
{
    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        if (s->handle == handle)
            return i;
    }
    return -1;
}

/*!
    Returns the handle to the right (or below) for the item in the splitter's layout at the
    given \a index.

    For Arabic, Hebrew and other right-to-left languages the layout is
    reversed. The handle will be to the left of the widget at \a index.
    \sa widget()
*/
QSplitterHandle *QSplitter::handle(int index) const
{
    if (index < 0 || index >= d->list.size())
        return 0;
    return d->list.at(index)->handle;
}

/*!
    Returns the widget at the given \a index in the splitter's layout.

    \sa handle() indexOf() insertWidget()
*/
QWidget *QSplitter::widget(int index) const
{
    if (index < 0 || index >= d->list.size())
        return 0;
    return d->list.at(index)->widget;
}

/*!
    Returns the number of widgets contained in the splitter's layout.
*/
int QSplitter::count() const
{
    return d->list.count();
}

/*!
    \reimp

    Tells the splitter that the child widget described by \a c has been
    inserted or removed.

    This method is also used to handle the situation where a widget is created
    with the splitter as a parent but not explicitly added with insertWidget()
    or addWidget(). This is for compatiblity and not the recommended way of
    putting widgets into a splitter in new code. Please use insertWidget() or
    addWidget() in new code.

    \sa addWidget() insertWidget()
*/

void QSplitter::childEvent(QChildEvent *c)
{
    if (!c->child()->isWidgetType())
        return;
    QWidget *w = static_cast<QWidget*>(c->child());

    if (c->added() && !d->blockChildAdd && !w->isTopLevel() && !d->findWidget(w)) {
        addWidget(w);
    } else  if (c->type() == QEvent::ChildRemoved) {
        for (int i = 0; i < d->list.size(); ++i) {
            QSplitterLayoutStruct *s = d->list.at(i);
            if (s->widget == w) {
                d->list.removeAt(i);
                delete s;
                d->doResize();
                return;
            }
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
#ifdef QT3_SUPPORT
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
  \fn QSplitter::splitterMoved(int position, int index)

  This signal is emitted when the splitter handle at a particular \a index
  has been moved to the given \a position .

  For Arabic, Hebrew and other right-to-left languages the layout is
  reversed. The \a position is then the distance from the right edge
  of the widget (or top edge for vertical splitters).

  \sa moveSplitter()
*/


/*!
    Moves the left/top edge of the splitter handle at \a index as
    close as possible to position \a p, which is the distance from the
    left (or top) edge of the widget.

    For Arabic, Hebrew and other right-to-left languages the layout is
    reversed.  \a p is then the distance from the right (or top) edge
    of the widget.

    \sa splitterMoved()
*/
void QSplitter::moveSplitter(int p, int index)
{
    QSplitterLayoutStruct *s = d->list.at(index);
    int farMin;
    int min;
    int max;
    int farMax;

#ifdef QSPLITTER_DEBUG
    int debugp = p;
#endif

    p = d->adjustPos(p, index, &farMin, &min, &max, &farMax);
    int oldP = d->pick(s->rect.topLeft());
#ifdef QSPLITTER_DEBUG
    qDebug() << "QSplitter::moveSplitter" << debugp << index << "adjusted" << p << "oldP" << oldP;
#endif

    QVarLengthArray<int, 32> poss(d->list.count());
    QVarLengthArray<int, 32> ws(d->list.count());
    bool upLeft;

    d->doMove(false, p, index, +1, (p > max), poss.data(), ws.data());
    d->doMove(true, p, index - 1, +1, (p < min), poss.data(), ws.data());
    upLeft = (p < oldP);

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
        if (!sls->widget->isHidden())
            d->setGeo(sls, poss[wid], ws[wid]);
    }
    d->storeSizes();

    emit splitterMoved(p, index);
}


/*!
    Returns the valid range of the splitter with index \a index in
    \c{*}\a{min} and \c{*}\a{max} if \a min and \a max are not 0.
*/

void QSplitter::getRange(int index, int *min, int *max) const
{
    d->getRange(index, min, 0, 0, max);
}


/*!
    Returns the closest legal position to \a pos of the widget with index
    \a index.

    For Arabic, Hebrew and other right-to-left languages the layout is
    reversed.  Positions are then measured from the right (or top) edge
    of the widget.
*/

int QSplitter::closestLegalPosition(int pos, int index)
{
    int x, i, n, u;
    return d->adjustPos(pos, index, &u, &n, &i, &x);
}

/*!
    \property QSplitter::opaqueResize
    \brief whether resizing is opaque

    Opaque resizing is on by default.
*/

bool QSplitter::opaqueResize() const
{
    return d->opaque;
}


void QSplitter::setOpaqueResize(bool on)
{
    d->opaque = on;
}

#ifdef QT3_SUPPORT
/*!
\fn void QSplitter::moveToFirst(QWidget *w)

    Moves widget \a w to the leftmost/top position.
*/


/*!
\fn void QSplitter::moveToLast(QWidget *w)

    Moves widget \a w to the rightmost/bottom position.
*/

#endif

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
            QSize s = qSmartMinSize(static_cast<QWidget *>(o));
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

    The easiest way to iterate over the list is to use the Java-style iterators.

    \quotefromfile ../../../doc/src/snippets/splitter/splitter.cpp
    \skipto   QListIterator<int>
    \printuntil processSize(it.next());

    \sa setSizes()
*/

QList<int> QSplitter::sizes() const
{
    ensurePolished();

    QList<int> list;
    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);
        list.append(d->pick(s->rect.size()));
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

        s->collapsed = false;
        s->sizer = qMax(list.value(j++), 0);
        int smartMinSize = d->pick(qSmartMinSize(s->widget));

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




/*!
    Saves the state of the splitter's layout.

    Typically this is used in conjunction with QSettings to remember the size
    for a future session. Here is an example:

    \quotefromfile ../../../doc/src/snippets/splitter/splitter.cpp
    \skipto  QSettings
    \printuntil  settings.setValue("splitterSizes", splitter->saveState());
*/
QByteArray QSplitter::saveState() const
{
    QByteArray b;

    bool first = true;
    b.append('[');

    for (int i = 0; i < d->list.size(); ++i) {
        QSplitterLayoutStruct *s = d->list.at(i);

        if (!first)
            b.append(',');

        if (s->widget->isHidden()) {
            b.append('H');
        } else {
            b.append(QByteArray::number(d->pick(s->rect.size())));
        }
        first = false;

    }
    b.append(']');
    return b;
}

/*!
    Restores the splitter's layout to the \a state specified.

    Typically this is used in conjunction with QSettings to restore the size
    from a past session. Here is an example:

    Restore the splitters's state:
    \quotefromfile ../../../doc/src/snippets/splitter/splitter.cpp
    \skipto  QSettings restoreSettings
    \printuntil splitter->restoreState(restoreSettings.value("splitterSizes").toByteArray());

*/
bool QSplitter::restoreState(const QByteArray &state)
{
    int index = 0;
    int i = 0;
    int n = d->list.count();

    if (i < state.size() && state.at(i) == '[') {
        ++i;
        while (i < state.size() && state.at(i) != ']') {
            if (index == n)
                break;
            QSplitterLayoutStruct *s = d->list.at(index);
            if (state.at(i) == 'H') {
                s->widget->hide();
                ++i;
            } else {
                s->widget->show();
                int dim = 0;
                while (i < state.size() && isdigit(state.at(i))) {
                    dim *= 10;
                    dim += state.at(i) - '0';
                    ++i;
                }
                s->sizer = dim;
                if (dim == 0)
                    d->setGeo(s, 0, 0);
            }
            if (state.at(i) == ',') {
                ++i;
            } else {
                break;
            }
            ++index;
        }
        d->doResize();
        return (state.at(i) == ']');
    }
    return false;
}


//#ifdef QT3_SUPPORT
#ifndef QT_NO_TEXTSTREAM
/*!
    \relates QSplitter

    Writes the sizes and the hidden state of the widgets in the
    splitter \a splitter to the text stream \a ts.

    \sa operator>>(), sizes(), QWidget::isHidden()
*/

QTextStream& operator<<(QTextStream& ts, const QSplitter& splitter)
{
    ts << splitter.saveState() << endl;
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
    QString line = ts.readLine();
    line = line.simplified();
    line.replace(' ', QString());
    line = line.toUpper();

    splitter.restoreState(line.toAscii());
    return ts;
}
#endif // QT_NO_TEXTSTREAM
//#endif // QT3_SUPPORT
#endif // QT_NO_SPLITTER

/*!
    \enum QSplitter::ResizeMode

    \compat

    \value Stretch
    \value KeepSize
    \value FollowSizeHint
    \value Auto

*/

