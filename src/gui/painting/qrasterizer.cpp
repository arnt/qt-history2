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

#include "qrasterizer_p.h"

#include <QPoint>
#include <QRect>

#include <private/qmath_p.h>
#include <private/qdatabuffer_p.h>
#include <private/qdrawhelper_p.h>

typedef int Q16Dot16;
#define Q16Dot16ToFloat(i) ((i)/65536.)
#define FloatToQ16Dot16(i) (int)((i) * 65536.)
#define IntToQ16Dot16(i) ((i) << 16)
#define Q16Dot16ToInt(i) ((i) >> 16)
#define Q16Dot16Factor 65536

#define Q16Dot16Multiply(x, y) (int)((qlonglong(x) * qlonglong(y)) >> 16)
#define Q16Dot16FastMultiply(x, y) (((x) * (y)) >> 16)

#define SPAN_BUFFER_SIZE 256

class QSpanBuffer {
public:
    QSpanBuffer(ProcessSpans blend, void *data, const QRect &clipRect)
        : m_spanCount(0)
        , m_blend(blend)
        , m_data(data)
        , m_clipRect(clipRect)
    {
    }

    ~QSpanBuffer()
    {
        flushSpans();
    }

    void addSpan(int x, unsigned int len, int y, unsigned char coverage)
    {
        if (!coverage || !len)
            return;

        Q_ASSERT(y >= m_clipRect.top());
        Q_ASSERT(y <= m_clipRect.bottom());
        Q_ASSERT(x >= m_clipRect.left());
        Q_ASSERT(x + int(len) - 1 <= m_clipRect.right());

        m_spans[m_spanCount].x = x;
        m_spans[m_spanCount].len = len;
        m_spans[m_spanCount].y = y;
        m_spans[m_spanCount].coverage = coverage;

        if (++m_spanCount == SPAN_BUFFER_SIZE)
            flushSpans();
    }

private:
    void flushSpans()
    {
        m_blend(m_spanCount, m_spans, m_data);
        m_spanCount = 0;
    }

    QT_FT_Span m_spans[SPAN_BUFFER_SIZE];
    int m_spanCount;

    ProcessSpans m_blend;
    void *m_data;

    QRect m_clipRect;
};

#define CHUNK_SIZE 64
class QScanConverter
{
public:
    QScanConverter();
    ~QScanConverter();

    void begin(int top, int bottom, int left, int right,
               Qt::FillRule fillRule, QSpanBuffer *spanBuffer);
    void end();

    void mergeCurve(const QT_FT_Vector &a, const QT_FT_Vector &b,
                    const QT_FT_Vector &c, const QT_FT_Vector &d);
    void mergeLine(QT_FT_Vector a, QT_FT_Vector b);

    struct Line
    {
        Q16Dot16 x;
        Q16Dot16 delta;

        int top, bottom;

        int winding;
    };

private:
    struct Intersection
    {
        int x;
        int winding;

        int left, right;
    };

    inline bool clip(Q16Dot16 &xFP, int &iTop, int &iBottom, Q16Dot16 slopeFP, Q16Dot16 invSlopeFP, Q16Dot16 edgeFP, int winding);
    inline void mergeIntersection(Intersection *head, const Intersection &isect);

    void prepareChunk();

    void emitNode(const Intersection *node);
    void emitSpans(int chunk);

    inline void allocate(int size);

    QDataBuffer<Line> m_lines;

    int m_alloc;
    int m_size;

    int m_top;
    int m_bottom;

    Q16Dot16 m_leftFP;
    Q16Dot16 m_rightFP;

    int m_fillRuleMask;

    int m_x;
    int m_y;
    int m_winding;

    Intersection *m_intersections;

    QSpanBuffer *m_spanBuffer;

    QDataBuffer<Line *> m_active;

    template <typename T>
    friend void scanConvert(QScanConverter &d, T allVertical);
};

class QRasterizerPrivate
{
public:
    bool antialiased;
    ProcessSpans blend;
    void *data;
    QRect clipRect;

    QScanConverter scanConverter;
};

QScanConverter::QScanConverter()
   : m_alloc(0)
   , m_size(0)
   , m_intersections(0)
{
}

QScanConverter::~QScanConverter()
{
    if (m_intersections)
        free(m_intersections);
}

void QScanConverter::begin(int top, int bottom, int left, int right,
                           Qt::FillRule fillRule, QSpanBuffer *spanBuffer)
{
    m_top = top;
    m_bottom = bottom;
    m_leftFP = IntToQ16Dot16(left);
    m_rightFP = IntToQ16Dot16(right + 1);

    m_lines.reset();

    m_fillRuleMask = fillRule == Qt::WindingFill ? ~0x0 : 0x1;
    m_spanBuffer = spanBuffer;
}

void QScanConverter::prepareChunk()
{
    m_size = CHUNK_SIZE;

    allocate(CHUNK_SIZE);
    memset(m_intersections, 0, CHUNK_SIZE * sizeof(Intersection));
}

