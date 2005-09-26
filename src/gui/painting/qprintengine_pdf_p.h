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

struct RGBA
{
    RGBA()
        : r(0), g(0), b(0), a(1)
    {}
    RGBA(double rr, double gg, double bb, double aa = 1.0)
        : r(rr), g(gg), b(bb), a(aa)
    {}
    double r,g,b,a;

    bool operator!=(RGBA const& rgba) const
    {
        return r!=rgba.r || g!=rgba.g || b!=rgba.b || a!=rgba.a;
    }
    bool operator==(RGBA const& rgba) const
    {
        return !operator!=(rgba);
    }
    bool isTransparent() const
    {
        return a<1;
    }
};

class PdfStream
{
public:
    PdfStream();
    void setCompression(bool val);
    bool isCompressed() const;
    void setStream(QDataStream& val);
    uint write(const char* val, uint len);

private:
    bool compressed_;
    QDataStream* stream_;
};

class PdfObject
{
public:
    PdfObject() : type(NONE), appended(0), hassoftmask_p(false) {}
    virtual ~PdfObject() {}

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
    static QString pdffloat(double val) {return QString::number(val, 'f', 6);}
    bool hassoftmask_p;

private:
    static bool transparency_;
    static bool compression_;
};

class PdfMatrix : public PdfObject
{
public:
    PdfMatrix();
    PdfMatrix* setMatrix(QMatrix const& m);
    QMatrix currentMatrix() const;
    QMatrix lastMatrix() const;
    QString streamText();

    static QString streamMatrix(QMatrix const m);

private:
    QVector<QMatrix> matrices_;
};

class PdfImage : public PdfObject
{
public:
    PdfImage();
    explicit PdfImage(const QImage& pm);
    PdfImage(const QImage& pm, const QImage& mask);
    ~PdfImage();

    QString streamText();
    QString getDefinition();
    const char* data() const
    {
        return rawdata_;
    }
    PdfImage* stencil;
    PdfImage* softmask;
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

class PdfGradient : public PdfObject
{
public:
    PdfGradient();
    ~PdfGradient();
    void setParameter(RGBA b, RGBA e, float x0, float y0, float x1, float y1);


    void setObjects(int mainobj, int funcobj);
    QString getMainDefinition();
    QString getFuncDefinition();
    QString getSoftMaskFormDefinition();
    QString getSoftMaskMainDefinition();
    QString getSoftMaskFuncDefinition();

    void setColorSpaceObject(int obj);
    void setSoftMaskColorSpaceObject(int obj);
    void setSoftMaskObjects(int formobj, int mainobj, int funcobj);
    void setSoftMaskRange(float x, float y, float w, float h);

    int mainObject() const {return mainobj_;}
    int functionObject() const {return funcobj_;}
    int softMaskFormObject() const {return smfmobj_;}
    int softMaskMainObject() const {return (softmask) ? softmask->mainObject() : -1;}
    int softMaskFunctionObject() const {return (softmask) ? softmask->functionObject() : -1;}
    QString softMaskGraphicStateName() const {return (softmask) ? name + "ExtGS" : "";}

    QString name;

private:
    bool issoftmask_;
    int mainobj_, funcobj_, smfmobj_, csrgbobj_, csgrayobj_;
    float x0_, y0_, x1_, y1_;
    RGBA beg_, end_;
    PdfGradient* softmask;
    int x_, y_, w_, h_;

    QString getSingleMainDefinition();
    QString getSingleFuncDefinition();
    QString softMaskName() const {return (softmask) ? name + "SM" : "";}
};

class PdfBrush : public PdfObject
{
public:
    explicit PdfBrush(const QString& id = QString());
    ~PdfBrush();

    float alpha() const;

    PdfBrush* setFixed(Qt::BrushStyle style, RGBA rgba, const QMatrix& mat = QMatrix());
    PdfBrush* setGradient(RGBA rgba, RGBA gradrgba, float bx, float by, float ex, float ey,
                          float bbox_xb, float bbox_xe, float bbox_yb, float bbox_ye, const QMatrix& mat = QMatrix());
    PdfBrush* setPixmap(const QPixmap& pm, const QMatrix& mat);

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
        explicit FixedPattern(const QString& n = QString(), int idx = -1, RGBA col = RGBA()
                              , const QMatrix& mat = QMatrix());
        RGBA rgba;
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
        explicit PixmapPattern(const QString& n = QString(), PdfImage* im = 0
                               , const QMatrix& mat = QMatrix());
        PdfImage* image;
        QString getDefinition(int objno);
    };

