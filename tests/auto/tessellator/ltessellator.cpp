#include "ltessellator.h"

#include "utils.h"

#include "XrenderFake.h"

#include <QPointF>
#include <QVector>
#include <QRect>
#include <QList>
#include <QVariant>
#include <QVarLengthArray>
#include <QDebug>

//#define DEBUG
#ifdef DEBUG
#define QDEBUG qDebug
#else
#define QDEBUG if (1); else qDebug
#endif

typedef int Q27Dot5;
#define Q27Dot5ToDouble(i) (i/32.)
#define FloatToQ27Dot5(i) (int)((i) * 32)
#define IntToQ27Dot5(i) ((i) << 5)
#define Q27Dot5ToXFixed(i) ((i) << 11)
#define Q27Dot5Factor 32

class QTessellatorPrivate;

class QTessellator {
public:
    QTessellator();
    virtual ~QTessellator();

    QRectF tesselate(const QPointF *points, int nPoints);

    void setWinding(bool w);

    struct Trapezoid {
        int top;
        int bottom;
        int topleft_x;
        int topleft_y;
        int bottomleft_x;
        int bottomleft_y;
        int topright_x;
        int topright_y;
        int bottomright_x;
        int bottomright_y;
    };
    virtual void addTrap(const Trapezoid &trap) = 0;

private:
    friend class QTessellatorPrivate;
    QTessellatorPrivate *d;
};


enum VertexFlags {
    LineBeforeStarts = 0x1,
    LineBeforeEnds = 0x2,
    LineBeforeHorizontal = 0x4,
    LineAfterStarts = 0x8,
    LineAfterEnds = 0x10,
    LineAfterHorizontal = 0x20
};

struct QTessellatorVertex
{
    Q27Dot5 x;
    Q27Dot5 y;
    int position;
    int flags;
};
Q_DECLARE_TYPEINFO(QTessellatorVertex, Q_PRIMITIVE_TYPE);




class QTessellatorPrivate {
public:
    QTessellatorPrivate() : nPoints(0) {}

    QRectF collectAndSortVertices(const QPointF *points, int *maxActiveEdges);
    void cancelCoincidingEdges();

    void markEdgesToEmit();
    void emitEdges(QTessellator *tessellator);
    void processIntersections();
    void processEdges();
    void addIntersections();

    struct Intersection
    {
        Q27Dot5 y;
        int edge;
        bool operator <(const Intersection &other) const {
            if (y != other.y)
                return y < other.y;
            return edge < other.edge;
        }
    };
    struct IntersectionLink
    {
        int next;
        int prev;
    };
    typedef QMap<Intersection, IntersectionLink> Intersections;

    struct Edge {
        Edge(int _edge, int _x0, int _y0, int _x1, int _y1);
        Q27Dot5 x0;
        Q27Dot5 y0;
        Q27Dot5 x1;
        Q27Dot5 y1;
        Q27Dot5 y_left;
        Q27Dot5 y_right;
        int edge;
        int winding;
        bool mark;
        bool intersect_left;
        bool intersect_right;
        mutable bool coincides_left;
        mutable bool coincides_right;
        bool isLeftOf(const Edge &other, Q27Dot5 y) const;
        Q27Dot5 positionAt(Q27Dot5 y) const;
        enum IntersectionStatus {
            DontIntersect,
            DoIntersect,
            Coincide
        };
        IntersectionStatus intersect(const Edge &other, Q27Dot5 *x, Q27Dot5 *y, qint64 *det = 0) const;

    };

    class EdgeSorter
    {
    public:
        EdgeSorter(int _y) : y(_y) {}
        bool operator() (const Edge *e1, const Edge *e2);
        int y;
    };

    class Scanline {
    public:
        Scanline();
        ~Scanline();

        void init(int maxActiveEdges);
        void done();

        int findEdgePosition(Q27Dot5 x, Q27Dot5 y) const;
        int findEdgePosition(const Edge &e) const;
        int findEdge(int edge) const;
        void clearMarks();

        void swap(int p1, int p2) {
            Edge *tmp = edges[p1];
            edges[p1] = edges[p2];
            edges[p2] = tmp;
        }
        void insert(int pos, const Edge &e);
        void removeAt(int pos);
        void markEdges(int pos1, int pos2);

        Edge **edges;
        int size;

    private:
        Edge *edge_table;
        int first_unused;
        int max_edges;
        enum { default_alloc = 32 };
    };

    int nPoints;
    QVector<QTessellatorVertex> vertexData;
    QVector<QTessellatorVertex *> verticesVector;
    const QTessellatorVertex *const *vertices;
    Intersections intersections;
    Scanline scanline;
    bool winding;
    Q27Dot5 y;
    int currentVertex;

private:
    void addIntersection(const Edge *e1, const Edge *e2);
    bool edgeInChain(Intersection i, int edge);
};


QTessellatorPrivate::Edge::Edge(int _edge, int _x0, int _y0, int _x1, int _y1)
{
    Q_ASSERT(_y0 != _y1);

    edge = _edge;
    if (_y0 < _y1) {
        x0 = _x0;
        y0 = _y0;
        x1 = _x1;
        y1 = _y1;
        winding = 1;
    } else {
        x0 = _x1;
        y0 = _y1;
        x1 = _x0;
        y1 = _y0;
        winding = -1;
    }
    y_left = y0;
    y_right = y0;
    intersect_left = true;
    intersect_right = true;
    coincides_left = false;
    coincides_right = false;
}

