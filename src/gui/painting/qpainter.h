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

#ifndef QPAINTER_H
#define QPAINTER_H

#include "qnamespace.h"
#include "qrect.h"
#include "qpoint.h"
#include "qpixmap.h"
#include "qimage.h"

#ifndef QT_INCLUDE_COMPAT
#include "qpointarray.h"
#include "qpen.h"
#include "qbrush.h"
#include "qwmatrix.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#endif

class QPaintDevice;
class QPainterPrivate;
class QPointArray;
class QPen;
class QBrush;
class QWMatrix;
class QFontInfo;
class QFontMetrics;
class QTextItem;

class Q_GUI_EXPORT QPainter : public Qt
{
public:
    enum CoordinateMode { CoordDevice, CoordPainter };
    enum TextDirection { Auto, RTL, LTR };

    QPainter();
    QPainter(QPaintDevice *, bool unclipped = false);
    ~QPainter();

    QPaintDevice *device() const;

    bool begin(QPaintDevice *, bool unclipped = false);
    bool end();
    bool isActive() const;

    const QFont &font() const;
    void setFont(const QFont &f);

    QFontMetrics fontMetrics() const;
    QFontInfo fontInfo() const;

    void setPen(const QColor &color);
    void setPen(const QPen &pen);
    void setPen(PenStyle style);
    const QPen &pen() const;

    void setBrush(const QBrush &brush);
    void setBrush(BrushStyle style);
    const QBrush &brush() const;

    // attributes/modes
    void setRasterOp(RasterOp op);

    void setBackgroundMode(BGMode mode);
    BGMode backgroundMode() const;
    void setBackgroundColor(const QColor &color);
    const QColor &backgroundColor() const;

    const QPoint &brushOrigin() const;
    void setBrushOrigin(int x, int y);
    void setBrushOrigin(const QPoint &);

    const QBrush &background() const;
    const QPoint &backgroundOrigin() const;

    QRegion clipRegion(CoordinateMode = CoordDevice) const;
    void setClipRect(const QRect &, CoordinateMode = CoordDevice);
    void setClipRect(int x, int y, int w, int h, CoordinateMode = CoordDevice);
    void setClipRegion(const QRegion &, CoordinateMode = CoordDevice);
    void setClipping(bool enable);
    bool hasClipping() const;

    bool hasViewXForm() const;
    bool hasWorldXForm() const;

    void save();
    void restore();

#ifndef QT_NO_TRANSFORMATIONS
    void setWorldMatrix(const QWMatrix &wm, bool combine=false);
    const QWMatrix &worldMatrix() const;
    void setWorldXForm(bool enable);

    void setViewXForm(bool enable);
    QRect window() const;
    void setWindow(const QRect &window);
    void setWindow(int x, int y, int w, int h);
    QRect viewport() const;
    void setViewport(const QRect &viewport);
    void setViewport(int x, int y, int w, int h);

    void scale(double sx, double sy);
    void shear(double sh, double sv);
    void rotate(double a);
#endif
    void translate(double dx, double dy);
    void resetXForm();
    double translationX() const;
    double translationY() const;

    // drawing functions
    void drawLine(int x1, int y1, int x2, int y2);
    void drawLine(const QPoint &p1, const QPoint &p2);
    void drawRect(int x1, int y1, int w, int h);
    void drawRect(const QRect &r);
    void drawPoint(int x, int y);
    void drawPoint(const QPoint &p);
    void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawWinFocusRect(int x, int y, int w, int h);
    void drawWinFocusRect(int x, int y, int w, int h, const QColor &bgColor);
    void drawWinFocusRect(const QRect &r);
    void drawWinFocusRect(const QRect &r, const QColor &bgColor);
    void drawRoundRect(int x, int y, int w, int h, int = 25, int = 25);
    void drawRoundRect(const QRect &r, int = 25, int = 25);
    void drawEllipse(int x, int y, int w, int h);
    void drawEllipse(const QRect &r);
    void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawArc(const QRect &, int a, int alen);
    void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawPie(const QRect &, int a, int alen);
    void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawChord(const QRect &, int a, int alen);
    void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1);
    void drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1);
#ifndef QT_NO_BEZIER
    void drawCubicBezier(const QPointArray &pa, int index = 0);
#endif

    void drawTiledPixmap( int x, int y, int w, int h, const QPixmap &, int sx=0, int sy=0 );
    void drawTiledPixmap( const QRect &, const QPixmap &, const QPoint & );
    void drawTiledPixmap( const QRect &, const QPixmap & );
#ifndef QT_NO_PICTURE
    void drawPicture( const QPicture & );
    void drawPicture( int x, int y, const QPicture & );
    void drawPicture( const QPoint &, const QPicture & );
