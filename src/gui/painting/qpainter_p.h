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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qbrush.h"
#include "qfont.h"
#include "qpen.h"
#include "qregion.h"
#include "qvector.h"
#include "qmatrix.h"

#include "qpainter.h"
#include "qpainterpath.h"

class QPaintEngine;

class QPainterClipInfo
{
public:
    enum ClipType { RegionClip, PathClip };

    QPainterClipInfo(const QPainterPath &p, Qt::ClipOperation op, const QMatrix &m) :
        clipType(PathClip), matrix(m), operation(op), path(p) { }

    QPainterClipInfo(const QRegion &r, Qt::ClipOperation op, const QMatrix &m) :
        clipType(RegionClip), matrix(m), operation(op), region(r) { }

    ClipType clipType;
    QMatrix matrix;
    Qt::ClipOperation operation;
    QPainterPath path;
    QRegion region;
};

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
        tmpClipRegion = QRegion(s->tmpClipRegion);
        tmpClipPath = s->tmpClipPath;
        tmpClipOp = s->tmpClipOp;
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
        clipInfo = s->clipInfo;
        changeFlags = 0;
    }

    ~QPainterState() {
        delete pfont;
    }

    void init(QPainter *p) {
        bgBrush = Qt::white;
        bgMode = Qt::TransparentMode;
        WxF = false;
        VxF = false;
        wx = wy = ww = wh = 0;
        vx = vy = vw = vh = 0;
        changeFlags = 0;
        pfont = 0;
        painter = p;
        pen = QPen();
        bgOrigin = QPointF(0, 0);
        brush = QBrush();
        font = deviceFont = QFont();
        tmpClipRegion = QRegion();
        tmpClipPath = QPainterPath();
        tmpClipOp = Qt::NoClip;
#ifndef QT_NO_TRANSFORMATIONS
        worldMatrix.reset();
        matrix.reset();
        txop = 0;
#else
        xlatex = xlatey = 0;
#endif
    }

    QPointF bgOrigin;
    QFont font;
    QFont deviceFont;
    QFont *pfont;
    QPen pen;
    QBrush brush;
    QBrush bgBrush;             // background brush
    QRegion tmpClipRegion;
    QPainterPath tmpClipPath;
    Qt::ClipOperation tmpClipOp;
    QList<QPainterClipInfo> clipInfo;
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

    uint WxF:1;                 // World transformation
    uint VxF:1;                 // View transformation

    Qt::BGMode bgMode;
    QPainter *painter;
    uint changeFlags;
};


class QPainterPrivate
{
    Q_DECLARE_PUBLIC(QPainter)
public:
    QPainterPrivate(QPainter *painter)
        : q_ptr(painter), txinv(0), device(0), engine(0)
    {
        states.push_back(new QPainterState());
        state = states.back();
    }

    ~QPainterPrivate()
    {
        for (int i=0; i<states.size(); ++i)
            delete states.at(i);
    }

    QPainter *q_ptr;

    QPoint redirection_offset;

    QPainterState *state;
    QVector<QPainterState*> states;

#ifndef QT_NO_TRANSFORMATIONS
    QMatrix invMatrix;
    uint txinv:1;
#endif

    enum TransformationCodes {
        TxNone = 0,
        TxTranslate = 1,
        TxScale = 2,
        TxRotShear = 3
    };

    enum DrawOperation { StrokeDraw        = 0x1,
                         FillDraw          = 0x2,
                         StrokeAndFillDraw = 0x3
    };

    enum ShapeType { LineShape,
                     RectangleShape,
                     EllipseShape,
                     PolygonShape,
                     PathShape
    };

    QPolygonF draw_helper_xpolygon(const void *data, ShapeType type);
    void draw_helper(const void *data, Qt::FillRule fillRule, ShapeType type,
                     DrawOperation operation, uint emulationSpecifier);
    void draw_helper(const void *data, Qt::FillRule fillRule, ShapeType type,
                     DrawOperation operation = StrokeAndFillDraw);

    // Refactored draw_helper functionallity
    QRect draw_helper_setclip(const void *data, Qt::FillRule fillRule, ShapeType type);
    void draw_helper_fill_lineargradient(const void *data, Qt::FillRule fillRule, ShapeType type);
    void draw_helper_fill_alpha(const void *data, Qt::FillRule fillRule, ShapeType type);
    void draw_helper_fill_pattern(const void *data, Qt::FillRule fillRule, ShapeType type);
    void draw_helper_stroke_normal(const void *data, ShapeType type, uint emulate);
    void draw_helper_stroke_pathbased(const void *data, ShapeType type);

    void updateMatrix();
    void updateInvMatrix();
    void init();

    int rectSubtraction() const {
        return state->pen.style() != Qt::NoPen && state->pen.width() == 0 ? 1 : 0;
    }

    QPaintDevice *device;
    QPaintEngine *engine;
};

#endif // QPAINTER_P_H
