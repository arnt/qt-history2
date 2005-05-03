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

#include "QtGui/qgfxrasterbase_qws.h"

template <const int depth, const int type>
class QGfxRaster : public QGfxRasterBase {
public:
    QGfxRaster(unsigned char *, int w, int h);
    ~QGfxRaster();

    virtual void fillRect(int, int, int, int);
    virtual void blt(int, int, int, int, int, int);
    virtual void tiledBlt(int, int, int, int);

    virtual int bitDepth() { return depth; }

    virtual void setSource(const QImage *);
    virtual void setSource(const QPixmap *);
    virtual void setSource(unsigned char *, int, int, int, int, QRgb *, int);

protected:
    void buildSourceClut(const QRgb *cols , int numcols);
    void hline(int, int, int); // Optimised horizontal line drawing
    void hlineUnclipped(int, int, unsigned char *);
    void hImageLineUnclipped(int x1, int x2, unsigned char *l, unsigned const char *srcdata,
                             bool reverse);
    void calcPacking(void *, int, int, int &, int &, int &);
};

#endif // QGFXRASTER_QWS_H
