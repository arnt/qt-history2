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

#include <qbitmap.h>
#include <qdebug.h>
#include <qiodevice.h>
#include <qlist.h>
#include <qmatrix.h>
#include <qpen.h>
#include <qpolygon.h>
#include <qtextlayout.h>
#include <qvarlengtharray.h>

#include <private/qbezier_p.h>
#include <private/qfontengine_p.h>
#include <private/qmath_p.h>
#include <private/qnumeric_p.h>
#include <private/qobject_p.h>
#include <private/qstroker_p.h>
#include <private/qtextengine_p.h>

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
    \ingroup multimedia

    \brief The QPainterPath class provides a container for painting operations,
    enabling graphical shapes to be constructed and reused.

    A painter path is an object composed of a number of graphical
    building blocks, such as rectangles, ellipses, lines, and curves.
    Building blocks can be joined in closed subpaths, for example as a
    rectangle or an ellipse. A closed path has coinciding start and
    end points. Or they can exist independently as unclosed subpaths,
    such as lines and curves.

    A QPainterPath object can be used for filling, outlining, and
    clipping. To generate fillable outlines for a given painter path,
    use the QPainterPathStroker class.  The main advantage of painter
    paths over normal drawing operations is that complex shapes only
    need to be created once; then they can be drawn many times using
    only calls to the QPainter::drawPath() function.

    QPainterPath provides a collection of functions that can be used
    to obtain information about the path and its elements. In addition
    it is possible to reverse the order of the elements using the
    toReversed() function. There are also several functions to convert
    this painter path object into a polygon representation.

    \tableofcontents

    \section1 Composing a QPainterPath

    A QPainterPath object can be constructed as an empty path, with a
    given start point, or as a copy of another QPainterPath object.
    Once created, lines and curves can be added to the path using the
    lineTo(), arcTo(), cubicTo() and quadTo() functions. The lines and
    curves stretch from the currentPosition() to the position passed
    as argument.

    The currentPosition() of the QPainterPath object is always the end
    position of the last subpath that was added (or the initial start
    point). Use the moveTo() function to move the currentPosition()
    without adding a component. The moveTo() function implicitly
    starts a new subpath, and closes the previous one.  Another way of
    starting a new subpath is to call the closeSubpath() function
    which closes the current path by adding a line from the
    currentPosition() back to the path's start position. Note that the
    new path will have (0, 0) as its initial currentPosition().

    QPainterPath class also provides several convenience functions to
    add closed subpaths to a painter path: addEllipse(), addPath(),
    addRect(), addRegion() and addText(). The addPolygon() function
    adds an \e unclosed subpath. In fact, these functions are all
    collections of moveTo(), lineTo() and cubicTo() operations.

    In addition, a path can be added to the current path using the
    connectPath() function. But note that this function will connect
    the last element of the current path to the first element of given
    one by adding a line.

    Below is a code snippet that shows how a QPainterPath object can
    be used:

    \table 100%
    \row
    \o \inlineimage qpainterpath-construction.png
    \o
    \code
        QPainterPath path;
        path.addRect(20, 20, 60, 60);

        path.moveTo(0, 0);
        path.cubicTo(99, 0,  50, 50,  99, 99);
        path.cubicTo(0, 99,  50, 50,  0, 0);

        QPainter painter(this);
        painter.fillRect(0, 0, 100, 100, Qt::white);
        painter.setPen(QPen(QColor(79, 106, 25), 1, Qt::SolidLine,
                            Qt::FlatCap, Qt::MiterJoin));
        painter.setBrush(QColor(122, 163, 39));

        painter.drawPath(path);
    \endcode
    \endtable

    The painter path is initially empty when constructed. We first add
    a rectangle, which is a closed subpath. Then we add two bezier
    curves which together form a closed subpath even though they are
    not closed individually. Finally we draw the entire path. The path
    is filled using the default fill rule, Qt::OddEvenFill. Qt
    provides two methods for filling paths:

    \table
    \row
    \o \inlineimage qt-fillrule-oddeven.png
    \o \inlineimage qt-fillrule-winding.png
    \header
    \o Qt::OddEvenFill
    \o Qt::WindingFill
    \endtable

    See the Qt::Fillrule documentation for the definition of the
    rules. A painter path's currently set fill rule can be retrieved
    using the fillRule() function, and altered using the setFillRule()
    function.

    \section1 QPainterPath Information

    The QPainterPath class provides a collection of functions that
    returns information about the path and its elements.

    The currentPosition() function returns the end point of the last
    subpath that was added (or the initial start point). The
    elementAt() function can be used to retrieve the various subpath
    elements, the \e number of elements can be retrieved using the
    elementCount() function, and the isEmpty() function tells whether
    this QPainterPath object contains any elements at all.

    The controlPointRect() function returns the rectangle containing
    all the points and control points in this path. This function is
    significantly faster to compute than the exact boundingRect()
    which returns the bounding rectangle of this painter path with
    floating point precision.

    Finally, QPainterPath provides the contains() function which can
    be used to determine whether a given point or rectangle is inside
    the path, and the intersects() function which determines if any of
    the points inside a given rectangle also are inside this path.

    \section1 QPainterPath Conversion

    For compatibilty reasons, it might be required to simplify the
    representation of a painter path: QPainterPath provides the
    toFillPolygon(), toFillPolygons() and toSubpathPolygons()
    functions which convert the painter path into a polygon. The
    toFillPolygon() returns the painter path as one single polygon,
    while the two latter functions return a list of polygons.

    The toFillPolygons() and toSubpathPolygons() functions are
    provided because it is usually faster to draw several small
    polygons than to draw one large polygon, even though the total
    number of points drawn is the same. The difference between the two
    is the \e number of polygons they return: The toSubpathPolygons()
    creates one polygon for each subpath regardless of intersecting
    subpaths (i.e. overlapping bounding rectangles), while the
    toFillPolygons() functions creates only one polygon for
    overlapping subpaths.

    The toFillPolygon() and toFillPolygons() functions first convert
    all the subpaths to polygons, then uses a rewinding technique to
    make sure that overlapping subpaths can be filled using the
    correct fill rule. Note that rewinding inserts additional lines in
    the polygon so the outline of the fill polygon does not match the
    outline of the path.

    \section1 Examples

    Qt provides the \l {painting/painterpaths}{Painter Paths Example}
    and the \l {demos/deform}{Vector Deformation Demo} which are
    located in Qt's example and demo directories respectively.

    The \l {painting/painterpaths}{Painter Paths Example} shows how
    painter paths can be used to build complex shapes for rendering
    and lets the user experiment with the filling and stroking.  The
    \l {demos/deform}{Vector Deformation Demo} shows how to use
    QPainterPath to draw text.

    \table
    \row
    \o \inlineimage qpainterpath-example.png
    \o \inlineimage qpainterpath-demo.png
    \header
    \o \l {painting/painterpaths}{Painter Paths Example}
    \o \l {demos/deform}{Vector Deformation Demo}
    \endtable

    \sa QPainterPathStroker, QPainter, QRegion
*/

/*!
    \enum QPainterPath::ElementType

    This enum describes the types of elements used to connect vertices
    in subpaths.

    Note that elements added as closed subpaths using the
    addEllipse(), addPath(), addPolygon(), addRect(), addRegion() and
    addText() convenience functions, is actually added to the path as
    a collection of separate elements using the moveTo(), lineTo() and
    cubicTo() functions.

    \value MoveToElement          A new subpath. See also moveTo().
    \value LineToElement            A line. See also lineTo().
    \value CurveToElement         A curve. See also cubicTo() and quadTo().
    \value CurveToDataElement  The extra data required to describe a curve in
                                               a CurveToElement element.

    \sa elementAt(),  elementCount()
*/

