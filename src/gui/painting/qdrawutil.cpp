/****************************************************************************
**
** Implementation of draw utilities.
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

#include "qdrawutil.h"
#ifndef QT_NO_DRAWUTIL
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"

/*!
    \relates QPainter

    \c{#include <qdrawutil.h>}

    Draws a horizontal (\a y1 == \a y2) or vertical (\a x1 == \a x2)
    shaded line using the painter \a p.

    Nothing is drawn if \a y1 != \a y2 and \a x1 != \a x2 (i.e. the
    line is neither horizontal nor vertical).

    The palette \a pal specifies the shading colors (\link
    QPalette::light() light\endlink, \link QPalette::dark()
    dark\endlink and \link QPalette::mid() middle\endlink colors).

    The line appears sunken if \a sunken is true, or raised if \a
    sunken is false.

    The \a lineWidth argument specifies the line width for each of the
    lines. It is not the total line width.

    The \a midLineWidth argument specifies the width of a middle line
    drawn in the QPalette::mid() color.

    If you want to use a QFrame widget instead, you can make it
    display a shaded line, for example \c{QFrame::setFrameStyle(
    QFrame::HLine | QFrame::Sunken)}.

    \warning This function does not look at QWidget::style() or
    QApplication::style(). Use the drawing functions in QStyle to make
    widgets that follow the current GUI style.

    \sa qDrawShadeRect(), qDrawShadePanel(), QStyle::drawPrimitive()
*/

void qDrawShadeLine(QPainter *p, int x1, int y1, int x2, int y2,
                     const QPalette &pal, bool sunken,
                     int lineWidth, int midLineWidth)
{
    if (!(p && lineWidth >= 0 && midLineWidth >= 0))        {
        qWarning("qDrawShadeLine invalid parameters.");
        return;
    }
    int tlw = lineWidth*2 + midLineWidth;        // total line width
    QPen oldPen = p->pen();                        // save pen
    if (sunken)
        p->setPen(pal.color(QPalette::Dark));
    else
        p->setPen(pal.light());
    QPointArray a;
    int i;
    if (y1 == y2) {                                // horizontal line
        int y = y1 - tlw/2;
        if (x1 > x2) {                        // swap x1 and x2
            int t = x1;
            x1 = x2;
            x2 = t;
        }
        x2--;
        for (i=0; i<lineWidth; i++) {                // draw top shadow
            a.setPoints(3, x1+i, y+tlw-1-i,
                         x1+i, y+i,
                         x2-i, y+i);
            p->drawPolyline(a);
        }
        if (midLineWidth > 0) {
            p->setPen(pal.mid());
            for (i=0; i<midLineWidth; i++)        // draw lines in the middle
                p->drawLine(x1+lineWidth, y+lineWidth+i,
                             x2-lineWidth, y+lineWidth+i);
        }
        if (sunken)
            p->setPen(pal.light());
        else
            p->setPen(pal.dark());
        for (i=0; i<lineWidth; i++) {                // draw bottom shadow
            a.setPoints(3, x1+i, y+tlw-i-1,
                         x2-i, y+tlw-i-1,
                         x2-i, y+i+1);
            p->drawPolyline(a);
        }
    }
    else if (x1 == x2) {                        // vertical line
        int x = x1 - tlw/2;
        if (y1 > y2) {                        // swap y1 and y2
            int t = y1;
            y1 = y2;
            y2 = t;
        }
        y2--;
        for (i=0; i<lineWidth; i++) {                // draw left shadow
            a.setPoints(3, x+i, y2,
                         x+i, y1+i,
                         x+tlw-1, y1+i);
            p->drawPolyline(a);
        }
        if (midLineWidth > 0) {
            p->setPen(pal.mid());
            for (i=0; i<midLineWidth; i++)        // draw lines in the middle
                p->drawLine(x+lineWidth+i, y1+lineWidth, x+lineWidth+i, y2);
        }
        if (sunken)
            p->setPen(pal.light());
        else
            p->setPen(pal.dark());
        for (i=0; i<lineWidth; i++) {                // draw right shadow
            a.setPoints(3, x+lineWidth, y2-i,
                         x+tlw-i-1, y2-i,
                         x+tlw-i-1, y1+lineWidth);
            p->drawPolyline(a);
        }
    }
    p->setPen(oldPen);
}


