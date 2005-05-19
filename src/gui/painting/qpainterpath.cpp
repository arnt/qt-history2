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

#include "qpainterpath.h"
#include "qpainterpath_p.h"

#include <qiodevice.h>
#include <private/qbezier_p.h>
#include <private/qfontengine_p.h>
#include <private/qobject_p.h>
#include <private/qtextengine_p.h>
#include <private/qnumeric_p.h>
#include <private/qmath_p.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qlist.h>
#include <qmatrix.h>
#include <qpen.h>
#include <qpolygon.h>
#include <qtextlayout.h>
#include <qvarlengtharray.h>
#include <limits.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if 0
#include <performance.h>
#else
#define PM_INIT
#define PM_MEASURE(x)
#define PM_DISPLAY
#endif

// This value is used to determine the length of control point vectors
// when approximating arc segments as curves. The factor is multiplied
// with the radius of the circle.
#define KAPPA 0.5522847498

// #define QPP_DEBUG
// #define QPP_STROKE_DEBUG
//#define QPP_FILLPOLYGONS_DEBUG

QPainterPath qt_stroke_dash(const QPainterPath &path, qreal *dashes, int dashCount);

void qt_find_ellipse_coords(const QRectF &r, qreal angle, qreal length,
                            QPointF* startPoint, QPointF *endPoint)
{
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
    qreal a = r.width() / 2.0;
    qreal b = r.height() / 2.0;

    if (startPoint) {
        *startPoint = r.center()
                      + QPointF(a * cos(ANGLE(angle)), -b * sin(ANGLE(angle)));
    }
    if (endPoint) {
        *endPoint = r.center()
                    + QPointF(a * cos(ANGLE(angle + length)), -b * sin(ANGLE(angle + length)));
    }
}

#ifdef QPP_DEBUG
static void qt_debug_path(const QPainterPath &path)
{
    const char *names[] = {
        "MoveTo     ",
        "LineTo     ",
        "CurveTo    ",
        "CurveToData"
    };

    printf("\nQPainterPath: elementCount=%d\n", path.elementCount());
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        Q_ASSERT(e.type >= 0 && e.type <= QPainterPath::CurveToDataElement);
        printf(" - %3d:: %s, (%.2f, %.2f)\n", i, names[e.type], e.x, e.y);
    }
}
#endif

/*!
    \class QPainterPath
    \brief The QPainterPath class provides a container for painting operations,
    enabling graphical shapes to be constructed and reused.

    A painter path is an object composed of a number of graphical
    building blocks, such as rectangles, ellipses, lines, and curves.
    A painter path can be used for filling, outlining, and clipping.
    The main advantage of painter paths over normal drawing
    operations is that complex shapes only need to be created once,
    but they can be drawn many times using only calls to
    QPainter::drawPath().

    Building blocks can be joined in closed subpaths, such as a
    rectangle or an ellipse, or they can exist independently as unclosed
    subpaths. Note that unclosed paths will not be filled.

    Below is a code snippet that shows how a path can be used. The
    painter in this case has a pen width of 3 and a light blue brush.
    The painter path is initially empty when constructed.
    We first add a rectangle, which becomes a closed subpath.  We
    then add two bezier curves, and finally draw the entire path.

    \table
    \row
    \i \inlineimage qpainterpath-example.png
    \i \quotefromfile snippets/painterpath/painterpath.cpp
    \skipto PATH
    \skipto QPainterPath
    \printuntil drawPath
    \endtable

    \sa QPainter QRegion QPolygonF QRectF QPointF
*/

/*!
    \enum QPainterPath::ElementType
    \internal

    This enum describes the types of elements used to connect vertices
    in subpaths.

    \value MoveToElement      A new line begins at the element.
    \value LineToElement      A line is used to connect the previous element to
                              the new current element.
    \value CurveToElement     A curve is used to connect the previous element to
                              the new current element.
    \value CurveToDataElement Provides extra data required to describe a curve in
                              a \c CurveToElement element.
*/

/*!
    \class QPainterPath::Element
    \internal
*/

/*!
    \fn void QPainterPath::addEllipse(qreal x, qreal y, qreal width, qreal height)
    \overload

    Creates an ellipse within a bounding rectangle defined by its top-left
    corner at (\a x, \a y), \a width and \a height, and adds it to the
    painter path.

    If the current subpath is closed, a new subpath is started. The ellipse
    is composed of a clockwise curve, starting and finishing at zero degrees
    (the 3 o'clock position).
*/

/*!
    \fn void QPainterPath::addText(qreal x, qreal y, const QFont &font, const QString &text)
    \overload

    Adds the given \a text to this path as a set of closed subpaths created
    from the \a font supplied. The subpaths are positioned so that the left
    end of the text's baseline lies at the point specified by (\a x, \a y).

    \sa QPainter::drawText()
*/

/*!
    \fn int QPainterPath::elementCount() const

    Returns the number of path elements in the painter path.
*/

/*!
    \fn const QPainterPath::Element &QPainterPath::elementAt(int index) const

    Returns the element at the given \a index in the painter path.
*/

/*###
    \fn QPainterPath &QPainterPath::operator +=(const QPainterPath &other)

    Appends the \a other painter path to this painter path and returns a
    reference to the result.
*/

/*!
    Constructs a new empty QPainterPath.
*/
QPainterPath::QPainterPath()
    : d_ptr(0)
{
}

/*!
    Creates a new painter path that is a copy of the \a other painter
    path.
*/
QPainterPath::QPainterPath(const QPainterPath &other)
    : d_ptr(other.d_ptr)
{
    if (d_func())
        d_func()->ref.ref();
}

/*!
    Creates a new painter path with \a startPoint as starting poing
*/

QPainterPath::QPainterPath(const QPointF &startPoint)
    : d_ptr(new QPainterPathData)
{
    Element e = { startPoint.x(), startPoint.y(), MoveToElement };
    d_func()->elements << e;
}

/*!
    \internal
*/
void QPainterPath::detach_helper()
{
    QPainterPathPrivate *data = new QPainterPathData(*d_func());
    data = qAtomicSetPtr(&d_ptr, data);
    if (data && !data->ref.deref())
        delete (QPainterPathData *) data;
}

/*!
    \internal
*/
void QPainterPath::ensureData_helper()
{
    QPainterPathPrivate *data = new QPainterPathData;
    data->elements.reserve(16);
    QPainterPath::Element e = { 0, 0, QPainterPath::MoveToElement };
    data->elements << e;
    data = qAtomicSetPtr(&d_ptr, data);
    if (data && !data->ref.deref())
        delete (QPainterPathData *) data;
    Q_ASSERT(d_ptr != 0);
}

/*!
    Assigns the \a other painter path to this painter path.
*/
QPainterPath &QPainterPath::operator=(const QPainterPath &other)
{
    if (other.d_func() != d_func()) {
        QPainterPathPrivate *data = other.d_func();
        if (data) data->ref.ref();
        data = qAtomicSetPtr(&d_ptr, data);
        if (data && !data->ref.deref())
            delete (QPainterPathData *) data;
    }
    return *this;
}

/*!
    Destroys the painter path.
*/
QPainterPath::~QPainterPath()
{
    if (d_func() && !d_func()->ref.deref())
        delete d_func();
}

