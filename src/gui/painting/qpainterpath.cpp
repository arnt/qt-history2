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

//#define QPP_DEBUG
//#define QPP_STROKE_DEBUG
//#define QPP_FILLPOLYGONS_DEBUG

#define d d_func()
#define q q_func()

void qt_find_ellipse_coords(const QRectF &r, qReal angle, qReal length,
                            QPointF* startPoint, QPointF *endPoint)
{
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
    qReal a = r.width() / 2.0;
    qReal b = r.height() / 2.0;

    if (startPoint) {
        *startPoint = r.center()
                      + QPointF(a * cos(ANGLE(angle)), -b * sin(ANGLE(angle)));
    }
    if (endPoint) {
        *endPoint = r.center()
                    + QPointF(a * cos(ANGLE(angle + length)), -b * sin(ANGLE(angle + length)));
    }
}

/* Helper class for qt_vectorize_region */
class QVLineSets {

public:
    QList< QList<QPoint> > closedSets;
    QList< QList<QPoint> > openSets;

#ifndef QT_NO_DEBUG
    void debug() {
        printf("QVLineSets, closed=%d, open=%d\n", closedSets.size(), openSets.size());
        for (int i=0; i<closedSets.size(); ++i) {
            printf(" - Closed set: %d\n", i);
            for (int j=0; j<closedSets.at(i).size(); ++j)
                printf(" --- (%d, %d)\n", closedSets.at(i).at(j).x(), closedSets.at(i).at(j).y());
        }
        for (int i=0; i<openSets.size(); ++i) {
            printf(" - Open set: %d\n", i);
            for (int j=0; j<openSets.at(i).size(); ++j)
                printf(" --- (%d, %d)\n", openSets.at(i).at(j).x(), openSets.at(i).at(j).y());
        }
    }
#endif

    // Adds the linesets to the specified path.
    void addToPath(QPainterPath *path)
    {
        for (int iset=0; iset<closedSets.size(); ++iset) {
            const QList<QPoint> &set = closedSets.at(iset);
            Q_ASSERT(!set.isEmpty());
            path->moveTo(set.at(0));
            for (int elmi=1; elmi<closedSets.at(iset).size(); ++elmi)
                path->lineTo(set.at(elmi));
        }
    }

    // Adds the line to the sets. This is done by first checking of the line
    // can be connected to any of the current sets, in which case the line
    // is added to that. A new set is started if not.
    void addLine(int x1, int y1, int x2, int y2, bool ordered)
    {
        if (x1 == x2 && y1 == y2)
            return;
        Q_ASSERT((ordered && x1 == x2) || (!ordered && y1 == y2));

        int insert = -1;

        // Find match
        for (int iset=0; iset<openSets.size() && insert<0; ++iset) {
            if (!openSets.at(iset).isEmpty()) {
                int ilast = openSets.at(iset).size() - 1;

                // Check if line is connectable to the front/end of the set
                if (openSets.at(iset).at(0) == QPoint(x2, y2)) {
                    openSets[iset].insert(0, QPoint(x1, y1));
                    insert = iset;
                } else if (openSets.at(iset).at(ilast) == QPoint(x1, y1)) {
                    openSets[iset].append(QPoint(x2, y2));
                    insert = iset;

                // Check if the line is connectable if we reverse it.
                } else if (!ordered) {
                    if (openSets.at(iset).at(0) == QPoint(x1, y1)) {
                        openSets[iset].insert(0, QPoint(x2, y2));
                        insert = iset;
                    } else if (openSets.at(iset).at(ilast) == QPoint(x2, y2)) {
                        openSets[iset].append(QPoint(x1, y1));
                        insert = iset;
                    }

                // Sets of size 1 could be ordered the wrong way, so do extra check,
                // only need the check on the left since we go left to right.
                } else if (openSets.at(iset).size() == 2) {
                    if (openSets.at(iset).at(0) == QPoint(x1, y1)) {
                        QPoint pt1 = openSets.at(iset).at(0);
                        QPoint pt2 = openSets.at(iset).at(1);
                        openSets[iset].clear();
                        openSets[iset] << pt2 << pt1 << QPoint(x2, y2);
                        insert = iset;
                    }
                }


            }
        }

        // If we inserted we want to check if the set is closeable
        if (insert>=0) {
            // Try to connect existing sets
            for (int i=0; i<openSets.size(); ++i) {
                // Selfcheck doesn't make sense
                if (i == insert)
                    continue;
                if (openSets.at(i).first() == openSets.at(insert).last()) {
                    // Remove first element to avoid duplication.
                    openSets[i].removeAt(0);
                    // Add other set to insert.
                    openSets[insert] += openSets[i];
                    openSets.removeAt(i);
                    if (i < insert)
                        --insert;
                } else if (openSets.at(i).last() == openSets.at(insert).first()) {
                    openSets[insert].removeAt(0);
                    openSets[i] += openSets[insert];
                    openSets.removeAt(insert);
                    insert = insert < i ? i - 1 : i;
                }
            }
            if (openSets.at(insert).first() == openSets.at(insert).last()) {
                closedSets.append(openSets.at(insert));
                openSets.removeAt(insert);
            }

        // Create a new set if no connection was found.
        } else {
            QList<QPoint> line;
            line << QPoint(x1, y1);
            line << QPoint(x2, y2);
            openSets << line;
        }
    }
};


