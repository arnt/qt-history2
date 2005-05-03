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

#ifndef QGFX_QWS_H
#define QGFX_QWS_H

#include "QtCore/qnamespace.h"
#include "QtCore/qpoint.h"
#include "QtGui/qcolor.h"


class QImage;
class QScreen;
class QScreenCursor;
class QPen;
class QBrush;
class QRegion;
class QPolygon;
class QPaintDevice;
class QPixmap;

class Q_GUI_EXPORT QGfx {

public:
    // With loadable drivers, do probe here
    static QGfx *createGfx(int depth, unsigned char *buffer,
                            int w, int h, int linestep);

    virtual ~QGfx() {}

    virtual void setBrush(const QBrush &)=0;
    virtual void setClipDeviceRegion(const QRegion &)=0;

    // Fill operations - these use the current source (pixmap,
    // color, etc), and draws outline
    virtual void fillRect(int,int,int,int)=0;

    virtual void setLineStep(int)=0;

    // Special case of rect-with-pixmap-fill for speed/hardware acceleration
    virtual void blt(int,int,int,int,int,int)=0;
    virtual void tiledBlt(int,int,int,int)=0;

    enum SourceType { SourcePen, SourceImage, SourceAccel };

    // Setting up source data - can be solid color or pixmap data
    virtual void setSource(const QImage *)=0;
    virtual void setSource(const QPixmap *)=0;
    virtual void setSource(unsigned char *,int,int,int,int,QRgb *,int);

    // These apply only to blt's. For alpha values for general
    // drawing operations we should probably have a separate QGfx
    // class. It's not a high priority though.

    // Enum values: Ignore alpha information, alpha information encoded in
    // 32-bit rgba along with colors, alpha information in 8bpp
    // format in alphabits

    enum AlphaType { IgnoreAlpha, InlineAlpha, SeparateAlpha,
                     LittleEndianMask, BigEndianMask, SolidAlpha };

    // Can be no alpha, inline (32bit data), separate (for images),
    // LittleEndianMask/BigEndianMask 1bpp masks, constant alpha
    // value
    virtual void setAlphaType(AlphaType)=0;
    // Pointer to data, linestep
    virtual void setClut(QRgb *,int)=0;

    virtual void setScreen(QScreen *,QScreenCursor *,bool,int *,int *);
    void setShared(void * v) { shared_data=v; }
    bool isScreenGfx() { return is_screen_gfx; } //for cursor..

protected:
    bool is_screen_gfx;
    void * shared_data;
};

#endif // QGFX_QWS_H