/*!
    \relates QPainter

    \c{#include <qdrawutil.h>}

    Draws the shaded rectangle specified by (\a x, \a y, \a w, \a h)
    using the painter \a p.

    The paletted \a pal specifies the shading colors (\link
    QPalette::light() light\endlink, \link QPalette::dark()
    dark\endlink and \link QPalette::mid() middle\endlink colors).

    The rectangle appears sunken if \a sunken is true, or raised if \a
    sunken is false.

    The \a lineWidth argument specifies the line width for each of the
    lines. It is not the total line width.

    The \a midLineWidth argument specifies the width of a middle line
    drawn in the QPalette::mid() color.

    The rectangle's interior is filled with the \a fill brush unless
    \a fill is 0.

    If you want to use a QFrame widget instead, you can make it
    display a shaded rectangle, for example \c{QFrame::setFrameStyle(
    QFrame::Box | QFrame::Raised)}.

    \warning This function does not look at QWidget::style() or
    QApplication::style(). Use the drawing functions in QStyle to make
    widgets that follow the current GUI style.

    \sa qDrawShadeLine(), qDrawShadePanel(), qDrawPlainRect(),
    QStyle::drawItem(), QStyle::drawControl()
    QStyle::drawComplexControl()
*/

void qDrawShadeRect(QPainter *p, int x, int y, int w, int h,
                     const QPalette &pal, bool sunken,
                     int lineWidth, int midLineWidth,
                     const QBrush *fill)
{
    if (w == 0 || h == 0)
        return;
    if (! (w > 0 && h > 0 && lineWidth >= 0 && midLineWidth >= 0)) {
        qWarning("qDrawShadeRect(): Invalid parameters");
        return;
    }
    QPen oldPen = p->pen();
    if (sunken)
        p->setPen(pal.dark());
    else
        p->setPen(pal.light());
    int x1=x, y1=y, x2=x+w-1, y2=y+h-1;
    QPointArray a;

    if (lineWidth == 1 && midLineWidth == 0) {// standard shade rectangle
        p->drawRect(x1, y1, w-1, h-1);
        if (sunken)
            p->setPen(pal.light());
        else
            p->setPen(pal.dark());
        a.setPoints(8, x1+1,y1+1, x2-2,y1+1, x1+1,y1+2, x1+1,y2-2,
                     x1,y2, x2,y2,  x2,y1, x2,y2-1);
        p->drawLineSegments(a);                // draw bottom/right lines
    } else {                                        // more complicated
        int m = lineWidth+midLineWidth;
        int i, j=0, k=m;
        for (i=0; i<lineWidth; i++) {                // draw top shadow
            a.setPoints(8, x1+i, y2-i, x1+i, y1+i, x1+i, y1+i, x2-i, y1+i,
                         x1+k, y2-k, x2-k, y2-k, x2-k, y2-k, x2-k, y1+k);
            p->drawLineSegments(a);
            k++;
        }
        p->setPen(pal.mid());
        j = lineWidth*2;
        for (i=0; i<midLineWidth; i++) {        // draw lines in the middle
            p->drawRect(x1+lineWidth+i, y1+lineWidth+i, w-j, h-j);
            j += 2;
        }
        if (sunken)
            p->setPen(pal.light());
        else
            p->setPen(pal.dark());
        k = m;
        for (i=0; i<lineWidth; i++) {                // draw bottom shadow
            a.setPoints(8, x1+1+i,y2-i, x2-i, y2-i, x2-i, y2-i, x2-i, y1+i+1,
                         x1+k, y2-k, x1+k, y1+k, x1+k, y1+k, x2-k, y1+k);
            p->drawLineSegments(a);
            k++;
        }
    }
    if (fill) {
        QBrush oldBrush = p->brush();
        int tlw = lineWidth + midLineWidth;
        p->setPen(Qt::NoPen);
        p->setBrush(*fill);
        p->drawRect(x+tlw, y+tlw, w-2*tlw, h-2*tlw);
        p->setBrush(oldBrush);
    }
    p->setPen(oldPen);                        // restore pen
}


