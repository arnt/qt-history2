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

#include "qcolor.h"
#include "qcolormap.h"
#include "qvector.h"
#include "qt_windows.h"

class QColormapPrivate
{
public:
    inline QColormapPrivate()
        : ref(0), mode(QColormap::Direct), depth(0), hpal(0)
    { }

    QAtomic ref;

    QColormap::Mode mode;
    int depth;
    int numcolors;

    HPALETTE hpal;
    QVector<QColor> palette;
};

static QColormapPrivate *screenMap = 0;

void QColormap::initialize()
{
    HDC dc = qt_win_display_dc();

    screenMap = new QColormapPrivate;
    screenMap ->ref = 1;
    screenMap->depth = GetDeviceCaps(dc, BITSPIXEL);

    screenMap->numcolors = -1;
    if (GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE)
        screenMap->numcolors = GetDeviceCaps(dc, SIZEPALETTE);

    if (screenMap->numcolors <= 16 || screenMap->numcolors > 256)        // no need to create palette
        return;

    LOGPALETTE* pal = 0;
    int numPalEntries = 6*6*6; // color cube

    pal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + numPalEntries * sizeof(PALETTEENTRY));
    // Make 6x6x6 color cube
    int idx = 0;
    for(int ir = 0x0; ir <= 0xff; ir+=0x33) {
        for(int ig = 0x0; ig <= 0xff; ig+=0x33) {
            for(int ib = 0x0; ib <= 0xff; ib+=0x33) {
                pal->palPalEntry[idx].peRed = ir;
                pal->palPalEntry[idx].peGreen = ig;
                pal->palPalEntry[idx].peBlue = ib;
                pal->palPalEntry[idx].peFlags = 0;
                idx++;
            }
        }
    }

    pal->palVersion = 0x300;
    pal->palNumEntries = numPalEntries;

    screenMap->hpal = CreatePalette(pal);
    if (!screenMap->hpal)
        qErrnoWarning("QColor::initialize: Failed to create logical palette");
    free (pal);

    SelectPalette(dc, screenMap->hpal, false);
    RealizePalette(dc);

    PALETTEENTRY paletteEntries[256];
    screenMap->numcolors = GetPaletteEntries(screenMap->hpal, 0, 255, paletteEntries);

    screenMap->palette.resize(screenMap->numcolors);
    for (int i = 0; i < screenMap->numcolors; i++) {
        screenMap->palette[i] = qRgb(paletteEntries[i].peRed,
                                     paletteEntries[i].peGreen,
                                     paletteEntries[i].peBlue);
    }
}

void QColormap::cleanup()
{
    if (!screenMap)
        return;

    if (screenMap->hpal) {                                // delete application global
        DeleteObject(screenMap->hpal);                        // palette
        screenMap->hpal = 0;
    }

    delete screenMap;
    screenMap = 0;
}

QColormap QColormap::instance(int)
{ return QColormap(); }

QColormap::QColormap()
    : d(screenMap)
{ d->ref.ref(); }

QColormap::QColormap(const QColormap &colormap)
    :d (colormap.d)
{ d->ref.ref(); }

QColormap::~QColormap()
{
    if (!d->ref.deref())
        delete d;
}

QColormap::Mode QColormap::mode() const
{ return d->mode; }

int QColormap::depth() const
{ return d->depth; }

int QColormap::size() const
{ return d->numcolors; }

uint QColormap::pixel(const QColor &color) const
{
    const QColor c = color.toRgb();
    COLORREF rgb = RGB(c.red(), c.green(), c.blue());
    if (d->hpal)
        return PALETTEINDEX(GetNearestPaletteIndex(d->hpal, rgb));
    return rgb;
}

const QColor QColormap::colorAt(uint pixel) const
{
    if (d->hpal) {
        if (pixel < uint(d->numcolors))
            return d->palette.at(pixel);
        return QColor();
    }
    return QColor(GetRValue(pixel), GetGValue(pixel), GetBValue(pixel));
}


HPALETTE QColormap::hPal()
{ return screenMap ? screenMap->hpal : 0; }


const QVector<QColor> QColormap::colormap() const
{ return d->palette; }