    class GradientPattern : public Pattern
    {
    public:
        explicit GradientPattern(const QString& n = QString(), PdfGradient* grad = 0
                                 , const QMatrix& mat = QMatrix());
        PdfGradient* shader;
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
    bool frontIsGradient() const;

private:
    enum SUBTYPE{
        FIXED,
        GRADIENT,
        PIXMAP,
    };

    static QMap<Qt::BrushStyle,int> map_;
    static void static_init_map();
    QVector<SUBTYPE> streamstate_;
    QVector<float> alpha_;
    bool nobrush_;
    QString id_;
};

class PdfPen : public PdfObject
{
public:
    PdfPen();
    QString streamText();

    PdfPen* setLineWidth(double v);
    PdfPen* setLineCap(unsigned v);
    PdfPen* setLineJoin(unsigned v);
    PdfPen* setMiterLimit(double v);
    PdfPen* setDashArray(const QPen& pen, double phase);
    PdfPen* setColor(RGBA rgba);
    bool stroking() const;
    float alpha() const;

private:
    struct DashArray
    {
        DashArray(){}
        DashArray(const QVector<float>& v, float p)
            :seq(v), phase(p) {}

        QVector<float> seq;
        float phase;
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
    QVector<RGBA> col_;
    QVector<bool> stroking_;
};

class PdfPath : public PdfObject
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
        void setLine(float x, float y)
        {
            line.x=x;
            line.y=y,
                type = LINE;
        }
        void setCurve(float x1, float y1,float x2, float y2,float xnew, float ynew)
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
            struct { float x, y; } line;
            struct { float x1, y1, x2, y2, xnew, ynew; } curve;
        };
    };

    struct SubPath
    {
        enum SUBTYPE{
            RECT,    // rectangles are complete subpaths for PDF
            NORECT
        };
        SubPath() : type(NORECT), closed(false), initialized(false)
        {}

        QVector<Element> elements;
        struct { float x, y, w, h; } rect;
        struct { float x,y; } start;

        SUBTYPE type;

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
        void setRect(float x, float y, float w, float h)
        {
            elements.clear();
            type = RECT;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;
        }
    };

    explicit PdfPath(const PdfPen* = 0, const PdfBrush* = 0, int brushflags = NONE,  bool closed = false);

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
    float ca_, CA_;
    QString alphaname_;
    int alphaobj_;
    bool gradientstrokealpha_;

    bool hasTrueStrokeAlpha() const {return transparencySupported() && CA_ >= 0.0 && CA_ < 1.0 ;}
    bool hasTrueNonStrokeAlpha() const {return transparencySupported() && ca_ >= 0.0 && ca_ < 1.0 ;}
    bool hasGradientNonStrokeAlpha() const {return transparencySupported() && gradientstrokealpha_;}
    QString streamCoreText();
};

class PdfPage : public PdfObject
{
public:
    PdfPage();
    void  destroy();
    QString streamText();
    PdfObject* append(PdfObject* val, bool protect = false);

    QVector<PdfImage*> images;
    QVector<PdfPath*> paths;

private:
    ~PdfPage(){}
    QVector<PdfObject*> gobjects_;
    int width_, height_;
    bool landscape_;

    bool predType(int i, PdfObject::TYPE);
    bool nextType(int i, PdfObject::TYPE);
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

    PdfPage* curPage;
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


    PdfMatrix* curMatrix;
    PdfPen* curPen;
    PdfBrush* curBrush;
    QString title, creator, author;

    void setDevice(QIODevice*);
    void unsetDevice();
    int width() const {return width_;}
    int height() const {return height_;}

    void writeHeader();
    void writeTail();

private:
    // Disabled
    QPdfEnginePrivate(const QPdfEnginePrivate &);
    QPdfEnginePrivate &operator=(const QPdfEnginePrivate &);

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
    void drawPoint(const QPointF & pf);
    void drawPoints(const QPointF *points, int pointCount);
    void drawLine(const QLineF & line);
    void drawLines(const QLineF *lines, int lineCount);
    void drawRect(const QRectF &rf);
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
    // Disabled
    QPdfEngine(const QPdfEngine &);
    QPdfEngine &operator=(const QPdfEngine &);

    void setBrush (PdfBrush& pbr, const QBrush & brush, const QPointF& origin);
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