void QScanConverter::emitNode(const Intersection *node)
{
tail_call:
    if (node->left)
        emitNode(node + node->left);

    if (m_winding & m_fillRuleMask)
        m_spanBuffer->addSpan(m_x, node->x - m_x, m_y, 0xff);

    m_x = node->x;
    m_winding += node->winding;

    if (node->right) {
        node += node->right;
        goto tail_call;
    }
}

void QScanConverter::emitSpans(int chunk)
{
    for (int dy = 0; dy < CHUNK_SIZE; ++dy) {
        m_x = 0;
        m_y = chunk + dy;
        m_winding = 0;

        emitNode(&m_intersections[dy]);
    }
}

// split control points b[0] ... b[3] into
// left (b[0] ... b[3]) and right (b[3] ... b[6])
static void split(QT_FT_Vector *b)
{
    b[6] = b[3];

    {
        const QT_FT_Pos temp = (b[1].x + b[2].x)/2;

        b[1].x = (b[0].x + b[1].x)/2;
        b[5].x = (b[2].x + b[3].x)/2;
        b[2].x = (b[1].x + temp)/2;
        b[4].x = (b[5].x + temp)/2;
        b[3].x = (b[2].x + b[4].x)/2;
    }
    {
        const QT_FT_Pos temp = (b[1].y + b[2].y)/2;

        b[1].y = (b[0].y + b[1].y)/2;
        b[5].y = (b[2].y + b[3].y)/2;
        b[2].y = (b[1].y + temp)/2;
        b[4].y = (b[5].y + temp)/2;
        b[3].y = (b[2].y + b[4].y)/2;
    }
}

static inline bool topOrder(const QScanConverter::Line &a, const QScanConverter::Line &b)
{
    return a.top < b.top;
}

static inline bool xOrder(const QScanConverter::Line *a, const QScanConverter::Line *b)
{
    return a->x < b->x;
}

struct True
{
    inline bool operator()() const { return true; }
};

struct False
{
    inline bool operator()() const { return false; }
};

// should be a member function but VC6 doesn't support member template functions
template <typename T>
static void scanConvert(QScanConverter &d, T allVertical)
{
    qSort(d.m_lines.data(), d.m_lines.data() + d.m_lines.size(), topOrder);
    int line = 0;
    for (int y = d.m_lines.first().top; y <= d.m_bottom; ++y) {
        for (; line < d.m_lines.size() && d.m_lines.at(line).top == y; ++line) {
            // add node to active list
            if (allVertical()) {
                QScanConverter::Line *l = &d.m_lines.at(line);
                d.m_active.resize(d.m_active.size() + 1);
                int j;
                for (j = d.m_active.size() - 2; j >= 0 && xOrder(l, d.m_active.at(j)); --j)
                    d.m_active.at(j+1) = d.m_active.at(j);
                d.m_active.at(j+1) = l;
            } else {
                d.m_active << &d.m_lines.at(line);
            }
        }

        int numActive = d.m_active.size();
        if (!allVertical()) {
        // use insertion sort instead of qSort, as the active edge list is quite small
        // and in the average case already sorted
            for (int i = 1; i < numActive; ++i) {
                QScanConverter::Line *l = d.m_active.at(i);
                int j;
                for (j = i-1; j >= 0 && xOrder(l, d.m_active.at(j)); --j)
                    d.m_active.at(j+1) = d.m_active.at(j);
                d.m_active.at(j+1) = l;
            }
        }

        int x = 0;
        int winding = 0;
        for (int i = 0; i < numActive; ++i) {
            QScanConverter::Line *node = d.m_active.at(i);

            const int current = Q16Dot16ToInt(node->x);
            if (winding & d.m_fillRuleMask)
                d.m_spanBuffer->addSpan(x, current - x, y, 0xff);

            x = current;
            winding += node->winding;

            if (node->bottom == y) {
                // remove node from active list
                for (int j = i; j < numActive - 1; ++j)
                    d.m_active.at(j) = d.m_active.at(j+1);

                d.m_active.resize(--numActive);
                --i;
            } else if (!allVertical())
                node->x += node->delta;
        }
    }
    d.m_active.reset();
}

void QScanConverter::end()
{
    if (m_lines.isEmpty())
        return;

    if (m_lines.size() <= 32) {
        bool allVertical = true;
        for (int i = 0; i < m_lines.size(); ++i) {
            if (m_lines.at(i).delta) {
                allVertical = false;
                break;
            }
        }
        if (allVertical)
            scanConvert(*this, True());
        else
            scanConvert(*this, False());
    } else {
        for (int chunkTop = m_top; chunkTop <= m_bottom; chunkTop += CHUNK_SIZE) {
            prepareChunk();

            Intersection isect = { 0, 0, 0, 0 };

            const int chunkBottom = chunkTop + CHUNK_SIZE;
            for (int i = 0; i < m_lines.size(); ++i) {
                Line &line = m_lines.at(i);

                if ((line.bottom < chunkTop) || (line.top > chunkBottom))
                    continue;

                const int top = qMax(0, line.top - chunkTop);
                const int bottom = qMin(CHUNK_SIZE, line.bottom + 1 - chunkTop);
                allocate(m_size + bottom - top);

                isect.winding = line.winding;

                Intersection *it = m_intersections + top;
                Intersection *end = m_intersections + bottom;

                if (line.delta) {
                    for (; it != end; ++it) {
                        isect.x = Q16Dot16ToInt(line.x);
                        line.x += line.delta;
                        mergeIntersection(it, isect);
                    }
                } else {
                    isect.x = Q16Dot16ToInt(line.x);
                    for (; it != end; ++it)
                        mergeIntersection(it, isect);
                }
            }

            emitSpans(chunkTop);
        }
    }

    if (m_alloc > 1024) {
        free(m_intersections);
        m_alloc = 0;
        m_size = 0;
        m_intersections = 0;
    }

    if (m_lines.size() > 1024)
        m_lines.shrink(1024);
}

