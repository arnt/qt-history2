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
#include <private/qcolor_p.h>
#include "qapplication.h"
#include "qwidget.h"
#include "qt_windows.h"


/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

HPALETTE QColor::hpal = 0;                        // application global palette

static QRgb* colArray = 0;                        // allocated pixel value
static int* ctxArray = 0;                        // allocation context
static int numPalEntries = 0;

static int current_alloc_context = 0;

inline COLORREF qrgb2colorref(QRgb rgb)
{
    return RGB(qRed(rgb),qGreen(rgb),qBlue(rgb));
}

int QColor::maxColors()
{
    static int maxcols = 0;
    if (maxcols == 0) {
        HDC hdc = qt_display_dc();
        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
            maxcols = GetDeviceCaps(hdc, SIZEPALETTE);
        else
            maxcols = -1;
    }
    return maxcols;
}

int QColor::numBitPlanes()
{
    return GetDeviceCaps(qt_display_dc(), BITSPIXEL);
}


void QColor::initialize()
{
    if (color_init)
        return;

    color_init = true;
    if (QApplication::colorSpec() == QApplication::NormalColor)
        return;

    int numCols = maxColors();
    if (numCols <= 16 || numCols > 256)        // no need to create palette
        return;

    colormodel = d8;

    HDC dc = qt_display_dc();                        // get global DC

    LOGPALETTE* pal = 0;
    if (QApplication::colorSpec() == QApplication::ManyColor) {
        numPalEntries = 20 + 6*6*6; // System + cube

        pal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + numPalEntries * sizeof(PALETTEENTRY));
        // Fill with system colors
        GetSystemPaletteEntries(dc, 0, 10, pal->palPalEntry);
        GetSystemPaletteEntries(dc, 246, 10, pal->palPalEntry + 10);
        // Make 6x6x6 color cube
        int idx = 20;
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
    } else {
        numPalEntries = 2; // black&white + allocate in alloc()

        pal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + numPalEntries * sizeof(PALETTEENTRY));

        pal->palPalEntry[0].peRed = 0;
        pal->palPalEntry[0].peGreen = 0;
        pal->palPalEntry[0].peBlue = 0;
        pal->palPalEntry[0].peFlags = 0;

        pal->palPalEntry[1].peRed = 255;
        pal->palPalEntry[1].peGreen = 255;
        pal->palPalEntry[1].peBlue = 255;
        pal->palPalEntry[1].peFlags = 0;
    }

    // Store palette in our own array
    colArray = new QRgb[256];                // Maximum palette size
    ctxArray = new int[256];                // Maximum palette size

    for(int i = 0; i < 256; i++) {
        if (i < numPalEntries) {
            colArray[i] = qRgb(pal->palPalEntry[i].peRed,
                                pal->palPalEntry[i].peGreen,
                                pal->palPalEntry[i].peBlue) & RGB_MASK;
            ctxArray[i] = current_alloc_context;
        } else {
            colArray[i] = 0;
            ctxArray[i] = -1;
        }
    }

    pal->palVersion = 0x300;
    pal->palNumEntries = numPalEntries;

    hpal = CreatePalette(pal);
    if (!hpal)
        qSystemWarning("QColor: Failed to create logical palette");
    free (pal);

    SelectPalette(dc, hpal, false);
    RealizePalette(dc);
}

/*! \internal */
const QRgb* QColor::palette(int* numEntries)
{
    if (numEntries)
        *numEntries = numPalEntries;
    if (!hpal)
        return 0;
    return colArray;
}

/*! \internal */
int QColor::setPaletteEntries(const QRgb* pal, int numEntries, int base)
{
    if (!hpal || !pal || numEntries < 1)
        return -1;
    if (base < 0)
        base = 20;                        // default: leave syscolors alone
    int maxSize = maxColors();
    if (base >= maxSize)
        return -1;
    if (base + numEntries > maxSize)
        numEntries = maxSize - base;
    int newSize = base + numEntries;
    if (newSize > numPalEntries)
        ResizePalette(hpal, newSize);
    PALETTEENTRY* newEntries = new PALETTEENTRY[numEntries];
    for (int i = 0; i < numEntries; i++) {
        newEntries[i].peRed = qRed(pal[i]);
        newEntries[i].peGreen = qGreen(pal[i]);
        newEntries[i].peBlue = qBlue(pal[i]);
        newEntries[i].peFlags = 0;
    }
    SetPaletteEntries(hpal, base, numEntries, newEntries);
    delete[] newEntries;
    newEntries = 0;

    numPalEntries = newSize;
    if (QApplication::colorSpec() == QApplication::CustomColor) {
        for (int j = base; j < base+numEntries; j++) {
            colArray[j] = pal[j-base];
            ctxArray[j] = current_alloc_context;
        }
    }

    HDC dc = qt_display_dc();
#ifndef Q_OS_TEMP
    UnrealizeObject(hpal);
#else
        GetStockObject(DEFAULT_PALETTE);
#endif
    SelectPalette(dc, hpal, false);
    RealizePalette(dc);

    return base;
}

void QColor::cleanup()
{
    if (hpal) {                                // delete application global
        DeleteObject(hpal);                        // palette
        hpal = 0;
    }
    if (colArray) {
        delete colArray;
        colArray = 0;
    }
    if (ctxArray) {
        delete ctxArray;
        ctxArray = 0;
    }
    color_init = false;
}