/*!
    Closes the current subpath. If the subpath does not contain any
    elements, the function does nothing. A new subpath is automatically
    begun when the current subpath is closed. The current point of the
    new path is (0, 0).
 */
void QPainterPath::closeSubpath()
{
#ifdef QPP_DEBUG
    printf("QPainterPath::closeSubpath()\n");
#endif
    if (isEmpty())
        return;
    detach();

    d_func()->close();
    moveTo(QPointF(0, 0));
}

/*!
    \fn void QPainterPath::moveTo(qreal x, qreal y)

    \overload

    Moves the current point to (\a{x}, \a{y}). Moving the current
    point will also start a new subpath. The previously current path
    will not be closed implicitly before the new one is started.
*/

/*!
    \fn void QPainterPath::moveTo(const QPointF &point)

    Moves the current point to the given \a point. Moving the current
    point will also start a new subpath. The previously current path
    will not be closed implicitly before the new one is started.
*/
void QPainterPath::moveTo(const QPointF &p)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::moveTo() (%.2f,%.2f)\n", p.x(), p.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(p.x()) || qIsNan(p.y()))
        qWarning("QPainterPath::moveTo(): adding point where x or y is nan, results are undefined.");
#endif
    ensureData();
    detach();

    QPainterPathData *d = d_func();
    Q_ASSERT(!d->elements.isEmpty());
    if (d->elements.last().type == MoveToElement) {
        d->elements.last().x = p.x();
        d->elements.last().y = p.y();
    } else {
        Element elm = { p.x(), p.y(), MoveToElement };
        d->elements.append(elm);
    }
    d->cStart = d->elements.size() - 1;

    d->makeDirty();
}

/*!
    \fn void QPainterPath::lineTo(qreal x, qreal y)

    \overload

    Draws a line from the current point to the point at (\a{x}, \a{y}).
    After the line is drawn, the current point is updated to be at the
    end point of the line.
*/

/*!
    \fn void QPainterPath::lineTo(const QPointF &endPoint)

    Adds a straight line from the current point to the given \a endPoint.
    After the line is drawn, the current point is updated to be at the
    end point of the line.
 */
void QPainterPath::lineTo(const QPointF &p)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::lineTo() (%.2f,%.2f)\n", p.x(), p.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(p.x()) || qIsNan(p.y()))
        qWarning("QPainterPath::lineTo(): adding point where x or y is nan, results are undefined.");
#endif
    ensureData();
    detach();

    QPainterPathData *d = d_func();
    Q_ASSERT(!d->elements.isEmpty());
    if (p == QPointF(d->elements.last()))
        return;
    Element elm = { p.x(), p.y(), LineToElement };
    d->elements.append(elm);

    d->makeDirty();
}

/*!
    \fn void QPainterPath::cubicTo(qreal ctrlPt1x, qreal ctrlPt1y, qreal ctrlPt2x,
                                   qreal ctrlPt2y, qreal endPtx, qreal endPty);

    \overload

    Adds a Bezier curve between the current point and the endpoint
    (\a{endPtx}, \a{endPty}) with control points specified by
    (\a{ctrlPt1x}, \a{ctrlPt1y}) and (\a{ctrlPt2x}, \a{ctrlPt2y}).
    After the curve is added, the current point is updated to be at
    the end point of the curve.
*/

/*!
    \fn void QPainterPath::cubicTo(const QPointF &c1, const QPointF &c2, const QPointF &endPoint)

    Adds a Bezier curve between the current point and \a endPoint with control
    points specified by \a c1, and \a c2. After the curve is added, the current
    point is updated to be at the end point of the curve.
*/
void QPainterPath::cubicTo(const QPointF &c1, const QPointF &c2, const QPointF &e)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::cubicTo() (%.2f,%.2f), (%.2f,%.2f), (%.2f,%.2f)\n",
           c1.x(), c1.y(), c2.x(), c2.y(), e.x(), e.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(c1.x()) || qIsNan(c1.y()) || qIsNan(c2.x()) || qIsNan(c2.y())
        || qIsNan(e.x()) || qIsNan(e.y()))
        qWarning("QPainterPath::cubicTo(): adding point where x or y is nan, results are undefined.");
#endif
    ensureData();
    detach();

    QPainterPathData *d = d_func();
    Q_ASSERT(!d->elements.isEmpty());
    Element ce1 = { c1.x(), c1.y(), CurveToElement };
    Element ce2 = { c2.x(), c2.y(), CurveToDataElement };
    Element ee = { e.x(), e.y(), CurveToDataElement };
    d->elements << ce1 << ce2 << ee;

    d->makeDirty();
}

/*!
    \fn void QPainterPath::quadTo(qreal ctrlPtx, qreal ctrlPty, qreal endPtx, qreal endPty);

    \overload

    Adds a quadratic Bezier curve between the current point and the endpoint
    (\a{endPtx}, \a{endPty}) with the control point specified by
    (\a{ctrlPtx}, \a{ctrlPty}).
    After the curve is added, the current point is updated to be at
    the end point of the curve.
*/

/*!
    \fn void QPainterPath::quadTo(const QPointF &c, const QPointF &endPoint)

    Adds a quadratic Bezier curve between the current point and \a
    endPoint with control point specified by \a c. After the curve is
    added, the current point is updated to be at the end point of the
    curve.
*/
void QPainterPath::quadTo(const QPointF &c, const QPointF &e)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::quadTo() (%.2f,%.2f), (%.2f,%.2f)\n",
           c.x(), c.y(), e.x(), e.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(c.x()) || qIsNan(c.y()) || qIsNan(e.x()) || qIsNan(e.y()))
        qWarning("QPainterPath::quadTo(): adding point where x or y is nan, results are undefined.");
#endif
    ensureData();
    detach();

    Q_D(QPainterPath);
    Q_ASSERT(!d->elements.isEmpty());
    const QPainterPath::Element &elm = d->elements.at(elementCount()-1);
    QPointF prev(elm.x, elm.y);
    QPointF c1((prev.x() + 2*c.x()) / 3, (prev.y() + 2*c.y()) / 3);
    QPointF c2((e.x() + 2*c.x()) / 3, (e.y() + 2*c.y()) / 3);
    cubicTo(c1, c2, e);
}

/*!
    \fn void QPainterPath::arcTo(qreal x, qreal y, qreal width, qreal
    height, qreal startAngle, qreal sweepLength)

    \overload

    The arc's lies within the rectangle given by the point (\a{x},
    \a{y}), \a width and \a height, beginning at \a startAngle and
    extending \a sweepLength degrees counter-clockwise. Angles are
    specified in degrees. Clockwise arcs can be specified using
    negative angles.

    This function connects the current point to the starting point of
    the arc if they are not already connected.

    \sa QPainter::drawArc()
*/

