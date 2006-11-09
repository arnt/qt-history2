/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbezier_p.h"
#include <qdebug.h>
#include <qline.h>
#include <qpolygon.h>
#include <qvector.h>
#include <qlist.h>

#include <private/qnumeric_p.h>
#include <private/qmath_p.h>

//#define QDEBUG_BEZIER

#ifdef FLOAT_ACCURACY
#define INV_EPS (1L<<23)
#else
/* The value of 1.0 / (1L<<14) is enough for most applications */
#define INV_EPS (1L<<14)
#endif

#ifndef M_SQRT2
#define M_SQRT2	1.41421356237309504880
#endif

#define log2(x) (log(x)/log(2.))

static inline double log4(double x)
{
    return 0.5 * log2(x);
}

/*!
  \internal
*/
QBezier QBezier::fromPoints(const QPointF &p1, const QPointF &p2,
                              const QPointF &p3, const QPointF &p4)
{
    QBezier b;
    b.x1 = p1.x();
    b.y1 = p1.y();
    b.x2 = p2.x();
    b.y2 = p2.y();
    b.x3 = p3.x();
    b.y3 = p3.y();
    b.x4 = p4.x();
    b.y4 = p4.y();
    return b;
}

/*!
  \internal
*/
QPolygonF QBezier::toPolygon() const
{
    // flattening is done by splitting the bezier until we can replace the segment by a straight
    // line. We split further until the control points are close enough to the line connecting the
    // boundary points.
    //
    // the Distance of a point p from a line given by the points (a,b) is given by:
    //
    // d = abs( (bx - ax)(ay - py) - (by - ay)(ax - px) ) / line_length
    //
    // We can stop splitting if both control points are close enough to the line.
    // To make the algorithm faster we use the manhattan length of the line.

    QPolygonF polygon;
    polygon.append(QPointF(x1, y1));
    addToPolygon(&polygon);
    return polygon;
}

void QBezier::addToPolygon(QPolygonF *polygon) const
{
    QBezier beziers[32];
    beziers[0] = *this;
    QBezier *b = beziers;
    while (b >= beziers) {
        // check if we can pop the top bezier curve from the stack
        qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
        qreal d;
        if (l > 1.) {
            d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2) - (b->y4 - b->y1)*(b->x1 - b->x2) )
                + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3) - (b->y4 - b->y1)*(b->x1 - b->x3) );
        } else {
            d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
            l = 1.;
        }
        if (d < .5*l || b == beziers + 31) {
            // good enough, we pop it off and add the endpoint
            polygon->append(QPointF(b->x4, b->y4));
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
            ++b;
        }
    }
}

void QBezier::split(QBezier *firstHalf, QBezier *secondHalf) const
{
    Q_ASSERT(firstHalf);
    Q_ASSERT(secondHalf);

    qreal c = (x2 + x3)/2;
    firstHalf->x2 = (x1 + x2)/2;
    secondHalf->x3 = (x3 + x4)/2;
    firstHalf->x1 = x1;
    secondHalf->x4 = x4;
    firstHalf->x3 = (firstHalf->x2 + c)/2;
    secondHalf->x2 = (secondHalf->x3 + c)/2;
    firstHalf->x4 = secondHalf->x1 = (firstHalf->x3 + secondHalf->x2)/2;

    c = (y2 + y3)/2;
    firstHalf->y2 = (y1 + y2)/2;
    secondHalf->y3 = (y3 + y4)/2;
    firstHalf->y1 = y1;
    secondHalf->y4 = y4;
    firstHalf->y3 = (firstHalf->y2 + c)/2;
    secondHalf->y2 = (secondHalf->y3 + c)/2;
    firstHalf->y4 = secondHalf->y1 = (firstHalf->y3 + secondHalf->y2)/2;
}