/*!
    \relates QPainter

    \c{#include <qdrawutil.h>}

    Draws the shaded panel specified by (\a x, \a y, \a w, \a h) using
    the painter \a p.

    The palette \a pal specifies the shading colors (\link
    QPalette::light() light\endlink, \link QPalette::dark()
    dark\endlink and \link QPalette::mid() middle\endlink colors).

    The panel appears sunken if \a sunken is true, or raised if \a
    sunken is false.

    The \a lineWidth argument specifies the line width.

    The panel's interior is filled with the \a fill brush unless \a
    fill is 0.

    If you want to use a QFrame widget instead, you can make it
    display a shaded panel, for example \c{QFrame::setFrameStyle(
    QFrame::Panel | QFrame::Sunken)}.

    \warning This function does not look at QWidget::style() or
    QApplication::style(). Use the drawing functions in QStyle to make
    widgets that follow the current GUI style.

    \sa qDrawWinPanel(), qDrawShadeLine(), qDrawShadeRect(),
    QStyle::drawPrimitive()
*/

void qDrawShadePanel(QPainter *p, int x, int y, int w, int h,
                      const QPalette &pal, bool sunken,
                      int lineWidth, const QBrush *fill)
{
    if (w == 0 || h == 0)
        return;
    if (!(w > 0 && h > 0 && lineWidth >= 0)) {
        qWarning("qDrawShadePanel() Invalid parameters.");
    }
    QColor shade = pal.dark();
    QColor light = pal.light();
    if (fill) {
        if (fill->color() == shade)
            shade = pal.shadow();
        if (fill->color() == light)
            light = pal.midlight();
    }
    QPen oldPen = p->pen();                        // save pen
    QPointArray a(4*lineWidth);
    if (sunken)
        p->setPen(shade);
    else
        p->setPen(light);
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for (i=0; i<lineWidth; i++) {                // top shadow
        a.setPoint(n++, x1, y1++);
        a.setPoint(n++, x2--, y2++);
    }
    x2 = x1;
    y1 = y+h-2;
    for (i=0; i<lineWidth; i++) {                // left shadow
        a.setPoint(n++, x1++, y1);
        a.setPoint(n++, x2++, y2--);
    }
    p->drawLineSegments(a);
    n = 0;
    if (sunken)
        p->setPen(light);
    else
        p->setPen(shade);
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for (i=0; i<lineWidth; i++) {                // bottom shadow
        a.setPoint(n++, x1++, y1--);
        a.setPoint(n++, x2, y2--);
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for (i=0; i<lineWidth; i++) {                // right shadow
        a.setPoint(n++, x1--, y1++);
        a.setPoint(n++, x2--, y2);
    }
    p->drawLineSegments(a);
    if (fill) {                                // fill with fill color
        QBrush oldBrush = p->brush();
        p->setPen(Qt::NoPen);
        p->setBrush(*fill);
        p->drawRect(x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2);
        p->setBrush(oldBrush);
    }
    p->setPen(oldPen);                        // restore pen
}


/*!
  \internal
  This function draws a rectangle with two pixel line width.
  It is called from qDrawWinButton() and qDrawWinPanel().

  c1..c4 and fill are used:

    1 1 1 1 1 2
    1 3 3 3 4 2
    1 3 F F 4 2
    1 3 F F 4 2
    1 4 4 4 4 2
    2 2 2 2 2 2
*/

static void qDrawWinShades(QPainter *p,
                           int x, int y, int w, int h,
                           const QColor &c1, const QColor &c2,
                           const QColor &c3, const QColor &c4,
                           const QBrush *fill)
{
    if (w < 2 || h < 2)                        // can't do anything with that
        return;
    QPen oldPen = p->pen();
    QPointArray a(3);
    a.setPoints(3, x, y+h-2, x, y, x+w-2, y);
    p->setPen(c1);
    p->drawPolyline(a);
    a.setPoints(3, x, y+h-1, x+w-1, y+h-1, x+w-1, y);
    p->setPen(c2);
    p->drawPolyline(a);
    if (w > 4 && h > 4) {
        a.setPoints(3, x+1, y+h-3, x+1, y+1, x+w-3, y+1);
        p->setPen(c3);
        p->drawPolyline(a);
        a.setPoints(3, x+1, y+h-2, x+w-2, y+h-2, x+w-2, y+1);
        p->setPen(c4);
        p->drawPolyline(a);
        if (fill) {
            QBrush oldBrush = p->brush();
            p->setBrush(*fill);
            p->setPen(Qt::NoPen);
            p->drawRect(x+2, y+2, w-4, h-4);
            p->setBrush(oldBrush);
        }
    }
    p->setPen(oldPen);
}


/*!
    \relates QPainter

    \c{#include <qdrawutil.h>}

    Draws the Windows-style button specified by (\a x, \a y, \a w, \a
    h) using the painter \a p.

    The palette \a pal specifies the shading colors (\link
    QPalette::light() light\endlink, \link QPalette::dark()
    dark\endlink and \link QPalette::mid() middle\endlink colors).

    The button appears sunken if \a sunken is true, or raised if \a
    sunken is false.

    The line width is 2 pixels.

    The button's interior is filled with the \a *fill brush unless \a
    fill is 0.

    \warning This function does not look at QWidget::style() or
    QApplication::style(). Use the drawing functions in QStyle to make
    widgets that follow the current GUI style.

    \sa qDrawWinPanel(), QStyle::drawControl()
*/

void qDrawWinButton(QPainter *p, int x, int y, int w, int h,
                     const QPalette &pal, bool sunken,
                     const QBrush *fill)
{
    if (sunken)
        qDrawWinShades(p, x, y, w, h,
                       pal.shadow(), pal.light(), pal.dark(), pal.button(), fill);
    else
        qDrawWinShades(p, x, y, w, h,
                       pal.light(), pal.shadow(), pal.button(), pal.dark(), fill);
}

/*!
    \relates QPainter

    \c{#include <qdrawutil.h>}

    Draws the Windows-style panel specified by (\a x, \a y, \a w, \a
    h) using the painter \a p.

    The palette \a pal specifies the shading colors.

    The panel appears sunken if \a sunken is true, or raised if \a
    sunken is false.

    The line width is 2 pixels.

    The button's interior is filled with the \a fill brush unless \a
    fill is 0.

    If you want to use a QFrame widget instead, you can make it
    display a shaded panel, for example \c{QFrame::setFrameStyle(
    QFrame::WinPanel | QFrame::Raised)}.

    \warning This function does not look at QWidget::style() or
    QApplication::style(). Use the drawing functions in QStyle to make
    widgets that follow the current GUI style.

    \sa qDrawShadePanel(), qDrawWinButton(), QStyle::drawPrimitive()
*/

void qDrawWinPanel(QPainter *p, int x, int y, int w, int h,
                    const QPalette &pal, bool        sunken,
                    const QBrush *fill)
{
    if (sunken)
        qDrawWinShades(p, x, y, w, h,
                        pal.dark(), pal.light(), pal.shadow(), pal.midlight(), fill);
    else
        qDrawWinShades(p, x, y, w, h,
                        pal.light(), pal.shadow(), pal.midlight(), pal.dark(), fill);
}


/*!
    \relates QPainter

    \c{#include <qdrawutil.h>}

    Draws the plain rectangle specified by (\a x, \a y, \a w, \a h)
    using the painter \a p.

    The color argument \a c specifies the line color.

    The \a lineWidth argument specifies the line width.

    The rectangle's interior is filled with the \a fill brush unless
    \a fill is 0.

    If you want to use a QFrame widget instead, you can make it
    display a plain rectangle, for example \c{QFrame::setFrameStyle(
    QFrame::Box | QFrame::Plain)}.

    \warning This function does not look at QWidget::style() or
    QApplication::style(). Use the drawing functions in QStyle to make
    widgets that follow the current GUI style.

    \sa qDrawShadeRect(), QStyle::drawPrimitive()
*/

void qDrawPlainRect(QPainter *p, int x, int y, int w, int h, const QColor &c,
                     int lineWidth, const QBrush *fill)
{
    if (w == 0 || h == 0)
        return;
    if (!(w > 0 && h > 0 && lineWidth >= 0)) {
        qWarning("qDrawPlainRect() Invalid parameters.");
    }
    QPen   oldPen   = p->pen();
    QBrush oldBrush = p->brush();
    p->setPen(c);
    p->setBrush(Qt::NoBrush);
    for (int i=0; i<lineWidth; i++)
        p->drawRect(x+i, y+i, w-i*2, h-i*2);
    if (fill) {                                // fill with fill color
        p->setPen(Qt::NoPen);
        p->setBrush(*fill);
        p->drawRect(x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2);
    }
    p->setPen(oldPen);
    p->setBrush(oldBrush);
}


QRect qItemRect(QPainter *p, Qt::GUIStyle gs,
                int x, int y, int w, int h,
                int flags,
                bool enabled,
                const QPixmap *pixmap,
                const QString& text, int len)
{
    QRect result;

    if (pixmap) {
        if ((flags & Qt::AlignVCenter) == Qt::AlignVCenter)
            y += h/2 - pixmap->height()/2;
        else if ((flags & Qt::AlignBottom) == Qt::AlignBottom)
            y += h - pixmap->height();
        if ((flags & Qt::AlignRight) == Qt::AlignRight)
            x += w - pixmap->width();
        else if ((flags & Qt::AlignHCenter) == Qt::AlignHCenter)
            x += w/2 - pixmap->width()/2;
        else if ((flags & Qt::AlignLeft) != Qt::AlignLeft && QApplication::reverseLayout())
            x += w - pixmap->width();
        result = QRect(x, y, pixmap->width(), pixmap->height());
    } else if (!text.isNull() && p) {
        result = p->boundingRect(x, y, w, h, flags, text, len);
        if (gs == Qt::WindowsStyle && !enabled) {
            result.setWidth(result.width()+1);
            result.setHeight(result.height()+1);
        }
    } else {
        result = QRect(x, y, w, h);
    }

    return result;
}


void qDrawItem(QPainter *p, Qt::GUIStyle gs,
                int x, int y, int w, int h,
                int flags,
                const QPalette &pal, bool enabled,
                const QPixmap *pixmap,
                const QString& text, int len , const QColor* penColor)
{
    p->setPen(penColor?*penColor:pal.foreground().color());
    if (pixmap) {
        QPixmap  pm(*pixmap);
        bool clip = (flags & Qt::TextDontClip) == 0;
        if (clip) {
            if (pm.width() < w && pm.height() < h)
                clip = false;
            else
                p->setClipRect(x, y, w, h);
        }
        if ((flags & Qt::AlignVCenter) == Qt::AlignVCenter)
            y += h/2 - pm.height()/2;
        else if ((flags & Qt::AlignBottom) == Qt::AlignBottom)
            y += h - pm.height();
        if ((flags & Qt::AlignRight) == Qt::AlignRight)
            x += w - pm.width();
        else if ((flags & Qt::AlignHCenter) == Qt::AlignHCenter)
            x += w/2 - pm.width()/2;
        else if (((flags & Qt::AlignLeft) != Qt::AlignLeft) && QApplication::reverseLayout()) // Qt::AlignAuto && rightToLeft
            x += w - pm.width();

        if (!enabled) {
            if (pm.mask()) {                        // pixmap with a mask
                if (!pm.selfMask()) {                // mask is not pixmap itself
                    QPixmap pmm(*pm.mask());
                    pmm.setMask(*((QBitmap *)&pmm));
                    pm = pmm;
                }
            } else if (pm.depth() == 1) {        // monochrome pixmap, no mask
                pm.setMask(*((QBitmap *)&pm));
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
            } else {                                // color pixmap, no mask
                QString k;
                k.sprintf("$qt-drawitem-%x", pm.serialNumber());
                if (!QPixmapCache::find(k, pm)) {
                    pm = pm.createHeuristicMask();
                    pm.setMask((QBitmap&)pm);
                    QPixmapCache::insert(k, pm);
                }
#endif
            }
            if (gs == Qt::WindowsStyle) {
                p->setPen(pal.light());
                p->drawPixmap(x+1, y+1, pm);
                p->setPen(pal.text());
            }
        }
        p->drawPixmap(x, y, pm);
        if (clip)
            p->setClipping(false);
    } else if (!text.isNull()) {
        if (gs == Qt::WindowsStyle && !enabled) {
            p->setPen(pal.light());
            p->drawText(x+1, y+1, w, h, flags, text, len);
            p->setPen(pal.text());
        }
        p->drawText(x, y, w, h, flags, text, len);
    }
}


/*****************************************************************************
  Overloaded functions.
 *****************************************************************************/

/*!
    \fn void qDrawShadeLine(QPainter *p, const QPoint &p1, const QPoint &p2,
    const QPalette &pal, bool sunken, int lineWidth, int midLineWidth)

    \overload
*/

void qDrawShadeLine(QPainter *p, const QPoint &p1, const QPoint &p2,
                     const QPalette &pal, bool sunken,
                     int lineWidth, int midLineWidth)
{
    qDrawShadeLine(p, p1.x(), p1.y(), p2.x(), p2.y(), pal, sunken,
                    lineWidth, midLineWidth);
}

/*!
    \fn void qDrawShadeRect(QPainter *p, const QRect &r, const QPalette &pal,
    bool sunken, int lineWidth, int midLineWidth, const QBrush *fill)

    \overload
*/

void qDrawShadeRect(QPainter *p, const QRect &r,
                     const QPalette &pal, bool sunken,
                     int lineWidth, int midLineWidth,
                     const QBrush *fill)
{
    qDrawShadeRect(p, r.x(), r.y(), r.width(), r.height(), pal, sunken,
                    lineWidth, midLineWidth, fill);
}

/*!
    \fn void qDrawShadePanel(QPainter *p, const QRect &r, const QPalette &pal, bool sunken, int lineWidth, const QBrush *fill)
    \overload
*/

void qDrawShadePanel(QPainter *p, const QRect &r,
                      const QPalette &pal, bool sunken,
                      int lineWidth, const QBrush *fill)
{
    qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(), pal, sunken,
                     lineWidth, fill);
}