/*!
    \fn void QPainterPath::arcTo(const QRectF &rectangle, qreal startAngle, qreal sweepLength)

    Creates an arc that occupies the given \a rectangle, beginning at
    \a startAngle and extending \a sweepLength degrees counter-clockwise.
    Angles are specified in degrees. Clockwise arcs can be specified using
    negative angles.

    This function connects the current point to the starting point of
    the arc if they are not already connected.

    \sa QPainter::drawArc()
*/
void QPainterPath::arcTo(const QRectF &rect, qreal startAngle, qreal sweepLength)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::arcTo() (%.2f, %.2f, %.2f, %.2f, angle=%.2f, sweep=%.2f\n",
           rect.x(), rect.y(), rect.width(), rect.height(), startAngle, sweepLength);
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(rect.x()) || qIsNan(rect.y()) || qIsNan(rect.width()) || qIsNan(rect.height())
        || qIsNan(startAngle) || qIsNan(sweepLength))
        qWarning("QPainterPath::arcTo(): adding arc where a parameter is nan, results are undefined.");
#endif
    if (rect.isNull())
        return;

    ensureData();
    detach();

    Q_D(QPainterPath);
    Q_ASSERT(!d->elements.isEmpty());
//     printf("   -> arcTo: rect=(%.1f,%.1f,%.1f,%.1f), angle=%.1f, len=%.1f\n",
//            rect.x(), rect.y(), rect.width(), rect.height(),
//            startAngle, sweepLength);
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
#define SIGN(t) (t > 0 ? 1 : -1)
    qreal a = rect.width() / 2.0;
    qreal b = rect.height() / 2.0;

    qreal absSweepLength = (sweepLength < 0 ? -sweepLength : sweepLength);
    int iterations = qIntCast((absSweepLength + 89) / 90);
    qreal clength = sweepLength / iterations;
    qreal cosangle1, sinangle1, cosangle2, sinangle2;
    for (int i=0; i<iterations; ++i) {
        qreal cangle = startAngle + i * clength;

        cosangle1 = cos(ANGLE(cangle));
        sinangle1 = sin(ANGLE(cangle));
        cosangle2 = cos(ANGLE(cangle + clength));
        sinangle2 = sin(ANGLE(cangle + clength));

        // Find the start and end point of the curve.
        QPointF startPoint = rect.center() + QPointF(a * cosangle1, -b * sinangle1);
        QPointF endPoint = rect.center() + QPointF(a * cosangle2, -b * sinangle2);

        // The derived at the start and end point.
        qreal sdx = -a * sinangle1;
        qreal sdy = -b * cosangle1;
        qreal edx = -a * sinangle2;
        qreal edy = -b * cosangle2;

        // Creating the tangent lines. We need to reverse their direction if the
        // sweep is negative (clockwise)
        QLineF controlLine1(startPoint, startPoint + SIGN(sweepLength) * QPointF(sdx, sdy));
        QLineF controlLine2(endPoint, endPoint - SIGN(sweepLength) * QPointF(edx, edy));

        // We need to scale down the control lines to match that of the current sweeplength.
        // qAbs because we only want to scale, not change direction.
        qreal kappa = KAPPA * qAbs(clength) / 90.0;
        // Adjust their length to fit the magic KAPPA length.
        controlLine1.setLength(controlLine1.length() * kappa);
        controlLine2.setLength(controlLine2.length() * kappa);

        if (startPoint != QPointF(d->elements.last().x, d->elements.last().y))
            lineTo(startPoint);
        cubicTo(controlLine1.p2(), controlLine2.p2(), endPoint);
    }
}


/*!
    \fn QPointF QPainterPath::currentPosition() const

    Returns the current position of the path.
*/
QPointF QPainterPath::currentPosition() const
{
    return isEmpty() ? QPointF() : QPointF(d_func()->elements.last().x, d_func()->elements.last().y);
}


/*!
    \fn void QPainterPath::addRect(qreal x, qreal y, qreal width, qreal height)

    \overload

    Adds a rectangle at position (\a{x}, \a{y}), with the given \a
    width and \a height. The rectangle is added as a clockwise set of
    lines. Current position after the rect has been added is (\a{x}, \a{y}).
*/

/*!
    \fn void QPainterPath::addRect(const QRectF &rectangle)

    Adds the \a rectangle to this path as a closed subpath. The
    rectangle is added as a clockwise set of lines. The painter path's
    current position after the rect has been added is at the top-left
    corner of the rectangle.
*/
void QPainterPath::addRect(const QRectF &r)
{
#ifndef QT_NO_DEBUG
    if (qIsNan(r.x()) || qIsNan(r.y()) || qIsNan(r.width()) || qIsNan(r.height()))
        qWarning("QPainterPath::addRect(): adding rect where a parameter is nan, results are undefined.");
#endif
    if (r.isNull())
        return;

    ensureData();
    detach();

    d_func()->elements.reserve(d_func()->elements.size() + 5);
    moveTo(r.x(), r.y());

    Element l1 = { r.x() + r.width(), r.y(), LineToElement };
    Element l2 = { r.x() + r.width(), r.y() + r.height(), LineToElement };
    Element l3 = { r.x(), r.y() + r.height(), LineToElement };
    Element l4 = { r.x(), r.y(), LineToElement };

    d_func()->elements << l1 << l2 << l3 << l4;
}

/*!
    Adds the \a polygon to path as a new subpath. Current position
    after the rect has been added is the last point in \a polygon.
*/
void QPainterPath::addPolygon(const QPolygonF &polygon)
{
    if (polygon.isEmpty())
        return;

    ensureData();
    detach();

    d_func()->elements.reserve(d_func()->elements.size() + polygon.size());

    moveTo(polygon.first());
    for (int i=1; i<polygon.size(); ++i) {
        Element elm = { polygon.at(i).x(), polygon.at(i).y(), LineToElement };
        d_func()->elements << elm;
    }
}

/*!
    Creates an ellipse within the bounding rectangle specified by
    \a boundingRect and adds it to the painter path.

    If the current subpath is closed, a new subpath is started. The ellipse
    is composed of a clockwise curve, starting and finishing at zero degrees
    (the 3 o'clock position).
*/
void QPainterPath::addEllipse(const QRectF &boundingRect)
{
#ifndef QT_NO_DEBUG
    if (qIsNan(boundingRect.x()) || qIsNan(boundingRect.y())
        || qIsNan(boundingRect.width()) || qIsNan(boundingRect.height()))
        qWarning("QPainterPath::addEllipse(): adding ellipse where a parameter is nan, results are undefined.");
#endif
    if (boundingRect.isNull())
        return;

    ensureData();
    detach();

    Q_D(QPainterPath);
    d->elements.reserve(d->elements.size() + 13);

    qreal x = boundingRect.x();
    qreal y = boundingRect.y();

    qreal w = boundingRect.width();
    qreal w2 = boundingRect.width() / 2;
    qreal w2k = w2 * KAPPA;

    qreal h = boundingRect.height();
    qreal h2 = boundingRect.height() / 2;
    qreal h2k = h2 * KAPPA;

    moveTo(x + w, y + h2);

    // 0 -> 270 degrees
    Element cp11 = { x + w, y + h2 + h2k, CurveToElement };
    Element cp21 = { x + w2 + w2k, y + h, CurveToDataElement };
    Element end1 = { x + w2, y + h, CurveToDataElement };
    d->elements << cp11 << cp21 << end1;

    // 270 -> 180 degrees
    Element cp12 = { x + w2 - w2k, y + h, CurveToElement };
    Element cp22 = { x, y + h2 + h2k, CurveToDataElement };
    Element end2 = { x, y + h2, CurveToDataElement };
    d->elements << cp12 << cp22 << end2;

    // 180 -> 90 degrees
    Element cp13 = { x, y + h2 - h2k, CurveToElement };
    Element cp23 = { x + w2 - w2k, y, CurveToDataElement };
    Element end3 = { x + w2, y, CurveToDataElement };
    d->elements << cp13 << cp23 << end3;

    // 90 -> 0 degrees
    Element cp14 = { x + w2 + w2k, y, CurveToElement };
    Element cp24 = { x + w, y + h2 - h2k, CurveToDataElement };
    Element end4 = { x + w, y + h2, CurveToDataElement };
    d->elements << cp14 << cp24 << end4;
}