// This is basically the algorithm from graphics gems. The algorithm is cubic in the coordinates at one place.
// Since we use 64bit integers, this implies, that the allowed range for our coordinates is limited to 21 bits.
// With 5 bits behind the decimal, this implies that differences in coordaintes can range from 2*SHORT_MIN to 2*SHORT_MAX,
// giving us efficiently a coordinate system from SHORT_MIN to SHORT_MAX.

#define SAME_SIGNS( a, b )	\
		(((qint64) ((quint64) a ^ (quint64) b)) >= 0 )

QTessellatorPrivate::Edge::IntersectionStatus QTessellatorPrivate::Edge::intersect(const Edge &other, Q27Dot5 *x, Q27Dot5 *y, qint64 *det) const
{
    qint64 a1 = y1 - y0;
    qint64 b1 = x0 - x1;
    qint64 c1 = qint64(x1) * y0 - qint64(x0) * y1;

    qint64 r3 = a1 * other.x0 + b1 * other.y0 + c1;
    qint64 r4 = a1 * other.x1 + b1 * other.y1 + c1;

    if (r3 == 0 && r4 == 0)
        return Coincide;

    // Check signs of r3 and r4.  If both point 3 and point 4 lie on
    // same side of line 1, the line segments do not intersect.
//     QDEBUG() << "        " << r3 << r4;
    if ( r3 != 0 &&
         r4 != 0 &&
         SAME_SIGNS( r3, r4 ))
        return ( DontIntersect );

    qint64 a2 = other.y1 - other.y0;
    qint64 b2 = other.x0 - other.x1;
    qint64 c2 = qint64(other.x1) * other.y0 - qint64(other.x0) * other.y1;

    qint64 r1 = a2 * x0 + b2 * y0 + c2;
    qint64 r2 = a2 * x1 + b2 * y1 + c2;

    // Check signs of r1 and r2.  If both point 1 and point 2 lie
    // on same side of second line segment, the line segments do not intersect.
//     QDEBUG() << "        " << r1 << r2;
    if ( r1 != 0 &&
         r2 != 0 &&
         SAME_SIGNS( r1, r2 ))
        return ( DontIntersect );

    // Line segments intersect: compute intersection point.

    qint64 denom = a1 * b2 - a2 * b1;
    if (det)
        *det = denom;
    if ( denom == 0 )
        return ( DontIntersect );
    qint64 offset = denom < 0 ? - denom / 2 : denom / 2;

    // The denom/2 is to get rounding instead of truncating.  It
    // is added or subtracted to the numerator, depending upon the
    // sign of the numerator.
    qint64 num = b1 * c2 - b2 * c1;
    *x = ( num < 0 ? num - offset : num + offset ) / denom;

    num = a2 * c1 - a1 * c2;
    *y = ( num < 0 ? num - offset : num + offset ) / denom;

    return ( DoIntersect );
}

#undef SAME_SIGNS

static inline bool compareVertex(const QTessellatorVertex *p1, const QTessellatorVertex *p2)
{
    if (p1->y == p2->y)
        return p1->x < p2->x;
    return p1->y < p2->y;
}

static inline int nextPoint(int point, int nPoints)
{
    ++point;
    if (point == nPoints)
        point = 0;
    return point;
}

static inline int previousPoint(int point, int nPoints)
{
    --point;
    if (point < 0)
        point = nPoints - 1;
    return point;
}

bool QTessellatorPrivate::Edge::isLeftOf(const Edge &other, Q27Dot5 y) const
{
    int p1 = positionAt(y);
    int p2 = other.positionAt(y);
    if (p1 == p2) {
        y = qMax(y1, other.y1);
        p1 = positionAt(y);
        p2 = other.positionAt(y);
    }
    // get some fixed ordering for coinciding edges
    if (p1 == p2)
        return edge < other.edge;
    return (p1 < p2);
}


Q27Dot5 QTessellatorPrivate::Edge::positionAt(Q27Dot5 y) const
{
    if (y == y0)
        return x0;
    else if (y == y1)
        return x1;

    qint64 d = x1 - x0;
    return (x0 + d*(y - y0)/(y1-y0));
}

bool QTessellatorPrivate::EdgeSorter::operator() (const Edge *e1, const Edge *e2)
{
    // this compares the slopes of the two edges. If they are equal we sort
    // after edge index.
    int xi, yi;
    qint64 det;
    Edge::IntersectionStatus res = e1->intersect(*e2, &xi, &yi, &det);
    if (res == Edge::DontIntersect)
        return e1->isLeftOf(*e2, y);
    else if (res == Edge::Coincide)
        return e1->edge < e2->edge;
    bool result = (det < 0);
    if (yi > y)
        result = !result;
//         qDebug() << "Checking e1=" << e1->edge << "e2=" << e2->edge << "i=("<< xi << yi << ") det=" << det << result;
    return result;
}


QTessellatorPrivate::Scanline::Scanline()
{
    edges = 0;
    edge_table = 0;
    init(default_alloc);
}

void QTessellatorPrivate::Scanline::init(int maxActiveEdges)
{
    if (!edges || maxActiveEdges > default_alloc) {
        max_edges = maxActiveEdges;
        int size = qMax(maxActiveEdges + 1, default_alloc + 1);
        edges = (Edge **)realloc(edges, size*sizeof(Edge *));
        edge_table = (Edge *)realloc(edge_table, size*sizeof(Edge));
    }
    size = 0;
    first_unused = 0;
    for (int i = 0; i < maxActiveEdges; ++i)
        edge_table[i].edge = i+1;
    edge_table[maxActiveEdges].edge = -1;
}

