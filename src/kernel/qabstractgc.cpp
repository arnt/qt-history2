#include "qabstractgc.h"

QAbstractGC::QAbstractGC(GCCaps caps)
    : state(0),
      gccaps(caps)
{
    d_ptr = new QAbstractGCPrivate;
}

void QAbstractGC::updateInternal(QPainterState *s)
{
    if (s) {
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