#endif

    void drawPixmap(int x, int y, const QPixmap &, int sx=0, int sy=0, int sw=-1, int sh=-1);
    void drawPixmap(const QPoint &, const QPixmap &, const QRect &sr);
    void drawPixmap(const QPoint &, const QPixmap &);
    void drawPixmap(const QRect &, const QPixmap &);

    void drawImage(int x, int y, const QImage &,
		   int sx = 0, int sy = 0, int sw = -1, int sh = -1,
		   int conversionFlags = 0);
    void drawImage(const QPoint &, const QImage &, const QRect &sr, int c2onversionFlags = 0);
    void drawImage(const QPoint &, const QImage &, int conversion_flags = 0);
    void drawImage(const QRect &, const QImage &);

    void drawText(int x, int y, const QString &, TextDirection dir = Auto);
    void drawText(const QPoint &, const QString &, TextDirection dir = Auto);

#ifdef QT_COMPAT
    QT_COMPAT void drawText(int x, int y, const QString &s, int pos, int len, TextDirection dir = Auto)
	{ drawText(x, y, s.mid(pos, len), dir); }
    QT_COMPAT void drawText(const QPoint &p, const QString &s, int pos, int len, TextDirection dir = Auto)
	{ drawText(p, s.mid(pos, len), dir); }
    QT_COMPAT void drawText(int x, int y, const QString &s, int len, TextDirection dir = Auto)
	{ drawText(x, y, s.left(len), dir); }
    QT_COMPAT void drawText(const QPoint &p, const QString &s, int len, TextDirection dir = Auto)
	{ drawText(p, s.left(len), dir); }
#endif

    void drawText(int x, int y, int w, int h, int flags, const QString&, int len = -1,
		  QRect *br=0);
    void drawText(const QRect &, int flags, const QString&, int len = -1, QRect *br=0);

    void drawTextItem(int x, int y, const QTextItem &ti, int textflags = 0);
    void drawTextItem(const QPoint& p, const QTextItem &ti, int textflags = 0);

    QRect boundingRect(int x, int y, int w, int h, int flags,
		       const QString&, int len = -1);
    QRect boundingRect(const QRect &, int flags,
		       const QString&, int len = -1);

    void fillRect(int x, int y, int w, int h, const QBrush &);
    void fillRect(const QRect &, const QBrush &);
    void eraseRect(int x, int y, int w, int h);
    void eraseRect(const QRect &);

    void map(int x, int y, int *rx, int *ry) const;
    QPoint 	xForm(const QPoint &) const;	// map virtual -> deviceb
    QRect 	xForm(const QRect &)	const;
    QPointArray xForm(const QPointArray &) const;
    QPointArray xForm(const QPointArray &, int index, int npoints) const;
    QPoint 	xFormDev(const QPoint &) const; // map device -> virtual
    QRect 	xFormDev(const QRect &)  const;
    QPointArray xFormDev(const QPointArray &) const;
    QPointArray xFormDev(const QPointArray &, int index, int npoints) const;

#if defined Q_WS_WIN // ### not liking this!!
    HDC handle() const;
#else
    Qt::HANDLE handle() const;
#endif

    static void setRedirected(const QPaintDevice *device, QPaintDevice *replacement,
			      const QPoint& offset = QPoint());
    static QPaintDevice *redirected(const QPaintDevice *device, QPoint *offset = 0);
    static void restoreRedirected(const QPaintDevice *device);

#ifdef QT_COMPAT
    static inline QT_COMPAT void redirect(QPaintDevice *pdev, QPaintDevice *replacement)
	{ setRedirected(pdev, replacement); }
    static inline QT_COMPAT QPaintDevice *redirect( QPaintDevice *pdev )
	{ return const_cast<QPaintDevice*>(redirected(pdev)); }
#endif

private:
    friend class QFontEngine;
    friend void qt_format_text( const QFont& font, const QRect &_r,
				int tf, const QString& str, int len, QRect *brect,
				int tabstops, int* tabarray, int tabarraylen,
				QPainter* painter );

    enum TransformationCodes {
	TxNone      = 0,
	TxTranslate = 1,
	TxScale     = 2,
	TxRotShear  = 3
    };

    void updateXForm();
    void updateInvXForm();
    void init();

    double m11() const;
    double m12() const;
    double m21() const;
    double m22() const;
    double dx() const;
    double dy() const;
    double im11() const;
    double im12() const;
    double im21() const;
    double im22() const;
    double idx() const;
    double idy() const;

    QPainterPrivate *d;
