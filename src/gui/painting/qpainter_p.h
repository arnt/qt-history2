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

#ifndef QPAINTER_P_H
#define QPAINTER_P_H

#include "qbrush.h"
#include "qfont.h"
#include "qpen.h"
#include "qregion.h"
#include "qvector.h"
#include "qmatrix.h"

#include "qpainter.h"
#include "qpainterpath.h"

class QPaintEngine;

class QPainterState
{
public:
    QPainterState() {
        init(0);
    }

    QPainterState(const QPainterState *s) {
        font = s->font;
        deviceFont = s->deviceFont;
        pfont = s->pfont ? new QFont(*s->pfont) : 0;
        pen = QPen(s->pen);
        brush = QBrush(s->brush);
        bgOrigin = s->bgOrigin;
        bgBrush = QBrush(s->bgBrush);
        clipRegion = QRegion(s->clipRegion);
        clipRegionMatrix = s->clipRegionMatrix;
        clipPath = s->clipPath;
        clipPathRegion = s->clipPathRegion;
        clipPathMatrix = s->clipPathMatrix;
        clipEnabled = s->clipEnabled;
        bgMode = s->bgMode;
        VxF = s->VxF;
        WxF = s->WxF;
#ifndef QT_NO_TRANSFORMATIONS
        worldMatrix = s->worldMatrix;
        matrix = s->matrix;
        txop = s->txop;
#else
        xlatex = s->xlatex;
        xlatey = s->xlatey;
#endif
        wx = s->wx;
        wy = s->wy;
        ww = s->ww;
        wh = s->wh;
        vx = s->vx;
        vy = s->vy;
        vw = s->vw;
        vh = s->vh;
        painter = s->painter;
        changeFlags = 0;
    }

    ~QPainterState() {
        delete pfont;
    }

    void init(QPainter *p) {
        bgBrush = Qt::white;
        bgMode = Qt::TransparentMode;
        clipEnabled = false;
        WxF = false;
        VxF = false;
        wx = wy = ww = wh = 0;
        vx = vy = vw = vh = 0;
        changeFlags = 0;
        pfont = 0;
        pfont = 0;
        painter = p;
        pen = QPen();
        bgOrigin = QPoint(0, 0);
        brush = bgBrush = QBrush();
        font = deviceFont = QFont();
        clipRegion = clipPathRegion = QRegion();
        clipPath = QPainterPath();
        clipRegionMatrix.reset();
        clipPathMatrix.reset();
#ifndef QT_NO_TRANSFORMATIONS
        worldMatrix.reset();
        matrix.reset();
        txop = 0;
#else
        xlatex = xlatey = 0;
#endif
    }

    QPoint bgOrigin;
    QFont font;
    QFont deviceFont;
    QFont *pfont;
    QPen pen;
    QBrush brush;
    QBrush bgBrush;             // background brush
    QRegion clipRegion;
    QMatrix clipRegionMatrix;
    QPainterPath clipPath;
    QRegion clipPathRegion;
    QMatrix clipPathMatrix;
#ifndef QT_NO_TRANSFORMATIONS
    QMatrix worldMatrix;       // World transformation matrix, not window and viewport
    QMatrix matrix;            // Complete transformation matrix, including win and view.
    int txop;
#else
    int xlatex;
    int xlatey;
#endif
    int wx, wy, ww, wh;         // window rectangle
    int vx, vy, vw, vh;         // viewport rectangle

    uint        clipEnabled:1;
    uint         WxF:1;                        // World transformation
    uint            VxF:1;                        // View transformation

    Qt::BGMode bgMode;
    QPainter *painter;
    uint changeFlags;
};


class QPainterPrivate
{
public:
    QPainterPrivate()
        : txinv(0), device(0), engine(0)
    {
        states.push_back(new QPainterState());
        state = states.back();
    }

    ~QPainterPrivate()
    {
        for (int i=0; i<states.size(); ++i)
            delete states.at(i);
    }

    QPoint redirection_offset;

    QPainterState *state;
    QVector<QPainterState*> states;

#ifndef QT_NO_TRANSFORMATIONS
    QMatrix invMatrix;
    uint txinv:1;
#endif

    QPaintDevice *device;
    QPaintEngine *engine;
};

#endif // QPAINTER_P_H
