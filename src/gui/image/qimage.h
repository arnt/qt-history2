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

#ifndef QIMAGE_H
#define QIMAGE_H

#include "QtGui/qrgb.h"
#include "QtGui/qpaintdevice.h"
#include "QtCore/qrect.h"
#include "QtCore/qbytearray.h"

class QIODevice;
class QStringList;
class QMatrix;
class QVariant;
template <class T> class QList;
template <class T> class QVector;

struct QImageData;
class QImageDataMisc; // internal
#ifndef QT_NO_IMAGE_TEXT
class Q_GUI_EXPORT QImageTextKeyLang {
public:
    QImageTextKeyLang(const char* k, const char* l) : key(k), lang(l) { }
    QImageTextKeyLang() { }

    QByteArray key;
    QByteArray lang;

    bool operator< (const QImageTextKeyLang& other) const
        { return key < other.key || key==other.key && lang < other.lang; }
    bool operator== (const QImageTextKeyLang& other) const
        { return key==other.key && lang==other.lang; }
};
#endif //QT_NO_IMAGE_TEXT


class Q_GUI_EXPORT QImage : public QPaintDevice
{
public:
    enum Endian { BigEndian, LittleEndian, IgnoreEndian };
    enum InvertMode { InvertRgb, InvertRgba };
    enum Format {
        Format_Invalid,
        Format_Mono,
        Format_MonoLSB,
        Format_Indexed8,
        Format_RGB32,
        Format_ARGB32,
        Format_ARGB32_Premultiplied
    };

    QImage();
    QImage(const QSize &size, Format format);
    QImage(int with, int height, Format format);

    QImage(int width, int height, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    QImage(const QSize&, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    explicit QImage(const char * const xpm[]);
#ifndef QT_NO_IMAGEIO
    explicit QImage(const QString &fileName, const char *format = 0);
#ifndef QT_NO_CAST_FROM_ASCII
    explicit QImage(const char *fileName, const char *format = 0);
#endif
#endif
    QImage(uchar *data, int w, int h, int depth, const QRgb *colortable, int numColors, Endian bitOrder);
#ifdef Q_WS_QWS
    QImage(uchar *data, int w, int h, int depth, int pbl, const QRgb *colortable, int numColors, Endian bitOrder);
#endif
    QImage(const QImage &);
    ~QImage();

    QImage &operator=(const QImage &);
    bool isNull() const;

    bool operator==(const QImage &) const;
    bool operator!=(const QImage &) const;
    operator QVariant() const;
    void detach();
    bool isDetached() const;

    QImage copy() const;
    QImage copy(const QRect &rect) const;
    inline QImage copy(int x, int y, int w, int h) const
        { return copy(QRect(x, y, w, h)); }

    Format format() const;

    QImage convertToFormat(Format f) const;

    int width() const;
    int height() const;
    QSize size() const;
    QRect rect() const;

    int depth() const;
    int numColors() const;
    Endian bitOrder() const;

    QRgb color(int i) const;
    void setColor(int i, QRgb c);
    void setNumColors(int);

    bool hasAlphaBuffer() const;
    void setAlphaBuffer(bool);

    void setAlphaChannel(const QImage &alphaChannel);
    QImage alphaChannel() const;

    bool allGray() const;
    bool isGrayscale() const;

    uchar *bits();
    const uchar *bits() const;
    uchar *scanLine(int);
    const uchar *scanLine(int) const;

    QVector<QRgb> colorTable() const;
    void setColorTable(const QVector<QRgb> colors);

    int numBytes() const;
    int bytesPerLine() const;

    void fill(uint pixel);
    void invertPixels(InvertMode = InvertRgb);

    bool valid(int x, int y) const;
    int pixelIndex(int x, int y) const;
    QRgb pixel(int x, int y) const;
    void setPixel(int x, int y, uint index_or_rgb);

    QImage convertDepthWithPalette(int, QRgb* p, int pc, Qt::ImageConversionFlags flags = Qt::AutoColor) const;
    QImage convertDepth(int, Qt::ImageConversionFlags flags = Qt::AutoColor) const;
    QImage convertBitOrder(Endian) const;

#ifndef QT_NO_IMAGE_TRANSFORMATION
    inline QImage scaled(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                        Qt::TransformationMode mode = Qt::FastTransformation) const
        { return scaled(QSize(w, h), aspectMode, mode); }
    QImage scaled(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                 Qt::TransformationMode mode = Qt::FastTransformation) const;
    QImage scaledToWidth(int w) const;
    QImage scaledToHeight(int h) const;
    QImage transformed(const QMatrix &matrix, Qt::TransformationMode mode = Qt::FastTransformation) const;
    static QMatrix trueMatrix(const QMatrix &, int w, int h);
#endif

#ifndef QT_NO_IMAGE_DITHER_TO_1
    QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const;
#endif
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QImage createHeuristicMask(bool clipTight = true) const;
#endif
#ifndef QT_NO_IMAGE_MIRROR
    QImage mirrored(bool horizontally = false, bool vertically = true) const;
#endif
    QImage rgbSwapped() const;

#ifndef QT_NO_IMAGEIO
    bool load(const QString &fileName, const char* format=0);
    bool loadFromData(const uchar *buf, int len, const char *format = 0);
    inline bool loadFromData(const QByteArray &data, const char* format=0)
        { return loadFromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format); }

    bool save(const QString &fileName, const char* format, int quality=-1) const;
    bool save(QIODevice * device, const char* format, int quality=-1) const;

    static QImage fromData(const uchar *data, int size, const char *format = 0);
    inline static QImage fromData(const QByteArray &data, const char *format = 0)
        { return fromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format); }
#endif //QT_NO_IMAGEIO

