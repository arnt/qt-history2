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

#include "q3frame.h"
#include "qevent.h"
#include "qpainter.h"


/*! \class Q3Frame

    \compat

*/

Q3Frame::Q3Frame(QWidget* parent, const char* name, Qt::WFlags f)
    :QFrame(parent, f), marg(0)
{
    if (name)
        setObjectName(name);
    setAttribute(Qt::WA_LayoutOnEntireRect);
}

Q3Frame::~Q3Frame()
{
}

void Q3Frame::paintEvent(QPaintEvent * event)
{
    QPainter paint(this);
    if (!contentsRect().contains(event->rect())) {
        paint.save();
        paint.setClipRegion(event->region().intersect(frameRect()));
        drawFrame(&paint);
        paint.restore();
    }
    if (event->rect().intersects(contentsRect())) {
        paint.setClipRegion(event->region().intersect(contentsRect()));
        drawContents(&paint);
    }
}

/*!
    Virtual function that draws the contents of the frame.

    The QPainter is already open when you get it, and you must leave
    it open. Painter \link QPainter::setWorldMatrix()
    transformations\endlink are switched off on entry. If you
    transform the painter, remember to take the frame into account and
    \link QPainter::resetXForm() reset transformation\endlink before
    returning.

    This function is reimplemented by subclasses that draw something
    inside the frame. It should only draw inside contentsRect(). The
    default function does nothing.

    \sa contentsRect(), QPainter::setClipRect()
*/

void Q3Frame::drawContents(QPainter *)
{
}

/*!
    Draws the frame using the painter \a p and the current frame
    attributes and color group. The rectangle inside the frame is not
    affected.

    This function is virtual, but in general you do not need to
    reimplement it. If you do, note that the QPainter is already open
    and must remain open.

    \sa frameRect(), contentsRect(), drawContents(), frameStyle(), setPalette()
*/

void Q3Frame::drawFrame(QPainter *p)
{
    QFrame::drawFrame(p);
}

void Q3Frame::resizeEvent(QResizeEvent *)
{
    frameChanged();
}

/*!
    Virtual function that is called when the frame style, line width
    or mid-line width changes.

    This function can be reimplemented by subclasses that need to know
    when the frame attributes change.
*/

void Q3Frame::frameChanged()
{
}


/*!
    \property Q3Frame::margin
    \brief the width of the margin

    The margin is the distance between the innermost pixel of the
    frame and the outermost pixel of contentsRect(). It is included in
    frameWidth().

    The margin is filled according to backgroundMode().

    The default value is 0.

    \sa setMargin(), lineWidth(), frameWidth()
*/

void Q3Frame::setMargin(int w)
{
    if (marg == w)
        return;
    marg = w;
    update();
    frameChanged();
}

QRect Q3Frame::contentsRect() const
{
    QRect cr(QFrame::contentsRect());
    cr.addCoords(marg, marg, -marg, -marg);
    return cr;
}

int Q3Frame::frameWidth() const
{
    return QFrame::frameWidth() + marg;
}