/*! \internal */
uint QColor::realizePal(QWidget *widget)
{
    if (!hpal)                                // not using palette
        return 0;
    HDC hdc = GetDC(widget->winId());
    HPALETTE hpalT = SelectPalette(hdc, hpal, false);
    uint i = RealizePalette(hdc);
#ifndef Q_OS_TEMP
    UpdateColors(hdc);
#endif
    SelectPalette(hdc, hpalT, false);
    ReleaseDC(widget->winId(), hdc);
    return i;
}

/*! \fn HPALETTE QColor::hPal()
  \internal
*/

/*****************************************************************************
  QColor member functions
 *****************************************************************************/

uint QColor::alloc(int)
{
    if (!color_init) {
        return d.d32.pix = 0;
    } else {
        int pix = qrgb2colorref(d.argb);
        if (hpal) {
            uchar idx = GetNearestPaletteIndex(hpal, pix);
            pix = PALETTEINDEX(idx);
            if (QApplication::colorSpec() == QApplication::CustomColor) {
                PALETTEENTRY fe;
                GetPaletteEntries(hpal, idx, 1, &fe);
                QRgb fc = qRgb(fe.peRed, fe.peGreen, fe.peBlue);
                if (fc != d.argb) {        // Color not found in palette
                    // Find a free palette entry
                    bool found = false;
                    for (int i = 0; i < numPalEntries; i++) {
                        if (ctxArray[i] < 0) {
                            found = true;
                            idx = i;
                            break;
                        }
                    }
                    if (!found && numPalEntries < 256) {
                        idx = numPalEntries;
                        numPalEntries++;
                        ResizePalette(hpal, numPalEntries);
                        found = true;
                    }
                    if (found) {
                        // Change unused palette entry into the new color
                        PALETTEENTRY ne;
                        ne.peRed = qRed(d.argb);
                        ne.peGreen = qGreen(d.argb);
                        ne.peBlue = qBlue(d.argb);
                        ne.peFlags = 0;
                        SetPaletteEntries(hpal, idx, 1, &ne);
                        pix = PALETTEINDEX(idx);
                        colArray[idx] = d.argb;
                        ctxArray[idx] = current_alloc_context;
                        HDC dc = qt_display_dc();
#ifndef Q_OS_TEMP
                    UnrealizeObject(hpal);
#else
                        GetStockObject(DEFAULT_PALETTE);
#endif
                        SelectPalette(dc, hpal, false);
                        RealizePalette(dc);
                    }
                }
                if (idx < numPalEntries) {         // Sanity check
                    if (ctxArray[idx] < 0)
                        ctxArray[idx] = current_alloc_context; // mark it
                    else if (ctxArray[idx] != current_alloc_context)
                        ctxArray[idx] = 0;        // Set it to default ctx
                }
            }
            d.d8.pix = idx;
            d.d8.dirty = false;
            return pix;
        } else {
            return d.d32.pix = pix;
        }
    }
}

void QColor::setSystemNamedColor(const QString& name)
{
    // setSystemNamedColor should look up rgb values from the built in
    // color tables first (see qcolor_p.cpp), and failing that, use
    // the window system's interface for translating names to rgb values...
    // we do this so that things like uic can load an XPM file with named colors
    // and convert it to a png without having to use window system functions...
    d.argb = qt_get_rgb_val(name.latin1());
    QRgb rgb;
    if (qt_get_named_rgb(name.latin1(), &rgb)) {
        setRgb(qRed(rgb), qGreen(rgb), qBlue(rgb));
        if (colormodel == d8) {
            d.d8.invalid = false;
            d.d8.dirty = true;
            d.d8.pix = 0;
        } else {
            alloc();
        }
    } else {
        // set to invalid color
        *this = QColor();
    }
}


#define MAX_CONTEXTS 16
static int  context_stack[MAX_CONTEXTS];
static int  context_ptr = 0;

static void init_context_stack()
{
    static bool did_init = false;
    if (!did_init) {
        did_init = true;
        context_stack[0] = current_alloc_context = 0;
    }
}


int QColor::enterAllocContext()
{
    static int context_seq_no = 0;
    init_context_stack();
    if (context_ptr+1 == MAX_CONTEXTS) {
        qWarning("QColor::enterAllocContext: Context stack overflow");
        return 0;
    }
    current_alloc_context = context_stack[++context_ptr] = ++context_seq_no;
    return current_alloc_context;
}


void QColor::leaveAllocContext()
{
    init_context_stack();
    if (context_ptr == 0) {
        qWarning("QColor::leaveAllocContext: Context stack underflow");
        return;
    }

    current_alloc_context = context_stack[--context_ptr];
}


int QColor::currentAllocContext()
{
    return current_alloc_context;
}


void QColor::destroyAllocContext(int context)
{
    if (!hpal || QApplication::colorSpec() != QApplication::CustomColor)
        return;

    init_context_stack();

    for (int i = 2; i < numPalEntries; i++) {          // 2: keep black & white
        switch (context) {
        case -2:
            if (ctxArray[i] > 0)
                ctxArray[i] = -1;
            break;
        case -1:
            ctxArray[i] = -1;
            break;
        default:
            if (ctxArray[i] == context)
                ctxArray[i] = -1;
        break;
        }
    }

    //# Should reset unused entries in hpal to 0, to minimize the app's demand

}
