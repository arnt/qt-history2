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

#include <private/qbezier_p.h>
#include <private/qfontengine_p.h>
#include <private/qobject_p.h>
#include <private/qtextengine_p.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qlist.h>
#include <qmatrix.h>
#include <qpen.h>
#include <qpolygon.h>
#include <qtextlayout.h>
#include <qvarlengtharray.h>

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

//#define QPP_DEBUG
//#define QPP_STROKE_DEBUG
//#define QPP_FILLPOLYGONS_DEBUG

#define d d_func()
#define q q_func()

void qt_find_ellipse_coords(const QRectF &r, float angle, float length,
                            QPointF* startPoint, QPointF *endPoint)
{
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
    float a = r.width() / 2.0;
    float b = r.height() / 2.0;

    if (startPoint) {
        *startPoint = r.center()
                      + QPointF(a * cos(ANGLE(angle)), -b * sin(ANGLE(angle)));
    }
    if (endPoint) {
        *endPoint = r.center()
                    + QPointF(a * cos(ANGLE(angle + length)), -b * sin(ANGLE(angle + length)));
    }
}

#if defined (QPP_DEBUG) || defined (QPP_STROKE_DEBUG)
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

#if 0
void QPainterSubpath::removeBrokenSegments()
{
    if (brokenSegments.isEmpty())
        return;

#define REMOVE_BROKEN_DEBUG
// #define QT_PATH_NEAR 7
//     printf("QPainterSubpath::removeBrokenSegments()\n");
//     QVarLengthArray<QLineF, QT_PATH_NEAR> clSegs;
    QPointF isectPt;
    QPointF currPt;

//     int checks = 0;
    for (int i=0; i<brokenSegments.size(); ++i) {
        int bseg = brokenSegments.at(i);
//         qDebug() << " -> segment at" << bseg << (bseg == 1
//                                                       ? startPoint
//                                                       : elements.at(bseg-1).end());

//         // Try to match the segments close to us first and build up
//         // a list of them.
//         int start = bseg - QT_PATH_NEAR;
//         if (start < 0) start = 0;
//         for (int j=start; j<bseg; ++j) {
//             Q_ASSERT(elements.at(j).type == QPainterPathElement::Line);
//             clSegs[j-start] = QLineF(elements.at(j).lineData.x, elements.at(j).lineData.y,
//                                      elements.at(j+1).lineData.x, elements.at(j+1).lineData.y);
//         }
        bool isectFound = false;
        int elmiBefore = -1;
        int elmiAfter = -1;
        for (int elmi=bseg; elmi<elements.size() && !isectFound; ++elmi) {
            QLineF l(elements.at(elmi-1).end(), elements.at(elmi).end());
//              qDebug() << "  -> checking line" << l;

            currPt = startPoint;
            for (int k=0; k<bseg; ++k) {
                if (elements.at(k).type == QPainterPathElement::Line) {
                    QLineF l2(currPt, elements.at(k).end());
//                     qDebug() << "   -> vs line" << l2;
//                     ++checks;
                    if (l.intersect(l2, &isectPt) == QLineF::BoundedIntersection) {
                        isectFound = true;
                        elmiBefore = k;
                        elmiAfter = elmi;
//                         qDebug() << "    -> intersection" << elmiBefore << elmiAfter;
                        break;
                    }

                } else {
//                     printf(" -------> curves not supported..\n");
                }
                currPt = elements.at(k).end();
            }
        }
        if (isectFound) {
            QPainterPathElement &eBefore = elements[elmiBefore];
            Q_ASSERT(eBefore.type == QPainterPathElement::Line);
            eBefore.lineData.x = isectPt.x();
            eBefore.lineData.y = isectPt.y();

            Q_ASSERT(elements.at(elmiAfter).type == QPainterPathElement::Line);

            // Remove the interfering elements.
            for (int del=0; del<elmiAfter - elmiBefore - 1; ++del) {
                Q_ASSERT(elements.size() >= elmiBefore);
                elements.removeAt(elmiBefore+1);
            }

            // Remove the brokenSegments in the interfering range.
            while (i+1 < brokenSegments.size() && brokenSegments.at(i+1) < elmiAfter) {
                brokenSegments.removeAt(i+1);
            }
        }
    }
//     printf("=== total checks: %d\n", checks);

    brokenSegments.clear();
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
    \l QPainter::drawPath().

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
    \i \code
    QPainterPath path;
    path.addRect(20, 20, 80, 80);

    path.moveTo(0, 0);
    path.curveTo(99, 0,  50, 50,  99, 99);
    path.curveTo(0, 99,  50, 50,  0, 0);

    painter.drawPath(path);
    \endcode
    \endtable

    \sa QPainter
*/

/*!
    \enum QPainterPath::FillMode

    Specifies which method should be used to fill the path.

    \value OddEven Specifies that the region is filled using the odd
    even fill rule. With this rule, one determines wheter a point is
    inside the path as follows: Draw a horizontal line from the point
    to outside the path and count the number of intersections. If the
    number of intersections is an odd number the point is inside the
    path. This mode is the default.

    \value Winding Specifies that the region is filled using the non
    zero winding rule. With this rule, one determines wheter a point
    is inside the path as follows: Draw a horizontal line from the
    path to the outside of the path. Determine the direction of the
    path in each intersection point, up or down. The winding number is
    determined by summing the direction of each intersection. If the
    number is non zero, the point is inside the path. This fill mode
    can also in most cases be considered as the intersection of closed
    shapes.
*/

/*!
    \enum QPainterPath::ElementType
    \internal

    This enum describes the types of elements used to connect vertices
    in subpaths.

    \value MoveToElement      A new line begins at the element.
    \value LineToElement      A line is drawn to the element.
    \value CurveToElement     A curve is drawn to the element.
    \value CurveToDataElement Provides extra data required to draw a curve to
                              a \c CurveToElement element.
*/

/*!
    \class QPainterPath::Element
    \internal
*/

/*!
    \fn void QPainterPath::addEllipse(float x, float y, float width, float height)
    \overload

    Creates an ellipse within a bounding rectangle defined by its top-left
    corner at (\a x, \a y), \a width and \a height, and adds it to the
    painter path.

    If the current subpath is closed, a new subpath is started. The ellipse
    is clockwise starting and starting zero degrees.
*/

/*!
    \fn void QPainterPath::addText(float x, float y, const QFont &font, const QString &text)
    \overload

    Adds the given \a text to this path as a set of closed subpaths created
    from the \a font supplied. The subpaths are positioned so that the left
    end of the text's baseline lies at the point specified by (\a x, \a y).

    \sa QPainter::drawText
*/

/*!
    \fn int QPainterPath::elementCount() const

    Returns the number of path elements in the painter path.
*/

/*!
    \fn const QPainterPath::Element &QPainterPath::elementAt(int index) const

    Returns the element at the given \a index in the painter path.
*/

/*!
    \fn QPainterPath &QPainterPath::operator +=(const QPainterPath &other)

    Appends the \a other painter path to this painter path and returns a
    reference to the result.
*/

/*!
 Constructs a new empty QPainterPath.
 */
QPainterPath::QPainterPath()
    : d_ptr(new QPainterPathPrivate(this))
{
    Element e = { 0, 0, MoveToElement };
    elements << e;
}

/*!
    Creates a new painter path that is a copy of the \a other painter
    path.
*/
QPainterPath::QPainterPath(const QPainterPath &other)
    : d_ptr(new QPainterPathPrivate(*other.d_ptr)), elements(other.elements)
{
    Q_ASSERT(!elements.isEmpty());
    d_ptr->q_ptr = this;
}

/*!
    Creates a new painter path with \a startPoint as starting poing
*/

QPainterPath::QPainterPath(const QPointF &startPoint)
    : d_ptr(new QPainterPathPrivate(this))
{
    Element e = { startPoint.x(), startPoint.y(), MoveToElement };
    elements << e;
}

/*!
    Assigns the \a other painter path to this painter path.
*/
QPainterPath &QPainterPath::operator=(const QPainterPath &other)
{
    Q_ASSERT(!other.elements.isEmpty());
    *d_ptr = *other.d_ptr;
    elements = other.elements;
    return *this;
}

/*!
    Destroys the painter path.
*/
QPainterPath::~QPainterPath()
{
    delete d;
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

    d->close();
    moveTo(QPointF(0, 0));
}

/*!
    \fn void QPainterPath::moveTo(float x, float y)

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

    Q_ASSERT(!elements.isEmpty());
    if (elements.last().type == MoveToElement) {
        elements.last().x = p.x();
        elements.last().y = p.y();
    } else {
        Element elm = { p.x(), p.y(), MoveToElement };
        elements.append(elm);
    }
    d->cStart = elements.size() - 1;
}

/*!
    \fn void QPainterPath::lineTo(float x, float y)

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
    Q_ASSERT(!elements.isEmpty());
    Element elm = { p.x(), p.y(), LineToElement };
    elements.append(elm);
}

/*!
    \fn void QPainterPath::curveTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x,
                                   float ctrlPt2y, float endPtx, float endPty);

    \overload

    Adds a Bezier curve between the current point and the endpoint
    (\a{endPtx}, \a{endPty}) with control points specified by
    (\a{ctrlPt1x}, \a{ctrlPt1y}) and (\a{ctrlPt2x}, \a{ctrlPt2y}).
    After the curve is added, the current point is updated to be at
    the end point of the curve.
*/

/*!
    \fn void QPainterPath::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &endPoint)

    Adds a Bezier curve between the current point and \a endPoint with control
    points specified by \a c1, and \a c2. After the curve is added, the current
    point is updated to be at the end point of the curve.
*/
void QPainterPath::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &e)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::curveTo() (%.2f,%.2f), (%.2f,%.2f), (%.2f,%.2f)\n",
           c1.x(), c1.y(), c2.x(), c2.y(), e.x(), e.y());