QRectF QBezier::bounds() const
{
    qreal xmin = x1;
    qreal xmax = x1;
    if (x2 < xmin)
        xmin = x2;
    else if (x2 > xmax)
        xmax = x2;
    if (x3 < xmin)
        xmin = x3;
    else if (x3 > xmax)
        xmax = x3;
    if (x4 < xmin)
        xmin = x4;
    else if (x4 > xmax)
        xmax = x4;

    qreal ymin = y1;
    qreal ymax = y1;
    if (y2 < ymin)
        ymin = y2;
    else if (y2 > ymax)
        ymax = y2;
    if (y3 < ymin)
        ymin = y3;
    else if (y3 > ymax)
        ymax = y3;
    if (y4 < ymin)
        ymin = y4;
    else if (y4 > ymax)
        ymax = y4;
    return QRectF(xmin, ymin, xmax-xmin, ymax-ymin);
}


enum ShiftResult {
    Ok,
    Discard,
    Split,
    Circle
};

static ShiftResult good_offset(const QBezier *b1, const QBezier *b2, qreal offset, qreal threshold)
{
    const qreal o2 = offset*offset;
    const qreal max_dist_line = threshold*offset*offset;
    const qreal max_dist_normal = threshold*offset;
    const qreal spacing = 0.25;
    for (qreal i = spacing; i < 0.99; i += spacing) {
        QPointF p1 = b1->pointAt(i);
        QPointF p2 = b2->pointAt(i);
        qreal d = (p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y());
        if (qAbs(d - o2) > max_dist_line)
            return Split;

        QPointF normalPoint = b1->normalVector(i);
        qreal l = qAbs(normalPoint.x()) + qAbs(normalPoint.y());
        if (l != 0.) {
            d = qAbs( normalPoint.x()*(p1.y() - p2.y()) - normalPoint.y()*(p1.x() - p2.x()) ) / l;
            if (d > max_dist_normal)
                return Split;
        }
    }
    return Ok;
}

static inline QLineF qline_shifted(const QPointF &p1, const QPointF &p2, qreal offset)
{
    QLineF l(p1, p2);
    QLineF ln = l.normalVector().unitVector();
    l.translate(ln.dx() * offset, ln.dy() * offset);
    return l;
}

static bool qbezier_is_line(QPointF *points, int pointCount)
{
    Q_ASSERT(pointCount > 2);

    qreal dx13 = points[2].x() - points[0].x();
    qreal dy13 = points[2].y() - points[0].y();

    qreal dx12 = points[1].x() - points[0].x();
    qreal dy12 = points[1].y() - points[0].y();

    if (pointCount == 3) {
        if (dx13 * dx12 != 0)
            return qFuzzyCompare(dy12 / dx12, dy13 / dx13);
        else
            return qFuzzyCompare(dx12 / dy12, dx13 / dy13);

    } else if (pointCount == 4) {
        qreal dx14 = points[3].x() - points[0].x();
        qreal dy14 = points[3].y() - points[0].y();

        if (dx14*dx13*dx12 != 0) {
            qreal b14 = dy14 / dx14;
            qreal b13 = dy13 / dx13;
            qreal b12 = dy12 / dx12;
            return qFuzzyCompare(b14, b13) && qFuzzyCompare(b14, b12);

        } else {
            qreal a14 = dx14 / dy14;
            qreal a13 = dx13 / dy13;
            qreal a12 = dx12 / dy12;
            return qFuzzyCompare(a14, a13) && qFuzzyCompare(a14, a12);
        }
    }

    return false;
}

