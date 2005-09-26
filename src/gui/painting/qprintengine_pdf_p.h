#ifndef QPRINTENGINE_PDF_P_H
#define QPRINTENGINE_PDF_P_H

#include <qmap.h>
#include <qmatrix.h>
#include <qstring.h>
#include <qvector.h>

#include "qpaintengine.h"
#include "qprintengine.h"

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;

class QPdfStream
{
public:
    QPdfStream();
    void setCompression(bool val);
    bool isCompressed() const;
    void setStream(QDataStream& val);
    uint write(const char* val, uint len);

private:
    bool compressed_;
    QDataStream* stream_;
};

class QPdfObject
{
public:
    QPdfObject() : type(NONE), appended(0), hassoftmask_p(false) {}
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

    virtual QString streamText() {return QString();}

    //! Defines transparency handling for all PdfObjects. Default: true
    static void setTransparency(bool val)
    {
        transparency_ = val;
    }

    static void setCompression(bool val)
    {
#ifndef QT_NO_COMPRESS
        compression_ = val;
#endif
    }

    static bool transparencySupported()
    {
        return transparency_;
    }

    bool hasSoftMask()
    {
        return transparencySupported() && hassoftmask_p;
    }

    static bool compression()
    {
        return compression_;
    }


protected:
    static QString pdfqreal(double val) {return QString::number(val, 'f', 6);}
    bool hassoftmask_p;

private:
    static bool transparency_;
    static bool compression_;
};

class QPdfMatrix : public QPdfObject
{
public:
    QPdfMatrix();
    QPdfMatrix* setMatrix(QMatrix const& m);
    QMatrix currentMatrix() const;
    QMatrix lastMatrix() const;
    QString streamText();

    static QString streamMatrix(QMatrix const m);

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

    QString streamText();
    QString getDefinition();
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

    static void setInterpolation(bool val);
    void setMaskObj(int obj);
    void setSoftMaskObj(int obj);
    void setLenObj(int obj);
    int lenObj() const {return lenobj_;}
    int hardMaskObj() const {return maskobj_;}
    int softMaskObj() const {return softmaskobj_;}
    bool hasHardMask() const {return hashardmask_;}
    QString name;

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
    QString getMainDefinition();
    QString getFuncDefinition();
    QString getSoftMaskFormDefinition();
    QString getSoftMaskMainDefinition();
    QString getSoftMaskFuncDefinition();

    void setColorSpaceObject(int obj);
    void setSoftMaskColorSpaceObject(int obj);
    void setSoftMaskObjects(int formobj, int mainobj, int funcobj);
    void setSoftMaskRange(qreal x, qreal y, qreal w, qreal h);

    int mainObject() const {return mainobj_;}
    int functionObject() const {return funcobj_;}
    int softMaskFormObject() const {return smfmobj_;}
    int softMaskMainObject() const {return (softmask) ? softmask->mainObject() : -1;}
    int softMaskFunctionObject() const {return (softmask) ? softmask->functionObject() : -1;}
    QString softMaskGraphicStateName() const {return (softmask) ? name + "ExtGS" : "";}

    QString name;

private:
    QPdfGradient* softmask;
    bool issoftmask_;
    int mainobj_, funcobj_, smfmobj_, csrgbobj_, csgrayobj_;
    qreal x0_, y0_, x1_, y1_;
    QColor beg_, end_;
    qreal x_, y_, w_, h_;

    QString getSingleMainDefinition();
    QString getSingleFuncDefinition();
    QString softMaskName() const {return (softmask) ? name + "SM" : "";}
};

class QPdfBrush : public QPdfObject
{
public:
    explicit QPdfBrush(const QString& id = QString());
    ~QPdfBrush();

    qreal alpha() const;

    QPdfBrush* setFixed(Qt::BrushStyle style, const QColor &rgba, const QMatrix& mat = QMatrix());
    QPdfBrush* setGradient(const QColor &rgba, const QColor &gradrgba, qreal bx, qreal by, qreal ex, qreal ey,
                          qreal bbox_xb, qreal bbox_xe, qreal bbox_yb, qreal bbox_ye, const QMatrix& mat = QMatrix());
    QPdfBrush* setPixmap(const QPixmap& pm, const QMatrix& mat);

    bool noBrush() const {return nobrush_;}
    QString streamText();

    class Pattern
    {
    public:
        QString name;

    protected:
        QString defBegin(int ptype, int w, int h);
        QString getDefinition(const QString& res);
        QMatrix matrix;
    };

    class FixedPattern : public Pattern
    {
    public:
        explicit FixedPattern(const QString& n = QString(), int idx = -1, const QColor &col = QColor(),
                              const QMatrix& mat = QMatrix());
        QColor rgba;
        bool isEmpty() const {return patternidx == 13;}
        bool isSolid() const {return patternidx == 14;}
        bool isTruePattern() const {return patternidx < 13;}

        QString getDefinition();
    private:
        int patternidx;
        static const char* fixedPattern[];
    };

    class PixmapPattern : public Pattern
    {
    public:
        explicit PixmapPattern(const QString& n = QString(), QPdfImage* im = 0
                               , const QMatrix& mat = QMatrix());
        QPdfImage* image;
        QString getDefinition(int objno);
    };