/*!
    \fn void qDrawWinButton(QPainter *p, const QRect &r, const QPalette &pal, bool sunken, const QBrush *fill)
    \overload
*/

void qDrawWinButton(QPainter *p, const QRect &r,
                     const QPalette &pal, bool sunken, const QBrush *fill)
{
    qDrawWinButton(p, r.x(), r.y(), r.width(), r.height(), pal, sunken, fill);
}

/*!
    \fn void qDrawWinPanel(QPainter *p, const QRect &r, const QPalette &pal, bool sunken, const QBrush *fill)
    \overload
*/

void qDrawWinPanel(QPainter *p, const QRect &r,
                    const QPalette &pal, bool sunken,  const QBrush *fill)
{
    qDrawWinPanel(p, r.x(), r.y(), r.width(), r.height(), pal, sunken, fill);
}

/*!
    \fn void qDrawPlainRect(QPainter *p, const QRect &r, const QColor &c, int lineWidth, const QBrush *fill)
    \overload
*/

void qDrawPlainRect(QPainter *p, const QRect &r, const QColor &c,
                     int lineWidth, const QBrush *fill)
{
    qDrawPlainRect(p, r.x(), r.y(), r.width(), r.height(), c,
                    lineWidth, fill);
}


static void qDrawWinArrow(QPainter *p, Qt::ArrowType type, bool down,
                           int x, int y, int w, int h,
                           const QPalette &pal, bool enabled)
{
    QPointArray a;                                // arrow polygon
    switch (type) {
    case Qt::UpArrow:
        a.setPoints(7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2);
        break;
    case Qt::DownArrow:
        a.setPoints(7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2);
        break;
    case Qt::LeftArrow:
        a.setPoints(7, 1,-3, 1,3, 0,-2, 0,2, -1,-1, -1,1, -2,0);
        break;
    case Qt::RightArrow:
        a.setPoints(7, -1,-3, -1,3, 0,-2, 0,2, 1,-1, 1,1, 2,0);
        break;
    }
    if (a.isEmpty())
        return;

    if (down) {
        x++;
        y++;
    }

    QPen savePen = p->pen();                        // save current pen
    if (down)
        p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
    p->fillRect(x, y, w, h, pal.brush(QPalette::Button));
    if (down)
        p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
    if (enabled) {
        a.translate(x+w/2, y+h/2);
        p->setPen(pal.foreground());
        p->drawLineSegments(a, 0, 3);                // draw arrow
        p->drawPoint(a[6]);
    } else {
        a.translate(x+w/2+1, y+h/2+1);
        p->setPen(pal.light());
        p->drawLineSegments(a, 0, 3);                // draw arrow
        p->drawPoint(a[6]);
        a.translate(-1, -1);
        p->setPen(pal.mid());
        p->drawLineSegments(a, 0, 3);                // draw arrow
        p->drawPoint(a[6]);
    }
    p->setPen(savePen);                        // restore pen
}


