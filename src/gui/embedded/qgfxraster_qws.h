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

#ifndef QGFXRASTER_QWS_H
#define QGFXRASTER_QWS_H

#include "qgfxrasterbase_qws.h"

template <const int depth, const int type>
class QGfxRaster : public QGfxRasterBase, protected QWSPolygonScanner {
public:
    QGfxRaster(unsigned char *, int w, int h);
    ~QGfxRaster();

    virtual void drawPoint(int, int);
    virtual void drawPoints(const QPolygon &, int, int);
    virtual void drawLine(int, int, int, int);
    virtual void fillRect(int, int, int, int);
    virtual void drawPolyline(const QPolygon &, int, int);
    virtual void drawPolygon(const QPolygon &, bool, int, int);
    virtual void blt(int, int, int, int, int, int);
    virtual void scroll(int, int, int, int, int, int);
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS) || !defined(QT_NO_PIXMAP_TRANSFORMATION)
    virtual void stretchBlt(int, int, int, int, int, int);
#endif
    virtual void tiledBlt(int, int, int, int);

    virtual int bitDepth() { return depth; }

    virtual void setSource(const QImage *);
    virtual void setSource(const QPaintDevice *);
    virtual void setSource(unsigned char *, int, int, int, int, QRgb *, int);

protected:
    virtual void drawThickLine(int, int,int, int);
    virtual void drawThickPolyline(const QPolygon &,int, int);

    void buildSourceClut(const QRgb *cols , int numcols);
    void processSpans(int n, QPoint *point, int *width);

    void vline(int, int, int); // Optimised vertical line drawing
    void hline(int, int, int); // Optimised horizontal line drawing
    void hlineUnclipped(int, int, unsigned char *);
    void hImageLineUnclipped(int x1, int x2, unsigned char *l, unsigned const char *srcdata,
                             bool reverse);
    void hAlphaLineUnclipped(int x1, int x2, unsigned char *l, unsigned const char *srcdata,
                             unsigned const char *alphas);
    void drawPointUnclipped(int, unsigned char *);
    void drawAlphaPointUnclipped(int, unsigned char *);

    void calcPacking(void *, int, int, int &, int &, int &);
};

#endif // QGFXRASTER_QWS_H
