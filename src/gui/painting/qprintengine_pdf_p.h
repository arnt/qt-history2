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

#include "QtCore/qmap.h"
#include "QtGui/qmatrix.h"
#include "QtCore/qstring.h"
#include "QtCore/qvector.h"
#include "QtGui/qpaintengine.h"
#include "QtGui/qprintengine.h"

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;
class QPdfByteStream;

class QPdfStream
{
public:
    QPdfStream();
    void setStream(QDataStream& val);
    uint write(const char* val, uint len);

private:
    QDataStream* stream_;
};

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
    QPdfImage();
    explicit QPdfImage(const QImage& pm);
    QPdfImage(const QImage& pm, const QImage& mask);
    ~QPdfImage();

    void streamText(QPdfByteStream &stream);
    QByteArray getDefinition();
    const char* data() const
    {
        return rawdata_;
    }
    QPdfImage* stencil;
    QPdfImage* softmask;
    int rawLength() const
    {
        return rawlen_;
    }

    int w() const {return w_;}
    int h() const {return h_;}

    void setMaskObj(int obj);
    void setSoftMaskObj(int obj);
    void setLenObj(int obj);
    int lenObj() const {return lenobj_;}
    int hardMaskObj() const {return maskobj_;}
    int softMaskObj() const {return softmaskobj_;}
    bool hasHardMask() const {return hashardmask_;}
    QByteArray name;

private:
    void init();
    static bool interpolation_;
    int w_, h_;
    char* rawdata_;
    int rawlen_;
    bool isgray_, hashardmask_, ismask_;
    int softmaskobj_, maskobj_, lenobj_;
    int convert(const QImage& img, const QImage& mask);
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
    QPdfBrush* setPixmap(const QPixmap& pm, const QMatrix& mat);

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
        QByteArray getDefinition(int objno);
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
    QByteArray id_;
};

class QPdfPen : public QPdfObject
{
public:
    QPdfPen();
    void streamText(QPdfByteStream &stream);

    QPdfPen* setLineWidth(double v);
    QPdfPen* setLineCap(unsigned v);
    QPdfPen* setLineJoin(unsigned v);
    QPdfPen* setMiterLimit(double v);
    QPdfPen* setDashArray(const QPen& pen, double phase);
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
    QVector<double> lw_, ml_;
    QVector<int>   lc_, lj_;
    QVector<QByteArray> ri_;
    QVector<QColor> col_;
    QVector<bool> stroking_;
};

class QPdfPath : public QPdfObject
{
public:
    enum PAINTTYPE {
        NONE					= 0L,
        CLOSE         = 1L<<0,
        STROKE        = 1L<<1,
        FILLNONZERO   = 1L<<2,
        FILLEVENODD   = 1L<<3,
        CLIPPING			= 1L<<4
    };

    struct Element
    {
        enum SUBTYPE{
            NONE,
            LINE,
            CURVE
        };

        Element() : type(NONE) {} // for QVector
        void setLine(qreal x, qreal y)
        {
            line.x=x;
            line.y=y,
                type = LINE;
        }
        void setCurve(qreal x1, qreal y1,qreal x2, qreal y2,qreal xnew, qreal ynew)
        {
            curve.x1=x1;
            curve.y1=y1;
            curve.x2=x2;
            curve.y2=y2;
            curve.xnew=xnew;
            curve.ynew=ynew;
            type = CURVE;
        }

        SUBTYPE type;

        union
        {
            struct { qreal x, y; } line;
            struct { qreal x1, y1, x2, y2, xnew, ynew; } curve;
        };
    };

    struct SubPath
    {
        SubPath() : closed(false), initialized(false)
        {}

        QVector<Element> elements;
        struct { qreal x, y, w, h; } rect;
        struct { qreal x,y; } start;

        bool closed;
        bool initialized;
        void close()
        {
            closed = true;
        }
        bool valid() const
        {
            if (elements.empty() || !initialized)
                return false;
            return true;
        }
    };

    explicit QPdfPath(const QPdfPen* = 0, const QPdfBrush* = 0, int brushflags = NONE,  bool closed = false);

    QVector<SubPath> subpaths;
    void streamText(QPdfByteStream &stream);
    int painttype;
    QMatrix currentMatrix;
    bool hasTrueAlpha() const {return hasTrueStrokeAlpha() || hasTrueNonStrokeAlpha();}
    int alphaObject() const {return alphaobj_;}
    QByteArray alphaName() const {return alphaname_;};
    void setAlpha(const QByteArray& prefix, int objno) {alphaname_ = prefix + QByteArray::number(objno); alphaobj_ = objno;}
    QByteArray getAlphaDefinition() const;

private:
    QByteArray paintOperator() const;
    qreal ca_, CA_;
    QByteArray alphaname_;
    int alphaobj_;
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

    QVector<QPdfImage*> images;
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
    
private:
    Q_DISABLE_COPY(QPdfEnginePrivate)
    void writeInfo();
    void writeCatalog();
    void writePageRoot();
    void flushPage();
    uint requestObjNumber()
    {
        return objnumber_++;
    }

    QVector<int> xrefpos_;
    int streampos_;
    int options_;
    bool landscape_;
    int width_, height_;
    QDataStream* stream_;

    int xprintf(const char* fmt, ...);
    int addxentry(int objnumber, bool printostr = true);

    int objnumber_, pagesobjnumber_, root_, info_, gsobjnumber_, pcsobjnumber_, csobjnumber_,csgobjnumber_;
    QVector<QVector<uint> > pageobjnumber_;
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
    void drawEllipse (const QRectF & rectangle);
    void drawPath (const QPainterPath & path);
    void drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr);
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
    void adaptMonochromePixmap(QPixmap& pm);

    void drawPathPrivate (const QPainterPath & path);

    QPrinter::PageSize pagesize_;
    bool clipping_, tofile_;
    Qt::BGMode backgroundMode;
    QIODevice* device_;

    QBrush* lastBrush_, *bgBrush_;
    QPen* lastPen_;
    QMatrix* lastMatrix_;
    int pixmapnumber_;
    QPointF *lastBrushOrig_;
    QRegion* lastClipRegion_;
    QFile* outFile_;
};

#endif // QPRINTENGINE_PDF_P_H