/*
    Converts the region to sets of lines in the form of a path. The
    approach is as follows: Rectangles are ordered from to bottom and
    left to right on each line, so sorting is already done for us.

    All vertical rectangle edges can be added directly to the
    path. The left edge is swapped so that the lines follow a clock
    wise pattern.

    We find the horizontal lines by looking at where the vertical edges
    above and below. By sorting them from left to right, we get pairs
    which define the lines to use.

    Naming:
     - i for index
     - c for current
     - f for first
     - l for last
*/

void qt_vectorize_region(const QRegion &region, QPainterPath *path)
{
    if (region.isEmpty())
        return;

    QVLineSets sets;

    QVector<QRect> rects = region.rects();;

    int rectCount = rects.size();

    int ircf = 0;  // index to the first rect on current span
    int ircl = -1; // index to the last rect on the current span
    int irnf = -1; // index to the first rect on the next span
    int irnl = -1; // index to the last rect on the next span

    // The top of the first span.
    int yc = rects.at(0).y();
    for (int i=0; i<rectCount && rects.at(i).y() == yc; ++i) {
        int right = rects.at(i).x() + rects.at(i).width();
        sets.addLine(rects.at(i).x(), yc, right, yc, false);
    }

    // The list of x intersections.
    int xvals[1024];

    while (ircf<rectCount) {

        yc = rects.at(ircf).y(); // y pos of current span
        int yn = rects.at(ircf).y() + rects.at(ircf).height(); // y pos of next span

        // Locate last rect in current span
        for (ircl = ircf; ircl+1<rectCount && rects.at(ircl+1).y() == yc; ++ircl);

        // First rect in next span is the next after last in current.
        irnf = ircl+1;

        // Locate last rect in next span
        for (irnl = irnf; irnl+1<rectCount && rects.at(irnl+1).y() == yn; ++irnl);

        // Add the vertical lines.
        for (int i=ircf; i<=ircl; ++i) {
            int right = rects.at(i).x() + rects.at(i).width();
            int bottom = rects.at(i).y() + rects.at(i).height();
            sets.addLine(rects.at(i).x(), bottom, rects.at(i).x(), yc, true);
            sets.addLine(right, yc, right, bottom, true);
        }


        int xpos = 0;

        // add x positions of current span to list
        for (int i=ircf; i<=ircl; ++i) {
            xvals[xpos++] = rects.at(i).x();
            xvals[xpos++] = rects.at(i).x() + rects.at(i).width();
            Q_ASSERT(xpos<1024);
        }

        // add x positions of next span to list, except for last run
        if (irnl < rectCount) {
            for (int i=irnf; i<=irnl; ++i) {
                xvals[xpos++] = rects.at(i).x();
                xvals[xpos++] = rects.at(i).x() + rects.at(i).width();
                Q_ASSERT(xpos<1024);
            }
        }

        xvals[xpos] = INT_MAX;
        qHeapSort(&xvals[0], &xvals[xpos]);

        Q_ASSERT(xpos % 2 == 0);

        // Add the horizontal lines.
        for (int i=0; i<xpos; i+=2) {
            sets.addLine(xvals[i], yn, xvals[i+1], yn, false);
        }


        ircf = ircl+1;
    }

    sets.addToPath(path);

}

#ifdef QPP_STROKE_DEBUG
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
    \fn void QPainterPath::addEllipse(qReal x, qReal y, qReal width, qReal height)
    \overload

    Creates an ellipse within a bounding rectangle defined by its top-left
    corner at (\a x, \a y), \a width and \a height, and adds it to the
    painter path.

    If the current subpath is closed, a new subpath is started. The ellipse
    is clockwise starting and starting zero degrees.
*/