void QTessellatorPrivate::Scanline::done()
{
    if (max_edges > default_alloc) {
        free(edges);
        free(edge_table);
        edges = 0;
        edge_table = 0;
    }
}

QTessellatorPrivate::Scanline::~Scanline()
{
    free(edges);
    free(edge_table);
}

int QTessellatorPrivate::Scanline::findEdgePosition(Q27Dot5 x, Q27Dot5 y) const
{
    int min = 0;
    int max = size - 1;
    while (min < max) {
        int pos = min + ((max - min + 1) >> 1);
        Q27Dot5 ax = edges[pos]->positionAt(y);
        if (ax > x) {
            max = pos - 1;
        } else {
            min = pos;
        }
    }
    return min;
}

int QTessellatorPrivate::Scanline::findEdgePosition(const Edge &e) const
{
//     qDebug() << ">>      findEdgePosition";
    int min = 0;
    int max = size;
    while (min < max) {
        int pos = min + ((max - min) >> 1);
//         qDebug() << "        " << min << max << pos << edges[pos]->isLeftOf(e, e.y0);
        if (edges[pos]->isLeftOf(e, e.y0)) {
            min = pos + 1;
        } else {
            max = pos;
        }
    }
//     qDebug() << "<<      findEdgePosition got" << min;
    return min;
}

int QTessellatorPrivate::Scanline::findEdge(int edge) const
{
    for (int i = 0; i < size; ++i) {
        int item_edge = edges[i]->edge;
        if (item_edge == edge)
            return i;
    }
    Q_ASSERT(false);
    return -1;
}

void QTessellatorPrivate::Scanline::clearMarks()
{
    for (int i = 0; i < size; ++i) {
        edges[i]->mark = false;
        edges[i]->intersect_left = false;
        edges[i]->intersect_right = false;
    }
}

void QTessellatorPrivate::Scanline::insert(int pos, const Edge &e)
{
    Edge *edge = edge_table + first_unused;
    first_unused = edge->edge;
    Q_ASSERT(first_unused != -1);
    *edge = e;
    memmove(edges + pos + 1, edges + pos, (size - pos)*sizeof(Edge *));
    edges[pos] = edge;
    ++size;
}

void QTessellatorPrivate::Scanline::removeAt(int pos)
{
    Edge *e = edges[pos];
    e->edge = first_unused;
    first_unused = (e - edge_table);
    --size;
    memmove(edges + pos, edges + pos + 1, (size - pos)*sizeof(Edge *));
}

void QTessellatorPrivate::Scanline::markEdges(int pos1, int pos2)
{
    if (pos2 < pos1)
        return;
    while (pos1 > 0 && edges[pos1]->coincides_left)
        --pos1;
    while (pos2 < size - 1 && edges[pos2]->coincides_right)
        ++pos2;

    for (int i = pos1; i <= pos2; ++i)
        edges[i]->mark = true;
}

static inline void fillTrapezoid(Q27Dot5 y1, Q27Dot5 y2, int left, int right,
                                 const QVector<QTessellatorVertex> &_vertexData, QTessellator::Trapezoid *trap)
{
    int nPoints = _vertexData.size();
    const QTessellatorVertex *vertexData = _vertexData.constData();
    trap->top = y1;
    trap->bottom = y2;
    int next = nextPoint(left, nPoints);
    if (vertexData[left].y < vertexData[next].y) {
        trap->topleft_x = vertexData[left].x;
        trap->topleft_y = vertexData[left].y;
        trap->bottomleft_x = vertexData[next].x;
        trap->bottomleft_y = vertexData[next].y;
    } else {
        trap->topleft_x = vertexData[next].x;
        trap->topleft_y = vertexData[next].y;
        trap->bottomleft_x = vertexData[left].x;
        trap->bottomleft_y = vertexData[left].y;
    }
    next = nextPoint(right, nPoints);
    if (vertexData[right].y < vertexData[next].y) {
        trap->topright_x = vertexData[right].x;
        trap->topright_y = vertexData[right].y;
        trap->bottomright_x = vertexData[next].x;
        trap->bottomright_y = vertexData[next].y;
    } else {
        trap->topright_x = vertexData[next].x;
        trap->topright_y = vertexData[next].y;
        trap->bottomright_x = vertexData[right].x;
        trap->bottomright_y = vertexData[right].y;
    }
}