/*!
    \class QPainterPath::Element

    \brief The QPainterPath::Element class specifies the position and
    type of a subpath.

    Once a QPainterPath object is constructed, subpaths like lines and
    curves can be added to the path (creating
    QPainterPath::LineToElement and QPainterPath::CurveToElement
    components).

    The lines and curves stretch from the currentPosition() to the
    position passed as argument. The currentPosition() of the
    QPainterPath object is always the end position of the last subpath
    that was added (or the initial start point). The moveTo() function
    can be used to move the currentPosition() without adding a line or
    curve, creating a QPainterPath::MoveToElement component.

    \sa QPainterPath
*/

/*!
    \variable QPainterPath::Element::x
    \brief the x coordinate of the element's position.

    \sa {operator QPointF()}
*/

/*!
    \variable QPainterPath::Element::y
    \brief the y coordinate of the element's position.

    \sa {operator QPointF()}
*/

/*!
    \variable QPainterPath::Element::type
    \brief the type of element

    \sa isCurveTo(), isLineTo(), isMoveTo()
*/

/*!
    \fn bool QPainterPath::Element::operator== ( const Element & e ) const
    \internal
*/

/*!
    \fn bool QPainterPath::Element::isCurveTo () const

    Returns true if the element is a curve, otherwise returns false.

    \sa type, QPainterPath::CurveToElement
*/

/*!
    \fn bool QPainterPath::Element::isLineTo () const

    Returns true if the element is a line, otherwise returns false.

    \sa type, QPainterPath::LineToElement
*/

/*!
    \fn bool QPainterPath::Element::isMoveTo () const

    Returns true if the element is moving the current position,
    otherwise returns false.

    \sa type, QPainterPath::MoveToElement
*/

/*!
    \fn QPainterPath::Element::operator QPointF () const

    Returns the element's position.

    \sa x, y
*/

/*!
    \fn void QPainterPath::addEllipse(qreal x, qreal y, qreal width, qreal height)
    \overload

    Creates an ellipse within the bounding rectangle defined by its top-left
    corner at (\a x, \a y), \a width and \a height, and adds it to the
    painter path as a closed subpath.
*/

/*!
    \fn void QPainterPath::addText(qreal x, qreal y, const QFont &font, const QString &text)
    \overload

    Adds the given \a text to this path as a set of closed subpaths created
    from the \a font supplied. The subpaths are positioned so that the left
    end of the text's baseline lies at the point specified by (\a x, \a y).
*/

/*!
    \fn int QPainterPath::elementCount() const

    Returns the number of path elements in the painter path.

    \sa ElementType, elementAt(), isEmpty()
*/

/*!
    \fn const QPainterPath::Element &QPainterPath::elementAt(int index) const

    Returns the element at the given \a index in the painter path.

    \sa ElementType, elementCount(), isEmpty()
*/

/*!
    \fn void QPainterPath::setElementPositionAt(int index, qreal x, qreal y)

    Sets the x and y coordinate of the element at index \a index to \a
    x and \a y.
*/

/*###
    \fn QPainterPath &QPainterPath::operator +=(const QPainterPath &other)

    Appends the \a other painter path to this painter path and returns a
    reference to the result.
*/

/*!
    Constructs an empty QPainterPath object.
*/
QPainterPath::QPainterPath()
    : d_ptr(0)
{
}

/*!
    \fn QPainterPath::QPainterPath(const QPainterPath &path)

    Creates a QPainterPath object that is a copy of the given \a path.

    \sa operator=()
*/
QPainterPath::QPainterPath(const QPainterPath &other)
    : d_ptr(other.d_ptr)
{
    if (d_func())
        d_func()->ref.ref();
}

/*!
    Creates a QPainterPath object with the given \a startPoint as its
    current position.
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
    \fn QPainterPath &QPainterPath::operator=(const QPainterPath &path)

    Assigns the given \a path to this painter path.

    \sa QPainterPath()
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
    Destroys this QPainterPath object.
*/
QPainterPath::~QPainterPath()
{
    if (d_func() && !d_func()->ref.deref())
        delete d_func();
}

/*!
    Closes the current subpath by drawing a line to the beginning of
    the subpath, automatically starting a new path. The current point
    of the new path is (0, 0).

    If the subpath does not contain any elements, this function does
    nothing.

    \sa moveTo(), {QPainterPath#Composing a QPainterPath}{Composing
    a QPainterPath}
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
}

/*!
    \fn void QPainterPath::moveTo(qreal x, qreal y)

    \overload

    Moves the current position to (\a{x}, \a{y}) and starts a new
    subpath, implicitly closing the previous path.
*/

/*!
    \fn void QPainterPath::moveTo(const QPointF &point)

    Moves the current point to the given \a point, implicitly starting
    a new subpath and closing the previous one.

    \sa closeSubpath(), {QPainterPath#Composing a
    QPainterPath}{Composing a QPainterPath}
*/
void QPainterPath::moveTo(const QPointF &p)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::moveTo() (%.2f,%.2f)\n", p.x(), p.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(p.x()) || qIsNan(p.y()))
        qWarning("QPainterPath::moveTo: Adding point where x or y is NaN, results are undefined");
#endif
    ensureData();
    detach();

    QPainterPathData *d = d_func();
    Q_ASSERT(!d->elements.isEmpty());

    d->require_moveTo = false;

    if (d->elements.last().type == MoveToElement) {
        d->elements.last().x = p.x();
        d->elements.last().y = p.y();
    } else {
        Element elm = { p.x(), p.y(), MoveToElement };
        d->elements.append(elm);
    }
    d->cStart = d->elements.size() - 1;
}

/*!
    \fn void QPainterPath::lineTo(qreal x, qreal y)

    \overload

    Draws a line from the current position to the point (\a{x},
    \a{y}).
*/

/*!
    \fn void QPainterPath::lineTo(const QPointF &endPoint)

    Adds a straight line from the current position to the given \a
    endPoint.  After the line is drawn, the current position is updated
    to be at the end point of the line.

    \sa addPolygon(), addRect(), {QPainterPath#Composing a
    QPainterPath}{Composing a QPainterPath}
 */
void QPainterPath::lineTo(const QPointF &p)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::lineTo() (%.2f,%.2f)\n", p.x(), p.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(p.x()) || qIsNan(p.y()))
        qWarning("QPainterPath::lineTo: Adding point where x or y is NaN, results are undefined");
#endif
    ensureData();
    detach();

    QPainterPathData *d = d_func();
    Q_ASSERT(!d->elements.isEmpty());
    d->maybeMoveTo();
    if (p == QPointF(d->elements.last()))
        return;
    Element elm = { p.x(), p.y(), LineToElement };
    d->elements.append(elm);
}

/*!
    \fn void QPainterPath::cubicTo(qreal c1X, qreal c1Y, qreal c2X,
    qreal c2Y, qreal endPointX, qreal endPointY);

    \overload

    Adds a cubic Bezier curve between the current position and the end
    point (\a{endPointX}, \a{endPointY}) with control points specified
    by (\a{c1X}, \a{c1Y}) and (\a{c2X}, \a{c2Y}).
*/