inline void QScanConverter::allocate(int size)
{
    if (m_alloc < size) {
        m_alloc = qMax(size, 2 * m_alloc);
        m_intersections = (Intersection *)realloc(m_intersections, m_alloc * sizeof(Intersection));
    }
}

inline void QScanConverter::mergeIntersection(Intersection *it, const Intersection &isect)
{
    Intersection *current = it;

    while (isect.x != current->x) {
        int &next = isect.x < current->x ? current->left : current->right;
        if (next)
            current += next;
        else {
            Intersection *last = m_intersections + m_size;
            next = last - current;
            *last = isect;
            ++m_size;
            return;
        }
    }

    current->winding += isect.winding;
}

void QScanConverter::mergeCurve(const QT_FT_Vector &pa, const QT_FT_Vector &pb,
                                const QT_FT_Vector &pc, const QT_FT_Vector &pd)
{
    // make room for 32 splits
    QT_FT_Vector beziers[4 + 3 * 32];

    QT_FT_Vector *b = beziers;

    b[0] = pa;
    b[1] = pb;
    b[2] = pc;
    b[3] = pd;

    while (b >= beziers) {
        QT_FT_Vector delta = { b[3].x - b[0].x, b[3].y - b[0].y };
        QT_FT_Pos l = qAbs(delta.x) + qAbs(delta.y);
        QT_FT_Pos d;
        if (l > 64) {
            QT_FT_Pos d2 = qAbs((b[1].x-b[0].x)*delta.y - (b[1].y-b[0].y)*delta.x);
            QT_FT_Pos d3 = qAbs((b[2].x-b[0].x)*delta.y - (b[2].y-b[0].y)*delta.x);
            d = (d2 + d3) >> 6;
        } else {
            l = 64;
            d = qAbs(b[0].x-b[1].x) + qAbs(b[0].y-b[1].y) +
                qAbs(b[0].x-b[2].x) + qAbs(b[0].y-b[2].y);
        }

        if (2 * d <= l || b == beziers + 3 * 32) {
            QT_FT_Vector b23 = {
                (b[1].x + b[2].x)/2,
                (b[1].y + b[2].y)/2
            };

            mergeLine(b[0], b23);
            mergeLine(b23, b[3]);

            b -= 3;
            continue;
        }

        split(b);
        b += 3;
    }
}

inline bool QScanConverter::clip(Q16Dot16 &xFP, int &iTop, int &iBottom, Q16Dot16 slopeFP, Q16Dot16 invSlopeFP, Q16Dot16 edgeFP, int winding)
{
    bool right = edgeFP == m_rightFP;

    if (xFP == edgeFP) {
        if ((slopeFP > 0) ^ right)
            return false;
        else {
            Line line = { edgeFP, 0, iTop, iBottom, winding };
            m_lines.add(line);
            return true;
        }
    }

    Q16Dot16 lastFP = xFP + slopeFP * (iBottom - iTop);

    if (lastFP == edgeFP) {
        if ((slopeFP < 0) ^ right)
            return false;
        else {
            Line line = { edgeFP, 0, iTop, iBottom, winding };
            m_lines.add(line);
            return true;
        }
    }

    // does line cross edge?
    if ((lastFP < edgeFP) ^ (xFP < edgeFP)) {
        int iMiddle = Q16Dot16ToInt(IntToQ16Dot16(iTop) + Q16Dot16Multiply(invSlopeFP, edgeFP - xFP));

        if ((xFP < edgeFP) ^ right) {
            Line line = { edgeFP, 0, iTop, iMiddle, winding };
            m_lines.add(line);

            if (iMiddle != iBottom) {
                xFP += slopeFP * (iMiddle + 1 - iTop);
                iTop = iMiddle + 1;
            } else
                return true;
        } else if (iMiddle != iBottom) {
            Line line = { edgeFP, 0, iMiddle + 1, iBottom, winding };
            m_lines.add(line);

            iBottom = iMiddle;
        }
        return false;
    } else if ((xFP < edgeFP) ^ right) {
        Line line = { edgeFP, 0, iTop, iBottom, winding };
        m_lines.add(line);
        return true;
    }

    return false;
}