QRectF QTessellatorPrivate::collectAndSortVertices(const QPointF *points, int *maxActiveEdges)
{
    *maxActiveEdges = 0;
    QTessellatorVertex *v = vertexData.data();
    QTessellatorVertex **vv = verticesVector.data();

    qreal xmin(points[0].x());
    qreal xmax(points[0].x());
    qreal ymin(points[0].y());
    qreal ymax(points[0].y());

    // collect vertex data
    Q27Dot5 y_prev = FloatToQ27Dot5(points[nPoints-1].y());
    Q27Dot5 x_next = FloatToQ27Dot5(points[0].x());
    Q27Dot5 y_next = FloatToQ27Dot5(points[0].y());
    int j = 0;
    int i = 0;
    while (i < nPoints) {
        Q27Dot5 y = y_next;

        *vv = v;

        v->x = x_next;
        v->y = y_next;
        v->position = j;

    next_point:

        xmin = qMin(xmin, points[i+1].x());
        xmax = qMax(xmax, points[i+1].x());
        ymin = qMin(ymin, points[i+1].y());
        ymax = qMax(ymax, points[i+1].y());

        y_next = FloatToQ27Dot5(points[i+1].y());
        x_next = FloatToQ27Dot5(points[i+1].x());

        // skip vertices on top of each other
        if (v->x == x_next && v->y == y_next) {
            ++i;
            if (i < nPoints)
                goto next_point;
            QTessellatorVertex *v0 = vertexData.data();
            v0->flags &= ~(LineBeforeStarts|LineBeforeEnds|LineBeforeHorizontal);
            if (y_prev < y)
                v0->flags |= LineBeforeEnds;
            else if (y_prev > y)
                v0->flags |= LineBeforeStarts;
            else
                v0->flags |= LineBeforeHorizontal;
            if ((v0->flags & (LineBeforeStarts|LineAfterStarts))
                && !(v0->flags & (LineAfterEnds|LineBeforeEnds)))
                *maxActiveEdges += 2;
            break;
        }

        v->flags = 0;
        if (y_prev < y)
            v->flags |= LineBeforeEnds;
        else if (y_prev > y)
            v->flags |= LineBeforeStarts;
        else
            v->flags |= LineBeforeHorizontal;


        if (y < y_next)
            v->flags |= LineAfterStarts;
        else if (y > y_next)
            v->flags |= LineAfterEnds;
        else
            v->flags |= LineAfterHorizontal;
        // ### could probably get better limit by looping over sorted list and counting down on ending edges
        if ((v->flags & (LineBeforeStarts|LineAfterStarts))
            && !(v->flags & (LineAfterEnds|LineBeforeEnds)))
            *maxActiveEdges += 2;
        y_prev = y;
        ++v;
        ++vv;
        ++j;
        ++i;
    }
    nPoints = j;
    // #### get rid of me
    vertexData.resize(j);
    verticesVector.resize(j);

    QDEBUG() << "maxActiveEdges=" << *maxActiveEdges;
    vv = verticesVector.data();
    qSort(vv, vv + nPoints, compareVertex);

    return QRectF(xmin, ymin, xmax-xmin, ymax-ymin);
}

void QTessellatorPrivate::cancelCoincidingEdges()
{
    QTessellatorVertex *data = vertexData.data();
    QTessellatorVertex **vv = verticesVector.data();

    for (int i= 0; i < nPoints - 1; ++i) {
        QTessellatorVertex *v = vv[i];
        int j = i;
        while (j < nPoints - 1) {
            ++j;
            QTessellatorVertex *n = vv[j];
            if (v->y != n->y || v->x != n->x)
                break;

            if ((v->flags & LineBeforeStarts) && n->flags & LineAfterStarts) {
                int v_prev = previousPoint(v->position, nPoints);
                int n_next = nextPoint(n->position, nPoints);
                if ((data[v_prev].y == data[n_next].y)
                    && (data[v_prev].x == data[n_next].x)) {
                    v->flags &= ~LineBeforeStarts;
                    data[v_prev].flags &= ~LineAfterEnds;
                    n->flags &= ~LineAfterStarts;
                    data[n_next].flags &= ~LineBeforeEnds;
                }
            }
            if ((v->flags & LineAfterStarts) && n->flags & LineBeforeStarts) {
                int v_next = nextPoint(v->position, nPoints);
                int n_prev = previousPoint(n->position, nPoints);
                if ((data[v_next].y == data[n_prev].y)
                    && (data[v_next].x == data[n_prev].x)) {
                    v->flags &= ~LineAfterStarts;
                    data[v_next].flags &= ~LineBeforeEnds;
                    n->flags &= ~LineBeforeStarts;
                    data[n_prev].flags &= ~LineAfterEnds;
                }
            }
            if (winding)
                continue;
            if ((v->flags & LineBeforeStarts) && n->flags & LineBeforeStarts) {
                int v_prev = previousPoint(v->position, nPoints);
                int n_prev = previousPoint(n->position, nPoints);
                if ((data[v_prev].y == data[n_prev].y)
                    && (data[v_prev].x == data[n_prev].x)) {
                    v->flags &= ~LineBeforeStarts;
                    data[v_prev].flags &= ~LineAfterEnds;
                    n->flags &= ~LineBeforeStarts;
                    data[n_prev].flags &= ~LineAfterEnds;
                }
            }
            if ((v->flags & LineAfterStarts) && n->flags & LineAfterStarts) {
                int v_next = nextPoint(v->position, nPoints);
                int n_next = nextPoint(n->position, nPoints);
                if ((data[v_next].y == data[n_next].y)
                    && (data[v_next].x == data[n_next].x)) {
                    v->flags &= ~LineAfterStarts;
                    data[v_next].flags &= ~LineBeforeEnds;
                    n->flags &= ~LineAfterStarts;
                    data[n_next].flags &= ~LineBeforeEnds;
                }
            }
        }
    }
}