static ShiftResult shift(const QBezier *orig, QBezier *shifted, qreal offset, qreal threshold)
{
    int map[4];
    bool p1_p2_equal = (orig->x1 == orig->x2 && orig->y1 == orig->y2);
    bool p2_p3_equal = (orig->x2 == orig->x3 && orig->y2 == orig->y3);
    bool p3_p4_equal = (orig->x3 == orig->x4 && orig->y3 == orig->y4);

    QPointF points[4];
    int np = 0;
    points[np] = QPointF(orig->x1, orig->y1);
    map[0] = 0;
    ++np;
    if (!p1_p2_equal) {
        points[np] = QPointF(orig->x2, orig->y2);
        ++np;
    }
    map[1] = np - 1;
    if (!p2_p3_equal) {
        points[np] = QPointF(orig->x3, orig->y3);
        ++np;
    }
    map[2] = np - 1;
    if (!p3_p4_equal) {
        points[np] = QPointF(orig->x4, orig->y4);
        ++np;
    }
    map[3] = np - 1;
    if (np == 1)
        return Discard;

    // We need to specialcase lines of 3 or 4 points due to numerical
    // instability in intersections below
    if (np > 2 && qbezier_is_line(points, np)) {
        QLineF l = qline_shifted(points[0], points[np-1], offset);
        *shifted = QBezier::fromPoints(l.p1(), l.pointAt(0.33), l.pointAt(0.66), l.p2());
        return Ok;
    }

    QRectF b = orig->bounds();
    if (np == 4 && b.width() < .1*offset && b.height() < .1*offset) {
        qreal l = (orig->x1 - orig->x2)*(orig->x1 - orig->x2) +
                  (orig->y1 - orig->y2)*(orig->y1 - orig->y1) *
                  (orig->x3 - orig->x4)*(orig->x3 - orig->x4) +
                  (orig->y3 - orig->y4)*(orig->y3 - orig->y4);
        qreal dot = (orig->x1 - orig->x2)*(orig->x3 - orig->x4) +
                    (orig->y1 - orig->y2)*(orig->y3 - orig->y4);
        if (dot < 0 && dot*dot < 0.8*l)
            // the points are close and reverse dirction. Approximate the whole
            // thing by a semi circle
            return Circle;
    }

    QPointF points_shifted[4];

    QLineF prev = QLineF(QPointF(), points[1] - points[0]);
    QPointF prev_normal = prev.normalVector().unitVector().p2();

    points_shifted[0] = points[0] + offset * prev_normal;

    for (int i = 1; i < np - 1; ++i) {
        QLineF next = QLineF(QPointF(), points[i + 1] - points[i]);
        QPointF next_normal = next.normalVector().unitVector().p2();

        QPointF normal_sum = prev_normal + next_normal;

        qreal r = 1.0 + prev_normal.x() * next_normal.x()
                  + prev_normal.y() * next_normal.y();

        if (qFuzzyCompare(r, (qreal)0.0)) {
            points_shifted[i] = points[i] + offset * prev_normal;
        } else {
            qreal k = offset / r;
            points_shifted[i] = points[i] + k * normal_sum;
        }

        prev_normal = next_normal;
    }

    points_shifted[np - 1] = points[np - 1] + offset * prev_normal;

    *shifted = QBezier::fromPoints(points_shifted[map[0]], points_shifted[map[1]],
                                   points_shifted[map[2]], points_shifted[map[3]]);

    return good_offset(orig, shifted, offset, threshold);
}

// This value is used to determine the length of control point vectors
// when approximating arc segments as curves. The factor is multiplied
// with the radius of the circle.
#define KAPPA 0.5522847498