void QScanConverter::mergeLine(QT_FT_Vector a, QT_FT_Vector b)
{
    int winding = 1;

    if (a.y > b.y) {
        qSwap(a, b);
        winding = -1;
    }

    int iTop = qMax(m_top, int((a.y + 32) >> 6));
    int iBottom = qMin(m_bottom, int((b.y - 32) >> 6));

    if (iTop <= iBottom) {
        Q16Dot16 aFP = Q16Dot16Factor/2 + (a.x << 10);

        if (b.x == a.x) {
            Line line = { qBound(m_leftFP, aFP, m_rightFP), 0, iTop, iBottom, winding };
            m_lines.add(line);
        } else {
            const qreal slope = (b.x - a.x) / qreal(b.y - a.y);

            const Q16Dot16 slopeFP = FloatToQ16Dot16(slope);
            const Q16Dot16 invSlopeFP = FloatToQ16Dot16(1 / slope);

            Q16Dot16 xFP = aFP + Q16Dot16Multiply(slopeFP,
                                                  IntToQ16Dot16(iTop)
                                                  + Q16Dot16Factor/2 - (a.y << 10));

            if (clip(xFP, iTop, iBottom, slopeFP, invSlopeFP, m_leftFP, winding))
                return;

            if (clip(xFP, iTop, iBottom, slopeFP, invSlopeFP, m_rightFP, winding))
                return;

            Line line = { xFP, slopeFP, iTop, iBottom, winding };
            m_lines.add(line);
        }
    }
}

QRasterizer::QRasterizer()
    : d(new QRasterizerPrivate)
{
}

QRasterizer::~QRasterizer()
{
    delete d;
}

void QRasterizer::setAntialiased(bool antialiased)
{
    d->antialiased = antialiased;
}

void QRasterizer::initialize(ProcessSpans blend, void *data)
{
    d->blend = blend;
    d->data = data;
}

void QRasterizer::setClipRect(const QRect &clipRect)
{
    d->clipRect = clipRect;
}

static Q16Dot16 intersectPixelFP(int x, Q16Dot16 top, Q16Dot16 bottom, Q16Dot16 leftIntersectX, Q16Dot16 rightIntersectX, Q16Dot16 slope, Q16Dot16 invSlope)
{
    Q16Dot16 leftX = IntToQ16Dot16(x);
    Q16Dot16 rightX = IntToQ16Dot16(x) + Q16Dot16Factor;

    Q16Dot16 leftIntersectY, rightIntersectY;
    if (slope > 0) {
        leftIntersectY = top + Q16Dot16Multiply(leftX - leftIntersectX, invSlope);
        rightIntersectY = leftIntersectY + invSlope;
    } else {
        leftIntersectY = top + Q16Dot16Multiply(leftX - rightIntersectX, invSlope);
        rightIntersectY = leftIntersectY + invSlope;
    }

    if (leftIntersectX >= leftX && rightIntersectX <= rightX) {
        return Q16Dot16Multiply(bottom - top, leftIntersectX - leftX + ((rightIntersectX - leftIntersectX) >> 1));
    } else if (leftIntersectX >= rightX) {
        return bottom - top;
    } else if (leftIntersectX >= leftX) {
        if (slope > 0) {
            return (bottom - top) - Q16Dot16FastMultiply((rightX - leftIntersectX) >> 1, rightIntersectY - top);
        } else {
            return (bottom - top) - Q16Dot16FastMultiply((rightX - leftIntersectX) >> 1, bottom - rightIntersectY);
        }
    } else if (rightIntersectX <= leftX) {
        return 0;
    } else if (rightIntersectX <= rightX) {
        if (slope > 0) {
            return Q16Dot16FastMultiply((rightIntersectX - leftX) >> 1, bottom - leftIntersectY);
        } else {
            return Q16Dot16FastMultiply((rightIntersectX - leftX) >> 1, leftIntersectY - top);
        }
    } else {
        if (slope > 0) {
            return (bottom - rightIntersectY) + ((rightIntersectY - leftIntersectY) >> 1);
        } else {
            return (rightIntersectY - top) + ((leftIntersectY - rightIntersectY) >> 1);
        }
    }
}

static inline bool q16Dot16Compare(qreal p1, qreal p2)
{
    return FloatToQ16Dot16(p2 - p1) == 0;
}