/*!
    \fn void QPainterPath::cubicTo(const QPointF &c1, const QPointF &c2, const QPointF &endPoint)

    Adds a cubic Bezier curve between the current position and the
    given \a endPoint using the control points specified by \a c1, and
    \a c2.

    After the curve is added, the current position is updated to be at
    the end point of the curve.

    \table 100%
    \row
    \o \inlineimage qpainterpath-cubicto.png
    \o
    \code
        QLinearGradient myGradient;
        QPen myPen;

        QPainterPath myPath;
        myPath.cubicto(c1, c2, endPoint);

        QPainter painter(this);
        painter.setBrush(myGradient);
        painter.setPen(myPen);
        painter.drawPath(myPath);
    \endcode
    \endtable

    \sa quadTo(), {QPainterPath#Composing a QPainterPath}{Composing
    a QPainterPath}
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
        qWarning("QPainterPath::cubicTo: Adding point where x or y is NaN, results are undefined");
#endif
    ensureData();
    detach();

    QPainterPathData *d = d_func();
    Q_ASSERT(!d->elements.isEmpty());


    // Abort on empty curve as a stroker cannot handle this and the
    // curve is irrelevant anyway.
    if (d->elements.last() == c1 && c1 == c2 && c2 == e)
        return;

    d->maybeMoveTo();

    Element ce1 = { c1.x(), c1.y(), CurveToElement };
    Element ce2 = { c2.x(), c2.y(), CurveToDataElement };
    Element ee = { e.x(), e.y(), CurveToDataElement };
    d->elements << ce1 << ce2 << ee;
}

/*!
    \fn void QPainterPath::quadTo(qreal cx, qreal cy, qreal endPointX, qreal endPointY);

    \overload

    Adds a quadratic Bezier curve between the current point and the endpoint
    (\a{endPointX}, \a{endPointY}) with the control point specified by
    (\a{cx}, \a{cy}).
*/

/*!
    \fn void QPainterPath::quadTo(const QPointF &c, const QPointF &endPoint)

    Adds a quadratic Bezier curve between the current position and the
    given \a endPoint with the control point specified by \a c.

    After the curve is added, the current point is updated to be at
    the end point of the curve.

    \sa cubicTo(), {QPainterPath#Composing a QPainterPath}{Composing a
    QPainterPath}
*/
void QPainterPath::quadTo(const QPointF &c, const QPointF &e)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::quadTo() (%.2f,%.2f), (%.2f,%.2f)\n",
           c.x(), c.y(), e.x(), e.y());
#endif
#ifndef QT_NO_DEBUG
    if (qIsNan(c.x()) || qIsNan(c.y()) || qIsNan(e.x()) || qIsNan(e.y()))
        qWarning("QPainterPath::quadTo: Adding point where x or y is NaN, results are undefined");
#endif
    ensureData();
    detach();

    Q_D(QPainterPath);
    Q_ASSERT(!d->elements.isEmpty());
    const QPainterPath::Element &elm = d->elements.at(elementCount()-1);
    QPointF prev(elm.x, elm.y);

    // Abort on empty curve as a stroker cannot handle this and the
    // curve is irrelevant anyway.
    if (prev == c && c == e)
        return;

    QPointF c1((prev.x() + 2*c.x()) / 3, (prev.y() + 2*c.y()) / 3);
    QPointF c2((e.x() + 2*c.x()) / 3, (e.y() + 2*c.y()) / 3);
    cubicTo(c1, c2, e);
}

/*!
    \fn void QPainterPath::arcTo(qreal x, qreal y, qreal width, qreal
    height, qreal startAngle, qreal sweepLength)

    \overload

    Creates an arc that occupies the rectangle QRectF(\a x, \a y, \a
    width, \a height), beginning at the specified \a startAngle and
    extending \a sweepLength degrees counter-clockwise.

*/

/*!
    \fn void QPainterPath::arcTo(const QRectF &rectangle, qreal startAngle, qreal sweepLength)

    Creates an arc that occupies the given \a rectangle, beginning at
    the specified \a startAngle and extending \a sweepLength degrees
    counter-clockwise.

    Angles are specified in degrees. Clockwise arcs can be specified
    using negative angles.

    Note that this function connects the starting point of the arc to
    the current position if they are not already connected. After the
    arc has been added, the current position is the last point in
    arc. To draw a line back to the first point, use the
    closeSubpath() function.

    \table 100%
    \row
    \o \inlineimage qpainterpath-arcto.png
    \o
    \code
        QLinearGradient myGradient;
        QPen myPen;

        QPointF center, startPoint;

        QPainterPath myPath;
        myPath.moveTo(center);
        myPath.arcTo(boundingRect, startAngle,
                     sweepLength);

        QPainter painter(this);
        painter.setBrush(myGradient);
        painter.setPen(myPen);
        painter.drawPath(myPath);
    \endcode
    \endtable

    \sa addEllipse(), QPainter::drawArc(), QPainter::drawPie(),
    {QPainterPath#Composing a QPainterPath}{Composing a
    QPainterPath}
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
        qWarning("QPainterPath::arcTo: Adding arc where a parameter is NaN, results are undefined");
#endif
    if (rect.isNull())
        return;

    ensureData();
    detach();

    int point_count;
    QPointF pts[12];
    QPointF curve_start = qt_curves_for_arc(rect, startAngle, sweepLength, pts, &point_count);

    lineTo(curve_start);
    for (int i=0; i<point_count; i+=3) {
        cubicTo(pts[i].x(), pts[i].y(),
                pts[i+1].x(), pts[i+1].y(),
                pts[i+2].x(), pts[i+2].y());
    }

}


/*!
    \fn QPointF QPainterPath::currentPosition() const

    Returns the current position of the path.
*/
QPointF QPainterPath::currentPosition() const
{
    return !d_ptr || d_func()->elements.isEmpty()
        ? QPointF()
        : QPointF(d_func()->elements.last().x, d_func()->elements.last().y);
}


/*!
    \fn void QPainterPath::addRect(qreal x, qreal y, qreal width, qreal height)

    \overload

    Adds a rectangle at position (\a{x}, \a{y}), with the given \a
    width and \a height, as a closed subpath.
*/

/*!
    \fn void QPainterPath::addRect(const QRectF &rectangle)

    Adds the given \a rectangle to this path as a closed subpath.

    The \a rectangle is added as a clockwise set of lines. The painter
    path's current position after the \a rectangle has been added is
    at the top-left corner of the rectangle.

    \table 100%
    \row
    \o \inlineimage qpainterpath-addrectangle.png
    \o
    \code
        QLinearGradient myGradient;
        QPen myPen;
        QRectF myRectangle;

        QPainterPath myPath;
        myPath.addRect(myRectangle);

        QPainter painter(this);
        painter.setBrush(myGradient);
        painter.setPen(myPen);
        painter.drawPath(myPath);
    \endcode
    \endtable

    \sa addRegion(), lineTo(), {QPainterPath#Composing a
    QPainterPath}{Composing a QPainterPath}
*/
void QPainterPath::addRect(const QRectF &r)
{
#ifndef QT_NO_DEBUG
    if (qIsNan(r.x()) || qIsNan(r.y()) || qIsNan(r.width()) || qIsNan(r.height()))
        qWarning("QPainterPath::addRect: Adding rect where a parameter is NaN, results are undefined");
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
    d_func()->require_moveTo = true;
}