/*!
    \fn void QPainterPath::addText(qReal x, qReal y, const QFont &font, const QString &text)
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
    \fn void QPainterPath::moveTo(qReal x, qReal y)

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

    d->makeDirty();
}

/*!
    \fn void QPainterPath::lineTo(qReal x, qReal y)

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

    d->makeDirty();
}

/*!
    \fn void QPainterPath::curveTo(qReal ctrlPt1x, qReal ctrlPt1y, qReal ctrlPt2x,
                                   qReal ctrlPt2y, qReal endPtx, qReal endPty);

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

    d->makeDirty();
}

/*!
    \fn void QPainterPath::arcTo(qReal x, qReal y, qReal width, qReal
    height, qReal startAngle, qReal sweepLength)

    \overload

    The arc's lies within the rectangle given by the point (\a{x}, \a{y}),
    \a width and \a height, beginning at \a startAngle and extending
    \a sweepLength degrees anti-clockwise.
    Angles are specified in degrees. This function connects the current point
    to the starting point of the arc if they are not already connected.

    \sa QPainter::drawArc
*/

/*!
    \fn void QPainterPath::arcTo(const QRectF &rectangle, qReal startAngle, qReal sweepLength)

    Creates an arc that occupies the given \a rectangle, beginning at
    \a startAngle and extending \a sweepLength degrees anti-clockwise.
    Angles are specified in degrees. This function connects the current point
    to the starting point of the arc if they are not already connected.

    \sa QPainter::drawArc
*/
void QPainterPath::arcTo(const QRectF &rect, qReal startAngle, qReal sweepLength)
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
    qReal a = rect.width() / 2.0;
    qReal b = rect.height() / 2.0;

    qReal absSweepLength = (sweepLength < 0 ? -sweepLength : sweepLength);
    int iterations = qIntCast((absSweepLength + 89) / 90);
    qReal clength = sweepLength / iterations;
    qReal cosangle1, sinangle1, cosangle2, sinangle2;
    for (int i=0; i<iterations; ++i) {
        qReal cangle = startAngle + i * clength;

        cosangle1 = cos(ANGLE(cangle));
        sinangle1 = sin(ANGLE(cangle));
        cosangle2 = cos(ANGLE(cangle + clength));
        sinangle2 = sin(ANGLE(cangle + clength));

        // Find the start and end point of the curve.
        QPointF startPoint = rect.center() + QPointF(a * cosangle1, -b * sinangle1);
        QPointF endPoint = rect.center() + QPointF(a * cosangle2, -b * sinangle2);

        // The derived at the start and end point.
        qReal sdx = -a * sinangle1;
        qReal sdy = -b * cosangle1;
        qReal edx = -a * sinangle2;
        qReal edy = -b * cosangle2;

        // Creating the tangent lines. We need to reverse their direction if the
        // sweep is negative (clockwise)
        QLineF controlLine1(startPoint, startPoint + SIGN(sweepLength) * QPointF(sdx, sdy));
        QLineF controlLine2(endPoint, endPoint - SIGN(sweepLength) * QPointF(edx, edy));

        // We need to scale down the control lines to match that of the current sweeplength.
        // qAbs because we only want to scale, not change direction.
        qReal kappa = KAPPA * qAbs(clength) / 90.0;
        // Adjust their length to fit the magic KAPPA length.
        controlLine1.setLength(controlLine1.length() * kappa);
        controlLine2.setLength(controlLine2.length() * kappa);

        if (startPoint != QPointF(elements.last().x, elements.last().y))
            lineTo(startPoint);
        curveTo(controlLine1.end(), controlLine2.end(), endPoint);
    }
}

/*!
    \fn void QPainterPath::addRect(qReal x, qReal y, qReal width, qReal height)

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
        moveTo(r.x(), r.y());
    else
        lineTo(r.x(), r.y());
    lineTo(r.x() + r.width(), r.y());
    lineTo(r.x() + r.width(), r.y() + r.height());
    lineTo(r.x(), r.y() + r.height());
    lineTo(r.x(), r.y());
}

/*!
    Adds the \a polygon to path as a new subpath. If the current
    subpath is closed, a new subpath is started at the polygons first
    point.
*/
void QPainterPath::addPolygon(const QPolygonF &polygon)
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
    eng->setMode(QTextLayout::SingleLine);
    eng->itemize();

    QTextLine line = layout.createLine();
    line.layout(0x01000000);
    const QScriptLine &sl = eng->lines[0];
    if (!sl.length)
        return;

    int nItems = eng->items.size();

    qReal x(point.x());
    qReal y(point.y());

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
    Adds the region \a region to the path. This is done by converting
    the region into a set of lines which enclose it.
*/
void QPainterPath::addRegion(const QRegion &region)
{
    qt_vectorize_region(region, this);
}


/*!
    Returns the fill rule of the painter path. The default fill rule
    is \c Qt::OddEvenFill.

    \sa Qt::FillRule setFillRule()
*/
Qt::FillRule QPainterPath::fillRule() const
{
    return d->fillRule;
}