void QRasterizer::rasterizeLine(const QPointF &a, const QPointF &b, qreal width, bool squareCap)
{
    QPointF pa = a;
    QPointF pb = b;

    QSpanBuffer buffer(d->blend, d->data, d->clipRect);

    if (q16Dot16Compare(pa.y(), pb.y())) {
        const qreal x = (a.x() + b.x()) * 0.5f;
        const qreal dx = qAbs(b.x() - a.x()) * 0.5f;

        const qreal dy = width * dx;

        pa = QPointF(x, a.y() - dy);
        pb = QPointF(x, a.y() + dy);

        if (squareCap)
            width = 1 / width + 1.0f;
        else
            width = 1 / width;

        squareCap = false;
    }

    if (q16Dot16Compare(pa.x(), pb.x())) {
        if (pa.y() > pb.y())
            qSwap(pa, pb);

        const qreal dy = pb.y() - pa.y();
        const qreal halfWidth = 0.5f * width * dy;

        if (squareCap) {
            pa.ry() -= halfWidth;
            pb.ry() += halfWidth;
        }

        qreal left = pa.x() - halfWidth;
        qreal right = pa.x() + halfWidth;

        left = qBound(qreal(d->clipRect.left()), left, qreal(d->clipRect.right() + 1));
        right = qBound(qreal(d->clipRect.left()), right, qreal(d->clipRect.right() + 1));

        pa.ry() = qBound(qreal(d->clipRect.top()), pa.y(), qreal(d->clipRect.bottom() + 1));
        pb.ry() = qBound(qreal(d->clipRect.top()), pb.y(), qreal(d->clipRect.bottom() + 1));

        if (q16Dot16Compare(left, right) || q16Dot16Compare(pa.y(), pb.y()))
            return;

        if (d->antialiased) {
            int iTop = int(pa.y()) ;
            int iBottom = int(pb.y());
            int iLeft = int(left);
            int iRight = int(right);

            Q16Dot16 leftWidth = FloatToQ16Dot16(iLeft + 1.0f - left);
            Q16Dot16 rightWidth = FloatToQ16Dot16(right - iRight);

            Q16Dot16 coverage[3];
            int x[3];
            int len[3];

            int n = 1;
            if (iLeft == iRight) {
                coverage[0] = leftWidth + rightWidth;
                x[0] = iLeft;
                len[0] = 1;
            } else {
                coverage[0] = leftWidth;
                x[0] = iLeft;
                len[0] = 1;
                if (leftWidth == Q16Dot16Factor) {
                    len[0] = iRight - iLeft;
                } else if (iRight - iLeft > 1) {
                    coverage[1] = Q16Dot16Factor;
                    x[1] = iLeft + 1;
                    len[1] = iRight - iLeft - 1;
                    ++n;
                }
                if (rightWidth) {
                    coverage[n] = rightWidth;
                    x[n] = iRight;
                    len[n] = 1;
                    ++n;
                }
            }

            for (int y = iTop; y <= iBottom; ++y) {
                Q16Dot16 rowHeight = FloatToQ16Dot16(qMin(qreal(y + 1), pb.y()) - qMax(qreal(y), pa.y()));
                for (int i = 0; i < n; ++i)
                    buffer.addSpan(x[i], len[i], y, Q16Dot16ToInt(255 * Q16Dot16Multiply(rowHeight, coverage[i])));
            }
        } else { // aliased
            int iTop = int(pa.y() + 0.5f);
            int iBottom = int(pb.y() - 0.5f);
            int iLeft = int(left + 0.5f);
            int iRight = int(right - 0.5f);

            int iWidth = iRight - iLeft + 1;
            for (int y = iTop; y <= iBottom; ++y)
                buffer.addSpan(iLeft, iWidth, y, 255);
        }
    } else {
        if (pa.y() > pb.y())
            qSwap(pa, pb);

        QPointF delta = pb - pa;
        delta *= 0.5f * width;
        QPointF perp(delta.y(), -delta.x());

        if (squareCap) {
            pa -= delta;
            pb += delta;
        }

        QPointF top;
        QPointF left;
        QPointF right;
        QPointF bottom;

        if (pa.x() < pb.x()) {
            top = pa + perp;
            left = pa - perp;
            right = pb + perp;
            bottom = pb - perp;
        } else {
            top = pa - perp;
            left = pb - perp;
            right = pa + perp;
            bottom = pb + perp;
        }

        qreal topBound = qBound(qreal(d->clipRect.top()), top.y(), qreal(d->clipRect.bottom()));
        qreal bottomBound = qBound(qreal(d->clipRect.top()), bottom.y(), qreal(d->clipRect.bottom()));

        if (q16Dot16Compare(topBound, bottomBound))
            return;

        qreal leftSlope = (left.x() - top.x()) / (left.y() - top.y());
        qreal rightSlope = -1.0f / leftSlope;

        Q16Dot16 leftSlopeFP = FloatToQ16Dot16(leftSlope);
        Q16Dot16 rightSlopeFP = FloatToQ16Dot16(rightSlope);

        if (d->antialiased) {
            int iTop = int(topBound);
            int iLeft = int(left.y());
            int iRight = int(right.y());
            int iBottom = int(bottomBound);

            Q16Dot16 leftIntersectAf = FloatToQ16Dot16(top.x() + (iTop - top.y()) * leftSlope);
            Q16Dot16 rightIntersectAf = FloatToQ16Dot16(top.x() + (iTop - top.y()) * rightSlope);
            Q16Dot16 leftIntersectBf = 0;
            Q16Dot16 rightIntersectBf = 0;

            if (iLeft < iTop)
                leftIntersectBf = FloatToQ16Dot16(left.x() + (iTop - left.y()) * rightSlope);

            if (iRight < iTop)
                rightIntersectBf = FloatToQ16Dot16(right.x() + (iTop - right.y()) * leftSlope);

            Q16Dot16 rowTop, rowBottomLeft, rowBottomRight, rowTopLeft, rowTopRight, rowBottom;
            Q16Dot16 topLeftIntersectAf, topLeftIntersectBf, topRightIntersectAf, topRightIntersectBf;
            Q16Dot16 bottomLeftIntersectAf, bottomLeftIntersectBf, bottomRightIntersectAf, bottomRightIntersectBf;

            int leftMin, leftMax, rightMin, rightMax;

            for (int y = iTop; y <= iBottom; ++y) {
                rowTop = FloatToQ16Dot16(qMax(qreal(y), top.y()));
                rowBottomLeft = FloatToQ16Dot16(qMin(qreal(y + 1), left.y()));
                rowBottomRight = FloatToQ16Dot16(qMin(qreal(y + 1), right.y()));
                rowTopLeft = FloatToQ16Dot16(qMax(qreal(y), left.y()));
                rowTopRight = FloatToQ16Dot16(qMax(qreal(y), right.y()));
                rowBottom = FloatToQ16Dot16(qMin(qreal(y + 1), bottom.y()));

                Q16Dot16 yFP = IntToQ16Dot16(y);

                if (y == iTop) {
                    topLeftIntersectAf = leftIntersectAf + Q16Dot16Multiply(leftSlopeFP, rowTop - yFP);
                    topRightIntersectAf = rightIntersectAf + Q16Dot16Multiply(rightSlopeFP, rowTop - yFP);
                } else {
                    topLeftIntersectAf = leftIntersectAf;
                    topRightIntersectAf = rightIntersectAf;
                }

                if (y == iLeft) {
                    leftIntersectBf = FloatToQ16Dot16(left.x() + (y - left.y()) * rightSlope);
                    topLeftIntersectBf = leftIntersectBf + Q16Dot16Multiply(rightSlopeFP, rowTopLeft - yFP);
                    bottomLeftIntersectAf = leftIntersectAf + Q16Dot16Multiply(leftSlopeFP, rowBottomLeft - yFP);
                } else {
                    topLeftIntersectBf = leftIntersectBf;
                    bottomLeftIntersectAf = leftIntersectAf + leftSlopeFP;
                }

                if (y == iRight) {
                    rightIntersectBf = FloatToQ16Dot16(right.x() + (y - right.y()) * leftSlope);
                    topRightIntersectBf = rightIntersectBf + Q16Dot16Multiply(leftSlopeFP, rowTopRight - yFP);
                    bottomRightIntersectAf = rightIntersectAf + Q16Dot16Multiply(rightSlopeFP, rowBottomRight - yFP);
                } else {
                    topRightIntersectBf = rightIntersectBf;
                    bottomRightIntersectAf = rightIntersectAf + rightSlopeFP;
                }

                if (y == iBottom) {
                    bottomLeftIntersectBf = leftIntersectBf + Q16Dot16Multiply(rightSlopeFP, rowBottom - yFP);
                    bottomRightIntersectBf = rightIntersectBf + Q16Dot16Multiply(leftSlopeFP, rowBottom - yFP);
                } else {
                    bottomLeftIntersectBf = leftIntersectBf + rightSlopeFP;
                    bottomRightIntersectBf = rightIntersectBf + leftSlopeFP;
                }

                if (y < iLeft) {
                    leftMin = Q16Dot16ToInt(bottomLeftIntersectAf);
                    leftMax = Q16Dot16ToInt(topLeftIntersectAf);
                } else if (y == iLeft) {
                    leftMin = Q16Dot16ToInt(qMax(bottomLeftIntersectAf, topLeftIntersectBf));
                    leftMax = Q16Dot16ToInt(qMax(topLeftIntersectAf, bottomLeftIntersectBf));
                } else {
                    leftMin = Q16Dot16ToInt(topLeftIntersectBf);
                    leftMax = Q16Dot16ToInt(bottomLeftIntersectBf);
                }

                leftMin = qBound(d->clipRect.left(), leftMin, d->clipRect.right());
                leftMax = qBound(d->clipRect.left(), leftMax, d->clipRect.right());

                if (y < iRight) {
                    rightMin = Q16Dot16ToInt(topRightIntersectAf);
                    rightMax = Q16Dot16ToInt(bottomRightIntersectAf);
                } else if (y == iRight) {
                    rightMin = Q16Dot16ToInt(qMin(topRightIntersectAf, bottomRightIntersectBf));
                    rightMax = Q16Dot16ToInt(qMin(bottomRightIntersectAf, topRightIntersectBf));
                } else {
                    rightMin = Q16Dot16ToInt(bottomRightIntersectBf);
                    rightMax = Q16Dot16ToInt(topRightIntersectBf);
                }

                rightMin = qBound(d->clipRect.left(), rightMin, d->clipRect.right());
                rightMax = qBound(d->clipRect.left(), rightMax, d->clipRect.right());

                Q16Dot16 rowHeight = rowBottom - rowTop;

                int x = leftMin;
                while (x <= leftMax) {
                    Q16Dot16 excluded = 0;

                    if (y <= iLeft)
                        excluded += intersectPixelFP(x, rowTop, rowBottomLeft,
                                                     bottomLeftIntersectAf, topLeftIntersectAf,
                                                     leftSlopeFP, -rightSlopeFP);
                    if (y >= iLeft)
                        excluded += intersectPixelFP(x, rowTopLeft, rowBottom,
                                                     topLeftIntersectBf, bottomLeftIntersectBf,
                                                     rightSlopeFP, -leftSlopeFP);

                    if (x >= rightMin) {
                        if (y <= iRight)
                            excluded += (rowBottomRight - rowTop) - intersectPixelFP(x, rowTop, rowBottomRight,
                                                                                     topRightIntersectAf, bottomRightIntersectAf,
                                                                                     rightSlopeFP, -leftSlopeFP);
                        if (y >= iRight)
                            excluded += (rowBottom - rowTopRight) - intersectPixelFP(x, rowTopRight, rowBottom,
                                                                                     bottomRightIntersectBf, topRightIntersectBf,
                                                                                     leftSlopeFP, -rightSlopeFP);
                    }

                    Q16Dot16 coverage = rowHeight - excluded;
                    buffer.addSpan(x, 1, y, Q16Dot16ToInt(255 * coverage));
                    ++x;
                }
                if (x < rightMin) {
                    buffer.addSpan(x, rightMin - x, y, Q16Dot16ToInt(255 * rowHeight));
                    x = rightMin;
                }
                while (x <= rightMax) {
                    Q16Dot16 excluded = 0;
                    if (y <= iRight)
                        excluded += (rowBottomRight - rowTop) - intersectPixelFP(x, rowTop, rowBottomRight,
                                                                                 topRightIntersectAf, bottomRightIntersectAf,
                                                                                 rightSlopeFP, -leftSlopeFP);
                    if (y >= iRight)
                        excluded += (rowBottom - rowTopRight) - intersectPixelFP(x, rowTopRight, rowBottom,
                                                                                 bottomRightIntersectBf, topRightIntersectBf,
                                                                                 leftSlopeFP, -rightSlopeFP);

                    Q16Dot16 coverage = rowHeight - excluded;
                    buffer.addSpan(x, 1, y, Q16Dot16ToInt(255 * coverage));
                    ++x;
                }

                leftIntersectAf += leftSlopeFP;
                leftIntersectBf += rightSlopeFP;
                rightIntersectAf += rightSlopeFP;
                rightIntersectBf += leftSlopeFP;
            }
        } else { // aliased
            int iTop = int(top.y() + 0.5f);
            int iLeft = int(left.y() - 0.5f);
            int iRight = int(right.y() - 0.5f);
            int iBottom = int(bottom.y() - 0.5f);

            Q16Dot16 leftIntersectAf = FloatToQ16Dot16(top.x() + 0.5f + (iTop + 0.5f - top.y()) * leftSlope);
            Q16Dot16 leftIntersectBf = FloatToQ16Dot16(left.x() + 0.5f + (iLeft + 1.5f - left.y()) * rightSlope);
            Q16Dot16 rightIntersectAf = FloatToQ16Dot16(top.x() - 0.5f + (iTop + 0.5f - top.y()) * rightSlope);
            Q16Dot16 rightIntersectBf = FloatToQ16Dot16(right.x() - 0.5f + (iRight + 1.5f - right.y()) * leftSlope);

            Q16Dot16 iMiddle = qMin(iLeft, iRight);

            int y;
            for (y = iTop; y <= iMiddle; ++y) {
                if (y >= d->clipRect.top() && y <= d->clipRect.bottom()) {
                    int x1 = Q16Dot16ToInt(leftIntersectAf);
                    int x2 = Q16Dot16ToInt(rightIntersectAf);
                    if (x2 >= d->clipRect.left() && x1 <= d->clipRect.right()) {
                        x1 = qMax(x1, d->clipRect.left());
                        x2 = qMin(x2, d->clipRect.right());
                        buffer.addSpan(x1, x2 - x1 + 1, y, 255);
                    }
                }
                leftIntersectAf += leftSlopeFP;
                rightIntersectAf += rightSlopeFP;
            }
            for (; y <= iRight; ++y) {
                if (y >= d->clipRect.top() && y <= d->clipRect.bottom()) {
                    int x1 = Q16Dot16ToInt(leftIntersectBf);
                    int x2 = Q16Dot16ToInt(rightIntersectAf);
                    if (x2 >= d->clipRect.left() && x1 <= d->clipRect.right()) {
                        x1 = qMax(x1, d->clipRect.left());
                        x2 = qMin(x2, d->clipRect.right());
                        buffer.addSpan(x1, x2 - x1 + 1, y, 255);
                    }
                }
                leftIntersectBf += rightSlopeFP;
                rightIntersectAf += rightSlopeFP;
            }
            for (; y <= iLeft; ++y) {
                if (y >= d->clipRect.top() && y <= d->clipRect.bottom()) {
                    int x1 = Q16Dot16ToInt(leftIntersectAf);
                    int x2 = Q16Dot16ToInt(rightIntersectBf);
                    if (x2 >= d->clipRect.left() && x1 <= d->clipRect.right()) {
                        x1 = qMax(x1, d->clipRect.left());
                        x2 = qMin(x2, d->clipRect.right());
                        buffer.addSpan(x1, x2 - x1 + 1, y, 255);
                    }
                }
                leftIntersectAf += leftSlopeFP;
                rightIntersectBf += leftSlopeFP;
            }
            for (; y <= iBottom; ++y) {
                if (y >= d->clipRect.top() && y <= d->clipRect.bottom()) {
                    int x1 = Q16Dot16ToInt(leftIntersectBf);
                    int x2 = Q16Dot16ToInt(rightIntersectBf);
                    if (x2 >= d->clipRect.left() && x1 <= d->clipRect.right()) {
                        x1 = qMax(x1, d->clipRect.left());
                        x2 = qMin(x2, d->clipRect.right());
                        buffer.addSpan(x1, x2 - x1 + 1, y, 255);
                    }
                }
                leftIntersectBf += rightSlopeFP;
                rightIntersectBf += leftSlopeFP;
            }
        }
    }
}