/*!
    Adds the given \a polygon to the path as an (unclosed) subpath.

    Note that the current position after the polygon has been added,
    is the last point in \a polygon. To draw a line back to the first
    point, use the closeSubpath() function.

    \table 100%
    \row
    \o \inlineimage qpainterpath-addpolygon.png
    \o
    \code
        QLinearGradient myGradient;
        QPen myPen;
        QPolygonF myPolygon;

        QPainterPath myPath;
        myPath.addPolygon(myPolygon);

        QPainter painter(this);
        painter.setBrush(myGradient);
        painter.setPen(myPen);
        painter.drawPath(myPath);
    \endcode
    \endtable

    \sa lineTo(), {QPainterPath#Composing a QPainterPath}{Composing
    a QPainterPath}
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
    \fn void QPainterPath::addEllipse(const QRectF &boundingRectangle)

    Creates an ellipse within the the specified \a boundingRectangle
    and adds it to the painter path as a closed subpath.

    The ellipse is composed of a clockwise curve, starting and
    finishing at zero degrees (the 3 o'clock position).

    \table 100%
    \row
    \o \inlineimage qpainterpath-addellipse.png
    \o
    \code
        QLinearGradient myGradient;
        QPen myPen;
        QRectF boundingRectangle;

        QPainterPath myPath;
        myPath.addEllipse(boundingRectangle);

        QPainter painter(this);
        painter.setBrush(myGradient);
        painter.setPen(myPen);
        painter.drawPath(myPath);
    \endcode
    \endtable

    \sa arcTo(), QPainter::drawEllipse(), {QPainterPath#Composing a
    QPainterPath}{Composing a QPainterPath}
*/
void QPainterPath::addEllipse(const QRectF &boundingRect)
{
#ifndef QT_NO_DEBUG
    if (qIsNan(boundingRect.x()) || qIsNan(boundingRect.y())
        || qIsNan(boundingRect.width()) || qIsNan(boundingRect.height()))
        qWarning("QPainterPath::addEllipse: Adding ellipse where a parameter is NaN, results are undefined");
#endif
    if (boundingRect.isNull())
        return;

    ensureData();
    detach();

    Q_D(QPainterPath);
    d->elements.reserve(d->elements.size() + 13);

    QPointF pts[12];
    int point_count;
    QPointF start = qt_curves_for_arc(boundingRect, 0, 360, pts, &point_count);

    moveTo(start);
    cubicTo(pts[0], pts[1], pts[2]);           // 0 -> 270
    cubicTo(pts[3], pts[4], pts[5]);           // 270 -> 180
    cubicTo(pts[6], pts[7], pts[8]);           // 180 -> 90
    cubicTo(pts[9], pts[10], pts[11]);         // 90 - >0
    d_func()->require_moveTo = true;
}

/*!
    \fn void QPainterPath::addText(const QPointF &point, const QFont &font, const QString &text)

    Adds the given \a text to this path as a set of closed subpaths
    created from the \a font supplied. The subpaths are positioned so
    that the left end of the text's baseline lies at the specified \a
    point.

    \table 100%
    \row
    \o \inlineimage qpainterpath-addtext.png
    \o
    \code
        QLinearGradient myGradient;
        QPen myPen;
        QFont myFont;
        QPointF baseline(x, y);

        QPainterPath myPath;
        myPath.addText(baseline, myFont, tr("Qt"));

        QPainter painter(this);
        painter.setBrush(myGradient);
        painter.setPen(myPen);
        painter.drawPath(myPath);
    \endcode
    \endtable

    \sa QPainter::drawText(), {QPainterPath#Composing a
    QPainterPath}{Composing a QPainterPath}
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

            const qreal lw = fe->lineThickness().toReal();
            if (f.d->underline) {
                qreal pos = fe->underlinePosition().toReal();
                addRect(x, y + pos, si.width.toReal(), lw);
            }
            if (f.d->overline) {
                qreal pos = fe->ascent().toReal() + 1;
                addRect(x, y - pos, si.width.toReal(), lw);
            }
            if (f.d->strikeOut) {
                qreal pos = fe->ascent().toReal() / 3;
                addRect(x, y - pos, si.width.toReal(), lw);
            }
        }
        x += si.width.toReal();
    }
}

/*!
    \fn void QPainterPath::addPath(const QPainterPath &path)

    Adds the given \a path to \e this path as a closed subpath.

    \sa connectPath(), {QPainterPath#Composing a
    QPainterPath}{Composing a QPainterPath}
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

    d->require_moveTo = other.d_func()->isClosed();
}


/*!
    \fn void QPainterPath::connectPath(const QPainterPath &path)

    Connects the given \a path to \e this path by adding a line from the
    last element of this path to the first element of the given path.

    \sa addPath(), {QPainterPath#Composing a QPainterPath}{Composing
    a QPainterPath}
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
    Adds the given \a region to the path by adding each rectangle in
    the region as a separate closed subpath.

    \sa addRect(), {QPainterPath#Composing a QPainterPath}{Composing
    a QPainterPath}
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
    Returns the painter path's currently set fill rule.

    \sa setFillRule()
*/
Qt::FillRule QPainterPath::fillRule() const
{
    return isEmpty() ? Qt::OddEvenFill : d_func()->fillRule;
}

/*!
    \fn void QPainterPath::setFillRule(Qt::FillRule fillRule)

    Sets the fill rule of the painter path to the given \a
    fillRule. Qt provides two methods for filling paths:

    \table
    \row
    \o \inlineimage qt-fillrule-oddeven.png
    \o \inlineimage qt-fillrule-winding.png
    \header
    \o Qt::OddEvenFill (default)
    \o Qt::WindingFill
    \endtable

    \sa fillRule()
*/
void QPainterPath::setFillRule(Qt::FillRule fillRule)
{
    ensureData();
    detach();

    d_func()->fillRule = fillRule;
}

#define QT_BEZIER_A(bezier, coord) 3 * (-bezier.coord##1 \
                                        + 3*bezier.coord##2 \
                                        - 3*bezier.coord##3 \
                                        +bezier.coord##4)

#define QT_BEZIER_B(bezier, coord) 6 * (bezier.coord##1 \
                                        - 2*bezier.coord##2 \
                                        + bezier.coord##3)

#define QT_BEZIER_C(bezier, coord) 3 * (- bezier.coord##1 \
                                        + bezier.coord##2)

#define QT_BEZIER_CHECK_T(bezier, t) \
    if (t >= 0 && t <= 1) { \
        QPointF p(b.pointAt(t)); \
        if (p.x() < minx) minx = p.x(); \
        else if (p.x() > maxx) maxx = p.x(); \
        if (p.y() < miny) miny = p.y(); \
        else if (p.y() > maxy) maxy = p.y(); \
    }