static void addCircle(const QBezier *b, qreal offset, QBezier *o)
{
    QPointF normals[3];

    normals[0] = QPointF(b->y2 - b->y1, b->x1 - b->x2);
    qreal dist = sqrt(normals[0].x()*normals[0].x() + normals[0].y()*normals[0].y());
    if (qFuzzyCompare(dist, 0))
        return;
    normals[0] /= dist;
    normals[2] = QPointF(b->y4 - b->y3, b->x3 - b->x4);
    dist = sqrt(normals[2].x()*normals[2].x() + normals[2].y()*normals[2].y());
    if (qFuzzyCompare(dist, 0))
        return;
    normals[2] /= dist;

    normals[1] = QPointF(b->x1 - b->x2 - b->x3 + b->x4, b->y1 - b->y2 - b->y3 + b->y4);
    normals[1] /= -1*sqrt(normals[1].x()*normals[1].x() + normals[1].y()*normals[1].y());

    qreal angles[2];
    qreal sign = 1.;
    for (int i = 0; i < 2; ++i) {
        qreal cos_a = normals[i].x()*normals[i+1].x() + normals[i].y()*normals[i+1].y();
        if (cos_a > 1.)
            cos_a = 1.;
        if (cos_a < -1.)
            cos_a = -1;
        angles[i] = acos(cos_a)/Q_PI;
    }

    if (angles[0] + angles[1] > 1.) {
        // more than 180 degrees
        normals[1] = -normals[1];
        angles[0] = 1. - angles[0];
        angles[1] = 1. - angles[1];
        sign = -1.;

    }

    QPointF circle[3];
    circle[0] = QPointF(b->x1, b->y1) + normals[0]*offset;
    circle[1] = QPointF(0.5*(b->x1 + b->x4), 0.5*(b->y1 + b->y4)) + normals[1]*offset;
    circle[2] = QPointF(b->x4, b->y4) + normals[2]*offset;

    for (int i = 0; i < 2; ++i) {
        qreal kappa = 2.*KAPPA * sign * offset * angles[i];

        o->x1 = circle[i].x();
        o->y1 = circle[i].y();
        o->x2 = circle[i].x() - normals[i].y()*kappa;
        o->y2 = circle[i].y() + normals[i].x()*kappa;
        o->x3 = circle[i+1].x() + normals[i+1].y()*kappa;
        o->y3 = circle[i+1].y() - normals[i+1].x()*kappa;
        o->x4 = circle[i+1].x();
        o->y4 = circle[i+1].y();

        ++o;
    }
}

int QBezier::shifted(QBezier *curveSegments, int maxSegments, qreal offset, float threshold) const
{
    Q_ASSERT(curveSegments);
    Q_ASSERT(maxSegments > 0);

    if (x1 == x2 && x1 == x3 && x1 == x4 &&
        y1 == y2 && y1 == y3 && y1 == y4)
        return 0;

    --maxSegments;
    QBezier beziers[10];
redo:
    beziers[0] = *this;
    QBezier *b = beziers;
    QBezier *o = curveSegments;

    while (b >= beziers) {
        if (b - beziers == 9 || o - curveSegments == maxSegments) {
            threshold *= 1.5;
            if (threshold > 2.)
                goto give_up;
            goto redo;
        }
        ShiftResult res = shift(b, o, offset, threshold);
        if (res == Discard) {
            --b;
        } else if (res == Ok) {
            ++o;
            --b;
            continue;
        } else if (res == Circle && maxSegments - (o - curveSegments) >= 2) {
            // add semi circle
            addCircle(b, offset, o);
            --b;
            o += 2;
        } else {
            b->split(b+1, b);
            ++b;
        }
    }

give_up:
    while (b >= beziers) {
        shift(b, o, offset, threshold);
        ++o;
        --b;
    }

    Q_ASSERT(o - curveSegments <= maxSegments);
    return o - curveSegments;
}