#endif
    Q_ASSERT(!elements.isEmpty());
    Element ce1 = { c1.x(), c1.y(), CurveToElement };
    Element ce2 = { c2.x(), c2.y(), CurveToDataElement };
    Element ee = { e.x(), e.y(), CurveToDataElement };
    elements << ce1 << ce2 << ee;
}

/*!
    \fn void QPainterPath::arcTo(float x, float y, float width, float
    height, float startAngle, float sweepLength)

    \overload

    The arc's lies within the rectangle given by the point (\a{x}, \a{y}),
    \a width and \a height, beginning at \a startAngle and extending
    \a sweepLength degrees anti-clockwise.
    Angles are specified in degrees. This function connects the current point
    to the starting point of the arc if they are not already connected.

    \sa QPainter::drawArc
*/

/*!
    \fn void QPainterPath::arcTo(const QRectF &rectangle, float startAngle, float sweepLength)

    Creates an arc that occupies the given \a rectangle, beginning at
    \a startAngle and extending \a sweepLength degrees anti-clockwise.
    Angles are specified in degrees. This function connects the current point
    to the starting point of the arc if they are not already connected.

    \sa QPainter::drawArc
*/
void QPainterPath::arcTo(const QRectF &rect, float startAngle, float sweepLength)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::arcTo() (%.2f, %.2f, %.2f, %.2f, angle=%.2f, sweep=%.2f\n",
           rect.x(), rect.y(), rect.width(), rect.height(), startAngle, sweepLength);
