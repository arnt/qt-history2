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
    QPixmap();
    QPixmap(int w, int h);
    QPixmap(const QSize &);
#ifndef QT_NO_IMAGEIO
    QPixmap(const QString& fileName, const char *format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
#endif
    QPixmap(const char * const xpm[]);
    QPixmap(const QPixmap &);
    ~QPixmap();

    QPixmap &operator=(const QPixmap &);
    operator QVariant() const;

    bool isNull() const;
    int devType() const;

    int width() const;
    int height() const;
    QSize size() const;
    QRect rect() const;
    int depth() const;

    static int defaultDepth();

    void fill(const QColor &fillColor = Qt::white);
    void fill(const QWidget *widget, const QPoint &ofs);
    inline void fill(const QWidget *widget, int xofs, int yofs) { fill(widget, QPoint(xofs, yofs)); }

    QBitmap mask() const;
    void setMask(const QBitmap &);

    QPixmap alphaChannel() const;
    void setAlphaChannel(const QPixmap &);

    bool hasAlpha() const;
    bool hasAlphaChannel() const;

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QBitmap createHeuristicMask(bool clipTight = true) const;
#endif
    QBitmap createMaskFromColor(const QColor &maskColor) const;

    static QPixmap grabWindow(WId, int x=0, int y=0, int w=-1, int h=-1);
    static QPixmap grabWidget(QWidget *widget, const QRect &rect);
    static inline QPixmap grabWidget(QWidget *widget, int x=0, int y=0, int w=-1, int h=-1)
    { return grabWidget(widget, QRect(x, y, w, h)); }


#ifndef QT_NO_PIXMAP_TRANSFORMATION
    inline QPixmap scaled(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                          Qt::TransformationMode mode = Qt::FastTransformation) const
        { return scaled(QSize(w, h), aspectMode, mode); }
    QPixmap scaled(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                   Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap scaledToWidth(int w, Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap scaledToHeight(int h, Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap transformed(const QMatrix &, Qt::TransformationMode mode = Qt::FastTransformation) const;
    static QMatrix trueMatrix(const QMatrix &m, int w, int h);
#endif

    QImage toImage() const;
    static QPixmap fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);

#ifndef QT_NO_IMAGEIO
    bool load(const QString& fileName, const char *format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool loadFromData(const uchar *buf, uint len, const char* format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
    inline bool loadFromData(const QByteArray &data, const char* format = 0, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool save(const QString& fileName, const char* format, int quality = -1) const;
    bool save(QIODevice* device, const char* format, int quality = -1) const;
#endif

#if defined(Q_WS_WIN)
    enum HBitmapFormat {
        NoAlpha,
        PremultipliedAlpha
    };

    HBITMAP toWinHBITMAP(HBitmapFormat format = NoAlpha) const;
    static QPixmap fromWinHBITMAP(HBITMAP hbitmap, HBitmapFormat format = NoAlpha);
#endif

    inline QPixmap copy(int x, int y, int width, int height) const;
    QPixmap copy(const QRect &rect = QRect()) const;

    int serialNumber() const;

    bool isDetached() const;
    void detach();

    inline bool isQBitmap() const { return depth() == 1; }

#if defined(Q_WS_QWS)
    virtual const uchar * qwsScanLine(int) const;
    virtual int qwsBytesPerLine() const;
    QRgb *clut() const;
    int numCols() const;
#elif defined(Q_WS_MAC)
    Qt::HANDLE macQDHandle() const;
    Qt::HANDLE macQDAlphaHandle() const;
    Qt::HANDLE macCGHandle() const;
#elif defined(Q_WS_X11)
    static int x11SetDefaultScreen(int screen);
    void x11SetScreen(int screen);
    const QX11Info &x11Info() const;
    Qt::HANDLE x11PictureHandle() const;
#endif

#if !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
    Qt::HANDLE handle() const;
#endif

    QPaintEngine *paintEngine() const;

    inline bool operator!() const { return isNull(); }

protected:
    int metric(PaintDeviceMetric) const;

#ifdef QT3_SUPPORT
public:
#ifndef QT_NO_IMAGEIO
    enum ColorMode { Auto, Color, Mono };
    QT3_SUPPORT_CONSTRUCTOR QPixmap(const QString& fileName, const char *format, ColorMode mode);
    QT3_SUPPORT bool load(const QString& fileName, const char *format, ColorMode mode);
    QT3_SUPPORT bool loadFromData(const uchar *buf, uint len, const char* format, ColorMode mode);
#endif
    QT3_SUPPORT_CONSTRUCTOR QPixmap(const QImage& image);
    QT3_SUPPORT QPixmap &operator=(const QImage &);
    inline QT3_SUPPORT QImage convertToImage() const { return toImage(); }
    QT3_SUPPORT bool convertFromImage(const QImage &, ColorMode mode);
    QT3_SUPPORT bool convertFromImage(const QImage &img, Qt::ImageConversionFlags flags = Qt::AutoColor)
        { (*this) = fromImage(img, flags); return !isNull(); }
    inline QT3_SUPPORT operator QImage() const { return toImage(); }
    inline QT3_SUPPORT QPixmap xForm(const QMatrix &matrix) const { return transformed(matrix); }
    inline QT3_SUPPORT bool selfMask() const { return false; }
private:
    void resize_helper(const QSize &s);
public:
    inline QT3_SUPPORT void resize(const QSize &s) { resize_helper(s); }
    inline QT3_SUPPORT void resize(int width, int height) { resize_helper(QSize(width, height)); }
#endif

private:
    QPixmapData *data;

#ifndef QT_NO_IMAGEIO
    bool doImageIO(QImageWriter *io, int quality) const;
#endif
    enum Type { PixmapType, BitmapType };
    QPixmap(const QSize &s, Type);

    void init(int, int, Type = PixmapType);
    void deref();
#if defined(Q_WS_WIN)
    void initAlphaPixmap(uchar *bytes, int length, struct tagBITMAPINFO *bmi);
#endif
    Q_DUMMY_COMPARISON_OPERATOR(QPixmap)
#ifdef Q_WS_MAC
    friend CGContextRef qt_mac_cg_context(const QPaintDevice *);
    friend IconRef qt_mac_create_iconref(const QPixmap &);
#endif
    friend struct QPixmapData;
    friend class QBitmap;
    friend class QPaintDevice;
    friend class QPainter;
    friend class QGLWidget;
    friend class QX11PaintEngine;
    friend class QQuickDrawPaintEngine;
    friend class QCoreGraphicsPaintEngine;
    friend class QWidgetPrivate;
    friend class QRasterPaintEngine;
#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPixmap &);
#endif
};

Q_DECLARE_SHARED(QPixmap)


inline QPixmap QPixmap::copy(int x, int y, int width, int height) const
{
    return copy(QRect(x, y, width, height));
}

inline bool QPixmap::loadFromData(const QByteArray &buf, const char *format,
                                  Qt::ImageConversionFlags flags)
{
    return loadFromData((const uchar *)buf.constData(), buf.size(), format, flags);
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