#if 0
static inline bool IntersectBB(const QBezier &a, const QBezier &b)
{
    return a.bounds().intersects(b.bounds());
}
#else
int IntersectBB(const QBezier &a, const QBezier &b)
{
    // Compute bounding box for a
    double minax, maxax, minay, maxay;
    if (a.x1 > a.x4)	 // These are the most likely to be extremal
	minax = a.x4, maxax = a.x1;
    else
	minax = a.x1, maxax = a.x4;

    if (a.x3 < minax)
	minax = a.x3;
    else if (a.x3 > maxax)
	maxax = a.x3;

    if (a.x2 < minax)
	minax = a.x2;
    else if (a.x2 > maxax)
	maxax = a.x2;

    if (a.y1 > a.y4)
	minay = a.y4, maxay = a.y1;
    else
	minay = a.y1, maxay = a.y4;

    if (a.y3 < minay)
	minay = a.y3;
    else if (a.y3 > maxay)
	maxay = a.y3;

    if (a.y2 < minay)
	minay = a.y2;
    else if (a.y2 > maxay)
	maxay = a.y2;

    // Compute bounding box for b
    double minbx, maxbx, minby, maxby;
    if (b.x1 > b.x4)
	minbx = b.x4, maxbx = b.x1;
    else
	minbx = b.x1, maxbx = b.x4;

    if (b.x3 < minbx)
	minbx = b.x3;
    else if (b.x3 > maxbx)
	maxbx = b.x3;

    if (b.x2 < minbx)
	minbx = b.x2;
    else if (b.x2 > maxbx)
	maxbx = b.x2;

    if (b.y1 > b.y4)
	minby = b.y4, maxby = b.y1;
    else
	minby = b.y1, maxby = b.y4;

    if (b.y3 < minby)
	minby = b.y3;
    else if (b.y3 > maxby)
	maxby = b.y3;

    if (b.y2 < minby)
	minby = b.y2;
    else if (b.y2 > maxby)
	maxby = b.y2;

    // Test bounding box of b against bounding box of a
    if ((minax > maxbx) || (minay > maxby)  // Not >= : need boundary case
	|| (minbx > maxax) || (minby > maxay))
	return 0; // they don't intersect
    else
	return 1; // they intersect
}
#endif


static QDebug operator<<(QDebug dbg, const QBezier &bz)
{
    dbg <<"["<<bz.x1<<", "<<bz.y1<<"], "
        <<"["<<bz.x2<<", "<<bz.y2<<"], "
        <<"["<<bz.x3<<", "<<bz.y3<<"], "
        <<"["<<bz.x4<<", "<<bz.y4<<"]";
    return dbg;
}

void RecursivelyIntersect(const QBezier &a, double t0, double t1, int deptha,
			  const QBezier &b, double u0, double u1, int depthb,
                          QVector< QList<qreal> > &parameters)
{
#ifdef QDEBUG_BEZIER
    static int I = 0;
    int currentD = I;
    fprintf(stderr, "%d) t0 = %lf, t1 = %lf, deptha = %d\n"
            "u0 = %lf, u1 = %lf, depthb = %d\n", I++, t0, t1, deptha,
            u0, u1, depthb);
#endif
    if (deptha > 0) {
	QBezier A[2];
        a.split(&A[0], &A[1]);
	double tmid = (t0+t1)*0.5;
        //qDebug()<<"\t1)"<<A[0];
        //qDebug()<<"\t2)"<<A[1];
	deptha--;
	if (depthb > 0) {
	    QBezier B[2];
            b.split(&B[0], &B[1]);
            //qDebug()<<"\t3)"<<B[0];
            //qDebug()<<"\t4)"<<B[1];
	    double umid = (u0+u1)*0.5;
	    depthb--;
	    if (IntersectBB(A[0], B[0])) {
                //fprintf(stderr, "\t 1 from %d\n", currentD);
		RecursivelyIntersect(A[0], t0, tmid, deptha,
				     B[0], u0, umid, depthb,
				     parameters);
            }
	    if (IntersectBB(A[1], B[0])) {
                //fprintf(stderr, "\t 2 from %d\n", currentD);
		RecursivelyIntersect(A[1], tmid, t1, deptha,
                                     B[0], u0, umid, depthb,
                                     parameters);
            }
	    if (IntersectBB(A[0], B[1])) {
                //fprintf(stderr, "\t 3 from %d\n", currentD);
		RecursivelyIntersect(A[0], t0, tmid, deptha,
                                     B[1], umid, u1, depthb,
                                     parameters);
            }
	    if (IntersectBB(A[1], B[1])) {
                //fprintf(stderr, "\t 4 from %d\n", currentD);
		RecursivelyIntersect(A[1], tmid, t1, deptha,
				     B[1], umid, u1, depthb,
				     parameters);
            }
        } else {
	    if (IntersectBB(A[0], b)) {
                //fprintf(stderr, "\t 5 from %d\n", currentD);
		RecursivelyIntersect(A[0], t0, tmid, deptha,
				     b, u0, u1, depthb,
				     parameters);
            }
	    if (IntersectBB(A[1], b)) {
                //fprintf(stderr, "\t 6 from %d\n", currentD);
		RecursivelyIntersect(A[1], tmid, t1, deptha,
                                     b, u0, u1, depthb,
                                     parameters);
            }
        }
    } else {
	if (depthb > 0) {
	    QBezier B[2];
            b.split(&B[0], &B[1]);
	    double umid = (u0 + u1)*0.5;
	    depthb--;
	    if (IntersectBB(a, B[0])) {
                //fprintf(stderr, "\t 7 from %d\n", currentD);
		RecursivelyIntersect(a, t0, t1, deptha,
                                     B[0], u0, umid, depthb,
                                     parameters);
            }
	    if (IntersectBB(a, B[1])) {
                //fprintf(stderr, "\t 8 from %d\n", currentD);
		RecursivelyIntersect(a, t0, t1, deptha,
                                     B[1], umid, u1, depthb,
                                     parameters);
            }
        }
	else {
            // Both segments are fully subdivided; now do line segments
	    double xlk = a.x4 - a.x1;
	    double ylk = a.y4 - a.y1;
	    double xnm = b.x4 - b.x1;
	    double ynm = b.y4 - b.y1;
	    double xmk = b.x1 - a.x1;
	    double ymk = b.y1 - a.y1;
	    double det = xnm * ylk - ynm * xlk;
	    if (1.0 + det == 1.0) {
		return;
            } else {
		double detinv = 1.0 / det;
		double s = (xnm * ymk - ynm *xmk) * detinv;
		double t = (xlk * ymk - ylk * xmk) * detinv;
		if ((s < 0.0) || (s > 1.0) || (t < 0.0) || (t > 1.0))
		    return;
		parameters[0].append(t0 + s * (t1 - t0));
                parameters[1].append(u0 + t * (u1 - u0));
            }
        }
    }
}