#endif
    Q_ASSERT(!elements.isEmpty());
//     printf("   -> arcTo: rect=(%.1f,%.1f,%.1f,%.1f), angle=%.1f, len=%.1f\n",
//            rect.x(), rect.y(), rect.width(), rect.height(),
//            startAngle, sweepLength);
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
#define SIGN(t) (t > 0 ? 1 : -1)
    float a = rect.width() / 2.0;
    float b = rect.height() / 2.0;

    float absSweepLength = (sweepLength < 0 ? -sweepLength : sweepLength);
    int iterations = int((absSweepLength + 89) / 90);
    float clength = sweepLength / iterations;
    float cosangle1, sinangle1, cosangle2, sinangle2;
    for (int i=0; i<iterations; ++i) {
        float cangle = startAngle + i * clength;

        cosangle1 = cos(ANGLE(cangle));
        sinangle1 = sin(ANGLE(cangle));
        cosangle2 = cos(ANGLE(cangle + clength));
        sinangle2 = sin(ANGLE(cangle + clength));

        // Find the start and end point of the curve.
        QPointF startPoint = rect.center() + QPointF(a * cosangle1, -b * sinangle1);
        QPointF endPoint = rect.center() + QPointF(a * cosangle2, -b * sinangle2);

        // The derived at the start and end point.
        float sdx = -a * sinangle1;
        float sdy = -b * cosangle1;
        float edx = -a * sinangle2;
        float edy = -b * cosangle2;

        // Creating the tangent lines. We need to reverse their direction if the
        // sweep is negative (clockwise)
        QLineF controlLine1(startPoint, startPoint + SIGN(sweepLength) * QPointF(sdx, sdy));
        QLineF controlLine2(endPoint, endPoint - SIGN(sweepLength) * QPointF(edx, edy));

        // Adjust their length to fit the magic KAPPA length.
        controlLine1.setLength(controlLine1.length() * KAPPA);
        controlLine2.setLength(controlLine2.length() * KAPPA);

        if (startPoint != QPointF(elements.last().x, elements.last().y))
            lineTo(startPoint);
        curveTo(controlLine1.end(), controlLine2.end(), endPoint);
    }
}