void QTessellatorPrivate::markEdgesToEmit()
{
//     QDEBUG() << "MARK";
    for (Intersections::const_iterator it = intersections.constBegin();
         it != intersections.constEnd(); ++it) {
        const Intersection &i = it.key();
        if (i.y > y)
            break;
//         QDEBUG() << "    got intersection edge" << i.edge << "(next=" << it->next << ")";
        scanline.edges[scanline.findEdge(i.edge)]->mark = true;
    }
    for (int i = currentVertex; i < nPoints; ++i) {
        const QTessellatorVertex *v = vertices[i];
        if (v->y > y)
            break;
//         QDEBUG() << "    got vertex "
//                  << i << "point=" << vertices[i]->position;
        if (v->flags & LineBeforeEnds) {
            int pos = scanline.findEdge(previousPoint(v->position, nPoints));
            scanline.markEdges(pos, pos);
        }
        if (v->flags & LineAfterEnds) {
            int pos = scanline.findEdge(v->position);
            scanline.markEdges(pos, pos);
        }
        if (v->flags & (LineAfterStarts|LineBeforeStarts)) {
            int pos = scanline.findEdgePosition(v->x, v->y);
            int pos1 = pos;
            int pos2 = pos;
            if (pos1 > 0)
                --pos1;
            if (pos2 == scanline.size)
                --pos2;
            scanline.markEdges(pos1, pos2);
        }
        if (v->flags & LineAfterHorizontal) {
            int pos1 = scanline.findEdgePosition(v->x, v->y);
            const QTessellatorVertex &next = vertexData[nextPoint(v->position, nPoints)];
            Q_ASSERT(v->y == next.y);
            int pos2 = scanline.findEdgePosition(next.x, next.y);
            if (pos2 < pos1)
                qSwap(pos1, pos2);
            if (pos1 > 0)
                --pos1;
            if (pos2 == scanline.size)
                --pos2;
//             QDEBUG() << "marking horizontal edge from " << pos1 << "to" << pos2;
            scanline.markEdges(pos1, pos2);
        }

    }
}

void QTessellatorPrivate::emitEdges(QTessellator *tessellator)
{
    //QDEBUG() << "TRAPS:";
    if (!scanline.size)
        return;

    // emit edges
    if (winding) {
        // winding fill rule
        int winding = 0;

        scanline.edges[0]->y_left = y;

        for (int i = 0; i < scanline.size - 1; ++i) {
            Edge *left = scanline.edges[i];
            Edge *right = scanline.edges[i+1];
            winding += left->winding;
//             qDebug() << "i=" << i << "edge->winding=" << left->winding << "winding=" << winding;
            if (winding == 0) {
                left->y_right = y;
                right->y_left = y;
            } else if (left->mark || right->mark) {
                Q27Dot5 top = qMax(left->y_right, right->y_left);
                if (top != y) {
                    QTessellator::Trapezoid trap;
                    fillTrapezoid(top, y, left->edge, right->edge, vertexData, &trap);
                    tessellator->addTrap(trap);
                    QDEBUG() << "    top=" << Q27Dot5ToDouble(top) << "left=" << left->edge << "right=" << right->edge;
                }
                right->y_left = y;
                left->y_right = y;
            }
            left->mark = false;
        }
        if (scanline.edges[scanline.size - 1]->mark) {
            scanline.edges[scanline.size - 1]->y_right = y;
            scanline.edges[scanline.size - 1]->mark = false;
        }
    } else {
        // odd-even fill rule
        for (int i = 0; i < scanline.size; i += 2) {
            Edge *left = scanline.edges[i];
            Edge *right = scanline.edges[i+1];
            if (left->mark || right->mark) {
                Q27Dot5 top = qMax(left->y_right, right->y_left);
                if (top != y) {
                    QTessellator::Trapezoid trap;
                    fillTrapezoid(top, y, left->edge, right->edge, vertexData, &trap);
                    tessellator->addTrap(trap);
                }
//                 QDEBUG() << "    top=" << Q27Dot5ToDouble(top) << "left=" << left->edge << "right=" << right->edge;
                left->y_left = y;
                left->y_right = y;
                right->y_left = y;
                right->y_right = y;
                left->mark = right->mark = false;
            }
        }
    }
}


void QTessellatorPrivate::processIntersections()
{
    QDEBUG() << "PROCESS INTERSECTIONS";
    // process intersections
    while (!intersections.isEmpty()) {
        Intersections::iterator it = intersections.begin();
        if (it.key().y != y)
            break;

        // swap edges
        QDEBUG() << "    swapping intersecting edges ";
        int min = scanline.size;
        int max = 0;
        while (1) {
            const Intersection &i = it.key();
            int next = it->next;

            int edgePos = scanline.findEdge(i.edge);
            min = qMin(edgePos, min);
            max = qMax(edgePos, max);
            Edge *edge = scanline.edges[edgePos];
            edge->y_left = edge->y_right = y;
            intersections.remove(i);

            Intersection key;
            key.y = y;
            key.edge = next;
            it = intersections.find(key);
            if (it == intersections.end())
                break;
        }
        Q_ASSERT(min != max);
        QDEBUG() << "sorting between" << min << "and" << max;
        qSort(scanline.edges + min, scanline.edges + max + 1, EdgeSorter(y + 1));
#ifdef DEBUG
        for (int i = min; i <= max; ++i)
            QDEBUG() << "        " << scanline.edges[i]->edge << "at pos" << i;
#endif
        for (int i = min; i <= max; ++i) {
            scanline.edges[i]->intersect_left = true;
            scanline.edges[i]->intersect_right = true;
        }
    }
}

