/****************************************************************************
**
** Definition of QPainter class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractgc.h"
#include "qpainter_p.h"

QAbstractGC::QAbstractGC(GCCaps caps)
    : dirtyFlag(0),
      changeFlag(0),
      state(0),
      gccaps(caps)
{
    d_ptr = new QAbstractGCPrivate;
}

void QAbstractGC::updateInternal(QPainterState *s, bool updateGC)
{
    if (!s || !updateGC) {
	state = s;
	return;
    }

    // Same state, do minimal update...
    if (s==state) {
	if (testDirty(DirtyPen))
	    updatePen(s);
	if (testDirty(DirtyBrush))
	    updateBrush(s);
	if (testDirty(DirtyFont))
	    updateFont(s);
	if (testDirty(DirtyRasterOp))
	    updateRasterOp(s);
	if (testDirty(DirtyBackground))
	    updateBackground(s);
	if (testDirty(DirtyTransform))
	    updateXForm(s);
	if (testDirty(DirtyClip)) {
	    updateClipRegion(s);
	}
	// Same painter, restoring old state.
    } else if (state && s->painter == state->painter) {
	if ((changeFlag&DirtyPen)!=0)
	    updatePen(s);
	if ((changeFlag&DirtyBrush)!=0)
	    updateBrush(s);
	if ((changeFlag&DirtyFont)!=0)
	    updateFont(s);
	if ((changeFlag&DirtyRasterOp)!=0)
	    updateRasterOp(s);
	if ((changeFlag&DirtyBackground)!=0)
	    updateBackground(s);
	if ((changeFlag&DirtyTransform)!=0)
	    updateXForm(s);
	if ((changeFlag&DirtyClip)!=0 || (changeFlag&DirtyClip) != 0)
	    updateClipRegion(s);
	changeFlag = 0;
	// Different painter or state == 0 which is true for first time call
    } else {
	updatePen(s);
	updateBrush(s);
	updateFont(s);
	updateRasterOp(s);
	updateBackground(s);
	updateXForm(s);
	updateClipRegion(s);
	changeFlag = 0;
    }
    dirtyFlag = 0;
    state = s;
}
