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
**/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


template <typename Type> class QPolygonClipperBuffer
{
public:
    QPolygonClipperBuffer()
    {
        capacity = 64;
        buffer = (Type*) qMalloc(capacity * sizeof(Type));
        siz = 0;
    }

    ~QPolygonClipperBuffer()
    {
        qFree(buffer);
    }

    void reset() { siz = 0; }

    int size() const { return siz; }
    Type *data() const { return buffer; }

    const Type &at(int i) const { Q_ASSERT(i >= 0 && i < siz); return buffer[i]; }

    void add(const Type &t) {
        if (siz >= capacity) {
            capacity *= 2;
            buffer = (Type*) qRealloc(buffer, capacity * sizeof(Type));
        }
        buffer[siz] = t;
        ++siz;
    }

private:
    int capacity;
    int siz;
    Type *buffer;
};


/* based on sutherland-hodgman line-by-line clipping, as described in
   Computer Graphics and Principles */
template <typename InType, typename OutType, typename CastType> class QPolygonClipper
{
public:
    QPolygonClipper()
    {
        x1 = y1 = x2 = y2 = 0;
    }

    ~QPolygonClipper()
    {
    }

    void setBoundingRect(const QRect bounds)
    {
        x1 = bounds.x();
        x2 = bounds.x() + bounds.width();
        y1 = bounds.y();
        y2 = bounds.y() + bounds.height();
    }

    QRect boundingRect()
    {
        return QRect(QPoint(x1, y1), QPoint(x2, y2));
    }

    inline OutType intersectLeft(const OutType &p1, const OutType &p2)
    {
        OutType t;
        double dy = (p1.y - p2.y) / double(p1.x - p2.x);
        t.x = x1;
        t.y = static_cast<CastType>(p2.y + (x1 - p2.x) * dy);
        return t;
    }


    inline OutType intersectRight(const OutType &p1, const OutType &p2)
    {
        OutType t;
        double dy = (p1.y - p2.y) / double(p1.x - p2.x);
        t.x = x2;
        t.y = static_cast<CastType>(p2.y + (x2 - p2.x) * dy);
        return t;
    }


    inline OutType intersectTop(const OutType &p1, const OutType &p2)
    {
        OutType t;
        double dx = (p1.x - p2.x) / double(p1.y - p2.y);
        t.x = static_cast<CastType>(p2.x + (y1 - p2.y) * dx);
        t.y = y1;
        return t;
    }


    inline OutType intersectBottom(const OutType &p1, const OutType &p2)
    {
        OutType t;
        double dx = (p1.x - p2.x) / double(p1.y - p2.y);
        t.x = static_cast<CastType>(p2.x + (y2 - p2.y) * dx);
        t.y = y2;
        return t;
    }


    void clipPolygon(const InType *inPoints, int inCount, OutType **outPoints, int *outCount,
                     bool closePolygon = true)
    {
        Q_ASSERT(outPoints);
        Q_ASSERT(outCount);

        if (inCount < 2) {
            *outCount = 0;
            return;
        }

        buffer1.reset();
        buffer2.reset();

        QPolygonClipperBuffer<OutType> *source = &buffer1;
        QPolygonClipperBuffer<OutType> *clipped = &buffer2;

        // Gather some info since we are iterating through the points anyway..
        bool doLeft = false, doRight = false, doTop = false, doBottom = false;
        OutType ot;
        for (int i=0; i<inCount; ++i) {
            ot = inPoints[i];
            clipped->add(ot);

            if (ot.x < x1)
                doLeft = true;
            else if (ot.x > x2)
                doRight = true;
            if (ot.y < y1)
                doTop = true;
            else if (ot.y > y2)
                doBottom = true;
        }

        if (doLeft) {
            QPolygonClipperBuffer<OutType> *tmp = source;
            source = clipped;
            clipped = tmp;
            clipped->reset();
            int lastPos = inCount - 1;
            for (int i=0; i<inCount; ++i) {
                const OutType &cpt = source->at(i);
                const OutType &ppt = source->at(lastPos);

                if (cpt.x >= x1) {
                    if (ppt.x >= x1) {
                        clipped->add(cpt);
                    } else {
                        clipped->add(intersectLeft(cpt, ppt));
                        clipped->add(cpt);
                    }
                } else if (ppt.x >= x1) {
                    clipped->add(intersectLeft(cpt, ppt));
                }
                lastPos = i;
            }
        }

        if (doRight) {
            QPolygonClipperBuffer<OutType> *tmp = source;
            source = clipped;
            clipped = tmp;
            clipped->reset();
            int lastPos = source->size() - 1;
            for (int i=0; i<source->size(); ++i) {
                const OutType &cpt = source->at(i);
                const OutType &ppt = source->at(lastPos);

                if (cpt.x <= x2) {
                    if (ppt.x <= x2) {
                        clipped->add(cpt);
                    } else {
                        clipped->add(intersectRight(cpt, ppt));
                        clipped->add(cpt);
                    }
                } else if (ppt.x <= x2) {
                    clipped->add(intersectRight(cpt, ppt));
                }

                lastPos = i;
            }

        }

        if (doTop) {
            QPolygonClipperBuffer<OutType> *tmp = source;
            source = clipped;
            clipped = tmp;
            clipped->reset();
            int lastPos = source->size() - 1;
            for (int i=0; i<source->size(); ++i) {
                const OutType &cpt = source->at(i);
                const OutType &ppt = source->at(lastPos);

                if (cpt.y >= y1) {
                    if (ppt.y >= y1) {
                        clipped->add(cpt);
                    } else {
                        clipped->add(intersectTop(cpt, ppt));
                        clipped->add(cpt);
                    }
                } else if (ppt.y >= y1) {
                    clipped->add(intersectTop(cpt, ppt));
                }

                lastPos = i;
            }
        }

        if (doBottom) {
            QPolygonClipperBuffer<OutType> *tmp = source;
            source = clipped;
            clipped = tmp;
            clipped->reset();
            int lastPos = source->size() - 1;
            for (int i=0; i<source->size(); ++i) {
                const OutType &cpt = source->at(i);
                const OutType &ppt = source->at(lastPos);

                if (cpt.y <= y2) {
                    if (ppt.y <= y2) {
                        clipped->add(cpt);
                    } else {
                        clipped->add(intersectBottom(cpt, ppt));
                        clipped->add(cpt);
                    }
                } else if (ppt.y <= y2) {
                    clipped->add(intersectBottom(cpt, ppt));
                }
                lastPos = i;
            }
        }

        if (closePolygon && clipped->size() > 0) {
            // close clipped polygon
            if (clipped->at(0).x != clipped->at(clipped->size()-1).x ||
                clipped->at(0).y != clipped->at(clipped->size()-1).y)
                clipped->add(clipped->at(0));
        }
        *outCount = clipped->size();
        *outPoints = clipped->data();
    }

private:
    int x1, x2, y1, y2;
    QPolygonClipperBuffer<OutType> buffer1;
    QPolygonClipperBuffer<OutType> buffer2;
};
