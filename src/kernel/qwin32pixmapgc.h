#ifndef QWIN32PIXMAPGC_H
#define QWIN32PIXMAPGC_H

#include "qwin32gc.h"

class QPainterState;

class QWin32PixmapGC : public QWin32GC
{
public:
    QWin32PixmapGC(const QPaintDevice *target);

    bool begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped=FALSE);
};

#endif // QWIN32GC_H