/*!
    \fn void QPainterPath::addText(const QPointF &point, const QFont &font, const QString &text)

    Adds the given \a text to this path as a set of closed subpaths created
    from the \a font supplied. The subpaths are positioned so that the left
    end of the text's baseline lies at the \a point specified.

    \img qpainterpath-addtext.png

    \sa QPainter::drawText()
*/
void QPainterPath::addText(const QPointF &point, const QFont &f, const QString &text)
{
    if (text.isEmpty())
        return;

    ensureData();
    detach();

    QTextLayout layout(text, f);
    layout.setCacheEnabled(true);
    QTextEngine *eng = layout.engine();
    layout.beginLayout();
    QTextLine line = layout.createLine();
    layout.endLayout();
    const QScriptLine &sl = eng->lines[0];
    if (!sl.length || !eng->layoutData)
        return;

    int nItems = eng->layoutData->items.size();

    qreal x(point.x());
    qreal y(point.y());

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<uchar> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->layoutData->items[i].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i];
        QScriptItem &si = eng->layoutData->items[item];

        if (!si.isTab && !si.isObject) {
            QGlyphLayout *glyphs = eng->glyphs(&si);
            QFontEngine *fe = f.d->engineForScript(si.analysis.script);
            Q_ASSERT(fe);
            fe->addOutlineToPath(x, y, glyphs, si.num_glyphs, this,
                                 si.analysis.bidiLevel % 2
                                 ? QTextItem::RenderFlags(QTextItem::RightToLeft)
                                 : QTextItem::RenderFlags(0));
        }
        x += si.width;
    }
}

/*!

  Adds the path \a other to this path.
*/
void QPainterPath::addPath(const QPainterPath &other)
{
    if (other.isEmpty())
        return;

    ensureData();
    detach();

    QPainterPathData *d = reinterpret_cast<QPainterPathData *>(d_func());
    // Remove last moveto so we don't get multiple moveto's
    if (d->elements.last().type == MoveToElement)
        d->elements.remove(d->elements.size()-1);

    // Locate where our own current subpath will start after the other path is added.
    int cStart = d->elements.size() + other.d_func()->cStart;
    d->elements += other.d_func()->elements;
    d->cStart = cStart;
}


/*!
    Adds the path \a other to this path by connecting the last
    element of this to the first element of \a other.

    \sa addPath()
*/
void QPainterPath::connectPath(const QPainterPath &other)
{
    if (other.isEmpty())
        return;

    ensureData();
    detach();

    QPainterPathData *d = reinterpret_cast<QPainterPathData *>(d_func());
    // Remove last moveto so we don't get multiple moveto's
    if (d->elements.last().type == MoveToElement)
        d->elements.remove(d->elements.size()-1);

    // Locate where our own current subpath will start after the other path is added.
    int cStart = d->elements.size() + other.d_func()->cStart;
    int first = d->elements.size();
    d->elements += other.d_func()->elements;

    d->elements[first].type = LineToElement;
    if (cStart != first)
        d->cStart = cStart;
}

/*!
    Adds the region \a region to the path. This is done by
    adding each rectangle in the region as a separate subpath.
*/
void QPainterPath::addRegion(const QRegion &region)
{
    ensureData();
    detach();

    QVector<QRect> rects = region.rects();
    d_func()->elements.reserve(rects.size() * 5);
    for (int i=0; i<rects.size(); ++i)
        addRect(rects.at(i));
}


/*!
    Returns the fill rule of the painter path. The default fill rule
    is \c Qt::OddEvenFill.

    \sa Qt::FillRule setFillRule()
*/
Qt::FillRule QPainterPath::fillRule() const
{
    return isEmpty() ? Qt::OddEvenFill : d_func()->fillRule;
}

/*!
    \fn void QPainterPath::setFillRule(Qt::FillRule fillRule)

    Sets the fill rule of the painter path to \a fillRule.

    \sa fillRule()
*/
void QPainterPath::setFillRule(Qt::FillRule fillRule)
{
    ensureData();
    detach();

    d_func()->makeDirty();
    d_func()->fillRule = fillRule;
}

/*!
    Returns the bounding rectangle of this painter path as a rectangle with
    floating point precision.

    This function is costly. You may consider using controlPointRect() instead.
*/
QRectF QPainterPath::boundingRect() const
{
    // ### QBezier::boundingRect
    return toFillPolygon().boundingRect();
}

/*!

    Returns the rectangle containing the all the points and control
    points in this path. This rectangle is always at least as large
    as and will always include the boundingRect().

    This function is significantly faster to compute than the exact
    boundingRect();
*/
QRectF QPainterPath::controlPointRect() const
{
    Q_D(QPainterPath);
    if (isEmpty())
        return QRect();

    qreal minx, maxx, miny, maxy;
    minx = maxx = d->elements.at(0).x;
    miny = maxy = d->elements.at(0).y;
    for (int i=1; i<d->elements.size(); ++i) {
        const Element &e = d->elements.at(i);
        if (e.x > maxx) maxx = e.x;
        else if (e.x < minx) minx = e.x;
        if (e.y > maxy) maxy = e.y;
        else if (e.y < miny) miny = e.y;
    }
    return QRectF(minx, miny, maxx - minx, maxy - miny);
}


/*!
  \fn bool QPainterPath::isEmpty() const

    Returns true if there are no elements in this path.
*/

/*!
    Creates a reversed copy of this path and returns it
*/
QPainterPath QPainterPath::toReversed() const
{
    Q_D(const QPainterPath);
    QPainterPath rev;

    if (isEmpty()) {
        rev = *this;
        return rev;
    }

    rev.moveTo(d->elements.at(d->elements.size()-1).x, d->elements.at(d->elements.size()-1).y);

    for (int i=d->elements.size()-1; i>=1; --i) {
        const QPainterPath::Element &elm = d->elements.at(i);
        const QPainterPath::Element &prev = d->elements.at(i-1);
        switch (elm.type) {
        case LineToElement:
            rev.lineTo(prev.x, prev.y);
            break;
        case MoveToElement:
            rev.moveTo(prev.x, prev.y);
            break;
        case CurveToDataElement:
            {
                Q_ASSERT(i>=3);
                const QPainterPath::Element &cp1 = d->elements.at(i-2);
                const QPainterPath::Element &sp = d->elements.at(i-3);
                Q_ASSERT(prev.type == CurveToDataElement);
                Q_ASSERT(cp1.type == CurveToElement);
                rev.cubicTo(prev.x, prev.y, cp1.x, cp1.y, sp.x, sp.y);
                i -= 2;
                break;
            }
        default:
            Q_ASSERT(!"qt_reversed_path");
            break;
        }
    }
    //qt_debug_path(rev);
    return rev;
}


