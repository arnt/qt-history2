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

#include "QtGui/qpaintdevice.h"
#include "QtGui/qcolor.h"
#include "QtCore/qnamespace.h"
#include "QtCore/qstring.h" // char*->QString conversion
#include "QtGui/qimage.h"

class QImageWriter;
class QPixmapPrivate;
class QColor;
class QVariant;
class QX11Info;

struct QPixmapData;

class Q_GUI_EXPORT QPixmap : public QPaintDevice
{
public:
    enum Optimization { DefaultOptim, NoOptim, MemoryOptim=NoOptim,
                        NormalOptim, BestOptim, LoadOptim };

    QPixmap();
    QPixmap(const QImage& image);
    QPixmap(int w, int h, int depth = -1, Optimization = DefaultOptim);
    QPixmap(const QSize &, int depth = -1, Optimization = DefaultOptim);
#ifndef QT_NO_IMAGEIO
    QPixmap(const QString& fileName, const char *format = 0,
            Qt::ImageConversionFlags flags = Qt::AutoColor,
            Optimization = DefaultOptim);
#endif
    QPixmap(const char * const xpm[]);
    QPixmap(const QPixmap &);
    ~QPixmap();

    QPixmap &operator=(const QPixmap &);
    QPixmap &operator=(const QImage &);
    operator QVariant() const;

    bool isNull() const;

    int width() const;
    int height() const;
    QSize size() const;
    QRect rect() const;
    int depth() const;
    static int defaultDepth();

    void fill(const QColor &fillColor = Qt::white);
    inline void fill(const QWidget *, int xofs, int yofs);
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
    QBitmap createMaskFromColor(const QColor &maskColor) const;
    static QPixmap grabWindow(WId, int x=0, int y=0, int w=-1, int h=-1);
    static QPixmap grabWidget(QWidget *widget, const QRect &rect);
    static inline QPixmap grabWidget(QWidget *widget, int x=0, int y=0, int w=-1, int h=-1);

#ifndef QT_NO_PIXMAP_TRANSFORMATION
    inline QPixmap scale(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                        Qt::TransformationMode mode = Qt::FastTransformation) const
        { return scale(QSize(w, h), aspectMode, mode); }
    QPixmap scale(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                 Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap scaleWidth(int w) const;
    QPixmap scaleHeight(int h) const;
    QPixmap transform(const QMatrix &, Qt::TransformationMode mode = Qt::FastTransformation) const;
    static QMatrix trueMatrix(const QMatrix &m, int w, int h);
#endif

    QImage toImage() const;
    bool fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);

#ifndef QT_NO_IMAGEIO
    bool load(const QString& fileName, const char *format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool loadFromData(const uchar *buf, uint len, const char* format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool loadFromData(const QByteArray &data, const char* format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool save(const QString& fileName, const char* format, int quality = -1) const;
    bool save(QIODevice* device, const char* format, int quality = -1) const;
#endif

#if defined(Q_WS_WIN)
    HBITMAP hbm() const;
    HDC getDC() const;
    void releaseDC(HDC) const;
#endif

    int serialNumber() const;

    Optimization optimization() const;
    void setOptimization(Optimization);
    static Optimization defaultOptimization();
    static void setDefaultOptimization(Optimization);

    bool isDetached() const;
    virtual void detach();

    bool isQBitmap() const;

#if defined(Q_WS_QWS)
    virtual unsigned char * scanLine(int) const;
    virtual int bytesPerLine() const;
    QRgb *clut() const;
    int numCols() const;
#endif
#if defined(Q_WS_X11)
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

    Q_DUMMY_COMPARISON_OPERATOR(QPixmap)


#ifdef QT3_SUPPORT
#ifndef QT_NO_IMAGEIO
    enum ColorMode { Auto, Color, Mono };
    QT3_SUPPORT_CONSTRUCTOR QPixmap(const QString& fileName, const char *format, ColorMode mode);
    QT3_SUPPORT bool load(const QString& fileName, const char *format, ColorMode mode);
    QT3_SUPPORT bool loadFromData(const uchar *buf, uint len, const char* format, ColorMode mode);
#endif
    inline QT3_SUPPORT QImage convertToImage() const { return toImage(); }
    QT3_SUPPORT bool convertFromImage(const QImage &, ColorMode mode);
    QT3_SUPPORT bool convertFromImage(const QImage &img, Qt::ImageConversionFlags flags = Qt::AutoColor)
        { return fromImage(img, flags); }
    inline QT3_SUPPORT operator QImage() const { return toImage(); }
    inline QT3_SUPPORT QPixmap xForm(const QMatrix &matrix) const { return transform(matrix); }
#endif

protected:
    QPixmap(int w, int h, const uchar *data, bool isXbitmap);
    int metric(PaintDeviceMetric) const;

private:
    QPixmapData *data;
private:
#ifndef QT_NO_IMAGEIO
    bool doImageIO(QImageWriter *io, int quality) const;
#endif
    QPixmap(int w, int h, int depth, bool, Optimization);
    void init(int, int, int, bool, Optimization);
    void deref();
    QPixmap copy(bool ignoreMask = false) const;
#if defined(Q_WS_WIN)
    void initAlphaPixmap(uchar *bytes, int length, struct tagBITMAPINFO *bmi);
#endif
    static Optimization defOptim;
    friend struct QPixmapData;
    friend class QBitmap;
    friend class QPaintDevice;
    friend class QPainter;
    friend class QGLWidget;
    friend class QX11PaintEngine;
#if defined(Q_WS_MAC)
    friend CGImageRef qt_mac_create_cgimage(const QPixmap &, Qt::PixmapDrawingMode, bool);
#endif
    friend class QQuickDrawPaintEngine;
    friend class QCoreGraphicsPaintEngine;
    friend class QWSPaintEngine;
    friend class QFontEngineXft;
    friend class QwsPixmap;
    friend class QWidgetPrivate;
    friend void qt_bit_blt(QPaintDevice *, int, int, const QPaintDevice *, int, int, int, int, bool);
};

Q_DECLARE_SHARED(QPixmap)

inline void QPixmap::fill(const QWidget *w, int x, int y)
{
    fill(w, QPoint(x, y));
}

inline QPixmap QPixmap::grabWidget(QWidget *widget, int x, int y, int w, int h)
{
    return grabWidget(widget, QRect(x, y, w, h));
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
#ifdef QT3_SUPPORT
QT3_SUPPORT Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy, const QPixmap *src,
                                    int sx=0, int sy=0, int sw=-1, int sh=-1);
#endif // QT3_SUPPORT

#endif // QPIXMAP_H