/*!
    \fn void QPainterPath::addRect(float x, float y, float width, float height)

    \overload

    Adds a rectangle at position (\a{x}, \a{y}), with the given \a width and
    \a height. The rectangle is added as a clockwise set of lines. An empty
    subpath with current position at (0, 0) is in use after this function
    returns.
*/

/*!
    \fn void QPainterPath::addRect(const QRectF &rectangle)

    Adds the \a rectangle to this path as a closed subpath. The rectangle
    is added as a clockwise set of lines. An empty subpath with current
    position at (0, 0) is in use after this function returns. The rectangle
    is oriented clockwise starting at topleft.
*/
void QPainterPath::addRect(const QRectF &r)
{
    if (d->isClosed())
        moveTo(r.topLeft());
    else
        lineTo(r.topLeft());
    lineTo(r.topRight());
    lineTo(r.bottomRight());
    lineTo(r.bottomLeft());
    lineTo(r.topLeft());
}

/*!
    Adds the \a polygon to path as a new subpath. If the current
    subpath is closed, a new subpath is started at the polygons first
    point.
*/
void QPainterPath::addPolygon(const QPolygon &polygon)
{
    if (polygon.isEmpty())
        return;

    if (d->isClosed())
        moveTo(polygon.first());
    else
        lineTo(polygon.first());

    for (int i=1; i<polygon.size(); ++i) {
        Element elm = { polygon.at(i).x(), polygon.at(i).y(), LineToElement };
        elements << elm;
    }
}

/*!
    Creates an ellipse within the bounding rectangle specified by
    \a boundingRect and adds it to the painter path.

    If the current subpath is closed, a new subpath is started. The ellipse
    is clockwise starting and starting zero degrees.
*/
void QPainterPath::addEllipse(const QRectF &boundingRect)
{
    if (d->isClosed())
        moveTo(boundingRect.x() + boundingRect.width(), boundingRect.y() + boundingRect.height() / 2);
    else
        lineTo(boundingRect.x() + boundingRect.width(), boundingRect.y() + boundingRect.height() / 2);
    arcTo(boundingRect, 0, -360);
}

#undef d