/*!
    Returns the painter path as a list of polygons. One polygon is
    created for each subpath. The polygons are transformed using the
    transformation matrix \a matrix.
*/
QList<QPolygonF> QPainterPath::toSubpathPolygons(const QMatrix &matrix) const
{
    Q_D(const QPainterPath);
    QList<QPolygonF> flatCurves;
    if (isEmpty())
        return flatCurves;

    QPolygonF current;
    for (int i=0; i<elementCount(); ++i) {
        const QPainterPath::Element &e = d->elements.at(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (current.size() > 1)
                flatCurves += current;
            current.clear();
            current += QPointF(e.x, e.y) * matrix;
            break;
        case QPainterPath::LineToElement:
            current += QPointF(e.x, e.y) * matrix;
            break;
        case QPainterPath::CurveToElement: {
            Q_ASSERT(d->elements.at(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(d->elements.at(i+2).type == QPainterPath::CurveToDataElement);
            QPolygonF bezier = QBezier(QPointF(d->elements.at(i-1).x, d->elements.at(i-1).y) * matrix,
                                       QPointF(e.x, e.y) * matrix,
                                       QPointF(d->elements.at(i+1).x, d->elements.at(i+1).y) * matrix,
                                       QPointF(d->elements.at(i+2).x, d->elements.at(i+2).y) * matrix).toPolygon();
            bezier.remove(0);
            current += bezier;
            i+=2;
            break;
        }
        case QPainterPath::CurveToDataElement:
            Q_ASSERT(!"QPainterPath::toSubpathPolygons(), bad element type");
            break;
        }
    }

    if (current.size()>1)
        flatCurves += current;

    return flatCurves;
}

/*!
    Returns the painter path as one polygon that can be used for
    filling. This polygon is created by first converting all subpaths
    to polygons, then using a rewinding technique to make sure that
    overlapping subpaths can be filled using the correct fill rule.

    The polygon is transformed using the transformation matrix \a
    matrix.

    Note that rewinding inserts addition lines in the polygon so
    the outline of the fill polygon does not match the outline of
    the path.

    \sa toSubpathPolygons(), toFillPolygons()
*/
QPolygonF QPainterPath::toFillPolygon(const QMatrix &matrix) const
{

    QList<QPolygonF> flats = toSubpathPolygons(matrix);
    QPolygonF polygon;
    if (flats.isEmpty())
        return polygon;
    QPointF first = flats.first().first();
    for (int i=0; i<flats.size(); ++i) {
        polygon += flats.at(i);
        if (!flats.at(i).isClosed())
            polygon += flats.at(i).first();
        if (i > 0)
            polygon += first;
    }
    return polygon;
}

/*!
    Returns the path as a list of polygons. The polygons are
    transformed using the transformation matrix \a matrix.

    This function differs from toSubpathPolygons() and toFillPolygon()
    in that it creates one rewinded polygon for all subpaths that have
    overlapping bounding rectangles.

    This function is provided, because it is usually faster to draw
    several small polygons than to draw one large polygon, even though
    the total number of points drawn is the same.

    \sa toSubpathPolygons(), toFillPolygon()
*/
QList<QPolygonF> QPainterPath::toFillPolygons(const QMatrix &matrix) const
{
    QList<QPolygonF> polys;

    QList<QPolygonF> subpaths = toSubpathPolygons(matrix);
    int count = subpaths.size();

    if (count == 0)
        return polys;

    QList<QRectF> bounds;
    for (int i=0; i<count; ++i)
        bounds += subpaths.at(i).boundingRect();

    QVector< QList<int> > isects;
    isects.resize(count);

    for (int j=0; j<count; ++j) {
        QRectF cbounds = bounds.at(j);
        for (int i=j+1; i<count; ++i) {
            if (cbounds.intersects(bounds.at(i))) {
                isects[j] << i;
            }
        }
    }


#if QPP_FILLPOLYGONS_DEBUG
    for (int i = 0; i < count; ++i) {
        printf("%d: ", i);
        for (int j = 0; j < isects[i].size(); ++j) {
            printf("%d ", isects[i][j]);
        }
        printf("\n");
    }
#endif

    for (int i=0; i<count; ++i) {
        if (isects[i].isEmpty()) {
            polys += subpaths.at(i);
            // Close if not closed...
            if (!subpaths.at(i).isClosed())
                polys[polys.size()-1] += subpaths.at(i).first();
        } else {
            QList<int> l = isects[i];
            if (l.first() == -1)
                continue;
            QPolygonF buildUp = subpaths.at(i);
            QPointF rewindPt = buildUp.first();
            // Close if not closed...
            if (!buildUp.isClosed())
                buildUp += rewindPt;

            for (int il=0; il<l.size(); ++il) {
                const QList<int> &currentISects = isects.at(l.at(il));

                // Insert only unique new polys
                for (int ai=0; ai<currentISects.size(); ++ai)
                    if (!l.contains(currentISects.at(ai)) && currentISects.at(ai) != -1)
                        l.append(currentISects.at(ai));

                // They are added to current so skip for later.
                isects[l.at(il)].clear();
                isects[l.at(il)] += -1;

                // Add path to current buildup.
                buildUp += subpaths.at(l.at(il));
                if (!subpaths.at(l.at(il)).isClosed())
                    buildUp += subpaths.at(l.at(il)).first();
                buildUp += rewindPt;
            }

            polys += buildUp;
        }
    }

    return polys;
}

/*!
    Returns true if the point \a pt is contained by the path; otherwise
    returns false.
*/
bool QPainterPath::contains(const QPointF &pt) const
{
    if (isEmpty())
        return false;
    if (d_func()->containsCache.isEmpty()) {
        d_func()->containsCache = QRegion(toFillPolygon().toPolygon(), fillRule());
    }
    return d_func()->containsCache.contains(pt.toPoint());
}


/*!
    Returns true if the rect \a rect is inside the path; otherwise
    returns false.
*/
bool QPainterPath::contains(const QRectF &rect) const
{
    if (isEmpty())
        return false;
    if (d_func()->containsCache.isEmpty()) {
        d_func()->containsCache = QRegion(toFillPolygon().toPolygon(), fillRule());
    }
    return d_func()->containsCache.contains(rect.toRect());
}

/*!
    Returns true if any point in rect \a rect is inside the path; otherwise
    returns false.
*/
bool QPainterPath::intersects(const QRectF &rect) const
{
    if (isEmpty())
        return false;
    if (d_func()->containsCache.isEmpty()) {
        d_func()->containsCache = QRegion(toFillPolygon().toPolygon(), fillRule());
    }
    return !d_func()->containsCache.intersect(rect.toRect()).isEmpty();
}

/*!
    Returns true if this painterpath is equal to \a path.

    Comparing paths may involve a per element comparison which
    can be slow for complex paths.
*/

bool QPainterPath::operator==(const QPainterPath &path) const
{
    QPainterPathData *d = reinterpret_cast<QPainterPathData *>(d_func());
    if (path.d_func() == d)
        return true;
    else if (!d || !path.d_func())
        return false;
    bool equal = d->fillRule == path.d_func()->fillRule && d->elements.size() == path.d_func()->elements.size();
    for (int i = 0; i < d->elements.size() && equal; ++i)
        equal = d->elements.at(i) == path.d_func()->elements.at(i);
    return equal;
}

/*!
    Returns true if this painterpath differs from \a path.

    Comparing paths may involve a per element comparison which
    can be slow for complex paths.
*/

bool QPainterPath::operator!=(const QPainterPath &path) const
{
    return !(*this==path);
}

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QPainterPath &path)
    \relates QPainterPath

    Writes the painter path specified by \a path to the given \a stream, and
    returns a reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &s, const QPainterPath &p)
{
    if (p.isEmpty()) {
        s << 0;
        return s;
    }

    s << p.elementCount();
    for (int i=0; i < p.d_func()->elements.size(); ++i) {
        const QPainterPath::Element &e = p.d_func()->elements.at(i);
        s << int(e.type);
        s << e.x << e.y;
    }
    s << p.d_func()->cStart;
    s << int(p.d_func()->fillRule);
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QPainterPath &path)
    \relates QPainterPath

    Reads a painter path from the given \a stream into the specified \a path,
    and returns a reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &s, QPainterPath &p)
{
    int size;
    s >> size;

    if (size == 0)
        return s;

    p.ensureData(); // in case if p.d_func() == 0
    if (p.d_func()->elements.size() == 1) {
        Q_ASSERT(p.d_func()->elements.at(0).type == QPainterPath::MoveToElement);
        p.d_func()->elements.clear();
    }
    p.d_func()->elements.reserve(p.d_func()->elements.size() + size);
    for (int i=0; i<size; ++i) {
        int type;
        double x, y;
        s >> type;
        s >> x;
        s >> y;
        Q_ASSERT(type >= 0 && type <= 3);
#ifndef QT_NO_DEBUG
        if (qIsNan(x) || qIsNan(y))
            qWarning("operator>>(): adding a nan element to path, results are undefined.");
#endif
        QPainterPath::Element elm = { x, y, QPainterPath::ElementType(type) };
        p.d_func()->elements.append(elm);
    }
    s >> p.d_func()->cStart;
    int fillRule;
    s >> fillRule;
    Q_ASSERT(fillRule == Qt::OddEvenFill || Qt::WindingFill);
    p.d_func()->fillRule = Qt::FillRule(fillRule);
    return s;
}
#endif