#if defined(Q_CC_MSVC)
#pragma warning(disable: 4244)
#endif

#ifndef QT_NO_STYLE_MOTIF
// motif arrows look the same whether they are used or not
// is this correct?
static void qDrawMotifArrow(QPainter *p, Qt::ArrowType type, bool down,
                             int x, int y, int w, int h,
                             const QPalette &pal, bool)
{
    QPointArray bFill;                                // fill polygon
    QPointArray bTop;                                // top shadow.
    QPointArray bBot;                                // bottom shadow.
    QPointArray bLeft;                                // left shadow.
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix        matrix;                                // xform matrix
#endif
    bool vertical = type == Qt::UpArrow || type == Qt::DownArrow;
    bool horizontal = !vertical;
    int         dim = w < h ? w : h;
    int         colspec = 0x0000;                        // color specification array

    if (dim < 2)                                // too small arrow
        return;

    if (dim > 3) {
        if (dim > 6)
            bFill.resize(dim & 1 ? 3 : 4);
        bTop.resize((dim/2)*2);
        bBot.resize(dim & 1 ? dim + 1 : dim);
        bLeft.resize(dim > 4 ? 4 : 2);
        bLeft.putPoints(0, 2, 0,0, 0,dim-1);
        if (dim > 4)
            bLeft.putPoints(2, 2, 1,2, 1,dim-3);
        bTop.putPoints(0, 4, 1,0, 1,1, 2,1, 3,1);
        bBot.putPoints(0, 4, 1,dim-1, 1,dim-2, 2,dim-2, 3,dim-2);

        for(int i=0; i<dim/2-2 ; i++) {
            bTop.putPoints(i*2+4, 2, 2+i*2,2+i, 5+i*2, 2+i);
            bBot.putPoints(i*2+4, 2, 2+i*2,dim-3-i, 5+i*2,dim-3-i);
        }
        if (dim & 1)                                // odd number size: extra line
            bBot.putPoints(dim-1, 2, dim-3,dim/2, dim-1,dim/2);
        if (dim > 6) {                        // dim>6: must fill interior
            bFill.putPoints(0, 2, 1,dim-3, 1,2);
            if (dim & 1)                        // if size is an odd number
                bFill.setPoint(2, dim - 3, dim / 2);
            else
                bFill.putPoints(2, 2, dim-4,dim/2-1, dim-4,dim/2);
        }
    }
    else {
        if (dim == 3) {                        // 3x3 arrow pattern
            bLeft.setPoints(4, 0,0, 0,2, 1,1, 1,1);
            bTop .setPoints(2, 1,0, 1,0);
            bBot .setPoints(2, 1,2, 2,1);
        }
        else {                                        // 2x2 arrow pattern
            bLeft.setPoints(2, 0,0, 0,1);
            bTop .setPoints(2, 1,0, 1,0);
            bBot .setPoints(2, 1,1, 1,1);
        }
    }

    if (type == Qt::UpArrow || type == Qt::LeftArrow) {
#ifndef QT_NO_TRANSFORMATIONS        // #### fix me!
        matrix.translate(x, y);
        if (vertical) {
            matrix.translate(0, h - 1);
            matrix.rotate(-90);
        } else {
            matrix.translate(w - 1, h - 1);
            matrix.rotate(180);
        }
#endif
        if (down)
            colspec = horizontal ? 0x2334 : 0x2343;
        else
            colspec = horizontal ? 0x1443 : 0x1434;
    }
    else if (type == Qt::DownArrow || type == Qt::RightArrow) {
#ifndef QT_NO_TRANSFORMATIONS        // #### fix me!
        matrix.translate(x, y);
        if (vertical) {
            matrix.translate(w-1, 0);
            matrix.rotate(90);
        }
#endif
        if (down)
            colspec = horizontal ? 0x2443 : 0x2434;
        else
            colspec = horizontal ? 0x1334 : 0x1343;
    }

    const QColor *cols[5];
    cols[0] = 0;
    cols[1] = &pal.button().color();
    cols[2] = &pal.mid().color();
    cols[3] = &pal.light().color();
    cols[4] = &pal.dark().color();
#define CMID        *cols[(colspec>>12) & 0xf]
#define CLEFT        *cols[(colspec>>8) & 0xf]
#define CTOP        *cols[(colspec>>4) & 0xf]
#define CBOT        *cols[colspec & 0xf]

    QPen     savePen   = p->pen();                // save current pen
    QBrush   saveBrush = p->brush();                // save current brush
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix wxm = p->worldMatrix();
#endif
    QPen     pen(Qt::NoPen);
    const QBrush &brush = pal.brush(QPalette::Button);

    p->setPen(pen);
    p->setBrush(brush);
#ifndef QT_NO_TRANSFORMATIONS
    p->setWorldMatrix(matrix, true);                // set transformation matrix
#endif
    p->drawPolygon(bFill);                        // fill arrow
    p->setBrush(Qt::NoBrush);                        // don't fill

    p->setPen(CLEFT);
    p->drawLineSegments(bLeft);
    p->setPen(CTOP);
    p->drawLineSegments(bTop);
    p->setPen(CBOT);
    p->drawLineSegments(bBot);

#ifndef QT_NO_TRANSFORMATIONS
    p->setWorldMatrix(wxm);
#endif
    p->setBrush(saveBrush);                        // restore brush
    p->setPen(savePen);                        // restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT
}
#endif

void qDrawArrow(QPainter *p, Qt::ArrowType type, Qt::GUIStyle style, bool down,
                 int x, int y, int w, int h,
                 const QPalette &pal, bool enabled)
{
    switch (style) {
        case Qt::WindowsStyle:
            qDrawWinArrow(p, type, down, x, y, w, h, pal, enabled);
            break;
#ifndef QT_NO_STYLE_MOTIF
        case Qt::MotifStyle:
            qDrawMotifArrow(p, type, down, x, y, w, h, pal, enabled);
            break;
#endif
        default:
            qWarning("qDrawArrow: Requested GUI style not supported");
    }
}
#endif //QT_NO_DRAWUTIL
