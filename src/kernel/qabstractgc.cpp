#include "qabstractgc.h"

QAbstractGC::QAbstractGC()
    :
    state(0)
{
    d_ptr = new QAbstractGCPrivate;
}

void QAbstractGC::updateInternal(QPainterState *s)
{
    if (state) {
	updatePen(s);
	updateBrush(s);
	updateFont(s);
	updateRasterOp(s);
	updateBackground(s);
	updateXForm(s);
	updateClipRegion(s);
    }
    state = s;
}