/*******************************************************************************
 * Subpath Iterators
 */
QPainterPath::Element QSubpathReverseIterator::next()
{
    Q_ASSERT(hasNext());

    const QPainterPath::Element &pe = m_path->elementAt(m_pos+1); // previous element
    QPainterPath::Element ce = m_path->elementAt(m_pos);   // current element

    switch (pe.type) {
    case QPainterPath::LineToElement:
        ce.type = QPainterPath::LineToElement;
        break;
    case QPainterPath::CurveToDataElement:
        // First control point?
        if (ce.type == QPainterPath::CurveToElement) {
            ce.type = QPainterPath::CurveToDataElement;
        } else { // Second control point then
            ce.type = QPainterPath::CurveToElement;
        }
        break;
    case QPainterPath::CurveToElement:
        ce.type = QPainterPath::CurveToDataElement;
        break;
    default:
        qWarning("QSubpathReverseIterator::next(), unhandled case, %d", ce.type);
        break;
    }
    --m_pos;

    if (m_pos < m_start) {
        m_start = m_end + 1;
        m_end = indexOfSubpath(m_start+1);
        m_pos = m_end;
    }

    return ce;
}

QPainterPath::Element QSubpathFlatIterator::next()
{
    Q_ASSERT(hasNext());

    if (m_curve_index >= 0) {
        QPainterPath::Element e = { m_curve.at(m_curve_index).x(),
                                    m_curve.at(m_curve_index).y(),
                                    QPainterPath::LineToElement };
        ++m_curve_index;
        if (m_curve_index >= m_curve.size())
            m_curve_index = -1;
        return e;
    }

    QPainterPath::Element e = m_path->elementAt(m_pos);
    if (e.isCurveTo()) {
        Q_ASSERT(m_pos > 0);
        Q_ASSERT(m_pos < m_path->elementCount());
        m_curve = QBezier(m_path->elementAt(m_pos-1),
                          e,
                          m_path->elementAt(m_pos+1),
                          m_path->elementAt(m_pos+2)).toPolygon();
        m_curve_index = 1;
        e.type = QPainterPath::LineToElement;
        e.x = m_curve.at(0).x();
        e.y = m_curve.at(0).y();
        m_pos += 2;
    }
    Q_ASSERT(e.isLineTo());
    ++m_pos;
    return e;
}




/*******************************************************************************
 * class QPainterPathStroker
 */
#define QT_PATH_NO_JOIN Qt::PenJoinStyle(0xffff)

class QPainterPathStrokerPrivate
{
    Q_DECLARE_PUBLIC(QPainterPathStroker)
public:

    enum LineJoinMode {
        FlatJoin,
        SquareJoin,
        MiterJoin,
        RoundJoin,
        RoundCap
    };

    QPainterPathStrokerPrivate() :
        width(1),
        offset(0.5),
        miterLimit(2),
        curveThreshold(0.25),
        style(Qt::SolidLine),
        joinStyle(FlatJoin),
        capStyle(SquareJoin)
    {
        appliedMiterLimit = miterLimit * width;
    }



    void joinPoints(const QPointF &point, const QLineF &nextLine, QPainterPath *stroke,
                    LineJoinMode join) const;

    QPainterPathStroker *q_ptr;
    qreal width;
    qreal offset;
    qreal miterLimit;
    qreal appliedMiterLimit;
    qreal curveThreshold;
    Qt::PenStyle style;
    LineJoinMode joinStyle;
    LineJoinMode capStyle;
    QVector<qreal> dashPattern;
};


/*******************************************************************************
 * QLineF::angle gives us the smalles angle between two lines. Here we
 * want to identify the line's angle direction on the unit circle.
 */
static inline qreal adapted_angle_on_x(const QLineF &line)
{
    qreal angle = line.angle(QLineF(0, 0, 1, 0));
    if (line.dy() > 0)
        angle = 360 - angle;
    return angle;
}