    class GradientPattern : public Pattern
    {
    public:
        explicit GradientPattern(const QString& n = QString(), QPdfGradient* grad = 0
                                 , const QMatrix& mat = QMatrix());
        QPdfGradient* shader;
        void setMainObj(int obj);
        int getMainObj() const {return mainobj_;}
        QString getDefinition();
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

    static QMap<Qt::BrushStyle,int> map_;
    static void static_init_map();
    QVector<SUBTYPE> streamstate_;
    QVector<qreal> alpha_;
    bool nobrush_;
    QString id_;
};

class QPdfPen : public QPdfObject
{
public:
    QPdfPen();
    QString streamText();

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
    QVector<QString> ri_;
    QVector<QColor> col_;
    QVector<bool> stroking_;
};

class QPdfPath : public QPdfObject
{
public:
    enum PAINTTYPE{
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
    QString streamText();
    int painttype;
    QMatrix currentMatrix;
    bool hasTrueAlpha() const {return hasTrueStrokeAlpha() || hasTrueNonStrokeAlpha();}
    int alphaObject() const {return alphaobj_;}
    QString alphaName() const {return alphaname_;};
    void setAlpha(const QString& prefix, int objno) {alphaname_ = prefix + QString::number(objno); alphaobj_ = objno;}
    QString getAlphaDefinition() const;

private:
    QString paintOperator() const;
    qreal ca_, CA_;
    QString alphaname_;
    int alphaobj_;
    bool gradientstrokealpha_;

    bool hasTrueStrokeAlpha() const {return transparencySupported() && CA_ >= 0.0 && CA_ < 1.0 ;}
    bool hasTrueNonStrokeAlpha() const {return transparencySupported() && ca_ >= 0.0 && ca_ < 1.0 ;}
    bool hasGradientNonStrokeAlpha() const {return transparencySupported() && gradientstrokealpha_;}
    QString streamCoreText() const;
};

class QPdfPage : public QPdfObject
{
public:
    QPdfPage();
    void  destroy();
    QString streamText();
    QPdfObject* append(QPdfObject* val, bool protect = false);

    QVector<QPdfImage*> images;
    QVector<QPdfPath*> paths;

private:
    ~QPdfPage(){}
    QVector<QPdfObject*> gobjects_;
    int width_, height_;
    bool landscape_;

    bool predType(int i, QPdfObject::TYPE);
    bool nextType(int i, QPdfObject::TYPE);
};

class QPdfEnginePrivate
{
public:
    enum OPTION{
        NONE = ~1UL,
        COMPRESSED = 1UL << 0
    };

    QPdfEnginePrivate();
    ~QPdfEnginePrivate();

    QPdfPage* curPage;
    void newPage();
    void setOption(uint op)
    {
        options_ |= op;
    }
    void unsetOption(uint op)
    {
        options_ &= ~op;
    }
    bool isOption(OPTION op) const
    {
        return (options_ & op) ? true : false;
    }
    void setDimensions(int w, int h){width_ = w; height_ = h;}
    void setCompression(bool val);
    bool isCompressed() const;
    void setPageOrder(bool fpl);
    void setLandscape(bool val){landscape_ = val;}
    bool isLandscape() const {return landscape_;}
    bool firstPageFirst() const {return firstpagefirst_;}


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

private:
    Q_DISABLE_COPY(QPdfEnginePrivate);

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
    bool firstpagefirst_;
};


class QPdfEngine : public QPaintEngine, public QPrintEngine
{
public:
    QPdfEngine();
    virtual ~QPdfEngine();

    // reimplementations QPaintEngine
    bool begin(QPaintDevice * pdev);
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
    void updateBackground (Qt::BGMode bgmode, const QBrush & brush);
    void updateBrush (const QBrush & brush, const QPointF & origin);
    void updateClipPath(const QPainterPath & path, Qt::ClipOperation op);
    void updateClipRegion (const QRegion & region, Qt::ClipOperation op);
    void updateFont (const QFont & f);
    void updatePen (const QPen & pen);
    void updateMatrix (const QMatrix & matrix);
    Type type() const;
    // end reimplementations QPaintEngine

    // reimplementations QPrintEngine
    void setOutputFileName(const QString &);
    QString outputFileName() const;
    void setDocName(const QString &);
    QString docName() const;
    void setCreator(const QString &);
    QString creator() const;
    void setOrientation(QPrinter::Orientation orientation);
    QPrinter::Orientation orientation() const;
    void setPageSize(QPrinter::PageSize ps);
    QPrinter::PageSize pageSize() const;
    void setPageOrder(QPrinter::PageOrder);
    QPrinter::PageOrder pageOrder() const;

    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;

    QRect paperRect() const;

    bool abort() {return false;}
    int metric(QPaintDevice::PaintDeviceMetric) const;
    QPrinter::PrinterState printerState() const {return QPrinter::Idle;}

    void setAuthor(const QString &author);
    QString author() const;

    void setDevice(QIODevice* dev);
    bool newPage();

    void setCompression(bool val);
    void setTransparency(bool val);

private:
    Q_DISABLE_COPY(QPdfEngine)

    void setBrush (QPdfBrush& pbr, const QBrush & brush, const QPointF& origin);
    void adaptMonochromePixmap(QPixmap& pm);

    void drawPathPrivate (const QPainterPath & path);

    QPrinter::PageSize pagesize_;
    QPdfEnginePrivate* pdf_;
    bool clipping_, tofile_, transpbgbrush_;
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

