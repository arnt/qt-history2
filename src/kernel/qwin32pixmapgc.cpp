#include "qwin32pixmapgc.h"
#include "qwin32gc_p.h"
#include "q4painter_p.h"

#ifndef Q_Q4PAINTER
#include "q4paintdevice.h"
#else
#include "qpaintdevice.h"
#endif

#include "qpaintdevice.h"
#include "qpainter.h"

extern QRegion *paintEventClipRegion;
extern QPaintDevice *paintEventDevice;

QWin32PixmapGC::QWin32PixmapGC(const QPaintDevice *)
    : QWin32GC(0)
{
}

bool QWin32PixmapGC::begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped)
{
    Q_ASSERT(pdev->devType()==QInternal::Pixmap);
    if (isActive()) {				// already active painting
	qWarning("QWin32PixmapGC::begin: Painter is already active."
		 "\n\tYou must end() the painter before a second begin()");
	return false;
    }
    setActive(true);

    d->hdc = static_cast<const QPixmap*>(pdev)->handle();
    Q_ASSERT(d->hdc);

    QRegion *region = paintEventClipRegion;
    if (region)
	SelectClipRgn(d->hdc, region->handle());

    if (QColor::hPal()) {
	d->holdpal = SelectPalette(d->hdc, QColor::hPal(), TRUE);
	RealizePalette(d->hdc);
    }

    SetBkColor(d->hdc, COLOR_VALUE(state->bgBrush.color()));
    SetBkMode(d->hdc, state->bgMode == TransparentMode ? TRANSPARENT : OPAQUE);
    SetROP2(d->hdc, rasterOpCodes[state->rasterOp]);
    return true;
}

// void QWin32PixmapGC::initialize()
// {
// }