void QPainterPathStrokerPrivate::joinPoints(const QPointF &point, const QLineF &nextLine,
                                            QPainterPath *stroke, LineJoinMode join) const
{
#ifdef QPP_STROKE_DEBUG
    printf(" -----> joinPoints: around=(%.0f, %.0f), next_p1=(%.0f, %.f) next_p2=(%.0f, %.f)\n",
           point.x(), point.y(), nextLine.x1(), nextLine.y1(), nextLine.x2(), nextLine.y2());
#endif

    int elmCount = stroke->elementCount();
    Q_ASSERT(elmCount >= 2);
    const QPainterPath::Element &back1 = stroke->elementAt(elmCount-1);

    // points connected already, don't join
    if (qFuzzyCompare(back1.x, nextLine.x1()) && qFuzzyCompare(back1.y, nextLine.y1()))
        return;

    if (join == FlatJoin) {
        stroke->lineTo(nextLine.p1());
    } else {
        const QPainterPath::Element &back2 = stroke->elementAt(elmCount-2);
        QLineF prevLine(back2.x, back2.y, back1.x, back1.y);

        QPointF isect;
        QLineF::IntersectType type = prevLine.intersect(nextLine, &isect);

        if (join == MiterJoin) {
            // If we are on the inside, do the short cut...
            QLineF shortCut(prevLine.p2(), nextLine.p1());
            if (type == QLineF::BoundedIntersection
                || prevLine.angle(shortCut) > 90) {
                stroke->lineTo(nextLine.p1());
                return;
            }
            QLineF miterLine(QPointF(back1.x, back1.y), isect);
            if (miterLine.length() > appliedMiterLimit) {
                miterLine.setLength(appliedMiterLimit);

                QLineF l2(nextLine);
                l2.setLength(appliedMiterLimit);
                l2.translate(-l2.dx(), -l2.dy());

                stroke->lineTo(miterLine.p2());
                stroke->lineTo(l2.p1());
                stroke->lineTo(nextLine.p1());

            } else {
                stroke->lineTo(isect);
                stroke->lineTo(nextLine.p1());
            }

        } else if (join == SquareJoin) {
            QLineF l1(prevLine);
            l1.translate(l1.dx(), l1.dy());
            l1.setLength(offset);
            QLineF l2(nextLine.p2(), nextLine.p1());
            l2.translate(l2.dx(), l2.dy());
            l2.setLength(offset);
            stroke->lineTo(l1.p2());
            stroke->lineTo(l2.p2());
            stroke->lineTo(l2.p1());


        } else if (join == RoundJoin) {
            QLineF shortCut(prevLine.p2(), nextLine.p1());
            if (type == QLineF::BoundedIntersection
                || prevLine.angle(shortCut) > 90) {
                stroke->lineTo(nextLine.p1());
                return;
            }
            QLineF l1(prevLine);
            QLineF l2(nextLine);
            qreal l1_on_x = adapted_angle_on_x(l1);
            qreal l2_on_x = adapted_angle_on_x(l2);

            qreal sweepLength = qAbs(l2_on_x - l1_on_x);

            stroke->arcTo(point.x() - offset, point.y() - offset, offset * 2, offset * 2,
                          l1_on_x + 90, -sweepLength);

            stroke->lineTo(nextLine.p1());

        // Same as round join except we know its 180 degrees. Can also optimize this
        // later based on the addEllipse logic
        } else if (join == RoundCap) {
            QLineF l1(prevLine);
            qreal l1_on_x = adapted_angle_on_x(l1);
            stroke->arcTo(point.x() - offset, point.y() - offset, offset * 2, offset * 2,
                          l1_on_x + 90, -180);
        }
    }
}

/*!
  \brief The QPainterPathStroker class is used to process the stroke
  of a QPainterPath into a path that can be used for filling.

  The function createStroke is used to create a stroke from a given
  path. The same stroker object can be used to create a stroke for a
  number of paths.

  Note, not all operations are supported in Tech Preview 2. These will
  come later. Supported operations include width, Qt::SolidLine and the
  various Qt::PenJoinStyle's. The outline may also have some overlapping
  regions.
*/

QPainterPathStroker::QPainterPathStroker()
    : d_ptr(new QPainterPathStrokerPrivate)
{
}

QPainterPathStroker::~QPainterPathStroker()
{ delete d_ptr; }

/*
   Strokes a subpath side using the \a it as source. Results are put into
   \a stroke. The function returns true if the subpath side was closed.
   If \a capFirst is true, we will use capPoints instead of joinPoints to
   connect the first segment, other segments will be joined using joinPoints.
   This is to put capping in order...
*/
template <class Iterator> bool qt_stroke_subpath_side(Iterator *it, QPainterPath *stroke,
                                                      const QPainterPathStrokerPrivate *data,
                                                      bool capFirst)
{
    // Used in CurveToElement section below.
    const int MAX_OFFSET = 16;
    QBezier offsetCurves[MAX_OFFSET];

    int startPos = stroke->elementCount() - 1;

    QPointF start = it->nextSubpath();
#ifdef QPP_STROKE_DEBUG
    qDebug(" -> subpath [%.2f, %.2f], startPos=%d", start.x(), start.y(), startPos);
#endif

    QPointF prev = start;

    while (it->hasNext()) {
        QPainterPath::Element e = it->next();

        // LineToElement
        if (e.isLineTo()) {
#ifdef QPP_STROKE_DEBUG
            qDebug(" ---> lineto [%.2f, %.2f]", e.x, e.y);
#endif
            QLineF line(prev, e);
            QLineF normal = line.normalVector();
            normal.setLength(data->offset);
            QLineF ml(line);
            ml.translate(normal.dx(), normal.dy());

            // If we are starting a new subpath, move to correct starting point.
            if (stroke->elementAt(stroke->elementCount()-1).isMoveTo()) {
                stroke->moveTo(ml.p1());
            } else if (capFirst) {
                data->joinPoints(prev, ml, stroke, data->capStyle);
                capFirst = false;
            } else {
                data->joinPoints(prev, ml, stroke, data->joinStyle);
            }

            // Add the stroke for this line.
            stroke->lineTo(ml.p2());
            prev = e;

        // CurveToElement
        } else if (e.isCurveTo()) {
#ifdef QPP_STROKE_DEBUG
            qDebug(" ---> curveto [%.2f, %.2f]", e.x, e.y);
#endif

            QPainterPath::Element cp2 = it->next(); // control point 2
            QPainterPath::Element ep = it->next();  // end point

            QBezier bezier(prev, e, cp2, ep);
            int count = bezier.shifted(offsetCurves,
                                       MAX_OFFSET,
                                       data->offset,
                                       data->curveThreshold);

            if (count) {
                // If we are starting a new subpath, move to correct starting point
                if (stroke->elementAt(stroke->elementCount()-1).isMoveTo()) {
                    stroke->moveTo(offsetCurves[0].pt1());
                } else if (capFirst) {
                    data->joinPoints(prev, QLineF(offsetCurves[0].pt1(),
                                                offsetCurves[0].pt2()), stroke, data->capStyle);
                    capFirst = 0;
                } else {
                    data->joinPoints(prev, QLineF(offsetCurves[0].pt1(),
                                                offsetCurves[0].pt2()), stroke, data->joinStyle);
                }
                // Add these beziers
                for (int i=0; i<count; ++i) {
                    stroke->cubicTo(offsetCurves[i].pt2(),
                                    offsetCurves[i].pt3(),
                                    offsetCurves[i].pt4());
                }
            }

            prev = ep;
        }
    }

    if (prev == start) { // closed subpath, join first and last point
#ifdef QPP_STROKE_DEBUG
        qDebug(" ---> closed subpath");
#endif
        QLineF startTangent(stroke->elementAt(startPos), stroke->elementAt(startPos+1));
        data->joinPoints(prev, startTangent, stroke, data->joinStyle);
        stroke->moveTo(QPointF()); // start new subpath
        return true;
    } else {
#ifdef QPP_STROKE_DEBUG
        qDebug(" ---> open subpath");
#endif
        return false;
    }
}