static QRectF qt_painterpath_bezier_extrema(const QBezier &b)
{
    qreal minx, miny, maxx, maxy;

    // initialize with end points
    if (b.x1 < b.x4) {
        minx = b.x1;
        maxx = b.x4;
    } else {
        minx = b.x4;
        maxx = b.x1;
    }
    if (b.y1 < b.y4) {
        miny = b.y1;
        maxy = b.y4;
    } else {
        miny = b.y4;
        maxy = b.y1;
    }

    // Update for the X extrema
    {
        qreal ax = QT_BEZIER_A(b, x);
        qreal bx = QT_BEZIER_B(b, x);
        qreal cx = QT_BEZIER_C(b, x);
        // specialcase quadratic curves to avoid div by zero
        if (qFuzzyCompare(ax, 0)) {

            // linear curves are covered by initalization.
            if (!qFuzzyCompare(bx, 0)) {
                qreal t = -cx / bx;
                QT_BEZIER_CHECK_T(b, t);
            }

        } else {
            qreal t1 = (-bx + sqrt(bx * bx - 4 * ax * cx)) / (2 * ax);
            QT_BEZIER_CHECK_T(b, t1);

            qreal t2 = (-bx - sqrt(bx * bx - 4 * ax * cx)) / (2 * ax);
            QT_BEZIER_CHECK_T(b, t2);
        }
    }

    // Update for the Y extrema
    {
        qreal ay = QT_BEZIER_A(b, y);
        qreal by = QT_BEZIER_B(b, y);
        qreal cy = QT_BEZIER_C(b, y);

        // specialcase quadratic curves to avoid div by zero
        if (qFuzzyCompare(ay, 0)) {

            // linear curves are covered by initalization.
            if (!qFuzzyCompare(by, 0)) {
                qreal t = -cy / by;
                QT_BEZIER_CHECK_T(b, t);
            }

        } else {
            qreal t1 = (-by + sqrt(by * by - 4 * ay * cy)) / (2 * ay);
            QT_BEZIER_CHECK_T(b, t1);

            qreal t2 = (-by - sqrt(by * by - 4 * ay * cy)) / (2 * ay);
            QT_BEZIER_CHECK_T(b, t2);
        }
    }
    return QRectF(minx, miny, maxx - minx, maxy - miny);
}

/*!
    Returns the bounding rectangle of this painter path as a rectangle with
    floating point precision.

    \sa controlPointRect()
*/
QRectF QPainterPath::boundingRect() const
{
    Q_D(QPainterPath);
    if (isEmpty())
        return QRect();

    qreal minx, maxx, miny, maxy;
    minx = maxx = d->elements.at(0).x;
    miny = maxy = d->elements.at(0).y;
    for (int i=1; i<d->elements.size(); ++i) {
        const Element &e = d->elements.at(i);

        switch (e.type) {
        case MoveToElement:
        case LineToElement:
            if (e.x > maxx) maxx = e.x;
            else if (e.x < minx) minx = e.x;
            if (e.y > maxy) maxy = e.y;
            else if (e.y < miny) miny = e.y;
            break;
        case CurveToElement:
            {
                QBezier b = QBezier::fromPoints(d->elements.at(i-1),
                                                e,
                                                d->elements.at(i+1),
                                                d->elements.at(i+2));
                QRectF r = qt_painterpath_bezier_extrema(b);
                qreal right = r.right();
                qreal bottom = r.bottom();
                if (r.x() < minx) minx = r.x();
                if (right > maxx) maxx = right;
                if (r.y() < miny) miny = r.y();
                if (bottom > maxy) maxy = bottom;
                i += 2;
            }
            break;
        default:
            break;
        }
    }
    return QRectF(minx, miny, maxx - minx, maxy - miny);
}

/*!
    Returns the rectangle containing all the points and control points
    in this path.

    This function is significantly faster to compute than the exact
    boundingRect(), and the returned rectangle is always a superset of
    the rectangle returned by boundingRect().

    \sa boundingRect()
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

    Returns true if there are no elements in this path, otherwise
    returns false.

    \sa elementCount()
*/

/*!
    Creates and returns a reversed copy of the path.

    It is the order of the elements that is reversed: If a
    QPainterPath is composed by calling the moveTo(), lineTo() and
    cubicTo() functions in the specified order, the reversed copy is
    composed by calling cubicTo(), lineTo() and moveTo().
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
    Converts the path into a list of polygons using the given
    transformation \a matrix, and returns the list.

    This function creates one polygon for each subpath regardless of
    intersecting subpaths (i.e. overlapping bounding rectangles). To
    make sure that such overlapping subpaths are filled correctly, use
    the toFillPolygons() function instead.

    \sa toFillPolygons(), toFillPolygon(), {QPainterPath#QPainterPath
    Conversion}{QPainterPath Conversion}
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
            current.reserve(16);
            current += QPointF(e.x, e.y) * matrix;
            break;
        case QPainterPath::LineToElement:
            current += QPointF(e.x, e.y) * matrix;
            break;
        case QPainterPath::CurveToElement: {
            Q_ASSERT(d->elements.at(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(d->elements.at(i+2).type == QPainterPath::CurveToDataElement);
            QBezier bezier = QBezier::fromPoints(QPointF(d->elements.at(i-1).x, d->elements.at(i-1).y) * matrix,
                                       QPointF(e.x, e.y) * matrix,
                                       QPointF(d->elements.at(i+1).x, d->elements.at(i+1).y) * matrix,
                                                 QPointF(d->elements.at(i+2).x, d->elements.at(i+2).y) * matrix);
            bezier.addToPolygon(&current);
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
    Converts the path into a polygon using the given transformation \a
    matrix, and returns the polygon.

    The polygon is created by first converting all subpaths to
    polygons, then using a rewinding technique to make sure that
    overlapping subpaths can be filled using the correct fill rule.

    Note that rewinding inserts addition lines in the polygon so
    the outline of the fill polygon does not match the outline of
    the path.

    \sa toSubpathPolygons(), toFillPolygons(),
    {QPainterPath#QPainterPath Conversion}{QPainterPath Conversion}
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

static inline bool rect_intersects(const QRectF &r1, const QRectF &r2)
{
    return qMax(r1.left(), r2.left()) <= qMin(r1.right(), r2.right())
        && qMax(r1.top(), r2.top()) <= qMin(r1.bottom(), r2.bottom());
}
/*!
    Converts the path into a list of polygons using the given
    transformation \a matrix, and returns the list.

    The function differs from the toFillPolygon() function in that it
    creates several polygons. It is provided because it is usually
    faster to draw several small polygons than to draw one large
    polygon, even though the total number of points drawn is the same.

    The toFillPolygons() function differs from the toSubpathPolygons()
    function in that it create only polygon for subpaths that have
    overlapping bounding rectangles.

    Like the toFillPolygon() function, this function uses a rewinding
    technique to make sure that overlapping subpaths can be filled
    using the correct fill rule. Note that rewinding inserts addition
    lines in the polygons so the outline of the fill polygon does not
    match the outline of the path.

    \sa toSubpathPolygons(), toFillPolygon(),
    {QPainterPath#QPainterPath Conversion}{QPainterPath Conversion}
*/

// #define QPP_FILLPOLYGONS_DEBUG

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

#ifdef QPP_FILLPOLYGONS_DEBUG
    printf("QPainterPath::toFillPolygons, subpathCount=%d\n", count);
    for (int i=0; i<bounds.size(); ++i)
        qDebug() << " bounds" << i << bounds.at(i);
#endif

    QVector< QList<int> > isects;
    isects.resize(count);

    // find all intersections
    for (int j=0; j<count; ++j) {
        QRectF cbounds = bounds.at(j);
        for (int i=0; i<count; ++i) {
            if (rect_intersects(cbounds, bounds.at(i))) {
                isects[j] << i;
            }
        }
    }

#ifdef QPP_FILLPOLYGONS_DEBUG
    printf("Intersections before flattening:\n");
    for (int i = 0; i < count; ++i) {
        printf("%d: ", i);
        for (int j = 0; j < isects[i].size(); ++j) {
            printf("%d ", isects[i][j]);
        }
        printf("\n");
    }
