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

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;
class QPdfByteStream;

class QPdfObject
{
public:
    QPdfObject()
        : type(NONE), appended(0), hassoftmask_p(false) {}
    virtual ~QPdfObject() {}

    enum TYPE{
        NONE,
        MATRIX,
        PATH,
        PEN,
        BRUSH,
        IMAGE,
        GRADIENT,
        SOFTMASK
    };

    TYPE type;
    int appended;

    virtual void streamText(QPdfByteStream &) {}

    bool hasSoftMask()
    {
        return hassoftmask_p;
    }

protected:
    bool hassoftmask_p;
};

class QPdfMatrix : public QPdfObject
{
public:
    QPdfMatrix();
    QPdfMatrix* setMatrix(QMatrix const& m);
    QMatrix currentMatrix() const;
    QMatrix lastMatrix() const;
    void streamText(QPdfByteStream &stream);

    static void streamMatrix(const QMatrix &m, QPdfByteStream &stream);

private:
    QVector<QMatrix> matrices_;
};

class QPdfImage : public QPdfObject
{
public:
    QPdfImage(int w, int h, int object);
    void streamText(QPdfByteStream &stream);
    int w;
    int h;
    int object;
};

class QPdfGradient : public QPdfObject
{
public:
    QPdfGradient();
    ~QPdfGradient();
    void setParameter(const QColor &b, const QColor &e, qreal x0, qreal y0, qreal x1, qreal y1);


    void setObjects(int mainobj, int funcobj);
    QByteArray getMainDefinition();
    QByteArray getFuncDefinition();
    QByteArray getSoftMaskFormDefinition();
    QByteArray getSoftMaskMainDefinition();
    QByteArray getSoftMaskFuncDefinition();

    void setColorSpaceObject(int obj);
    void setSoftMaskColorSpaceObject(int obj);
    void setSoftMaskObjects(int formobj, int mainobj, int funcobj);
    void setSoftMaskRange(qreal x, qreal y, qreal w, qreal h);

    int mainObject() const {return mainobj_;}
    int functionObject() const {return funcobj_;}
    int softMaskFormObject() const {return smfmobj_;}
    int softMaskMainObject() const {return (softmask) ? softmask->mainObject() : -1;}
    int softMaskFunctionObject() const {return (softmask) ? softmask->functionObject() : -1;}
    QByteArray softMaskGraphicStateName() const {return (softmask) ? name + "ExtGS" : QByteArray();}

    QByteArray name;

private:
    QPdfGradient* softmask;
    bool issoftmask_;
    int mainobj_, funcobj_, smfmobj_, csrgbobj_, csgrayobj_;
    qreal x0_, y0_, x1_, y1_;
    QColor beg_, end_;
    qreal x_, y_, w_, h_;

    QByteArray getSingleMainDefinition();
    QByteArray getSingleFuncDefinition();
    QByteArray softMaskName() const {return (softmask) ? name + "SM" : QByteArray();}
};

class QPdfBrush : public QPdfObject
{
public:
    explicit QPdfBrush(const QByteArray& id = QByteArray());
    ~QPdfBrush();

    qreal alpha() const;

    QPdfBrush* setFixed(Qt::BrushStyle style, const QColor &rgba, const QMatrix& mat = QMatrix());
    QPdfBrush* setGradient(const QColor &rgba, const QColor &gradrgba,
                           qreal bx, qreal by, qreal ex, qreal ey,
                           qreal bbox_xb, qreal bbox_xe, qreal bbox_yb, qreal bbox_ye,
                           const QMatrix& mat = QMatrix());
    QPdfBrush* setPixmap(QPdfImage *image, const QMatrix& mat);

    bool noBrush() const {return nobrush_;}
    void streamText(QPdfByteStream &stream);

    class Pattern
    {
    public:
        QByteArray name;

    protected:
        QByteArray defBegin(int ptype, int w, int h);
        QByteArray getDefinition(const QByteArray &res);
        QMatrix matrix;
    };

    class FixedPattern : public Pattern
    {
    public:
        explicit FixedPattern(const QByteArray& n = QByteArray(), int idx = -1, const QColor &col = QColor(),
                              const QMatrix& mat = QMatrix());
        QColor rgba;
        bool isEmpty() const {return patternidx == Qt::NoBrush;}
        bool isSolid() const {return patternidx == Qt::SolidPattern;}
        bool isTruePattern() const {return patternidx > 1;}

        QByteArray getDefinition();
    private:
        int patternidx;
    };

    class PixmapPattern : public Pattern
    {
    public:
        explicit PixmapPattern(const QByteArray& n = QByteArray(), QPdfImage* im = 0,
                               const QMatrix& mat = QMatrix());
        QPdfImage* image;
        QByteArray getDefinition();
    };

    class GradientPattern : public Pattern
    {
    public:
        explicit GradientPattern(const QByteArray& n = QByteArray(), QPdfGradient* grad = 0,
                                 const QMatrix& mat = QMatrix());
        QPdfGradient* shader;
        void setMainObj(int obj);
        int getMainObj() const {return mainobj_;}
        QByteArray getDefinition();
    private:
        int mainobj_;
    };

    QVector<FixedPattern> fixeds;
    QVector<GradientPattern> gradients;
    QVector<PixmapPattern> pixmaps;

    bool isGradient() const;
    bool firstIsGradient() const;

private:
    enum SUBTYPE{
        FIXED,
        GRADIENT,
        PIXMAP,
    };

    QVector<SUBTYPE> streamstate_;
    QVector<qreal> alpha_;
    bool nobrush_;
public:
    QByteArray id_;
};