/*!
    \fn void QPainterPath::addText(const QPointF &point, const QFont &font, const QString &text)

    Adds the given \a text to this path as a set of closed subpaths created
    from the \a font supplied. The subpaths are positioned so that the left
    end of the text's baseline lies at the \a point specified.

    \img qpainterpath-addtext.png

    \sa QPainter::drawText
*/
void QPainterPath::addText(const QPointF &point, const QFont &f, const QString &text)
{
    if (text.isEmpty())
        return;

    QTextLayout layout(text, f);
    QTextEngine *eng = layout.engine();
    eng->itemize(QTextEngine::SingleLine);

    QTextLine line = layout.createLine();
    line.layout(0x01000000);
    const QScriptLine &sl = eng->lines[0];
    if (!sl.length)
        return;

    int textFlags = 0;
    if (f.underline()) textFlags |= Qt::TextUnderline;
    if (f.overline()) textFlags |= Qt::TextOverline;
    if (f.strikeOut()) textFlags |= Qt::TextStrikeOut;

    int nItems = eng->items.size();

    float x(point.x());
    float y(point.y());

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<uchar> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->items[i].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i];
        QScriptItem &si = eng->items[item];

        if (!si.isTab && !si.isObject) {
            QGlyphLayout *glyphs = eng->glyphs(&si);

            QFontEngine *fe = f.d->engineForScript((QFont::Script)si.analysis.script);
            Q_ASSERT(fe);

            fe->addOutlineToPath(x, y, glyphs, si.num_glyphs, this);
        }
        x += si.width;
    }

}

#define d d_func()

/*!

  Adds the path \a other to this path. If the current subpath is
  path is unclosed, \a other is joined with it.
*/
void QPainterPath::addPath(const QPainterPath &other)
{
    if (other.isEmpty())
        return;

    // Locate where our own current subpath will start after the other path is added.
    int cStart = elements.size() + other.d->cStart;

    if (d->isClosed()) {
        // Remove last moveto so we don't get multiple moveto's
        if (elements.last().type == MoveToElement)
            elements.remove(elements.size()-1);
        Q_ASSERT(other.elements.first().type == MoveToElement);
        elements += other.elements;
    } else {
        int otherFirst = elements.size();
        elements += other.elements;
        // Since elements are to be connected, we replace the others first moveto with
        // a lineto
        elements[otherFirst].type = QPainterPath::LineToElement;
    }

    d->cStart = cStart;
}

/*!
    Returns the fill mode of the painter path. The default fill mode
    is OddEven.

    \sa FillMode setFillMode()
*/
QPainterPath::FillMode QPainterPath::fillMode() const
{
    return d->fillMode;
}

/*!
    \fn void QPainterPath::setFillMode(FillMode fillMode)

    Sets the fill mode of the painter path to \a fillMode.

    \sa FillMode, fillMode
*/
void QPainterPath::setFillMode(QPainterPath::FillMode fillMode)
{
    d->fillMode = fillMode;
}