#endif

    // flatten the sets of intersections
    for (int i=0; i<count; ++i) {
        const QList<int> &current_isects = isects.at(i);
        for (int j=0; j<current_isects.size(); ++j) {
            int isect_j = current_isects.at(j);
            if (isect_j == i)
                continue;
            for (int k=0; k<isects[isect_j].size(); ++k) {
                int isect_k = isects[isect_j][k];
                if (isect_k != i && !isects.at(i).contains(isect_k)) {
                    isects[i] += isect_k;
                }
            }
            isects[isect_j].clear();
        }
    }

#ifdef QPP_FILLPOLYGONS_DEBUG
    printf("Intersections after flattening:\n");
    for (int i = 0; i < count; ++i) {
        printf("%d: ", i);
        for (int j = 0; j < isects[i].size(); ++j) {
            printf("%d ", isects[i][j]);
        }
        printf("\n");
    }
#endif

    // Join the intersected subpaths as rewinded polygons
    for (int i=0; i<count; ++i) {
        const QList<int> &subpath_list = isects[i];
        if (!subpath_list.isEmpty()) {
            QPolygonF buildUp;
            for (int j=0; j<subpath_list.size(); ++j) {
                buildUp += subpaths.at(subpath_list.at(j));
                if (!buildUp.isClosed())
                    buildUp += buildUp.first();
            }
            polys += buildUp;
        }
    }

    return polys;
}

static void qt_painterpath_isect_line(const QPointF &p1, const QPointF &p2, const QPointF &pos,
                                      int *winding)
{
    qreal x1 = p1.x();
    qreal y1 = p1.y();
    qreal x2 = p2.x();
    qreal y2 = p2.y();
    qreal y = pos.y();

    int dir = 1;

    if (qFuzzyCompare(y1, y2)) {
        // ignore horizontal lines according to scan conversion rule
        return;
    } else if (y2 < y1) {
        qreal x_tmp = x2; x2 = x1; x1 = x_tmp;
        qreal y_tmp = y2; y2 = y1; y1 = y_tmp;
        dir = -1;
    }

    if (y >= y1 && y < y2) {
        qreal x = x1 + ((x2 - x1) / (y2 - y1)) * (y - y1);

        // count up the winding number if we're
        if (x<=pos.x()) {
            (*winding) += dir;
        }
    }
}

static void qt_painterpath_isect_curve(const QBezier &bezier, const QPointF &pt,
                                       int *winding)
{
    qreal y = pt.y();
    qreal x = pt.x();
    QRectF bounds = bezier.bounds();

    // potential intersection, divide and try again..
    if (y >= bounds.y() && y < bounds.y() + bounds.height()) {

        // hit lower limit... This is a rough threshold, but its a
        // tradeoff between speed and precision.
        const qreal lower_bound = .01;
        if (bounds.width() < lower_bound && bounds.height() < lower_bound) {
            // We make the assumption here that the curve starts to
            // approximate a line after while (i.e. that it doesn't
            // change direction drastically during its slope)
            if (bezier.pt1().x() <= x) {
                (*winding) += (bezier.pt2().y() > bezier.pt1().y() ? 1 : -1);
            }
            return;
        }

        // split curve and try again...
        QBezier first_half, second_half;
        bezier.split(&first_half, &second_half);
        qt_painterpath_isect_curve(first_half, pt, winding);
        qt_painterpath_isect_curve(second_half, pt, winding);
    }
}

/*!
    \fn bool QPainterPath::contains(const QPointF &point) const

    Returns true if the given \a point is inside the path, otherwise
    returns false.

    \sa intersects()
*/
bool QPainterPath::contains(const QPointF &pt) const
{
    if (isEmpty())
        return false;

    QPainterPathData *d = d_func();

    int winding_number = 0;

    QPointF last_pt;
    QPointF last_start;
    for (int i=0; i<d->elements.size(); ++i) {
        const Element &e = d->elements.at(i);

        switch (e.type) {

        case MoveToElement:
            if (i > 0) // implicitly close all paths.
                qt_painterpath_isect_line(last_pt, last_start, pt, &winding_number);
            last_start = last_pt = e;
            break;

        case LineToElement:
            qt_painterpath_isect_line(last_pt, e, pt, &winding_number);
            last_pt = e;
            break;

        case CurveToElement:
            {
                const QPainterPath::Element &cp2 = d->elements.at(++i);
                const QPainterPath::Element &ep = d->elements.at(++i);
                qt_painterpath_isect_curve(QBezier::fromPoints(last_pt, e, cp2, ep),
                                           pt, &winding_number);
                last_pt = ep;

            }
            break;

        default:
            break;
        }
    }

    // implicitly close last subpath
    if (last_pt != last_start)
        qt_painterpath_isect_line(last_pt, last_start, pt, &winding_number);

    return (d->fillRule == Qt::WindingFill
            ? (winding_number != 0)
            : ((winding_number % 2) != 0));
}

static bool qt_painterpath_isect_line_rect(qreal x1, qreal y1, qreal x2, qreal y2,
                                           const QRectF &rect)
{
    qreal left = rect.left();
    qreal right = rect.right();
    qreal top = rect.top();
    qreal bottom = rect.bottom();

    enum { Left, Right, Top, Bottom };
    // clip the lines, after cohen-sutherland, see e.g. http://www.nondot.org/~sabre/graphpro/line6.html
    int p1 = ((x1 < left) << Left)
             | ((x1 > right) << Right)
             | ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
    int p2 = ((x2 < left) << Left)
             | ((x2 > right) << Right)
             | ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);

    if (p1 & p2)
        // completely inside
        return false;

    if (p1 | p2) {
        qreal dx = x2 - x1;
        qreal dy = y2 - y1;

        // clip x coordinates
        if (x1 < left) {
            y1 += dy/dx * (left - x1);
            x1 = left;
        } else if (x1 > right) {
            y1 -= dy/dx * (x1 - right);
            x1 = right;
        }
        if (x2 < left) {
            y2 += dy/dx * (left - x2);
            x2 = left;
        } else if (x2 > right) {
            y2 -= dy/dx * (x2 - right);
            x2 = right;
        }

        p1 = ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
        p2 = ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);

        if (p1 & p2)
            return false;

        // clip y coordinates
        if (y1 < top) {
            x1 += dx/dy * (top - y1);
            y1 = top;
        } else if (y1 > bottom) {
            x1 -= dx/dy * (y1 - bottom);
            y1 = bottom;
        }
        if (y2 < top) {
            x2 += dx/dy * (top - y2);
            y2 = top;
        } else if (y2 > bottom) {
            x2 -= dx/dy * (y2 - bottom);
            y2 = bottom;
        }

        p1 = ((x1 < left) << Left)
             | ((x1 > right) << Right);
        p2 = ((x2 < left) << Left)
             | ((x2 > right) << Right);

        if (p1 & p2)
            return false;

        return true;
    }
    return false;
}

static bool qt_isect_curve_horizontal(const QBezier &bezier, qreal y, qreal x1, qreal x2)
{
    QRectF bounds = bezier.bounds();

    if (y >= bounds.top() && y < bounds.bottom()
        && bounds.right() >= x1 && bounds.left() < x2) {
        const qreal lower_bound = .01;
        if (bounds.width() < lower_bound && bounds.height() < lower_bound)
            return true;

        QBezier first_half, second_half;
        bezier.split(&first_half, &second_half);
        qreal midpoint = x1 + (x2 - x1) / 2;
        if (qt_isect_curve_horizontal(first_half, y, x1, midpoint)
            || qt_isect_curve_horizontal(first_half, y, midpoint, x2)
            || qt_isect_curve_horizontal(second_half, y, x1, midpoint)
            || qt_isect_curve_horizontal(second_half, y, midpoint, x2))
            return true;
    }
    return false;
}