void QRasterizer::rasterize(const QT_FT_Outline *outline, Qt::FillRule fillRule)
{
    const QT_FT_Vector *points = outline->points;

    Q_ASSERT(outline->n_points >= 3);
    Q_ASSERT(outline->n_contours >= 1);

    QSpanBuffer buffer(d->blend, d->data, d->clipRect);

    QT_FT_Pos min_x = points[0].x, max_x = points[0].x;
    QT_FT_Pos min_y = points[0].y, max_y = points[0].y;
    for (int i = 1; i < outline->n_points; ++i) {
        const QT_FT_Vector &p = points[i];
        min_x = qMin(p.x, min_x);
        max_x = qMax(p.x, max_x);
        min_y = qMin(p.y, min_y);
        max_y = qMax(p.y, max_y);
    }

    int iTopBound = qMax(d->clipRect.top(), int((min_y + 32) >> 6));
    int iBottomBound = qMin(d->clipRect.bottom(), int((max_y - 32) >> 6));

    if (iTopBound > iBottomBound)
        return;

    d->scanConverter.begin(iTopBound, iBottomBound, d->clipRect.left(), d->clipRect.right(), fillRule, &buffer);

    int first = 0;
    for (int i = 0; i < outline->n_contours; ++i) {
        const int last = outline->contours[i];
        for (int j = first; j < last; ++j) {
            if (outline->tags[j+1] == QT_FT_CURVE_TAG_CUBIC) {
                Q_ASSERT(outline->tags[j+2] == QT_FT_CURVE_TAG_CUBIC);
                d->scanConverter.mergeCurve(points[j], points[j+1], points[j+2], points[j+3]);
                j += 2;
            } else {
                d->scanConverter.mergeLine(points[j], points[j+1]);
            }
        }

        first = last + 1;
    }

    d->scanConverter.end();
}