/*!
  Creates a new stroke from the path \a input.
*/
QPainterPath QPainterPathStroker::createStroke(const QPainterPath &path) const
{
    Q_D(const QPainterPathStroker);

#ifdef QPP_STROKE_DEBUG
    printf("QPainterPathPrivate::createStroke()\n");
#endif


#ifdef QPP_STROKE_DEBUG
    printf(" -> path size: %d\n", path.elementCount());
    qt_debug_path(path);
#endif

    QPainterPath input = path;

    // Create the dashed version to use.
    if (!d->dashPattern.isEmpty()) {
        QVarLengthArray<qreal, 16> pattern(d->dashPattern.size());
        for (int i=0; i<d->dashPattern.size(); ++i)
            pattern[i] = d->dashPattern.at(i) * d->width;
        input = qt_stroke_dash(path, pattern.data(), pattern.size());
    }

    QSubpathIterator fwit(&input);
    QSubpathReverseIterator bwit(&input);

    QPainterPath stroke;
    stroke.ensureData();
    stroke.d_func()->elements.reserve(input.elementCount() * 4);

    while (fwit.hasSubpath()) {
        Q_ASSERT(bwit.hasSubpath());

        int fwit_index = fwit.position();

        int bwStart = stroke.elementCount() - 1;

        bool fwclosed = qt_stroke_subpath_side(&fwit, &stroke, d, false);
        bool bwclosed = qt_stroke_subpath_side(&bwit, &stroke, d, !fwclosed);

        if (!bwclosed) {
            QLineF bwStartTangent(stroke.elementAt(bwStart), stroke.elementAt(bwStart+1));
            d->joinPoints(input.elementAt(fwit_index), bwStartTangent, &stroke, d->capStyle);
        }

        stroke.closeSubpath();
    }

    stroke.setFillRule(Qt::WindingFill);
#ifdef QPP_STROKE_DEBUG
    printf(" -> Final path:\n");
    qt_debug_path(stroke);
#endif

    return stroke;
}

void QPainterPathStroker::setWidth(qreal width)
{
    Q_D(QPainterPathStroker);
    if (width <= 0)
        width = 1;
    d->width = width;
    d->offset = width / 2;
    d->appliedMiterLimit = d->miterLimit * width;
}

qreal QPainterPathStroker::width() const
{
    return d_func()->width;
}

void QPainterPathStroker::setCapStyle(Qt::PenCapStyle style)
{
    Q_D(QPainterPathStroker);
    if (style == Qt::FlatCap)
        d->capStyle = QPainterPathStrokerPrivate::FlatJoin;
    else if (style == Qt::SquareCap)
        d->capStyle = QPainterPathStrokerPrivate::SquareJoin;
    else
        d->capStyle = QPainterPathStrokerPrivate::RoundCap;
}

Qt::PenCapStyle QPainterPathStroker::capStyle() const
{
    Q_D(const QPainterPathStroker);
    if (d->capStyle == QPainterPathStrokerPrivate::FlatJoin)
        return Qt::FlatCap;
    else if (d->capStyle == QPainterPathStrokerPrivate::SquareJoin)
        return Qt::SquareCap;
    else
        return Qt::RoundCap;
}

void QPainterPathStroker::setJoinStyle(Qt::PenJoinStyle style)
{
    Q_D(QPainterPathStroker);
    if (style == Qt::BevelJoin)
        d->joinStyle = QPainterPathStrokerPrivate::FlatJoin;
    else if (style == Qt::MiterJoin)
        d->joinStyle = QPainterPathStrokerPrivate::MiterJoin;
    else
        d->joinStyle = QPainterPathStrokerPrivate::RoundJoin;
}

Qt::PenJoinStyle QPainterPathStroker::joinStyle() const
{
    Q_D(const QPainterPathStroker);
    if (d->joinStyle == QPainterPathStrokerPrivate::FlatJoin)
        return Qt::BevelJoin;
    else if (d->joinStyle == QPainterPathStrokerPrivate::MiterJoin)
        return Qt::MiterJoin;
    else
        return Qt::RoundJoin;
}

void QPainterPathStroker::setMiterLimit(qreal limit)
{
    Q_D(QPainterPathStroker);
    d->miterLimit = limit;
    d->appliedMiterLimit = d->miterLimit * d->width;
}

qreal QPainterPathStroker::miterLimit() const
{
    return d_func()->miterLimit;
}


void QPainterPathStroker::setCurveThreshold(qreal threshold)
{
    d_func()->curveThreshold = threshold;
}

qreal QPainterPathStroker::curveThreshold() const
{
    return d_func()->curveThreshold;
}

void QPainterPathStroker::setDashPattern(Qt::PenStyle style)
{
    Q_D(QPainterPathStroker);
    d->dashPattern = QVector<qreal>();

    const qreal space = 2;
    const qreal dot = 1;
    const qreal dash = 4;

    switch (style) {
    case Qt::DashLine:
        d->dashPattern << dash << space;
        break;
    case Qt::DotLine:
        d->dashPattern << dot << space;
        break;
    case Qt::DashDotLine:
        d->dashPattern << dash << space << dot << space;
        break;
    case Qt::DashDotDotLine:
        d->dashPattern << dash << space << dot << space << dot << space;
        break;
    default:
        break;
    }
}

void QPainterPathStroker::setDashPattern(const QVector<qreal> &dashPattern)
{
    d_func()->dashPattern = dashPattern;
}

QVector<qreal> QPainterPathStroker::dashPattern() const
{
    return d_func()->dashPattern;
}


QPainterPath qt_stroke_dash(const QPainterPath &path,
                            qreal *dashes, int dashCount)
{
    Q_ASSERT(dashes);
    Q_ASSERT(dashCount > 0);

    dashCount = (dashCount / 2) * 2; // Round down to even number

    int idash = 0; // Index to current dash
    qreal pos = 0; // The position on the curve, 0 <= pos <= path.length
    qreal elen = 0; // element length
    qreal doffset = 0;

    qreal estart = 0; // The elements starting position
    qreal estop = 0; // The element stop position

    QLineF cline;

    QPainterPath dashPath;

    QSubpathFlatIterator it(&path);
    QPointF prev;
    while (it.hasSubpath()) {
        prev = it.nextSubpath();
        dashPath.moveTo(prev);

        pos = 0;
        idash = 0;
        doffset = 0;
        estart = 0;

        while (it.hasNext()) {
            QPainterPath::Element e = it.next();

            Q_ASSERT(e.isLineTo());
            cline = QLineF(prev, e);
            elen = cline.length();

            estop = estart + elen;

            // Dash away...
            while (pos < estop) {
                QPointF p2;

                int idash_incr = 0;
                qreal dpos = pos + dashes[idash] - doffset - estart;

                Q_ASSERT(dpos >= 0);

                if (dpos > elen) { // dash extends this line
                    doffset = dashes[idash] - (dpos - elen); // subtract the part already used
                    pos = estop; // move pos to next path element
                    p2 = cline.p2();
                } else { // Dash is on this line
                    p2 = cline.pointAt(dpos/elen);
                    pos = dpos + estart;
                    idash_incr = 1;
                    doffset = 0; // full segment so no offset on next.
                }

                if (idash % 2 == 0) {
                    dashPath.lineTo(p2);
                } else {
                    dashPath.moveTo(p2);
                }

                idash = (idash + idash_incr) % dashCount;
            }

            // Shuffle to the next cycle...
            estart = estop;
            prev = e;
        }
    }
    return dashPath;
}