static bool qt_isect_curve_vertical(const QBezier &bezier, qreal x, qreal y1, qreal y2)
{
    QRectF bounds = bezier.bounds();

    if (x >= bounds.left() && x < bounds.right()
        && bounds.top() >= y1 && bounds.bottom() < y2) {
        const qreal lower_bound = .01;
        if (bounds.width() < lower_bound && bounds.height() < lower_bound)
            return true;

        QBezier first_half, second_half;
        bezier.split(&first_half, &second_half);
        qreal midpoint = y1 + (y2 - y1) / 2;
        if (qt_isect_curve_horizontal(first_half, x, y1, midpoint)
            || qt_isect_curve_horizontal(first_half, x, midpoint, y2)
            || qt_isect_curve_horizontal(second_half, x, y1, midpoint)
            || qt_isect_curve_horizontal(second_half, x, midpoint, y2))
            return true;
    }
    return false;
}

/*
    Returns true if any lines or curves cross the four edges in of rect
*/
static bool qt_painterpath_check_crossing(const QPainterPath *path, const QRectF &rect)
{
    QPointF last_pt;
    QPointF last_start;
    for (int i=0; i<path->elementCount(); ++i) {
        const QPainterPath::Element &e = path->elementAt(i);

        switch (e.type) {

        case QPainterPath::MoveToElement:
            if (i > 0
                && qFuzzyCompare(last_pt.x(), last_start.y())
                && qFuzzyCompare(last_pt.y(), last_start.y())
                && qt_painterpath_isect_line_rect(last_pt.x(), last_pt.y(),
                                                  last_start.x(), last_start.y(), rect))
                return true;
            last_start = last_pt = e;
            break;

        case QPainterPath::LineToElement:
            if (qt_painterpath_isect_line_rect(last_pt.x(), last_pt.y(), e.x, e.y, rect))
                return true;
            last_pt = e;
            break;

        case QPainterPath::CurveToElement:
            {
                QPointF cp2 = path->elementAt(++i);
                QPointF ep = path->elementAt(++i);
                QBezier bezier = QBezier::fromPoints(last_pt, e, cp2, ep);
                if (qt_isect_curve_horizontal(bezier, rect.top(), rect.left(), rect.right())
                    || qt_isect_curve_horizontal(bezier, rect.bottom(), rect.left(), rect.right())
                    || qt_isect_curve_vertical(bezier, rect.left(), rect.top(), rect.bottom())
                    || qt_isect_curve_vertical(bezier, rect.right(), rect.top(), rect.bottom()))
                    return true;
                last_pt = ep;
            }
            break;

        default:
            break;
        }
    }

    // implicitly close last subpath
    if (last_pt != last_start
        && qt_painterpath_isect_line_rect(last_pt.x(), last_pt.y(),
                                          last_start.x(), last_start.y(), rect))
        return true;

    return false;
}

/*!
    \fn bool QPainterPath::intersects(const QRectF &rectangle) const

    Returns true if any point in the given \a rectangle is inside the
    path; otherwise returns false.

    \sa contains()
*/
bool QPainterPath::intersects(const QRectF &rect) const
{
    if (isEmpty() || !controlPointRect().intersects(rect))
        return false;

    // If any path element cross the rect its bound to be an intersection
    if (qt_painterpath_check_crossing(this, rect))
        return true;

    if (contains(rect.center()))
        return true;

    Q_D(QPainterPath);

    // Check if the rectangle surounds any subpath...
    for (int i=0; i<d->elements.size(); ++i) {
        const Element &e = d->elements.at(i);
        if (e.type == QPainterPath::MoveToElement && rect.contains(e))
            return true;
    }

    return false;
}



/*!
    \fn bool QPainterPath::contains(const QRectF &rectangle) const
    \overload

    Returns true if the given \a rectangle is inside the path,
    otherwise returns false.
*/
bool QPainterPath::contains(const QRectF &rect) const
{
    Q_D(QPainterPath);

    // the path is empty or the control point rect doesn't completly
    // cover the rectangle we abort stratight away.
    if (isEmpty() || !controlPointRect().contains(rect))
        return false;

    // if there are intersections, chances are that the rect is not
    // contained, except if we have winding rule, in which case it
    // still might.
    if (qt_painterpath_check_crossing(this, rect)) {
        if (fillRule() == Qt::OddEvenFill) {
            return false;
        } else {
            // Do some wague sampling in the winding case. This is not
            // precise but it should mostly be good enough.
            if (!contains(rect.topLeft()) ||
                !contains(rect.topRight()) ||
                !contains(rect.bottomRight()) ||
                !contains(rect.bottomLeft()))
                return false;
        }
    }

    // If there exists a point inside that is not part of the path its
    // because: rectangle lies completly outside path or a subpath
    // excludes parts of the rectangle. Both cases mean that the rect
    // is not contained
    if (!contains(rect.center()))
        return false;

    // If there are any subpaths inside this rectangle we need to
    // check if they are still contained as a result of the fill
    // rule. This can only be the case for WindingFill though. For
    // OddEvenFill the rect will never be contained if it surrounds a
    // subpath. (the case where two subpaths are completly identical
    // can be argued but we choose to neglect it).
    for (int i=0; i<d->elements.size(); ++i) {
        const Element &e = d->elements.at(i);
        if (e.type == QPainterPath::MoveToElement && rect.contains(e)) {
            if (fillRule() == Qt::OddEvenFill)
                return false;

            bool stop = false;
            for (; !stop && i<d->elements.size(); ++i) {
                const Element &el = d->elements.at(i);
                switch (el.type) {
                case MoveToElement:
                    stop = true;
                    break;
                case LineToElement:
                    if (!contains(el))
                        return false;
                    break;
                case CurveToElement:
                    if (!contains(d->elements.at(i+2)))
                        return false;
                    i += 2;
                    break;
                default:
                    break;
                }
            }

            // compensate for the last ++i in the inner for
            --i;
        }
    }

    return true;
}


/*!
    Returns true if this painterpath is equal to the given \a path.

    Note that comparing paths may involve a per element comparison
    which can be slow for complex paths.

    \sa operator!=()
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
    Returns true if this painter path differs from the given \a path.

    Note that comparing paths may involve a per element comparison
    which can be slow for complex paths.

    \sa operator==()
*/

bool QPainterPath::operator!=(const QPainterPath &path) const
{
    return !(*this==path);
}

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QPainterPath &path)
    \relates QPainterPath

    Writes the given painter \a path to the given \a stream, and
    returns a reference to the \a stream.

    \sa {Format of the QDataStream Operators}
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
    and returns a reference to the \a stream.

    \sa {Format of the QDataStream Operators}
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
            qWarning("QDataStream::operator>>: Adding a NaN element to path, results are undefined");
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
 * class QPainterPathStroker
 */

void qt_path_stroke_move_to(qfixed x, qfixed y, void *data)
{
    ((QPainterPath *) data)->moveTo(qt_fixed_to_real(x), qt_fixed_to_real(y));
}

void qt_path_stroke_line_to(qfixed x, qfixed y, void *data)
{
    ((QPainterPath *) data)->lineTo(qt_fixed_to_real(x), qt_fixed_to_real(y));
}