#if defined( Q_WS_X11 )
    friend class QFontEngineBox;
    friend class QFontEngineXLFD;
    friend class QFontEngineXft;
#elif defined( Q_WS_WIN )
    friend class QFontEngineWin;
#elif defined( Q_WS_QWS )
    friend class QWSManager;
    friend class QFontEngineBox;
    friend class QFontEngineFT;
#elif defined( Q_WS_MAC )
    friend class QFontEngineMac;
    friend class QMacStyleQDPainter;
#endif
};

//
// functions
//

inline void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
    drawLine(QPoint(x1, y1), QPoint(x2, y2));
}

inline void QPainter::drawRect(int x, int y, int w, int h)
{
    drawRect(QRect(x, y, w, h));
}

inline void QPainter::drawPoint(int x, int y)
{
    drawPoint(QPoint(x, y));
}

inline void QPainter::drawWinFocusRect(int x, int y, int w, int h)
{
    drawWinFocusRect(QRect(x, y, w, h));
}

inline void QPainter::drawWinFocusRect(int x, int y, int w, int h, const QColor &penColor)
{
    drawWinFocusRect(QRect(x, y, w, h), penColor);
}

inline void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    drawRoundRect(QRect(x, y, w, h), xRnd, yRnd);
}

inline void QPainter::drawEllipse(int x, int y, int w, int h)
{
    drawEllipse(QRect(x, y, w, h));
}

inline void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    drawArc(QRect(x, y, w, h), a, alen);
}

inline void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    drawPie(QRect(x, y, w, h), a, alen);
}

inline void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    drawChord(QRect(x, y, w, h), a, alen);
}

inline void QPainter::setClipRect(int x, int y, int w, int h, CoordinateMode mode)
{
    setClipRect(QRect(x, y, w, h), mode);
}

inline void QPainter::setWindow(const QRect &r)
{
    setWindow(r.x(), r.y(), r.width(), r.height());
}

inline void QPainter::setViewport(const QRect &r)
{
    setViewport(r.x(), r.y(), r.width(), r.height());
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr)
{
    drawPixmap(p.x(), p.y(), pm, sr.x(), sr.y(), sr.width(), sr.height());
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm)
{
    drawPixmap(p.x(), p.y(), pm, 0, 0, pm.width(), pm.height());
}

inline void QPainter::drawText(const QPoint &p, const QString &s, TextDirection dir)
{
    drawText(p.x(), p.y(), s, dir);
}

inline void QPainter::eraseRect(const QRect &r)
{
    eraseRect(r.x(), r.y(), r.width(), r.height());
}

inline void QPainter::fillRect(const QRect &r, const QBrush &b)
{
    fillRect(r.x(), r.y(), r.width(), r.height(), b);
}

inline void QPainter::setBrushOrigin(const QPoint &p)
{
    setBrushOrigin(p.x(), p.y());
}

inline void QPainter::drawTiledPixmap(const QRect &r, const QPixmap &pm, const QPoint &sp)
{
    drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), pm, sp.x(), sp.y());
}

inline void QPainter::drawTiledPixmap(const QRect &r, const QPixmap &pm)
{
    drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), pm, 0, 0);
}

inline void QPainter::drawPicture( const QPicture &pic )
{
    drawPicture(0, 0, pic);
}

inline void QPainter::drawPicture( const QPoint &p, const QPicture &pic )
{
    drawPicture(p.x(), p.y(), pic);
}

inline QRect QPainter::boundingRect(const QRect &r, int flags,
				     const QString&s, int len)
{
    return boundingRect(r.x(), r.y(), r.width(), r.height(), flags, s, len);
}

inline void QPainter::drawTextItem(int x, int y, const QTextItem &ti, int textflags)
{
    drawTextItem(QPoint(x, y), ti, textflags);
}

inline void QPainter::drawText(int x, int y, int w, int h, int flags, const QString &str,
				int len, QRect *br)
{
    drawText(QRect(x, y, w, h), flags, str, len, br);
}

inline void QPainter::drawImage(const QPoint &p, const QImage &i, const QRect &sr,
				 int conversionFlags)
{
    drawImage(p.x(), p.y(), i, sr.x(), sr.y(), sr.width(), sr.height(), conversionFlags);
}

inline void QPainter::drawImage(const QPoint &p, const QImage &i, int conversion_flags)
{
    drawImage(p.x(), p.y(), i, 0, 0, i.width(), i.height(), conversion_flags);
}

inline void QPainter::map(int x, int y, int *rx, int *ry) const
{
    QPoint p(x, y);
    p = xForm(p);
    *rx = p.x();
    *ry = p.y();
}

#endif // #ifndef QPAINTER_H