/*!
    Returns the bounding rectangle of this painter path as a rectangle with
    floating point precision.
*/
QRectF QPainterPath::boundingRect() const
{
    return toFillPolygon().boundingRect();
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
    QPainterPath rev;

    if (isEmpty()) {
        rev = *this;
        return rev;
    }

    rev.moveTo(elements.at(elements.size()-1).x, elements.at(elements.size()-1).y);

    for (int i=elements.size()-1; i>=1; --i) {
        const QPainterPath::Element &elm = elements.at(i);
        const QPainterPath::Element &prev = elements.at(i-1);
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
                const QPainterPath::Element &cp1 = elements.at(i-2);
                const QPainterPath::Element &sp = elements.at(i-3);
                Q_ASSERT(prev.type == CurveToDataElement);
                Q_ASSERT(cp1.type == CurveToElement);
                rev.curveTo(prev.x, prev.y, cp1.x, cp1.y, sp.x, sp.y);
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
    Returns a list of polygons corresponding to the subpaths in the painter
    path.
*/
QList<QPolygon> QPainterPath::toSubpathPolygons() const
{
    QList<QPolygon> flatCurves;
    if (isEmpty())
        return flatCurves;

    QPolygon current;
    for (int i=0; i<elementCount(); ++i) {
        const QPainterPath::Element &e = elements.at(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (current.size() > 1)
                flatCurves += current;
            current.clear();
            current += QPointF(e.x, e.y);
            break;
        case QPainterPath::LineToElement:
            current += QPointF(e.x, e.y);
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(elements.at(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(elements.at(i+2).type == QPainterPath::CurveToDataElement);
            current += QBezier(elements.at(i-1).x, elements.at(i-1).y,
                               e.x, e.y,
                               elements.at(i+1).x, elements.at(i+1).y,
                               elements.at(i+2).x, elements.at(i+2).y).toPolygon();
            i+=2;
            break;
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
    Returns the painter path as a filled polygon.
*/
QPolygon QPainterPath::toFillPolygon() const
{
    QList<QPolygon> flats = toSubpathPolygons();
    QPolygon polygon;
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
  Creates a polygon from the path.
*/
QList<QPolygon> QPainterPath::toFillPolygons() const
{
    QList<QPolygon> polys;

    QList<QPolygon> subpaths = toSubpathPolygons();
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
            QPolygon buildUp = subpaths.at(i);
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

/*******************************************************************************
 * class QPainterPathStroker
 */
#define QT_PATH_NO_JOIN Qt::PenJoinStyle(0xffff)

class QPainterPathStrokerPrivate
{
    Q_DECLARE_PUBLIC(QPainterPathStroker)
public:
    enum JoinMethod { NormalJoin, NoJoin };

    QPainterPathStrokerPrivate() :
        width(1),
        offset(0.5),
        miterLimit(5),
        appliedMiterLimit(10),
        style(Qt::SolidLine),
        joinStyle(Qt::BevelJoin),
        capStyle(Qt::FlatCap)
    {
    }

    void strokeLine(const QLineF &line, QPainterPath *out, JoinMethod method) const;
    void strokeCurve(int elmi, const QPainterPath &src, QPainterPath *out) const;
    void joinPoints(const QLineF &nextLine, QPainterPath *path, JoinMethod method) const;

    float width;
    float offset;
    float miterLimit;
    float appliedMiterLimit;
    Qt::PenStyle style;
    Qt::PenJoinStyle joinStyle;
    Qt::PenCapStyle capStyle;
    QPainterPathStroker *q_ptr;
};

void QPainterPathStrokerPrivate::joinPoints(const QLineF &nextLine,
                                            QPainterPath *path,
                                            JoinMethod joinMethod) const
{
#ifdef QPP_STROKE_DEBUG
    printf(" -----> joinPoints: (%.2f, %.2f) (%.2f, %.2f)\n",
           nextLine.startX(), nextLine.startY(), nextLine.endX(), nextLine.endY());
#endif
    int elmCount = path->elementCount();
    Q_ASSERT(elmCount >= 2);
    QPainterPath::Element &back1 = path->elements[elmCount-1];
    const QPainterPath::Element &back2 = path->elementAt(elmCount-2);

    Q_ASSERT(back1.type == QPainterPath::LineToElement);

    // Check for overlap
    QLineF pline(back2.x, back2.y, back1.x, back1.y);
    QLineF bevelLine(back1.x, back1.y, nextLine.startX(), nextLine.startY());

    QPointF isect;
    QLineF::IntersectType type = pline.intersect(nextLine, &isect);
    if (type == QLineF::BoundedIntersection) {
        back1.x = isect.x();
        back1.y = isect.y();
    } else if (joinMethod != NoJoin) {
        Qt::PenJoinStyle jStyle = joinStyle;
        // Actually a broken segment. Will be removed later, so minimize overhead
        if (pline.angle(bevelLine) > 90)
            jStyle = Qt::BevelJoin;
        switch (jStyle) {
        case Qt::MiterJoin: {
            QLineF miterLine(QPointF(back1.x, back1.y), isect);
            if (miterLine.length() > appliedMiterLimit) {
                miterLine.setLength(appliedMiterLimit);
                back1.x = miterLine.endX();
                back1.y = miterLine.endY();
            } else {
                back1.x = isect.x();
                back1.y = isect.y();
            }
        }
            break;
        case Qt::BevelJoin:
            path->lineTo(nextLine.start());
            break;
        case Qt::RoundJoin: {
            QLineF cp1Line(pline.end(), isect);
            cp1Line.setLength(cp1Line.length() * KAPPA);
            QLineF cp2Line(nextLine.start(), isect);
            cp2Line.setLength(cp2Line.length() * KAPPA);
            path->curveTo(cp1Line.end(), cp2Line.end(), nextLine.start());
            break;
        }
        default:
            break;
        }
    }
}

void QPainterPathStrokerPrivate::strokeLine(const QLineF &line,
                                            QPainterPath *path,
                                            JoinMethod joinMethod) const
{
#ifdef QPP_STROKE_DEBUG
    static int count = 0;
    printf(" ---> strokeLine(%c): (%.2f, %.2f) (%.2f, %.2f)\n",
           (++count)%2 ? 'u' : 'd', line.startX(), line.startY(), line.endX(), line.endY());
#endif
    if (line.isNull())
        return;
    QLineF normal = line.normalVector();
    if (normal.isNull())
        return;
    normal.setLength(offset);
    QLineF ml(line);
    ml.moveBy(normal);

    if (!path->isEmpty()) {
        joinPoints(ml, path, joinMethod);
    } else {
        path->moveTo(ml.start());
    }
    path->lineTo(ml.end());
}

void QPainterPathStrokerPrivate::strokeCurve(int elmi,
                                             const QPainterPath &in,
                                             QPainterPath *path) const
{
    QBezier bezier(in.elementAt(elmi-1).x, in.elementAt(elmi-1).y,
                   in.elementAt(elmi).x,   in.elementAt(elmi).y,
                   in.elementAt(elmi+1).x, in.elementAt(elmi+1).y,
                   in.elementAt(elmi+2).x, in.elementAt(elmi+2).y);
    QPolygon segments = bezier.toPolygon();

    strokeLine(QLineF(segments.at(0), segments.at(1)), path, NormalJoin);
    for (int i=2; i<segments.size(); ++i)
        strokeLine(QLineF(segments.at(i-1), segments.at(i)), path, NoJoin);
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

QPolygon qt_reversed_polygon(const QPolygon &p)
{
    QPolygon rev;
    rev.reserve(p.size());
    for (int i=p.size()-1; i>=0; --i)
        rev << p.at(i);
    return rev;
}

/*!
  Creates a new stroke from the path \a input.
*/
QPainterPath QPainterPathStroker::createStroke(const QPainterPath &input) const
{
#ifdef QPP_STROKE_DEBUG
    printf("QPainterPathPrivate::createStroke()\n");
#endif

    QPainterPath stroke;

    QList<QPolygon> flatCurves = input.toSubpathPolygons();
    QPainterPath reverse = input.toReversed();

#ifdef QPP_STROKE_DEBUG
    printf(" -> path size: %d\n", input.elementCount());
    qt_debug_path(input);
    qt_debug_path(reverse);
#endif

    int pathSize = input.elementCount();
    for (int elmi = 1; elmi < pathSize; ++elmi) {
        QPainterPath usegs;
        QPainterPath dsegs;

        QPointF startPoint(input.elementAt(elmi-1).x, input.elementAt(elmi-1).y);

        for (; elmi<pathSize; ++elmi) {
            const QPainterPath::Element &elm = input.elementAt(elmi);
            Q_ASSERT(elm.type != QPainterPath::CurveToDataElement);
            if (elm.type == QPainterPath::MoveToElement) {
                break;
            } else if (elm.type == QPainterPath::LineToElement) {
                QLineF ffw(input.elementAt(elmi-1).x, input.elementAt(elmi-1).y, elm.x, elm.y);
                d->strokeLine(ffw, &usegs, QPainterPathStrokerPrivate::NormalJoin);
            } else if (elm.type == QPainterPath::CurveToElement) {
                d->strokeCurve(elmi, input, &usegs);
                elmi += 2;
            }
        }

        pathSize = reverse.elementCount();
        for (int revPos = pathSize - elmi + 1; revPos < pathSize; ++revPos) {
            const QPainterPath::Element &elm = reverse.elementAt(revPos);
            Q_ASSERT(elm.type != QPainterPath::CurveToDataElement);
            if (elm.type == QPainterPath::MoveToElement) {
                break;
            } else if (elm.type == QPainterPath::LineToElement) {
                QLineF rev(reverse.elementAt(revPos-1).x, reverse.elementAt(revPos-1).y,
                           reverse.elementAt(revPos).x, reverse.elementAt(revPos).y);
                d->strokeLine(rev, &dsegs, QPainterPathStrokerPrivate::NormalJoin);
            } else if (elm.type == QPainterPath::CurveToElement) {
                d->strokeCurve(revPos, reverse, &dsegs);
                revPos += 2;
            }
        }

#ifdef QPP_STROKE_DEBUG
        printf("Paths before closing:\n");
        qt_debug_path(usegs);
        qt_debug_path(dsegs);
#endif

        // Check if previous subpath was closed...
        bool closed = startPoint == QPointF(input.elementAt(elmi-1).x,
                                            input.elementAt(elmi-1).y);
        if (!closed) {
            // ### Cap styles...
            usegs.lineTo(QPointF(dsegs.elements.first().x, dsegs.elements.first().y));
            dsegs.lineTo(QPointF(usegs.elements.first().x, usegs.elements.first().y));
            stroke += usegs;
            stroke += dsegs;
        } else {
            QPointF ufirst(usegs.elements.first().x, usegs.elements.first().y);
            QPointF dfirst(dsegs.elements.first().x, dsegs.elements.first().y);
            if (!usegs.d_ptr->isClosed()) {
                usegs.lineTo(ufirst);
            }
            if (!dsegs.d_ptr->isClosed()) {
                dsegs.lineTo(dfirst);
            }

            stroke += usegs;
            stroke.closeSubpath();

            stroke += dsegs;
            stroke.closeSubpath();
        }

#ifdef QPP_STROKE_DEBUG
        printf("Path after joining\n");
        qt_debug_path(stroke);
#endif
    }

    stroke.setFillMode(QPainterPath::Winding);

#ifdef QPP_STROKE_DEBUG
    printf(" -> Final path:\n");
    qt_debug_path(stroke);
#endif

    return stroke;
}

void QPainterPathStroker::setWidth(float width)
{
    d->width = width;
    d->offset = width / 2;
    d->appliedMiterLimit = d->miterLimit * width;
}

float QPainterPathStroker::width() const
{
    return d->width;
}

void QPainterPathStroker::setStyle(Qt::PenStyle style)
{
    d->style = style;
}

Qt::PenStyle QPainterPathStroker::style() const
{
    return d->style;
}

void QPainterPathStroker::setCapStyle(Qt::PenCapStyle style)
{
    d->capStyle = style;
}

Qt::PenCapStyle QPainterPathStroker::capStyle() const
{
    return d->capStyle;
}

void QPainterPathStroker::setJoinStyle(Qt::PenJoinStyle style)
{
    d->joinStyle = style;
}

Qt::PenJoinStyle QPainterPathStroker::joinStyle() const
{
    return d->joinStyle;
}

void QPainterPathStroker::setMiterLimit(float limit)
{
    d->miterLimit = limit;
    d->appliedMiterLimit = d->miterLimit * d->width;
}

float QPainterPathStroker::miterLimit() const
{
    return d->miterLimit;
}