QVector< QList<qreal> > QBezier::findIntersections(const QBezier &a, const QBezier &b)
{
    QVector< QList<qreal> > parameters(2);
    if (IntersectBB(a, b)) {
        QPointF la1(fabs((a.x3 - a.x2) - (a.x2 - a.x1)),
                    fabs((a.y3 - a.y2) - (a.y2 - a.y1)));
	QPointF la2(fabs((a.x4 - a.x3) - (a.x3 - a.x2)),
                    fabs((a.y4 - a.y3) - (a.y3 - a.y2)));
	QPointF la;
	if (la1.x() > la2.x()) la.setX(la1.x()); else la.setX(la2.x());
	if (la1.y() > la2.y()) la.setY(la1.y()); else la.setY(la2.y());
	QPointF lb1(fabs((b.x3 - b.x2) - (b.x2 - b.x1)),
                    fabs((b.y3 - b.y2) - (b.y2 - b.y1)));
	QPointF lb2(fabs((b.x4 - b.x3) - (b.x3 - b.x2)),
                    fabs((b.y4 - b.y3) - (b.y3 - b.y2)));
	QPointF lb;
	if (lb1.x() > lb2.x()) lb.setX(lb1.x()); else lb.setX(lb2.x());
	if (lb1.y() > lb2.y()) lb.setY(lb1.y()); else lb.setY(lb2.y());
	double l0;
	if (la.x() > la.y())
	    l0 = la.x();
	else
	    l0 = la.y();
	int ra;
	if (l0 * 0.75 * M_SQRT2 + 1.0 == 1.0)
	    ra = 0;
	else
	    ra = (int)ceil(log4(M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0));
	if (lb.x() > lb.y())
	    l0 = lb.x();
	else
	    l0 = lb.y();
	int rb;
	if (l0 * 0.75 * M_SQRT2 + 1.0 == 1.0)
	    rb = 0;
	else
	    rb = (int)ceil(log4(M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0));

	RecursivelyIntersect(a, 0., 1., ra, b, 0., 1., rb, parameters);
    }

    qSort(parameters[0].begin(), parameters[0].end(), qLess<qreal>());
    qSort(parameters[1].begin(), parameters[1].end(), qLess<qreal>());

    return parameters;
}