static inline QT_FT_Vector PointToVector(const QPointF &p)
{
    QT_FT_Vector result = { QT_FT_Pos(p.x() * 64), QT_FT_Pos(p.y() * 64) };
    return result;
}

void QRasterizer::rasterize(const QPainterPath &path, Qt::FillRule fillRule)
{
    if (path.isEmpty())
        return;

    QSpanBuffer buffer(d->blend, d->data, d->clipRect);

    QRectF bounds = path.controlPointRect();

    int iTopBound = qMax(d->clipRect.top(), int(bounds.top() + 0.5));
    int iBottomBound = qMin(d->clipRect.bottom(), int(bounds.bottom() - 0.5));

    if (iTopBound > iBottomBound)
        return;

    d->scanConverter.begin(iTopBound, iBottomBound, d->clipRect.left(), d->clipRect.right(), fillRule, &buffer);

    int subpathStart = 0;
    QT_FT_Vector last = { 0, 0 };
    for (int i = 0; i < path.elementCount(); ++i) {
        switch (path.elementAt(i).type) {
        case QPainterPath::LineToElement:
            {
                QT_FT_Vector p1 = last;
                QT_FT_Vector p2 = PointToVector(path.elementAt(i));
                d->scanConverter.mergeLine(p1, p2);
                last = p2;
                break;
            }
        case QPainterPath::MoveToElement:
            subpathStart = i;
            last = PointToVector(path.elementAt(i));
            break;
        case QPainterPath::CurveToElement:
            {
            QT_FT_Vector p1 = last;
            QT_FT_Vector p2 = PointToVector(path.elementAt(i));
            QT_FT_Vector p3 = PointToVector(path.elementAt(i+1));
            QT_FT_Vector p4 = PointToVector(path.elementAt(i+2));
            d->scanConverter.mergeCurve(p1, p2, p3, p4);
            last = p4;
            break;
            }
        default:
            Q_ASSERT(false);
            break;
        }
    }

    QT_FT_Vector first = PointToVector(path.elementAt(subpathStart));

    // close path
    if (first.x != last.x || first.y != last.y)
        d->scanConverter.mergeLine(last, first);

    d->scanConverter.end();
}