void QTessellatorPrivate::processEdges()
{
    QDEBUG() << "ADD/REMOVE";
    // add and remove edges
    while (currentVertex < nPoints) {
        const QTessellatorVertex *v = vertices[currentVertex];
        if (v->y > y)
            break;
        if (!v->flags)
            goto done;
        if ((v->flags & (LineBeforeEnds|LineAfterStarts)) == (LineBeforeEnds|LineAfterStarts)) {
            int pos = scanline.findEdge(previousPoint(v->position, nPoints));
            Edge *edge = scanline.edges[pos];
            QDEBUG() << "    changing edge" << previousPoint(v->position,nPoints)
                     << "to" << v->position;
            int end = nextPoint(v->position, nPoints);
            *edge = Edge(v->position, v->x, v->y, vertexData[end].x, vertexData[end].y);

            // ensure correct positioning
            while (pos > 0 && edge->isLeftOf(*scanline.edges[pos-1], y)) {
                QDEBUG() << "edge at wrong position, moving one left";
                scanline.edges[pos - 1]->intersect_right = true;
                qSwap(scanline.edges[pos], scanline.edges[pos-1]);
                --pos;
            }
            while (pos < scanline.size - 1 && scanline.edges[pos+1]->isLeftOf(*edge, y)) {
                QDEBUG() << "edge at wrong position, moving one right";
                scanline.edges[pos + 1]->intersect_left = true;
                qSwap(scanline.edges[pos], scanline.edges[pos+1]);
                ++pos;
            }
            goto done;
        } else if ((v->flags & (LineBeforeStarts|LineAfterEnds)) == (LineBeforeStarts|LineAfterEnds)) {
            int pos = scanline.findEdge(v->position);
            Edge *edge = scanline.edges[pos];
            QDEBUG() << "    changing edge" << v->position
                     << "to" << previousPoint(v->position, nPoints);
            int start = previousPoint(v->position, nPoints);
            *edge = Edge(start, vertexData[start].x, vertexData[start].y, v->x, v->y);

            // ensure correct positioning
            while (pos > 0 && edge->isLeftOf(*scanline.edges[pos-1], y)) {
                QDEBUG() << "edge at wrong position, moving one left";
                scanline.edges[pos - 1]->intersect_right = true;
                qSwap(scanline.edges[pos], scanline.edges[pos-1]);
                --pos;
            }
            while (pos < scanline.size - 1 && scanline.edges[pos+1]->isLeftOf(*edge, y)) {
                QDEBUG() << "edge at wrong position, moving one right";
                scanline.edges[pos + 1]->intersect_left = true;
                qSwap(scanline.edges[pos], scanline.edges[pos+1]);
                ++pos;
            }
            goto done;
        }

        if (v->flags & LineBeforeStarts) {
            // add new edge
            int start = previousPoint(v->position, nPoints);
            Edge e(start, vertexData[start].x, vertexData[start].y, v->x, v->y);
            int pos = scanline.findEdgePosition(e);
            QDEBUG() << "    adding edge" << start << "at position" << pos;
            scanline.insert(pos, e);
        }
        if (v->flags & LineAfterStarts) {
            int end = nextPoint(v->position, nPoints);
            Edge e(v->position, v->x, v->y, vertexData[end].x, vertexData[end].y);
            int pos = scanline.findEdgePosition(e);
            QDEBUG() << "    adding edge" << v->position << "at position" << pos;
            scanline.insert(pos, e);
        }
        if (v->flags & LineBeforeEnds) {
            QDEBUG() << "    removing edge" << previousPoint(v->position, nPoints);
            int pos = scanline.findEdge(previousPoint(v->position, nPoints));
            if (pos > 0)
                scanline.edges[pos - 1]->intersect_right = true;
            if (pos < scanline.size - 1)
                scanline.edges[pos + 1]->intersect_left = true;
            scanline.removeAt(pos);
        }
        if (v->flags & LineAfterEnds) {
            QDEBUG() << "    removing edge" << v->position;
            int pos = scanline.findEdge(v->position);
            if (pos > 0)
                scanline.edges[pos - 1]->intersect_right = true;
            if (pos < scanline.size - 1)
                scanline.edges[pos + 1]->intersect_left = true;
            scanline.removeAt(pos);
        }
    done:
        ++currentVertex;
#if 0 //def DEBUG
        QDEBUG()<< "scan: y =" << Q27Dot5ToDouble(y);
        for (int i = 0; i < scanline.size(); ++i) {
            QDEBUG() << "   " << scanline.at(i).edge
                     << "p0= (" << Q27Dot5ToDouble(scanline.at(i).x0)
                     << "/" << Q27Dot5ToDouble(scanline.at(i).y0)
                     << ") p1= (" << Q27Dot5ToDouble(scanline.at(i).x1)
                     << "/" << Q27Dot5ToDouble(scanline.at(i).y1) << ")";
        }
#endif
    }
}


#ifdef DEBUG
static void checkLinkChain(const Intersections &intersections, Intersection i)
{
//     qDebug() << "              Link chain: ";
    int end = i.edge;
    while (1) {
        IntersectionLink l = intersections.value(i);
//         qDebug() << "                     " << i.edge << "next=" << l.next << "prev=" << l.prev;
        if (l.next == end)
            break;
        Q_ASSERT(l.next != -1);
        Q_ASSERT(l.prev != -1);

        Intersection i2 = i;
        i2.edge = l.next;
        IntersectionLink l2 = intersections.value(i2);

        Q_ASSERT(l2.next != -1);
        Q_ASSERT(l2.prev != -1);
        Q_ASSERT(l.next == i2.edge);
        Q_ASSERT(l2.prev == i.edge);
        i = i2;
    }
}
#endif

