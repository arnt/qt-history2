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

#include "qcolormap.h"
#include "qvector.h"
#include "qt_windows.h"

class QColormapPrivate
{
public:
    QColormapPrivate()
        : mode(QColormap::Direct), depth(0), hpal(0)
    { ref = 0; }
    ~QColormapPrivate()
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
    HDC dc = qt_display_dc();

    screenMap = new QColormapPrivate;
    screenMap ->ref = 1;
    screenMap->depth = GetDeviceCaps(dc, BITSPIXEL);

    screenMap->numcolors = -1;
    if (GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE)
        screenMap->numcolors = GetDeviceCaps(dc, SIZEPALETTE);
    
    if (screenMap->numcolors <= 16 || screenMap->numcolors > 256)        // no need to create palette
        return;

    LOGPALETTE* pal = 0;
    int numPalEntries = 6*6*6; // System + cube

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
        qSystemWarning("QColor: Failed to create logical palette");
    free (pal);

    SelectPalette(dc, screenMap->hpal, false);
    RealizePalette(dc);

    PALETTEENTRY paletteEntries[256];
    screenMap->numcolors = GetPaletteEntries(screenMap->hpal, 0, 255, paletteEntries);

    screenMap->palette.resize(screenMap->numcolors);
    for(int i = 0; i < screenMap->numcolors; i++)
        screenMap->palette[i] = qRgb(pal->palPalEntry[i].peRed, pal->palPalEntry[i].peGreen, pal->palPalEntry[i].peBlue);
}

void QColormap::cleanup()
{
    if(!screenMap)
        return;

    if (screenMap->hpal) {                                // delete application global
        DeleteObject(screenMap->hpal);                        // palette
        screenMap->hpal = 0;
    }
    delete screenMap;
    screenMap = 0;

}

QColormap QColormap::instance(int screen)
{
    return QColormap();
}

QColormap::QColormap()
    : d(screenMap)
{ ++d->ref; }

QColormap::QColormap(const QColormap &colormap)
    :d (colormap.d)
{ ++d->ref; }

QColormap::~QColormap()
{
    if (!--d->ref)
        delete d;
}

QColormap::Mode QColormap::mode() const
{ return d->mode; }

int QColormap::depth() const
{ return d->depth; }

int QColormap::size() const
{
    return d->numcolors;
}

uint QColormap::pixel(const QColor &color) const
{
    QColor c = color.toRgb();
    COLORREF rgb = RGB(c.red(), c.green(), c.blue());
    if(d->hpal) {
        uint px = GetNearestPaletteIndex(d->hpal, rgb);
        return px;
    }
    return rgb;
}

const QColor QColormap::colorAt(uint pixel) const
{
    if(d->hpal) {
        if(pixel < d->numcolors)
            return d->palette.at(pixel);
        return QColor();
    }
    return QColor(GetRValue(pixel), GetGValue(pixel), GetBValue(pixel));
}


HPALETTE QColormap::hPal()
{
    return screenMap ? screenMap->hpal : 0;
}


const QVector<QColor> QColormap::colormap() const
{
    return d->palette;
}