    int serialNumber() const;

#ifdef Q_WS_QWS
    virtual const uchar * qwsScanLine(int) const;
    virtual int qwsBytesPerLine() const;
#endif
    QPaintEngine *paintEngine() const;

    // Auxiliary data
    int dotsPerMeterX() const;
    int dotsPerMeterY() const;
    void setDotsPerMeterX(int);
    void setDotsPerMeterY(int);
    QPoint offset() const;
    void setOffset(const QPoint&);
#ifndef QT_NO_IMAGE_TEXT
    QList<QImageTextKeyLang> textList() const;
    QStringList textLanguages() const;
    QStringList textKeys() const;
    QString text(const char* key, const char* lang=0) const;
    QString text(const QImageTextKeyLang&) const;
    void setText(const char* key, const char* lang, const QString&);
#endif

#ifdef QT3_SUPPORT
    QT3_SUPPORT uchar **jumpTable();
    QT3_SUPPORT const uchar * const *jumpTable() const;
    inline QT3_SUPPORT void reset() { *this = QImage(); }
    static inline QT3_SUPPORT Endian systemByteOrder()
        { return QSysInfo::ByteOrder == QSysInfo::BigEndian ? BigEndian : LittleEndian; }
    inline QT3_SUPPORT QImage swapRGB() const { return rgbSwapped(); }
    inline QT3_SUPPORT QImage mirror(bool horizontally = false, bool vertically = true) const
        { return mirrored(horizontally, vertically); }
    QT3_SUPPORT bool create(const QSize&, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    QT3_SUPPORT bool create(int width, int height, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    inline QT3_SUPPORT QImage xForm(const QMatrix &matrix) const { return transformed(matrix); }
    inline QT3_SUPPORT QImage smoothScale(int w, int h, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio) const
        { return scaled(QSize(w, h), mode, Qt::SmoothTransformation); }
    inline QImage QT3_SUPPORT smoothScale(const QSize &s, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio) const
        { return scaled(s, mode, Qt::SmoothTransformation); }
    inline QT3_SUPPORT QImage scaleWidth(int w) const { return scaledToWidth(w); }
    inline QT3_SUPPORT QImage scaleHeight(int h) const { return scaledToHeight(h); }
    inline QT3_SUPPORT void invertPixels(bool invertAlpha) { invertAlpha ? invertPixels(InvertRgba) : invertPixels(InvertRgb); }
    inline QT3_SUPPORT QImage copy(int x, int y, int w, int h, Qt::ImageConversionFlags) const
        { return copy(QRect(x, y, w, h)); }
    inline QT3_SUPPORT QImage copy(const QRect &rect, Qt::ImageConversionFlags) const
        { return copy(rect); }
    static QT3_SUPPORT Endian systemBitOrder();
#ifndef QT_NO_IMAGEIO
    inline QT3_SUPPORT_CONSTRUCTOR QImage(const QByteArray &data)
        : QPaintDevice(QInternal::Image)
        { *this = QImage::fromData(data); }
#endif
#endif

protected:
    virtual int metric(PaintDeviceMetric metric) const;

private:
    QImageData *d;

    friend class QPixmap;
};

Q_DECLARE_SHARED(QImage);
Q_DECLARE_TYPEINFO(QImage, Q_MOVABLE_TYPE);

// QImage stream functions

#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QImage &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QImage &);
#endif

#ifdef QT3_SUPPORT
Q_GUI_EXPORT QT3_SUPPORT void bitBlt(QImage* dst, int dx, int dy, const QImage* src,
                                     int sx=0, int sy=0, int sw=-1, int sh=-1, Qt::ImageConversionFlags flags = Qt::AutoColor);
#endif

#endif // QIMAGE_H
