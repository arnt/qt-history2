/****************************************************************************
** $Id$
**
** Definition of QImagePaintDevice classes
**
** Created : 991015
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/
 
#ifndef QIMAGEPAINTDEVICE_H
#define QIMAGEPAINTDEVICE_H

#ifndef QT_H
#include "qimage.h"
#include "qpaintdevice.h"
#include "qfont.h"
#include "qpen.h"
#include "qbrush.h"
#endif

class QTMap {
public:
    ~QTMap()
    {
	delete [] buffer;
    }

    const char* data() const
    {
	return buffer;
    }

    int width() const
    {
	return w;
    }

    int height() const
    {
	return h;
    }

    QPoint offset() const
    {
	return off;
    }

    QPoint advance() const
    {
	return adv;
    }

protected:
    char* buffer;
    int w, h;
    QPoint off;
    QPoint adv;
};

class QFontRenderer {
public:
    virtual ~QFontRenderer();
    virtual QTMap *mapFor( QChar ch )=0;
};

extern QFontRenderer* qt_font_renderer_ttf(const QFont& f);


class Q_EXPORT QImagePaintDevice32 : public QPaintDevice
{
public:
    QImagePaintDevice32(uchar* addr, int width, int height);
    QImagePaintDevice32(int width, int height);

    virtual ~QImagePaintDevice32();

    const QImage& image() const { return img; }

protected:
    bool cmd( int, QPainter *, QPDevCmdParam * );
    int  metric( int ) const;

private:
    void drawPoint(QPoint p);
    void moveTo(QPoint p);
    void lineTo(QPoint p);
    void setBrushOrigin(QPoint p);
    void drawLine(QPoint p0,QPoint p1);
    void drawRect(const QRect& r);
    void drawEllipse(const QRect& r);
    void drawRoundRect(const QRect& r, int i1, int i2);
    void drawArc(const QRect& r, int i1, int i2);
    void drawPie(const QRect& r, int i1, int i2);
    void drawChord(const QRect& r, int i1, int i2);
    void drawLineSegments(const QPointArray& pa);
    void drawPolyline(const QPointArray& pa);
    void drawCubicBezier(const QPointArray& pa);
    void drawPolygon(const QPointArray& pa, int i);
    void drawText(QPoint p, const QString& s);
    void drawPixmap(QPoint p, QPixmap pm);
    void drawImage(QPoint p, const QImage& im);
    void drawImage(QPoint p, const QImage& src, int sx, int sy,
			     int sw, int sh, int conversion_flags );
    void saveState();
    void restoreState();
    void setBkColor(const QColor& c);
    void setBkMode(int i);
    void setROP(int i);
    void setFont(const QFont& font);
    void setPen(const QPen& pen);
    void setBrush(const QBrush& brush);
    void setTabStops(int i);
    void setTabArray(int i, int* ivec);
    void setUnit(int i);
    void setVXform(int i);
    void setWXform(int i);
    void setClip(int i);
    void setWindow(const QRect& r);
    void setViewport(const QRect& r);
    void setMatrix(const QWMatrix& matrix, int i);
    void setClipRegion(const QRegion& rgn);

private:
    void init();
    QImage img;
    QRgb** rgb;
    QPen pen; QRgb fg/*redundant*/;
    QFont font;
    QBrush brush; QRgb br/*redundant*/;
    QPoint cursor;
    QPoint brushorg;
    //QMemArray<QRect> cliprect;

    // Current clip
    QRect* cliprect;
    int ncliprect;
    // Full-image clip
    QRect* cliprect1;
    int ncliprect1;
    // User-set clip
    QRect* ocliprect;
    int oncliprect;

    bool clipon;
    int clipcursor;

    bool find(QPoint);
    void findOutside(int x, int y, QRect& cr);

    void fillSpans(int n, QPoint* pt, int* w);
    void blit( int x, int y, const char* data, int w, int h );
    void blit( const QPoint&, const QTMap* );
    QFontRenderer* renderer;
};


#endif
