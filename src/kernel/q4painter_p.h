#ifndef Q4PAINTER_P_H
#define Q4PAINTER_P_H

#include "qbrush.h"
#include "qfont.h"
#include "qpen.h"
#include "qregion.h"
#include "qvector.h"
#include "qwmatrix.h"

#include "q4painter.h"

class QAbstractGC;

class QPainterState
{
public:
    QPainterState( const QPainterState *s = 0 )
    {
	if (s) {
	    font = QFont(s->font);
	    pen = QPen(s->pen);
	    brush = QBrush(s->brush);
	    bgOrigin = s->bgOrigin;
	    bgBrush = QBrush(s->bgBrush);
 	    clipRegion = QRegion(s->clipRegion);
	    clipEnabled = s->clipEnabled;
	    rasterOp = s->rasterOp;
	    bgMode = s->bgMode;
	    VxF = s->VxF;
	    WxF = s->WxF;
#ifndef QT_NO_TRANSFORMATIONS
	    worldMatrix = s->worldMatrix;
	    matrix = s->matrix;
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
	} else {
	    bgBrush = Qt::white;
	    bgMode = QPainter::TransparentMode;
	    rasterOp = Qt::CopyROP;
	    clipEnabled = false;
	    WxF = false;
	    VxF = false;
	    wx = wy = ww = wh = 0;
	    vx = vy = vw = vh = 0;
	}
    }

    QPoint 	bgOrigin;
    QFont 	font;
    QPen 	pen;
    QBrush 	brush;
    QBrush 	bgBrush;		// background brush
    QRegion	clipRegion;
    QColor      bgColor;
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix    worldMatrix; 	    	// World transformation matrix, not window and viewport
    QWMatrix    matrix;			// Complete transformation matrix, including win and view.
#else
    int         xlatex;
    int         xlatey;
#endif
    int	        wx, wy, ww, wh;		// window rectangle
    int 	vx, vy, vw, vh;		// viewport rectangle

    uint	clipEnabled:1;
    uint 	WxF:1;			// World transformation
    uint    	VxF:1;			// View transformation

    Qt::RasterOp rasterOp;
    Qt::BGMode bgMode;
};


class QPainterPrivate
{
public:
    QPainterPrivate()
	: device(0), gc(0)
    {
	states.push_back(new QPainterState());
	state = states.back();
    }

    void save()
    {
	Q_ASSERT(states.size()>0);
	state = new QPainterState(states.back());
	states.push_back(state);
    }

    void restore()
    {
	Q_ASSERT(states.size()>0);
	QPainterState *tmp = state;
	states.pop_back();
	state = states.back();
	delete tmp;
    };

    QPoint redirection_offset;

    QPainterState *state;
    QVector<QPainterState*> states;

#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix matrix; // reloaded stinks...
    QWMatrix invMatrix;

    int txop;
    uint txinv:1;
#endif

    QPaintDevice *device;
    QAbstractGC *gc;
};

#endif // Q4PAINTER_P_H
