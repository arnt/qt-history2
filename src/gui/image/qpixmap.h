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

#ifndef QPIXMAP_H
#define QPIXMAP_H

#include "qpaintdevice.h"
#include "qcolor.h"
#include "qnamespace.h"
#include "qstring.h" // char*->QString conversion
#include "qimage.h"

class QPixmapPrivate;
class QColor;

#if defined(Q_WS_X11)
// #### go away
#include <qx11info_x11.h>
class QX11PaintEngine;
#endif

struct QPixmapData;

class Q_GUI_EXPORT QPixmap : public QPaintDevice
{
public:
    enum ColorMode { Auto, Color, Mono };
    enum Optimization { DefaultOptim, NoOptim, MemoryOptim=NoOptim,
                        NormalOptim, BestOptim };

    QPixmap();
    QPixmap(const QImage& image);
    QPixmap(int w, int h, int depth = -1, Optimization = DefaultOptim);
    QPixmap(const QSize &, int depth = -1, Optimization = DefaultOptim);
#ifndef QT_NO_IMAGEIO
    QPixmap(const QString& fileName, const char *format=0, ColorMode mode=Auto);
    QPixmap(const QString& fileName, const char *format, int conversion_flags);
    explicit QPixmap(const char * const xpm[]);
#endif
    QPixmap(const QPixmap &);
    ~QPixmap();

    QPixmap &operator=(const QPixmap &);
    QPixmap &operator=(const QImage &);

    bool isNull() const;

    int width() const;
    int height() const;
    QSize size() const;
    QRect rect() const;
    int depth() const;
    static int defaultDepth();

    void fill(const QColor &fillColor = Qt::white);
    void fill(const QWidget *, int xofs, int yofs);
    void fill(const QWidget *, const QPoint &ofs);
    void resize(int width, int height);
    void resize(const QSize &);

    const QBitmap *mask() const;
    void setMask(const QBitmap &);
    bool selfMask() const;
    bool hasAlpha() const;
    bool hasAlphaChannel() const;
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QBitmap createHeuristicMask(bool clipTight = true) const;
#endif
    static QPixmap grabWindow(WId, int x=0, int y=0, int w=-1, int h=-1);
    static QPixmap grabWidget(QWidget *widget, int x=0, int y=0, int w=-1, int h=-1);

#ifndef QT_NO_PIXMAP_TRANSFORMATION
    QPixmap xForm(const QMatrix &) const;
    static QMatrix trueMatrix(const QMatrix &m, int w, int h);
#endif

    QImage convertToImage() const;
    bool convertFromImage(const QImage &, ColorMode mode=Auto);
    bool convertFromImage(const QImage &, int conversion_flags);
#ifndef QT_NO_IMAGEIO
    bool load(const QString& fileName, const char *format=0, ColorMode mode=Auto);
    bool load(const QString& fileName, const char *format, int conversion_flags);
    bool loadFromData(const uchar *buf, uint len, const char* format=0, ColorMode mode=Auto);
    bool loadFromData(const uchar *buf, uint len, const char* format, int conversion_flags);
    bool loadFromData(const QByteArray &data, const char* format=0, int conversion_flags=0);
    bool save(const QString& fileName, const char* format, int quality = -1) const;
    bool save(QIODevice* device, const char* format, int quality = -1) const;
#endif

#if defined(Q_WS_WIN)
    HBITMAP hbm() const;
    HDC getDC() const;
    void releaseDC(HDC) const;

    // These functions are internal and used by Windows 9x only
    bool isMultiCellPixmap() const;
    HBITMAP multiCellBitmap() const;
    int multiCellOffset() const;
    int allocCell();
    void freeCell(bool = false);
#endif

    int serialNumber() const;

    Optimization optimization() const;
    void setOptimization(Optimization);
    static Optimization defaultOptimization();
    static void setDefaultOptimization(Optimization);

    virtual void detach();

    bool isQBitmap() const;

#if defined(Q_WS_QWS)
    virtual unsigned char * scanLine(int) const;
    virtual int bytesPerLine() const;
    QRgb *clut() const;
    int numCols() const;
#elif defined(Q_WS_X11)
    static int x11SetDefaultScreen(int screen);
    void x11SetScreen(int screen);
    const QX11Info &x11Info() const;
    Qt::HANDLE xftPictureHandle() const;
    Qt::HANDLE xftDrawHandle() const;
#endif

#ifndef Q_WS_WIN
    Qt::HANDLE handle() const;
#endif

    QPaintEngine *paintEngine() const;

    inline bool operator!() const { return isNull(); }
    inline operator QImage() const { return convertToImage(); }

    Q_DUMMY_COMPARISON_OPERATOR(QPixmap)

protected:
    QPixmap(int w, int h, const uchar *data, bool isXbitmap);
    int metric(int) const;

private:
    QPixmapData *data;
private:
#ifndef QT_NO_IMAGEIO
    bool doImageIO(QImageIO* io, int quality) const;
#endif
    QPixmap(int w, int h, int depth, bool, Optimization);
    void init(int, int, int, bool, Optimization);
    void deref();
    QPixmap copy(bool ignoreMask = false) const;
#if defined(Q_WS_WIN)
    void initAlphaPixmap(uchar *bytes, int length, struct tagBITMAPINFO *bmi);
    void convertToAlphaPixmap(bool initAlpha=true);
#endif
    static Optimization defOptim;
    friend class QBitmap;
    friend class QPaintDevice;
    friend class QPainter;
    friend class QGLWidget;
    friend class QX11PaintEngine;
#if defined(Q_WS_MAC)
    friend CGImageRef qt_mac_create_cgimage(const QPixmap &, Qt::PixmapDrawingMode);
#endif
    friend class QQuickDrawPaintEngine;
    friend class QCoreGraphicsPaintEngine;
    friend class QWSPaintEngine;
    friend class QFontEngineXft;
    friend void qt_bit_blt(QPaintDevice *, int, int, const QPaintDevice *, int, int, int, int, bool);
};

inline void QPixmap::fill(const QWidget *w, int x, int y)
{
    fill(w, QPoint(x, y));
}

/*****************************************************************************
 QPixmap stream functions
*****************************************************************************/

#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPixmap &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPixmap &);
#endif

/*****************************************************************************
 QPixmap (and QImage) helper functions
*****************************************************************************/
#ifdef QT_COMPAT
QT_COMPAT Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy, const QPixmap *src,
                                    int sx=0, int sy=0, int sw=-1, int sh=-1);
#endif // QT_COMPAT


#endif // QPIXMAP_H
