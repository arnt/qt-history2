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

#ifndef QPRINTENGINE_PDF_P_H
#define QPRINTENGINE_PDF_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qprintengine.h"

#ifndef QT_NO_PRINTER
#include "QtCore/qmap.h"
#include "QtGui/qmatrix.h"
#include "QtCore/qstring.h"
#include "QtCore/qvector.h"
#include "QtGui/qpaintengine.h"
#include "QtGui/qpainterpath.h"
#include "QtCore/qdatastream.h"

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;

const char *qt_real_to_string(qreal val, char *buf);
const char *qt_int_to_string(int val, char *buf);

namespace QPdf {

    class ByteStream
    {
    public:
        ByteStream(QByteArray *b) :ba(b) {}
        ByteStream &operator <<(char chr) { *ba += chr; return *this; }
        ByteStream &operator <<(const char *str) { *ba += str; return *this; }
        ByteStream &operator <<(const QByteArray &str) { *ba += str; return *this; }
        ByteStream &operator <<(qreal val) { char buf[256]; *ba += qt_real_to_string(val, buf); return *this; }
        ByteStream &operator <<(int val) { char buf[256]; *ba += qt_int_to_string(val, buf); return *this; }
    private:
        QByteArray *ba;
    };

    enum PathFlags {
        PathNone, 
        PathIsClip,
        StrokeOnly,
        FillOnly,
        StrokeAndFill
    };
    QByteArray generatePath(const QPainterPath &path, PathFlags flags);
    QByteArray generateMatrix(const QMatrix &matrix);
    QByteArray generateDashes(const QPen &pen);
    QByteArray patternForBrush(const QBrush &b);

};


class QPdfPage : public QPdf::ByteStream
{
public:
    QPdfPage();
    QByteArray content() { return data; }

    QVector<uint> images;
    QVector<uint> graphicStates;
    QVector<uint> patterns;

    void streamImage(int w, int h, int object);
private:
    QByteArray data;
};

class QPdfEnginePrivate
{
public:
    QPdfEnginePrivate();
    ~QPdfEnginePrivate();

    QPdfPage* currentPage;
    void newPage();
    void setDimensions(int w, int h){width_ = w; height_ = h;}

    QString title, creator, author;

    void setDevice(QIODevice*);
    void unsetDevice();
    int width() const {return width_;}
    int height() const {return height_;}

    void writeHeader();
    void writeTail();

    QPrinter::PageOrder pageOrder;
    QPrinter::Orientation orientation;
    bool fullPage;

    int addImage(const QImage &image, bool *bitmap);
    int addPenGState(const QPen &pen);
    int addBrushGState(const QBrush &brush);
    int addBrushPattern(const QBrush &b, const QMatrix &matrix, const QPointF &brushOrigin, bool *specifyColor);

private:
    Q_DISABLE_COPY(QPdfEnginePrivate)
    void writeInfo();
    void writePageRoot();

    inline uint requestObject() { return currentObject++; }

    QVector<int> xrefPositions;
    int width_, height_;
    QDataStream* stream;
    int streampos;

    int writeImage(const QByteArray &data, int width, int height, int depth,
                   int maskObject, int softMaskObject);
    void writePage();

    int addXrefEntry(int object, bool printostr = true);
    void xprintf(const char* fmt, ...);
    inline void write(const QByteArray &data) {
        stream->writeRawData(data.constData(), data.size());
        streampos += data.size();
    }
    
    int writeCompressed(const char *src, int len);
    inline int writeCompressed(const QByteArray &data) { return writeCompressed(data.constData(), data.length()); }

    int currentObject;

    // various PDF objects
    int pageRoot, catalog, info, graphicsState, patternColorSpace, colorSpace, colorSpaceGray;
    QVector<uint> pages;
};


class QPdfEngine : public QPaintEngine, public QPrintEngine
{
public:
    QPdfEngine();
    virtual ~QPdfEngine();

    // reimplementations QPaintEngine
    bool begin(QPaintDevice *pdev);
    bool end();
    void drawPoints(const QPointF *points, int pointCount);
    void drawLines(const QLineF *lines, int lineCount);
    void drawRects(const QRectF *rects, int rectCount);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPath (const QPainterPath & path);
    void drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlags flags = Qt::AutoColor);
    void drawTiledPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QPointF & point);

    void updateState(const QPaintEngineState &state);
    Type type() const;
    // end reimplementations QPaintEngine

    // reimplementations QPrintEngine
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;
    int metric(QPaintDevice::PaintDeviceMetric) const;
    bool abort() {return false;}
    bool newPage();
    QPrinter::PrinterState printerState() const {return QPrinter::Idle;}
    // end reimplementations QPrintEngine

    void updateClipPath(const QPainterPath & path, Qt::ClipOperation op);

    void setPen();
    void setBrush();


    QRect paperRect() const;
    QRect pageRect() const;

    // ### unused, should have something for this in QPrintEngine
    void setAuthor(const QString &author);
    QString author() const;

    void setDevice(QIODevice* dev);

private:
    Q_DISABLE_COPY(QPdfEngine)
    QPdfEnginePrivate *d;

    QPrinter::PageSize pagesize_;

    QIODevice* device_;
    QFile* outFile_;

    Qt::BGMode backgroundMode;
    QBrush backgroundBrush;
    QPointF brushOrigin;
    QBrush brush;
    QPen pen;
    uint brushSoftMask;
    uint penSoftMask;
    QMatrix matrix;
    QList<QPainterPath> clips;
    bool clipEnabled;
    bool allClipped;
    bool hasPen;
    bool hasBrush;
};

#endif // QT_NO_PRINTER
#endif // QPRINTENGINE_PDF_P_H