class QPdfPen : public QPdfObject
{
public:
    QPdfPen();
    void streamText(QPdfByteStream &stream);

    QPdfPen* setLineWidth(qreal v);
    QPdfPen* setLineCap(unsigned v);
    QPdfPen* setLineJoin(unsigned v);
    QPdfPen* setMiterLimit(qreal v);
    QPdfPen* setDashArray(const QPen& pen, qreal phase);
    QPdfPen* setColor(const QColor &rgba);
    bool stroking() const;
    qreal alpha() const;

private:
    struct DashArray
    {
        DashArray(){}
        DashArray(const QVector<qreal>& v, qreal p)
            :seq(v), phase(p) {}

        QVector<qreal> seq;
        qreal phase;
    };

    enum SUBTYPE{
        LINEWIDTH,
        LINECAP,
        LINEJOIN,
        MITERLIMIT,
        DASHARRAY,
        COLOR
    };


    QVector<SUBTYPE> streamstate_;
    QVector<DashArray> da_;
    QVector<qreal> lw_, ml_;
    QVector<int>   lc_, lj_;
    QVector<QByteArray> ri_;
    QVector<QColor> col_;
    QVector<bool> stroking_;
};

class QPdfPath : public QPdfObject
{
public:
    enum PAINTTYPE {
        NONE	      = 0L,
        STROKE        = 1L<<0,
        FILLNONZERO   = 1L<<1,
        FILLEVENODD   = 1L<<2,
        CLIPPING      = 1L<<3
    };

    explicit QPdfPath(const QPdfPen *pen, const QPdfBrush *brush, const QPainterPath &p);

    QPainterPath path;

    void streamText(QPdfByteStream &stream);
    int painttype;
    QMatrix currentMatrix;
    bool hasTrueAlpha() const {return hasTrueStrokeAlpha() || hasTrueNonStrokeAlpha();}
    QByteArray getAlphaDefinition() const;

    int alphaObject;

private:
    QByteArray paintOperator() const;
    qreal ca_, CA_;
    bool gradientstrokealpha_;

    bool hasTrueStrokeAlpha() const {return CA_ >= 0.0 && CA_ < 1.0 ;}
    bool hasTrueNonStrokeAlpha() const {return ca_ >= 0.0 && ca_ < 1.0 ;}
    bool hasGradientNonStrokeAlpha() const {return gradientstrokealpha_;}
    void streamCoreText(QPdfByteStream &s) const;
};

class QPdfPage : public QPdfObject
{
public:
    QPdfPage();
    void  destroy();
    void streamText(QPdfByteStream &stream);
    QPdfObject* append(QPdfObject* val, bool protect = false);

    QVector<uint> images;
    QVector<QPdfPath*> paths;

private:
    QVector<QPdfObject*> gobjects_;
    int width_, height_;
    bool landscape_;

    bool predType(int i, QPdfObject::TYPE);
    bool nextType(int i, QPdfObject::TYPE);
};

class QPdfEnginePrivate
{
public:
    QPdfEnginePrivate();
    ~QPdfEnginePrivate();

    QPdfPage* curPage;
    void newPage();
    void setDimensions(int w, int h){width_ = w; height_ = h;}

    QPdfMatrix* curMatrix;
    QPdfPen* curPen;
    QPdfBrush* curBrush;
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

    int addImage(const QImage &image, bool maybeBitmap);

private:
    Q_DISABLE_COPY(QPdfEnginePrivate)
    void writeInfo();
    void writePageRoot();
    void flushPage();
    inline uint requestObject() { return currentObject++; }

    QVector<int> xrefPositions;
    int width_, height_;
    QDataStream* stream;
    int streampos;

    int writeImage(const QByteArray &data, int width, int height, int depth,
                   int maskObject, int softMaskObject);

    int addXrefEntry(int object, bool printostr = true);
    void xprintf(const char* fmt, ...);
    int write(const char *src, int len);
    inline int write(const QByteArray &data) { return write(data.constData(), data.length()); }

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
    void updateBackground (Qt::BGMode bgmode, const QBrush & brush);
    void updateBrush (const QBrush & brush, const QPointF & origin);
    void updateClipPath(const QPainterPath & path, Qt::ClipOperation op);
    void updateClipRegion (const QRegion & region, Qt::ClipOperation op);
    void updatePen (const QPen & pen);
    void updateMatrix (const QMatrix & matrix);

    // reimplementations QPrintEngine
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;
    int metric(QPaintDevice::PaintDeviceMetric) const;
    bool abort() {return false;}
    bool newPage();
    QPrinter::PrinterState printerState() const {return QPrinter::Idle;}
    // end reimplementations QPrintEngine


    QRect paperRect() const;
    QRect pageRect() const;
    // ### unused, should have something for this in QPrintEngine
    void setAuthor(const QString &author);
    QString author() const;

    void setDevice(QIODevice* dev);

private:
    Q_DISABLE_COPY(QPdfEngine)
    QPdfEnginePrivate *d;

    void setBrush (QPdfBrush& pbr, const QBrush & brush, const QPointF& origin);
    void drawPathPrivate (const QPainterPath & path, bool isClip = false);

    QPrinter::PageSize pagesize_;

    QIODevice* device_;
    QFile* outFile_;

    Qt::BGMode backgroundMode;
    QBrush backgroundBrush;
    QPointF brushOrigin;
    QBrush brush;
    QPen pen;
    QMatrix matrix;
    QRegion clipRegion;
};

#endif // QT_NO_PRINTER
#endif // QPRINTENGINE_PDF_P_H