/*!
    \fn void QPainterPath::setFillRule(Qt::FillRule fillRule)

    Sets the fill rule of the painter path to \a fillRule.

    \sa fillRule()
*/
void QPainterPath::setFillRule(Qt::FillRule fillRule)
{
    d->makeDirty();
    d->fillRule = fillRule;
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
    Returns the painter path as a list of polygons. One polygon is
    created for each subpath. The polygons are transformed using the
    transformation matrix \a matrix.
*/
QList<QPolygonF> QPainterPath::toSubpathPolygons(const QMatrix &matrix) const
{
    QList<QPolygonF> flatCurves;
    if (isEmpty())
        return flatCurves;

    QPolygonF current;
    for (int i=0; i<elementCount(); ++i) {
        const QPainterPath::Element &e = elements.at(i);
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
        case QPainterPath::CurveToElement:
            Q_ASSERT(elements.at(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(elements.at(i+2).type == QPainterPath::CurveToDataElement);
            current += QBezier(QPointF(elements.at(i-1).x, elements.at(i-1).y) * matrix,
                               QPointF(e.x, e.y) * matrix,
                               QPointF(elements.at(i+1).x, elements.at(i+1).y) * matrix,
                               QPointF(elements.at(i+2).x, elements.at(i+2).y) * matrix).toPolygon();
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
    if (d->containsCache.isEmpty()) {
        const_cast<QPainterPathPrivate*>(d)->containsCache =
            QRegion(toFillPolygon().toPolygon(), fillRule());
    }
    return d->containsCache.contains(pt.toPoint());
}


/*!
    Returns true if the rect \a rect is inside the path; otherwise
    returns false.
*/
bool QPainterPath::contains(const QRectF &rect) const
{
    if (d->containsCache.isEmpty()) {
        const_cast<QPainterPathPrivate*>(d)->containsCache =
            QRegion(toFillPolygon().toPolygon(), fillRule());
    }
    return d->containsCache.contains(rect.toRect());
}

/*!
    Returns true if this painterpath is equal to \a path.

    Comparing paths may involve a pr element comparrison which
    can be slow for complex paths.
*/

bool QPainterPath::operator==(const QPainterPath &path) const
{
    if (path.d == d)
        return true;
    bool equal = d->fillRule == path.d->fillRule && elements.size() == path.elements.size();
    for (int i=0; i<elements.size() && equal; ++i)
        equal = elements.at(i) == path.elements.at(i);
    return equal;
}

/*!
    Returns true if this painterpath differs from \a path.

    Comparing paths may involve a pr element comparrison which
    can be slow for complex paths.
*/

bool QPainterPath::operator!=(const QPainterPath &path) const
{
    return !(*this==path);
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &s, const QPainterPath &p)
{
    s << p.elements.size();
    for (int i=0; i<p.elements.size(); ++i) {
        const QPainterPath::Element &e = p.elements.at(i);
        s << int(e.type) << e.x << e.y;
    }
    s << p.d->cStart;
    s << int(p.d->fillRule);
    return s;
}

QDataStream &operator>>(QDataStream &s, QPainterPath &p)
{
    int size;
    s >> size;

    if (p.elements.size() == 1) {
        Q_ASSERT(p.elements.at(0).type == QPainterPath::MoveToElement);
        p.elements.clear();
    }
    p.elements.reserve(p.elements.size() + size);
    for (int i=0; i<size; ++i) {
        int type;
        qReal x, y;
        s >> type >> x >> y;
        Q_ASSERT(type >= 0 && type <= 3);
        QPainterPath::Element elm = { x, y, QPainterPath::ElementType(type) };
        p.elements.append(elm);
    }
    s >> p.d->cStart;
    int fillRule;
    s >> fillRule;
    Q_ASSERT(fillRule == Qt::OddEvenFill || Qt::WindingFill);
    p.d->fillRule = Qt::FillRule(fillRule);
    return s;
}
#endif



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

    qReal width;
    qReal offset;
    qReal miterLimit;
    qReal appliedMiterLimit;
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
        Qt::PenJoinStyle jStyle = type == QLineF::NoIntersection ? Qt::BevelJoin : joinStyle;
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
    ml.translate(normal);

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
    QPolygonF segments = bezier.toPolygon();

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

QPolygonF qt_reversed_polygon(const QPolygonF &p)
{
    QPolygonF rev;
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

    stroke.setFillRule(Qt::WindingFill);

#ifdef QPP_STROKE_DEBUG
    printf(" -> Final path:\n");
    qt_debug_path(stroke);
#endif

    return stroke;
}

void QPainterPathStroker::setWidth(qReal width)
{
    d->width = width;
    d->offset = width / 2;
    d->appliedMiterLimit = d->miterLimit * width;
}

qReal QPainterPathStroker::width() const
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

void QPainterPathStroker::setMiterLimit(qReal limit)
{
    d->miterLimit = limit;
    d->appliedMiterLimit = d->miterLimit * d->width;
}

qReal QPainterPathStroker::miterLimit() const
{
    return d->miterLimit;
}