bool QTessellatorPrivate::edgeInChain(Intersection i, int edge)
{
    int end = i.edge;
    while (1) {
        if (i.edge == edge)
            return true;
        IntersectionLink l = intersections.value(i);
        if (l.next == end)
            break;
        Q_ASSERT(l.next != -1);
        Q_ASSERT(l.prev != -1);

        Intersection i2 = i;
        i2.edge = l.next;
        IntersectionLink l2 = intersections.value(i2);

        Q_ASSERT(l2.next != -1);
        Q_ASSERT(l2.prev != -1);
        Q_ASSERT(l.next == i2.edge);
        Q_ASSERT(l2.prev == i.edge);
        i = i2;
    }
    return false;
}


void QTessellatorPrivate::addIntersection(const Edge *e1, const Edge *e2)
{
    const IntersectionLink emptyLink = {-1, -1};

    QDEBUG("checking edges %d and %d", e1->edge, e2->edge);
    Q27Dot5 xi, yi;
    QTessellatorPrivate::Edge::IntersectionStatus type = e1->intersect(*e2, &xi, &yi);
    if (type == QTessellatorPrivate::Edge::DontIntersect) {
        QDEBUG() << "    no intersection";
        return;
    }

    if (type == QTessellatorPrivate::Edge::Coincide) {
        QDEBUG() << "    coinciding";
        e1->coincides_right = true;
        e2->coincides_left = true;
        return;
    }
    // don't emit an intersection if it's at the start of a line segment or above us
    if (yi < y) {
        QDEBUG() << "    yi < y";
        return;
    }
    if ((xi == e1->x0 && yi == e1->y0)
        || (xi == e2->x0 && yi == e2->y0)) {
        QDEBUG() << "    top of one edge";
        return;
    }
    if (yi == y) {
        // PERF: avoid the EdgeSorter as it calls Edge::intersect again.
        // only add if the edges are not already in correct order
        QTessellatorPrivate::EdgeSorter sorter(y);
        QDEBUG() << "yi == y" << e1->isLeftOf(*e2, y) << e1->positionAt(y) << e2->positionAt(y) << sorter(e1, e2);
        if (sorter(e1, e2))
            return;
    }
    QDEBUG() << "   between edges " << e1->edge << "and" << e2->edge << "at point ("
             << Q27Dot5ToDouble(xi) << "/" << Q27Dot5ToDouble(yi) << ")";

    Intersection i1;
    i1.y = yi;
    i1.edge = e1->edge;
    IntersectionLink link1 = intersections.value(i1, emptyLink);
    Intersection i2;
    i2.y = yi;
    i2.edge = e2->edge;
    IntersectionLink link2 = intersections.value(i2, emptyLink);

    // new pair of edges
    if (link1.next == -1 && link2.next == -1) {
        link1.next = link1.prev = i2.edge;
        link2.next = link2.prev = i1.edge;
    } else if (link1.next == i2.edge || link1.prev == i2.edge
               || link2.next == i1.edge || link2.prev == i1.edge) {
#ifdef DEBUG
        checkLinkChain(intersections, i1);
        checkLinkChain(intersections, i2);
        Q_ASSERT(edgeInChain(i1, i2.edge));
#endif
        return;
    } else if (link1.next == -1 || link2.next == -1) {
        if (link2.next == -1) {
            qSwap(i1, i2);
            qSwap(link1, link2);
        }
        Q_ASSERT(link1.next == -1);
#ifdef DEBUG
        checkLinkChain(intersections, i2);
#endif
        // only i2 in list
        link1.next = i2.edge;
        link1.prev = link2.prev;
        link2.prev = i1.edge;
        Intersection other;
        other.y = yi;
        other.edge = link1.prev;
        IntersectionLink link = intersections.value(other, emptyLink);
        Q_ASSERT(link.next == i2.edge);
        Q_ASSERT(link.prev != -1);
        link.next = i1.edge;
        intersections.insert(other, link);
    } else {
        bool connected = edgeInChain(i1, i2.edge);
        if (connected)
            return;
#ifdef DEBUG
        checkLinkChain(intersections, i1);
        checkLinkChain(intersections, i2);
#endif
        // both already in some list. Have to make sure they are connected
        // this can be done by cutting open the ring(s) after the two eges and
        // connecting them again
        Intersection other1;
        other1.y = yi;
        other1.edge = link1.next;
        IntersectionLink linko1 = intersections.value(other1, emptyLink);
        Intersection other2;
        other2.y = yi;
        other2.edge = link2.next;
        IntersectionLink linko2 = intersections.value(other2, emptyLink);

        linko1.prev = i2.edge;
        link2.next = other1.edge;

        linko2.prev = i1.edge;
        link1.next = other2.edge;
        intersections.insert(other1, linko1);
        intersections.insert(other2, linko2);
    }
    intersections.insert(i1, link1);
    intersections.insert(i2, link2);
#ifdef DEBUG
    checkLinkChain(intersections, i1);
    checkLinkChain(intersections, i2);
    Q_ASSERT(edgeInChain(i1, i2.edge));
#endif
    return;

}