void qt_path_stroke_cubic_to(qfixed c1x, qfixed c1y,
                             qfixed c2x, qfixed c2y,
                             qfixed ex, qfixed ey,
                             void *data)
{
    ((QPainterPath *) data)->cubicTo(qt_fixed_to_real(c1x), qt_fixed_to_real(c1y),
                                     qt_fixed_to_real(c2x), qt_fixed_to_real(c2y),
                                     qt_fixed_to_real(ex), qt_fixed_to_real(ey));
}

/*!
    \since 4.1
    \class QPainterPathStroker
    \ingroup multimedia

    \brief The QPainterPathStroker class is used to generate fillable
    outlines for a given painter path.

    By calling the createStroke() function, passing a given
    QPainterPath as argument, a new painter path representing the
    outline of the given path is created. The newly created painter
    path can then be filled to draw the original painter path's
    outline.

    You can control the various design aspects (width, cap styles,
    join styles and dash pattern) of the outlining using the following
    functions:

    \list
    \o setWidth()
    \o setCapStyle()
    \o setJoinStyle()
    \o setDashPattern()
    \endlist

    The setDashPattern() function accepts both a Qt::PenStyle object
    and a vector representation of the pattern as argument.

    In addition you can specify a curve's threshold, controlling the
    granularity with which a curve is drawn, using the
    setCurveThreshold() fucntion. The default threshold is a well
    adjusted value (0.25), and normally you should not need to modify
    it. However, you can make the curve's appearance smoother by
    decreasing its value.

    You can also control the miter limit for the generated outline
    using the setMiterLimit() function. The miter limit describes how
    far from each join the miter join can extend. The limit is
    specified in the units of width so the pixelwise miter limit will
    be \c {miterlimit * width}. This value is only used if the join
    style is Qt::MiterJoin.

    The painter path generated by the createStroke() function should
    only be used for outlining the given painter path. Otherwise it
    may cause unexpected behavior. Generated outlines also require the
    Qt::WindingFill rule which is set by default.

    \sa QPen, QBrush
*/

class QPainterPathStrokerPrivate
{
public:
    QPainterPathStrokerPrivate()
    {
        stroker.setMoveToHook(qt_path_stroke_move_to);
        stroker.setLineToHook(qt_path_stroke_line_to);
        stroker.setCubicToHook(qt_path_stroke_cubic_to);
    }

    QStroker stroker;
    QVector<qfixed> dashPattern;
};

/*!
   Creates a new stroker.
 */
QPainterPathStroker::QPainterPathStroker()
    : d_ptr(new QPainterPathStrokerPrivate)
{
}

/*!
    Destroys the stroker.
*/
QPainterPathStroker::~QPainterPathStroker()
{
    delete d_ptr;
}


/*!
    Generates a new path that is a fillable area representing the
    outline of the given \a path.

    The various design aspects of the outline are based on the
    stroker's properties: width(), capStyle(), joinStyle(),
    dashPattern(), curveThreshold() and miterLimit().

    The generated path should only be used for outlining the given
    painter path. Otherwise it may cause unexpected
    behavior. Generated outlines also require the Qt::WindingFill rule
    which is set by default.
*/
QPainterPath QPainterPathStroker::createStroke(const QPainterPath &path) const
{
    QPainterPathStrokerPrivate *d = const_cast<QPainterPathStrokerPrivate *>(d_func());
    QPainterPath stroke;
    if (d->dashPattern.isEmpty()) {
        d->stroker.strokePath(path, &stroke, QMatrix());
    } else {
        QDashStroker dashStroker(&d->stroker);
        dashStroker.setDashPattern(d->dashPattern);
        dashStroker.strokePath(path, &stroke, QMatrix());
    }
    stroke.setFillRule(Qt::WindingFill);
    return stroke;
}

/*!
    Sets the width of the generated outline painter path to \a width.

    The generated outlines will extend approximately 50% of \a width
    to each side of the given input path's original outline.
*/
void QPainterPathStroker::setWidth(qreal width)
{
    Q_D(QPainterPathStroker);
    if (width <= 0)
        width = 1;
    d->stroker.setStrokeWidth(qt_real_to_fixed(width));
}

/*!
    Returns the width of the generated outlines.
*/
qreal QPainterPathStroker::width() const
{
    return qt_fixed_to_real(d_func()->stroker.strokeWidth());
}


/*!
    Sets the cap style of the generated outlines to \a style.  If a
    dash pattern is set, each segment of the pattern is subject to the
    cap \a style.
*/
void QPainterPathStroker::setCapStyle(Qt::PenCapStyle style)
{
    d_func()->stroker.setCapStyle(style);
}


/*!
    Returns the cap style of the generated outlines.
*/
Qt::PenCapStyle QPainterPathStroker::capStyle() const
{
    return d_func()->stroker.capStyle();
}

/*!
    Sets the join style of the generated outlines to \a style.
*/
void QPainterPathStroker::setJoinStyle(Qt::PenJoinStyle style)
{
    d_func()->stroker.setJoinStyle(style);
}

/*!
    Returns the join style of the generated outlines.
*/
Qt::PenJoinStyle QPainterPathStroker::joinStyle() const
{
    return d_func()->stroker.joinStyle();
}

/*!
    Sets the miter limit of the generated outlines to \a limit.

    The miter limit describes how far from each join the miter join
    can extend. The limit is specified in units of the currently set
    width. So the pixelwise miter limit will be \c { miterlimit *
    width}.

    This value is only used if the join style is Qt::MiterJoin.
*/
void QPainterPathStroker::setMiterLimit(qreal limit)
{
    d_func()->stroker.setMiterLimit(qt_real_to_fixed(limit));
}

/*!
    Returns the miter limit for the generated outlines.
*/
qreal QPainterPathStroker::miterLimit() const
{
    return qt_fixed_to_real(d_func()->stroker.miterLimit());
}


/*!
    Specifies the curve flattening \a threshold, controlling the
    granularity with which the generated outlines' curve is drawn.

    The default threshold is a well adjusted value (0.25), and
    normally you should not need to modify it. However, you can make
    the curve's appearance smoother by decreasing its value.
*/
void QPainterPathStroker::setCurveThreshold(qreal threshold)
{
    d_func()->stroker.setCurveThreshold(qt_real_to_fixed(threshold));
}

/*!
    Returns the curve flattening threshold for the generated
    outlines.
*/
qreal QPainterPathStroker::curveThreshold() const
{
    return qt_fixed_to_real(d_func()->stroker.curveThreshold());
}

/*!
    Sets the dash pattern for the generated outlines to \a style.
*/
void QPainterPathStroker::setDashPattern(Qt::PenStyle style)
{
    d_func()->dashPattern = QDashStroker::patternForStyle(style);
}

/*!
    \overload

    Sets the dash pattern for the generated outlines to \a
    dashPattern.  This function makes it possible to specify custom
    dash patterns.
*/
void QPainterPathStroker::setDashPattern(const QVector<qreal> &dashPattern)
{
    d_func()->dashPattern.clear();
    for (int i=0; i<dashPattern.size(); ++i)
        d_func()->dashPattern << qt_real_to_fixed(dashPattern.at(i));
}

/*!
    Returns the dash pattern for the generated outlines.
*/
QVector<qreal> QPainterPathStroker::dashPattern() const
{
    return d_func()->dashPattern;
}
