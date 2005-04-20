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

class Q_GUI_EXPORT QGfx {

public:
    // With loadable drivers, do probe here
    static QGfx *createGfx(int depth, unsigned char *buffer,
                            int w, int h, int linestep);

    virtual ~QGfx() {}

    virtual void setPen(const QPen &)=0;
    virtual void setBrush(const QBrush &)=0;
    virtual void setBrushOrigin(int, int) = 0;
    virtual void setClipRegion(const QRegion &, Qt::ClipOperation)=0;
    virtual void setClipDeviceRegion(const QRegion &)=0;
    virtual void setClipping (bool)=0;
    // These will be called from qwidget_qws or qwidget_mac
    // to update the drawing area when a widget is moved
    virtual void setOffset(int,int)=0;
    virtual void setWidgetRect(int,int,int,int)=0;
    virtual void setWidgetRegion(const QRegion &)=0;
    virtual void setWidgetDeviceRegion(const QRegion &)=0;
    virtual void setSourceWidgetOffset(int x, int y) = 0;
    virtual void setGlobalRegionIndex(int idx) = 0;

    virtual void setDashedLines(bool d) = 0;
    virtual void setDashes(char *, int) = 0;

    virtual void setOpaqueBackground(bool b)=0;
    virtual void setBackgroundColor(QColor c)=0;

    // Drawing operations
    virtual void drawPoint(int,int)=0;
    virtual void drawPoints(const QPolygon &,int,int)=0;
    virtual void moveTo(int,int)=0;
    virtual void lineTo(int,int)=0;
    virtual void drawLine(int,int,int,int)=0;
    virtual void drawPolyline(const QPolygon &,int,int)=0;

    // current position
    virtual QPoint pos() const = 0;

    // Fill operations - these use the current source (pixmap,
    // color, etc), and draws outline
    virtual void fillRect(int,int,int,int)=0;
    virtual void drawPolygon(const QPolygon &,bool,int,int)=0;

    virtual void setLineStep(int)=0;

    // Special case of rect-with-pixmap-fill for speed/hardware acceleration
    virtual void blt(int,int,int,int,int,int)=0;
    virtual void scroll(int,int,int,int,int,int)=0;

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS) || !defined(QT_NO_PIXMAP_TRANSFORMATION)
    virtual void stretchBlt(int,int,int,int,int,int)=0;
#endif
    virtual void tiledBlt(int,int,int,int)=0;

    enum SourceType { SourcePen, SourceImage, SourceAccel };

    // Setting up source data - can be solid color or pixmap data
    virtual void setSource(const QPaintDevice *)=0;
    virtual void setSource(const QImage *)=0;
    virtual void setSource(unsigned char *,int,int,int,int,QRgb *,int);
    // This one is pen
    virtual void setSourcePen()=0;

    virtual void drawAlpha(int,int,int,int,int,int,int,int) {}

    virtual void hsync(int) {}

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
    virtual void setAlphaSource(unsigned char *,int)=0;
    virtual void setAlphaSource(int,int=-1,int=-1,int=-1)=0;

    virtual void setClut(QRgb *,int)=0;

    // Save and restore pen and brush state - necessary when setting
    // up a bitBlt for example
    virtual void save()=0;
    virtual void restore()=0;

    virtual void setScreen(QScreen *,QScreenCursor *,bool,int *,int *);
    void setShared(void * v) { shared_data=v; }
    bool isScreenGfx() { return is_screen_gfx; } //for cursor..

protected:
    bool is_screen_gfx;
    void * shared_data;
};

#endif // QGFX_QWS_H