void QTessellatorPrivate::addIntersections()
{
    if (scanline.size) {
        QDEBUG() << "INTERSECTIONS";
        // check marked edges for intersections
#ifdef DEBUG
        for (int i = 0; i < scanline.size(); ++i) {
            Edge *e = scanline[i];
            QDEBUG() << "    " << i << e->edge << "isect=(" << e->intersect_left << e->intersect_right
                     << ") coincides=(" << e->coincides_left << e->coincides_right << ")";
        }
#endif

        for (int i = 0; i < scanline.size - 1; ++i) {
            Edge *e1 = scanline.edges[i];
            Edge *e2 = scanline.edges[i + 1];
            if (!e1->intersect_right && !e2->intersect_left
                && !e1->coincides_left && !e2->coincides_left)
                continue;

            // check for intersection
            if (e1->intersect_right || e2->intersect_left)
                addIntersection(e1, e2);
            if (e1->coincides_left || e2->coincides_left) {
                QDEBUG() << "    coincides" << e1->coincides_left << e2->coincides_left;
                int j = i;
                while (j > 0) {
                    --j;
                    Edge *e = scanline.edges[j];
                    //if (e->intersect_right || e2->intersect_left)
                    addIntersection(e, e2);
                    if (!e->coincides_right)
                        break;
                }
            }
        }
    }
#if 0
    if (intersections.constBegin().key().y == y) {
        QDEBUG() << "----------------> intersection on same line";
        scanline.clearMarks();
        scanline.processIntersections(y, &intersections);
        goto redo;
    }
#endif
}


QTessellator::QTessellator()
{
    d = new QTessellatorPrivate;
}

QTessellator::~QTessellator()
{
    delete d;
}

void QTessellator::setWinding(bool w)
{
    d->winding = w;
}


QRectF QTessellator::tesselate(const QPointF *points, int _nPoints)
{
    Q_ASSERT(points[0] == points[_nPoints-1]);
    --_nPoints;

#ifdef DEBUG
    QDEBUG()<< "POINTS:";
    for (int i = 0; i < _nPoints; ++i) {
        QDEBUG() << points[i];
    }
#endif

    d->nPoints = _nPoints;
    // collect edges and calculate bounds
    d->vertexData.resize(d->nPoints);
    d->verticesVector.resize(d->nPoints);
    d->vertices = d->verticesVector.constData();

    int maxActiveEdges = 0;
    QRectF br = d->collectAndSortVertices(points, &maxActiveEdges);
    d->cancelCoincidingEdges();

#ifdef DEBUG
    QDEBUG() << "nPoints = " << _nPoints << "using " << d->nPoints;
    QDEBUG()<< "VERTICES:";
    for (int i = 0; i < d->nPoints; ++i) {
        QDEBUG() << "    " << i << ": "
                 << "point=" << d->vertices[i]->position
                 << "flags=" << d->vertices[i]->flags
                 << "pos=(" << Q27Dot5ToDouble(d->vertices[i]->x) << "/"
                 << Q27Dot5ToDouble(d->vertices[i]->y) << ")";
    }
#endif

    d->scanline.init(maxActiveEdges);
    d->y = INT_MIN/256;
    d->currentVertex = 0;

    while (d->currentVertex < d->nPoints) {
        d->scanline.clearMarks();

        d->y = d->vertices[d->currentVertex]->y;
        if (!d->intersections.isEmpty())
            d->y = qMin(d->y, d->intersections.constBegin().key().y);

        // qDebug()<< "===== SCANLINE: y =" << Q27Dot5ToDouble(y) << " =====";
#ifdef DEBUG
        QDEBUG()<< "===== SCANLINE: y =" << Q27Dot5ToDouble(d->y) << " =====";
        for (int i = 0; i < d->scanline.size(); ++i) {
            QDEBUG() << "   " << d->scanline[i]->edge
                     << "p0= (" << Q27Dot5ToDouble(d->scanline[i]->x0)
                     << "/" << Q27Dot5ToDouble(d->scanline[i]->y0)
                     << ") p1= (" << Q27Dot5ToDouble(d->scanline[i]->x1)
                     << "/" << Q27Dot5ToDouble(d->scanline[i]->y1) << ")";
        }
#endif

        d->markEdgesToEmit();
        d->emitEdges(this);
        d->processIntersections();
        d->processEdges();
        d->addIntersections();
    }

    d->scanline.done();
    d->intersections.clear();
    return br;
}

class TestTessellator : public QTessellator
{
public:
    QVector<XTrapezoid> *traps;
    void addTrap(const Trapezoid &trap);
};

void TestTessellator::addTrap(const Trapezoid &trap)
{
    XTrapezoid xtrap;
    xtrap.top = Q27Dot5ToXFixed(trap.top);
    xtrap.bottom = Q27Dot5ToXFixed(trap.bottom);
    xtrap.left.p1.x = Q27Dot5ToXFixed(trap.topleft_x);
    xtrap.left.p1.y = Q27Dot5ToXFixed(trap.topleft_y);
    xtrap.left.p2.x = Q27Dot5ToXFixed(trap.bottomleft_x);
    xtrap.left.p2.y = Q27Dot5ToXFixed(trap.bottomleft_y);
    xtrap.right.p1.x = Q27Dot5ToXFixed(trap.topright_x);
    xtrap.right.p1.y = Q27Dot5ToXFixed(trap.topright_y);
    xtrap.right.p2.x = Q27Dot5ToXFixed(trap.bottomright_x);
    xtrap.right.p2.y = Q27Dot5ToXFixed(trap.bottomright_y);
    traps->append(xtrap);
}


void l_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *points, int nPoints,
                         bool winding)
{
    TestTessellator t;
    t.traps = traps;
    t.setWinding(winding);
    t.tesselate(points, nPoints);
}