void QBezier::parameterSplitLeft(double t, QBezier *left)
{
    left->x1 = x1;
    left->y1 = y1;

    left->x2 = x1 + t * ( x2 - x1 );
    left->y2 = y1 + t * ( y2 - y1 );

    left->x3 = x2 + t * ( x3 - x2 ); // temporary holding spot
    left->y3 = y2 + t * ( y3 - y2 ); // temporary holding spot

    x3 = x3 + t * ( x4 - x3 );
    y3 = y3 + t * ( y4 - y3 );

    x2 = left->x3 + t * ( x3 - left->x3);
    y2 = left->y3 + t * ( y3 - left->y3);

    left->x3 = left->x2 + t * ( left->x3 - left->x2 );
    left->y3 = left->y2 + t * ( left->y3 - left->y2 );

    left->x4 = x1 = left->x3 + t * (x2 - left->x3);
    left->y4 = y1 = left->y3 + t * (y2 - left->y3);
}

QVector< QList<QBezier> > QBezier::splitAtIntersections(QBezier &b)
{
    QVector< QList<QBezier> > curves(2);

    QVector< QList<qreal> > allInters = findIntersections(*this, b);

    const QList<qreal> &inters1 = allInters[0];
    const QList<qreal> &inters2 = allInters[1];
    Q_ASSERT(inters1.count() == inters2.count());

    int i;
    for (i = 0; i < inters1.count(); ++i) {
        qreal t1 = inters1.at(i);
        qreal t2 = inters2.at(i);

        QBezier curve1, curve2;
        parameterSplitLeft(t1, &curve1);
	b.parameterSplitLeft(t2, &curve2);
        curves[0].append(curve1);
        curves[0].append(curve2);
    }
    curves[0].append(*this);
    curves[1].append(b);

    return curves;
}

qreal QBezier::length(qreal error) const
{
    qreal length = 0.0;

    addIfClose(&length, error);

    return length;
}

void QBezier::addIfClose(qreal *length, qreal error) const
{
    QBezier left, right;     /* bez poly splits */

    double len = 0.0;        /* arc length */
    double chord;            /* chord length */

    len = len + QLineF(QPointF(x1, y1),QPointF(x2, y2)).length();
    len = len + QLineF(QPointF(x2, y2),QPointF(x3, y3)).length();
    len = len + QLineF(QPointF(x3, y3),QPointF(x4, y4)).length();

    chord = QLineF(QPointF(x1, y1),QPointF(x4, y4)).length();

    if((len-chord) > error) {
        split(&left, &right);                 /* split in two */
        left.addIfClose(length, error);       /* try left side */
        right.addIfClose(length, error);      /* try right side */
        return;
    }

    *length = *length + len;

    return;
}

qreal QBezier::tAtLength(qreal l) const
{
    qreal len = length();
    qreal t   = 1.0;
    const qreal error = 0.01;
    if (l > len || qFuzzyCompare(l, len))
        return t;

    t *= 0.5;
    //int iters = 0;
    //qDebug()<<"LEN is "<<l<<len;
    qreal lastBigger = 1.;
    while (1) {
        //qDebug()<<"\tt is "<<t;
        QBezier right = *this;
        QBezier left;
        right.parameterSplitLeft(t, &left);
        qreal lLen = left.length();
        if (qAbs(lLen - l) < error)
            break;

        if (lLen < l) {
            t += (lastBigger - t)*.5;
        } else {
            lastBigger = t;
            t -= t*.5;
        }
        //++iters;
    }
    //qDebug()<<"number of iters is "<<iters;
    return t;
}
