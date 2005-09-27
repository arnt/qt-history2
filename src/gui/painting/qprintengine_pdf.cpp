#include <math.h>
#include <time.h>

#include <qiodevice.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpainterpath.h>
#include <qpaintdevice.h>
#include <qfile.h>
#include <qtextstream.h>

#include "qprintengine_pdf_p.h"

#ifndef QT_NO_COMPRESS
#include <zlib.h>
#endif

// might be helpful for smooth transforms of images
bool QPdfImage::interpolation_ = false;

#ifdef QT_NO_COMPRESS
static const bool do_compress = false;
#else
static const bool do_compress = true;
#endif

static inline QString pdfqreal(double val) { return QString::number(val, 'f', 6); }

#undef MM
#define MM(n) int((n * 720 + 127) / 254)

#undef IN
#define IN(n) int(n * 72)

struct PaperSize {
    int width, height;
};

static const PaperSize paperSizes[QPrinter::NPageSize] = {
    {  MM(210), MM(297) },      // A4
    {  MM(176), MM(250) },      // B5
    {  IN(8.5), IN(11) },       // Letter
    {  IN(8.5), IN(14) },       // Legal
    {  IN(7.5), IN(10) },       // Executive
    {  MM(841), MM(1189) },     // A0
    {  MM(594), MM(841) },      // A1
    {  MM(420), MM(594) },      // A2
    {  MM(297), MM(420) },      // A3
    {  MM(148), MM(210) },      // A5
    {  MM(105), MM(148) },      // A6
    {  MM(74), MM(105)},        // A7
    {  MM(52), MM(74) },        // A8
    {  MM(37), MM(52) },        // A9
    {  MM(1000), MM(1414) },    // B0
    {  MM(707), MM(1000) },     // B1
    {  MM(31), MM(44) },        // B10
    {  MM(500), MM(707) },      // B2
    {  MM(353), MM(500) },      // B3
    {  MM(250), MM(353) },      // B4
    {  MM(125), MM(176) },      // B6
    {  MM(88), MM(125) },       // B7
    {  MM(62), MM(88) },        // B8
    {  MM(44), MM(62) },        // B9
    {  MM(162),    MM(229) },   // C5E
    {  IN(4.125),  IN(9.5) },   // Comm10E
    {  MM(110),    MM(220) },   // DLE
    {  IN(8.5),    IN(13) },    // Folio
    {  IN(17),     IN(11) },    // Ledger
    {  IN(11),     IN(17) }     // Tabloid
};

inline QPaintEngine::PaintEngineFeatures qt_pdf_decide_features()
{
    QPaintEngine::PaintEngineFeatures f = QPaintEngine::AllFeatures;
    f &= ~(QPaintEngine::LinearGradientFill
           | QPaintEngine::RadialGradientFill
           | QPaintEngine::ConicalGradientFill);
    return f;
}

QPdfEngine::QPdfEngine()
    : QPaintEngine(qt_pdf_decide_features()), outFile_(new QFile)
{
    device_ = 0;
    clipping_ = false;
    tofile_ = false;
    backgroundMode = Qt::TransparentMode;
    pixmapnumber_ = 0;

    d = new QPdfEnginePrivate;
    lastBrush_ = new QBrush;
    lastPen_ = new QPen;
    lastMatrix_ = new QMatrix;
    bgBrush_ = new QBrush;
    lastBrushOrig_ = new QPointF;
    lastClipRegion_ = new QRegion;

    pagesize_ = QPrinter::A4;
}

QPdfEngine::~QPdfEngine()
{
    delete d;

    delete lastBrush_;
    delete lastPen_;
    delete lastMatrix_;
    delete bgBrush_;
    delete lastBrushOrig_;
    delete lastClipRegion_;
    delete outFile_;
}


void QPdfEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    switch (key) {
    case PPK_Creator:
        d->creator = value.toString();
        break;
    case PPK_DocumentName:
        d->title = value.toString();
        break;
    case PPK_Orientation:
        d->orientation = QPrinter::Orientation(value.toInt());
    case PPK_OutputFileName: {
        if (isActive()) {
            qWarning("QPdfEngine::setFileName: Not possible while painting");
            return;
        }
        QString filename = value.toString();

        if (filename.isEmpty())
            return;

        outFile_->setFileName(filename);
        setDevice(outFile_);
    }
        break;
    case PPK_PageSize: {
        pagesize_ = QPrinter::PageSize(value.toInt());
        QRect r = paperRect();

        d->setDimensions(r.width(),r.height());
    }
        break;
    default:
        break;
    }
}

QVariant QPdfEngine::property(PrintEnginePropertyKey key) const
{
    switch (key) {
    case PPK_ColorMode:
        return QPrinter::Color;
    case PPK_Creator:
        return d->creator;
    case PPK_DocumentName:
        return d->title;
    case PPK_FullPage:
        return true;
    case PPK_NumberOfCopies:
        return 1;
    case PPK_Orientation:
        return d->orientation;
    case PPK_OutputFileName:
        return outFile_->fileName();
    case PPK_PageRect:
        return paperRect();
    case PPK_PageSize:
        return pagesize_;
    case PPK_PaperRect:
        return paperRect();
    case PPK_PaperSource:
        return QPrinter::Auto;
    case PPK_Resolution:
        return 600;
    case PPK_SupportedResolutions:
        return QList<QVariant>() << 72;
    default:
        break;
    }
    return QVariant();
}

void QPdfEngine::setAuthor(const QString &author)
{
    d->author = author;
}

QString QPdfEngine::author() const
{
    return d->author;
}

QRect QPdfEngine::paperRect() const
{
    PaperSize s = paperSizes[pagesize_];
    int w = qRound(s.width);
    int h = qRound(s.height);
    if (d->orientation == QPrinter::Portrait)
        return QRect(0, 0, w, h);
    else
        return QRect(0, 0, h, w);
}

void QPdfEngine::setDevice(QIODevice* dev)
{
    if (isActive()) {
        qWarning("QPdfEngine::setDevice: Device cannot be set while painting");
        return;
    }
    device_ = dev;
}

bool QPdfEngine::begin (QPaintDevice *)
{
    if (!device_) {
        qWarning("QPdfEngine::begin: No valid device");
        return false;
    }

    if (device_->isOpen())
        device_->close();
    if(!device_->open(QIODevice::WriteOnly)) {
        qWarning("QPdfEngine::begin: Cannot open IO device");
        return false;
    }

    d->unsetDevice();
    d->setDevice(device_);
    setActive(true);
    d->writeHeader();
    d->newPage();
    Q_ASSERT(painter());
    if (painter()) {
        QRect r = paperRect();
        painter()->setViewport(r.x(),r.y(),r.width(),r.height());
        painter()->setWindow(r.x(),r.y(),r.width(),r.height());
        painter()->resetMatrix();
    }

    return true;
}

bool QPdfEngine::end ()
{
    d->writeTail();

    device_->close();
    d->unsetDevice();
    setActive(false);
    return true;
}

void QPdfEngine::drawPoints (const QPointF *points, int pointCount)
{
    if (!points)
        return;

    QPainterPath p;
    for (int i=0; i!=pointCount;++i) {
        p.moveTo(points[i]);
        p.lineTo(points[i]);
    }
    drawPath(p);
}

void QPdfEngine::drawLines (const QLineF *lines, int lineCount)
{
    if (!lines)
        return;

    QPainterPath p;
    for (int i=0; i!=lineCount;++i) {
        p.moveTo(lines[i].p1());
        p.lineTo(lines[i].p2());
    }
    drawPath(p);
}

void QPdfEngine::drawRects (const QRectF *rects, int rectCount)
{
    if (!rects)
        return;

    QPainterPath p;
    for (int i=0; i!=rectCount; ++i) {
        p.addRect(rects[i]);
    }
    drawPath(p);
}

void QPdfEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    if (!points || !pointCount)
        return;

    QPainterPath p;
    QBrush br = *lastBrush_;

    switch(mode) {
    case OddEvenMode:
    case ConvexMode:
        p.setFillRule(Qt::OddEvenFill);
        break;
    case WindingMode:
        p.setFillRule(Qt::WindingFill);
        break;
    case PolylineMode:
        updateBrush(QBrush(Qt::NoBrush),*lastBrushOrig_);
        break;
    default:
        break;
    }

    QPolygonF pg;

    for (int i = 0; i != pointCount; ++i)
        pg.append(points[i]);

    p.addPolygon(pg);
    if (mode != PolylineMode)
        p.closeSubpath();
    drawPath(p);
    if (mode == PolylineMode)
        updateBrush(br,*lastBrushOrig_);
}

void QPdfEngine::drawEllipse (const QRectF & rectangle)
{
    QPainterPath p;
    p.addEllipse(rectangle);
    drawPath(p);
}

void QPdfEngine::drawPath (const QPainterPath &p)
{
    if (backgroundMode == Qt::OpaqueMode && lastBrush_->style() != Qt::NoBrush) {
        QBrush tmp = *lastBrush_;
        updateBrush(*bgBrush_,*lastBrushOrig_); //todo origin for bgBrush ?
        drawPathPrivate(p);
        updateBrush(tmp, *lastBrushOrig_);
    }
    drawPathPrivate(p);
}

void QPdfEngine::drawPathPrivate (const QPainterPath &p)
{
    if (p.isEmpty() && !clipping_)
        return;

    QPdfPath* path = new QPdfPath(d->curPen, d->curBrush,
                                  (p.fillRule() == Qt::WindingFill)
                                  ? QPdfPath::FILLNONZERO : QPdfPath::FILLEVENODD);

    if (clipping_)
        path->painttype |= QPdfPath::CLIPPING;

    QPdfPath::SubPath sb;

    QPointF subPathStart;
    QPainterPath::Element lastelm;

    bool dangling = true;
    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &elm  = p.elementAt(i);
        QPdfPath::Element el;

        dangling = false;
        switch (elm.type) {
        case QPainterPath::CurveToDataElement:
            Q_ASSERT(false);
            break;
        case QPainterPath::MoveToElement:
            dangling = true;
            if (i>0) {
                if (subPathStart == QPointF(lastelm.x,lastelm.y))
                    sb.close();
                path->subpaths.append(sb);
                sb = QPdfPath::SubPath();
            }
            sb.start.x = elm.x;
            sb.start.y = elm.y;
            subPathStart = QPointF(elm.x, elm.y);
            sb.initialized = true;
            break;
        case QPainterPath::LineToElement:
            el.setLine(elm.x, elm.y);
            sb.elements.append(el);
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(p.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(p.elementAt(i+2).type == QPainterPath::CurveToDataElement);
            el.setCurve(elm.x, elm.y,
                        p.elementAt(i+1).x, p.elementAt(i+1).y,
                        p.elementAt(i+2).x, p.elementAt(i+2).y);
            sb.elements.append(el);
            i += 2; // Skip the next two
        }
        lastelm  = elm;
    }
    if (subPathStart == QPointF(lastelm.x,lastelm.y))
        sb.close();

    if (!dangling)
        path->subpaths.append(sb);

    d->curPage->append(path);
}

void QPdfEngine::drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr)
{
    if (sr.isEmpty() || rectangle.isEmpty() || pixmap.isNull())
        return;

    QImage mask;
    QPixmap pm = pixmap;

    adaptMonochromePixmap(pm);
    //     const QBitmap* bm = pm.mask();
    //     if (bm)
    //         mask = bm->toImage();

    QImage im = pm.toImage();
    im = im.copy(sr.toRect());
    //     mask = mask.copy(sr.toRect());

    QMatrix mat = d->curMatrix->lastMatrix();

    updateMatrix(QMatrix(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                         rectangle.x(), rectangle.y()) * mat);
    QPdfImage* img = new QPdfImage(im, mask);
    img->name = QString("/Im%1").arg(pixmapnumber_++);
    d->curPage->append(img);
    updateMatrix(mat);
}

void QPdfEngine::drawTiledPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QPointF & point)
{
    QBrush b = *lastBrush_;
    QPointF pt = *lastBrushOrig_;
    QPen pen = *lastPen_;
    updateBrush(QBrush(pixmap),-point);
    updatePen(QPen(Qt::NoPen));
    drawRects(&rectangle, 1);
    updatePen(pen);
    updateBrush(b,pt);
}

void QPdfEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();
    if (flags & DirtyTransform)
        updateMatrix(state.matrix());
    if (flags & DirtyPen)
        updatePen(state.pen());
    if (flags & DirtyBrush)
        updateBrush(state.brush(), state.brushOrigin());
    if ((flags & DirtyBackground) || (flags & DirtyBackgroundMode))
        updateBackground(state.backgroundMode(), state.backgroundBrush());
#if 0
    if (flags & DirtyFont)
        updateFont(state.font());
#endif

    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled())
            updateClipPath(painter()->clipPath(), Qt::ReplaceClip);
        else
            updateClipPath(QPainterPath(), Qt::NoClip);
    }

    if (flags & DirtyClipPath)
        updateClipPath(state.clipPath(), state.clipOperation());
    if (flags & DirtyClipRegion)
        updateClipRegion(state.clipRegion(), state.clipOperation());
    //     if (flags & DirtyHints) updateRenderHints(state.renderHints());
}

void QPdfEngine::updateClipRegion (const QRegion & region, Qt::ClipOperation op)
{
    QPainterPath p;
    QRegion r = *lastClipRegion_;
    QMatrix mat = d->curMatrix->lastMatrix();
    switch(op) {
    case Qt::NoClip:
        r = QRegion(paperRect());
        mat = QMatrix();
        break;
    case Qt::ReplaceClip:
  	r = region;
        break;
    case Qt::UniteClip:
        r = r.unite(region);
        break;
    case Qt::IntersectClip:
        r = r.intersect(region);
        break;
    default:
        return;
    }
    *lastClipRegion_ = r;

    //todo p.addRegion(r);
    QVector<QRect> rects = r.rects();
    for (int i=0; i<rects.size(); ++i) p.addRect(rects.at(i));

    QRegion reg(p.toFillPolygon().toPolygon(), p.fillRule());
    p = QPainterPath();
    //todo p.addRegion(reg);
    rects = reg.rects();
    for (int i=0; i<rects.size(); ++i) p.addRect(rects.at(i));

    p = mat.map(p);

    if (p.isEmpty())
        p.addRect(-1,-1,1,1);

    clipping_ = true;
    drawPath(p);
    clipping_ = false;
}


void QPdfEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
    QPainterPath p = path;
    if (p.isEmpty()) // dummy path
        p.addRect(0,0,0,0);
    QRegion reg(p.toFillPolygon().toPolygon(), p.fillRule());
    updateClipRegion(reg,op);
}


/*
  void QPdfEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
  {
  QMatrix mat = d->curMatrix->lastMatrix();
  QPainterPath p = path;
  if (p.isEmpty()) // dummy path
  p.addRect(0,0,0,0);

  clipping_ = true;

  switch(op) {
  case Qt::NoClip:
  updateMatrix(QMatrix());
  p.setFillRule(Qt::OddEvenFill);
  p = QPainterPath();
  p.addRect(paperRect());
  drawPath(p);
  updateMatrix(mat);
  break;
  case Qt::ReplaceClip:
  drawPath(mat.map(p));
  break;
  case Qt::UniteClip:
  p.addPath(*lastClipPath_);
  drawPath(mat.map(p));
  break;
  case Qt::IntersectClip:
  clipping_ = false;
  return;
  default:
  break;
  }
  clipping_ = false;
  *lastClipPath_ = p;
  }
*/

void QPdfEngine::updateBrush (const QBrush & brush, const QPointF & origin)
{
    *lastBrushOrig_ = origin;
    setBrush(*d->curBrush,brush,*lastBrushOrig_);
    d->curPage->append(d->curBrush);
    *lastBrush_ = brush;
}

void QPdfEngine::updateBackground (Qt::BGMode bgmode, const QBrush & brush)
{
    backgroundMode = bgmode;
    *bgBrush_ = brush;
}

void QPdfEngine::updatePen (const QPen & pen)
{
    QPen tpen = pen;
    tpen.setWidth((pen.width()) ? pen.width() : 0);


    d->curPage->append(d->curPen->setColor(tpen.color()));
    d->curPage->append(d->curPen->setLineWidth(tpen.width()));

    switch(tpen.capStyle()) {
    case Qt::FlatCap:
        d->curPage->append(d->curPen->setLineCap(0));
        break;
    case Qt::SquareCap:
        d->curPage->append(d->curPen->setLineCap(2));
        break;
    case Qt::RoundCap:
        d->curPage->append(d->curPen->setLineCap(1));
        break;
    default:
        break;
    }

    switch(tpen.joinStyle()) {
    case Qt::MiterJoin:
        d->curPage->append(d->curPen->setLineJoin(0));
        break;
    case Qt::BevelJoin:
        d->curPage->append(d->curPen->setLineJoin(2));
        break;
    case Qt::RoundJoin:
        d->curPage->append(d->curPen->setLineJoin(1));
        break;
    default:
        break;
    }

    d->curPage->append(d->curPen->setDashArray(tpen,0));

    *lastPen_ = tpen;
}

void QPdfEngine::updateMatrix(const QMatrix & matrix)
{
    d->curMatrix->setMatrix(matrix);
    d->curPage->append(d->curMatrix);
    *lastMatrix_ = matrix;

    if (lastBrush_)
        //if (lastBrush_->style() != Qt::NoBrush && lastBrush_->style() != Qt::SolidPattern)
        updateBrush(*lastBrush_, QPointF(0,0));
    //	if (lastPen_)
    //		if (lastPen_->style() != Qt::NoPen)
    //			updatePen(*lastPen_);
}

int QPdfEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const
{
    int val;
    QRect r = paperRect();
    switch (metricType) {
    case QPaintDevice::PdmWidth:
        val = r.width();
        break;
    case QPaintDevice::PdmHeight:
        val = r.height();
        break;
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmDpiY:
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
        val = 72;
        break;
    case QPaintDevice::PdmWidthMM:
        val = qRound(r.width()*25.4/72.);
        break;
    case QPaintDevice::PdmHeightMM:
        val = qRound(r.height()*25.4/72.);
        break;
    case QPaintDevice::PdmNumColors:
        val = INT_MAX;
        break;
    case QPaintDevice::PdmDepth:
        val = 32;
        break;
    default:
        qWarning("QPdfEngine::metric: Invalid metric command");
        return 0;
    }
    return val;
}

QPaintEngine::Type QPdfEngine::type() const
{
    return QPaintEngine::User;
}

bool QPdfEngine::newPage()
{
    if (isActive()) {
        d->newPage();
        updateMatrix(*lastMatrix_);
        updateBrush(*lastBrush_, *lastBrushOrig_);
        updatePen(*lastPen_);
        return true;
    }
    return false;
}

void QPdfEngine::setBrush (QPdfBrush& pbr, const QBrush & brush, const QPointF & origin)
{
    QRect w = painter()->window();
    QRect vp = paperRect();

    QMatrix tmp(1.0, 0.0, 0.0, -1.0, 0.0, vp.height());

    tmp = d->curMatrix->lastMatrix() * tmp;
    tmp.translate(origin.x(),origin.y());

    switch(brush.style()) {
    case Qt::LinearGradientPattern: {
        Q_ASSERT(brush.gradient() && brush.gradient()->type() == QGradient::LinearGradient);

        const QLinearGradient *lg = static_cast<const QLinearGradient *>(brush.gradient());

        QPointF start = lg->start() + origin;
        QPointF stop = lg->finalStop() + origin;

        QColor c0 = lg->stops().first().second;
        QColor c1 = lg->stops().last().second;

        pbr.setGradient(c0, c1,
                        start.x(),start.y(), stop.x(),stop.y(),
                        w.left(), w.top(), w.width(), w.height(),
                        tmp);
        break;
    }
    case Qt::TexturePattern: {
        QPixmap pm = brush.texture();
        if (!pm.isNull()) {
            adaptMonochromePixmap(pm);
            pbr.setPixmap(pm, tmp);
        }
        break;
    }
    default:
        pbr.setFixed(brush.style(), brush.color(), tmp);
        break;
    }
}

void QPdfEngine::adaptMonochromePixmap(QPixmap& pm)
{
    if (pm.depth() == 1 && lastPen_) { // pm _is_ mask
        QBitmap bm0 = pm.mask();
        QBitmap bm = pm.createMaskFromColor(Qt::color0);

        if (!bm0.isNull()) { // combine masks
            QImage im0 = bm0.toImage();
            QImage im(im0.size(), QImage::Format_ARGB32_Premultiplied);
            for (int i=0; i!=im.width();++i)
                for (int j=0; j!=im.height();++j)
                    if(im0.pixelIndex(i,j) == Qt::color0)
                        im.setPixel(i,j,Qt::color0);
            bm.fromImage(im);
        }
        pm = QPixmap(pm.width(),pm.height());
        pm.fill(lastPen_->color());
        pm.setMask(bm);
    }
}


QPdfStream::QPdfStream()
{
    stream_ = 0;
}

void QPdfStream::setStream(QDataStream& val)
{
    stream_ = &val;
}

uint QPdfStream::write(const char* src, uint len)
{
    if (!stream_)
        return 0;

#ifndef QT_NO_COMPRESS
    if(do_compress) {
        uLongf destLen = (uLongf)ceil(1.001 * len + 12); // zlib requirement
        Bytef* dest = new Bytef[destLen];
        if (Z_OK == ::compress(dest, &destLen, (const Bytef*) src, (uLongf)len)) {
            stream_->writeRawData((const char*)dest,destLen);
        } else {
            qWarning("QPdfStream::write(): compress error");
            destLen = 0;
        }
        delete [] dest;
        return (uint)destLen;
    } else
#endif
    {
        stream_->writeRawData(src,len);
        return len;
    }
}

QPdfMatrix::QPdfMatrix()
    : QPdfObject()
{
    type = QPdfObject::MATRIX;
}

QString QPdfMatrix::streamMatrix(QMatrix const m)
{
    QString s;
    s += pdfqreal(m.m11());
    s += ' ';
    s += pdfqreal(m.m12());
    s += ' ';
    s += pdfqreal(m.m21());
    s += ' ';
    s += pdfqreal(m.m22());
    s += ' ';
    s += pdfqreal(m.dx());
    s += ' ';
    s += pdfqreal(m.dy());
    s += ' ';
    s += "cm\n";
    return s;
}

void QPdfMatrix::streamText(QTextStream &stream)
{
    if (matrices_.empty())
        return;

    QMatrix m = matrices_.first();
    matrices_.pop_front();

    stream << streamMatrix(m);
}

QPdfMatrix* QPdfMatrix::setMatrix(QMatrix const& m)
{
    matrices_.append(m);
    return this;
}

QMatrix QPdfMatrix::currentMatrix() const
{
    if (matrices_.empty())
        return QMatrix();
    return matrices_.first();
}

QMatrix QPdfMatrix::lastMatrix() const
{
    if (matrices_.empty())
        return QMatrix();
    return matrices_.last();
}

QPdfImage::QPdfImage()
    : QPdfObject()
{
    init();
}

QPdfImage::QPdfImage(const QImage& im)
    : QPdfObject()
{
    init();
    QImage dummy;
    rawlen_ = convert(im, dummy);
}

QPdfImage::QPdfImage(const QImage& im, const QImage& mask)
    : QPdfObject()
{
    init();
    rawlen_ = convert(im, mask);
}

void QPdfImage::init()
{
    type = QPdfObject::IMAGE;
    stencil = softmask = 0;
    rawdata_ = 0;
    isgray_ = hashardmask_ = ismask_ = false;
    softmaskobj_ = maskobj_ = lenobj_ = -1;
    rawlen_ = -1;
}

QPdfImage::~QPdfImage()
{
    delete stencil;
    delete softmask;
    delete [] rawdata_;
}

void QPdfImage::setMaskObj(int obj)
{
    maskobj_ = obj;
}

void QPdfImage::setSoftMaskObj(int obj)
{
    softmaskobj_ = obj;
}

void QPdfImage::setLenObj(int obj)
{
    lenobj_ = obj;
}

QString QPdfImage::getDefinition()
{
    QString s;
    s += "<<\n";
    s += "/Type /XObject\n";
    s += "/Subtype /Image\n";
    s += QString("/Width %1 \n").arg(w_);
    s += QString("/Height %1 \n").arg(h_);
    if (ismask_) {
        s += QString("/ImageMask true\n");
        s += "/Decode [1 0]\n";
    }
    else {
        s += QString("/BitsPerComponent %1\n").arg((ismask_) ? 1 : 8);
        s += "/ColorSpace ";
        s += (isgray_ || ismask_) ? "/DeviceGray\n" : "/DeviceRGB\n";
    }
    if (hashardmask_ && maskobj_>0)
        s += QString("/Mask %1 0 R\n").arg(maskobj_);
    if (hasSoftMask() && softmaskobj_>0)
        s += QString("/SMask %1 0 R\n").arg(softmaskobj_);
    if (lenobj_>0)
        s += QString("/Length %1 0 R\n").arg(lenobj_);
    if (interpolation_)
        s += "/Interpolate true\n";
    if (do_compress)
        s += "/Filter /FlateDecode\n";
    s += ">>\n";
    return s;
}


// \return length of raw data in byte
int QPdfImage::convert(const QImage& img, const QImage& mask)
{
    if (img.isNull())
        return -1;

    QImage im = img;

    w_ = im.width();
    h_ = im.height();


    int d = im.depth();

    ismask_ = (d==1); // is monochrome - deal with it as mask

    if (d == 8) {
        im = im.convertDepth(32);
        d = 32;
    }

    if (im.hasAlphaBuffer()) {
        delete softmask;
        softmask = new QPdfImage;
        softmask->w_ = im.width();
        softmask->h_ = im.height();
        softmask->rawlen_ = w_ * h_;
        softmask->isgray_ = true;
        softmask->rawdata_ = new char[softmask->rawlen_];
        hassoftmask_p = true;
    }
    else if (!mask.isNull()) {
        delete stencil;
        stencil = new QPdfImage(mask);
        stencil->ismask_ = true;
        hashardmask_ = true;
    }

    int rowlen = 0;
    switch(d) {
    case 1:
        rowlen = (im.width() % 8) ? im.width() / 8 + 1 : im.width() / 8;
        break;
    case 8:
        rowlen = im.width();
        break;
    case 32:
        rowlen = 3 * im.width();
        break;
    default:
        qWarning("QPdfImage::convert(): unsupported pixmap depth");
        return -1;
    }

    delete rawdata_;
    rawdata_ = new char[rowlen * im.height()];

    int i,j;
    switch(d) {
    case 1:
    case 8:
        for (i=0; i!=im.height(); ++i)
            memcpy(rawdata_+i*rowlen, im.scanLine(i), rowlen * sizeof(char));
        break;
    case 32: {
        int w = im.width();

        for (i=0; i!=im.height(); ++i)
            for (j=0; j!=w; ++j)
            {
                QRgb rgb = im.pixel(j,i);
                rawdata_[i*rowlen+3*j] = (char)qRed(rgb);
                rawdata_[i*rowlen+3*j+1] = (char)qGreen(rgb);
                rawdata_[i*rowlen+3*j+2] = (char)qBlue(rgb);
                if (hasSoftMask())
                {
                    softmask->rawdata_[i*w+j] = (char)qAlpha(rgb);
                    /*
                      if (!mask.isNull()) // integrate hard mask with soft mask
                      {
                      if (mask.bitOrder() == QImage::LittleEndian)
                      {
                      if (!(*(mask.scanLine(i) + (j >> 3)) & 1 << (j & 7)))
                      softmask->rawdata_[i*w+j] = 0;
                      }
                      else
                      {
                      if (!(*(mask.scanLine(i) + (j >> 3)) & 1 << (7 - (j & 7))))
                      softmask->rawdata_[i*w+j] = 0;
                      }
                      }
                    */
                }
            }
        break;
    }
    default:
        break;
    }

    return rowlen * im.height();
}

void QPdfImage::streamText(QTextStream &stream)
{
    stream << QString("q\n%1 0 0 %2 0 %3 cm\n").arg(w_).arg(-h_).arg(h_)
           << name
           <<" Do\nQ\n";
}



QPdfGradient::QPdfGradient()
    : QPdfObject(), softmask(0), issoftmask_(false), mainobj_(-1),
      funcobj_(-1), smfmobj_(-1), csrgbobj_(-1), csgrayobj_(-1), x0_(0), y0_(0), x1_(0), y1_(0),
      w_(0), h_(0)
{
    type = QPdfObject::GRADIENT;
}

QPdfGradient::~QPdfGradient()
{
    delete softmask;
}

void QPdfGradient::setParameter(const QColor &b, const QColor &e, qreal x0, qreal y0, qreal x1, qreal y1)
{
    beg_ = b;
    end_ = e;
    x0_ = x0;
    y0_ = y0;
    x1_ = x1;
    y1_ = y1;

    // maintain transparent parts
    if (b.alphaF() < 1.0 || e.alphaF() < 1.0) {
        hassoftmask_p = true;
        softmask = new QPdfGradient;
        softmask->x0_ = x0_;
        softmask->y0_ = y0_;
        softmask->x1_ = x1_;
        softmask->y1_ = y1_;
        softmask->beg_.setAlphaF(b.alphaF());
        softmask->end_.setAlphaF(e.alphaF());
        softmask->issoftmask_ = true;
    }
}

void QPdfGradient::setObjects(int mainobj, int funcobj)
{
    mainobj_ = mainobj;
    funcobj_ = funcobj;
}

void QPdfGradient::setSoftMaskObjects(int formobj, int mainobj, int funcobj)
{
    smfmobj_ = formobj;
    softmask->mainobj_ = mainobj;
    softmask->funcobj_ = funcobj;
}

void QPdfGradient::setColorSpaceObject(int obj)
{
    csrgbobj_ = obj;
}

void QPdfGradient::setSoftMaskColorSpaceObject(int obj)
{
    if (softmask)
        softmask->csgrayobj_ = obj;
}

QString QPdfGradient::getSingleMainDefinition()
{
    QString s;

    // main object
    s += "<<\n";
    s += "/ShadingType 2\n";
    s += "/ColorSpace ";
    s += (issoftmask_) ? QString("%1 0 R\n").arg(csgrayobj_) : QString("%1 0 R\n").arg(csrgbobj_);
    s += QString("/Coords [%1 %2 %3 %4]\n")
        .arg(x0_,0,'f',6)
        .arg(y0_,0,'f',6)
        .arg(x1_,0,'f',6)
        .arg(y1_,0,'f',6);
    s += QString("/Function %1 0 R\n").arg(funcobj_);
    s += "/Extend [true true]\n";
    s += ">>\n";
    s += "endobj\n";
    return s;
}

QString QPdfGradient::getSingleFuncDefinition()
{
    QString s;

    // function object
    s += "<<\n";
    s += "/FunctionType 2\n";
    s += "/Domain [0.0 1.0]\n";
    s += "/N 1\n";
    if (issoftmask_) {
        s += QString("/C0 [%1]")
            .arg(beg_.alphaF(),0,'f',6)
            +"\n";
        s += QString("/C1 [%1]")
            .arg(end_.alphaF(),0,'f',6)
            +"\n";
    }
    else {
        s += QString("/C0 [%1 %2 %3]")
            .arg(beg_.redF(),0,'f',6)
            .arg(beg_.greenF(),0,'f',6)
            .arg(beg_.blueF(),0,'f',6)
            +"\n";
        s += QString("/C1 [%1 %2 %3]")
            .arg(end_.redF(),0,'f',6)
            .arg(end_.greenF(),0,'f',6)
            .arg(end_.blueF(),0,'f',6)
            +"\n";
    }
    s += ">>\n";
    s += "endobj\n";
    return s;
}

QString QPdfGradient::getMainDefinition()
{
    return getSingleMainDefinition();
}

QString QPdfGradient::getFuncDefinition()
{
    return getSingleFuncDefinition();
}

QString QPdfGradient::getSoftMaskFormDefinition()
{
    QString s;

    if (!softmask)
        return s;

    s += "<<\n";
    s += "/Type /XObject\n";
    s += "/Subtype /Form\n";
    s += QString("/BBox [ %1 %2 %3 %4 ]\n").arg(x_).arg(y_).arg(w_).arg(h_);
    s += "/Group <</S /Transparency >>\n";
    s += "/Resources <<\n";
    s += "/Shading <<\n";
    s += softMaskName() + QString(" %1 0 R\n").arg(softmask->mainobj_) ;
    s += ">>\n";
    s += ">>\n";
    //	QString stream = "/PCSp cs\n";
    //	stream += softMaskName() + " scn\n";

    //	QString stream = "0.3 g\n20 30 10 20 re\n";

    QString stream = softMaskName() + " sh\n";
    s += "/Length " + QString::number(stream.length()) + "\n";
    s += ">>\n";
    s += "stream\n";
    s += stream;
    s +=	"endstream\n";
    s	+=  "endobj\n";

    return s;
}

QString QPdfGradient::getSoftMaskMainDefinition()
{
    return (softmask) ? softmask->getSingleMainDefinition() : "";
}

QString QPdfGradient::getSoftMaskFuncDefinition()
{
    return (softmask) ? softmask->getSingleFuncDefinition() : "";
}

void QPdfGradient::setSoftMaskRange(qreal x, qreal y, qreal w, qreal h)
{
    x_ = x;
    y_ = y;
    w_ = w;
    h_ = h;
}

static const char* pattern_for_brush[] = {
    0, // NoBrush
    0, // SolidPattern
    "0 J\n"
    "6 w\n"
    "[] 0 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "0 4 m\n"
    "8 4 l\n"
    "S\n", // Dense1Pattern

    "0 J\n"
    "2 w\n"
    "[6 2] 1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[] 0 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[6 2] -3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n", // Dense2Pattern

    "0 J\n"
    "2 w\n"
    "[6 2] 1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 2] -1 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[6 2] -3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n", // Dense3Pattern

    "0 J\n"
    "2 w\n"
    "[2 2] 1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 2] -1 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[2 2] 1 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n", // Dense4Pattern

    "0 J\n"
    "2 w\n"
    "[2 6] -1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 2] 1 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[2 6] 3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n", // Dense5Pattern

    "0 J\n"
    "2 w\n"
    "[2 6] -1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 6] 3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n", // Dense6Pattern

    "0 J\n"
    "2 w\n"
    "[2 6] -1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n", // Dense7Pattern

    "1 w\n"
    "0 4 m\n"
    "8 4 l\n"
    "S\n", // HorPattern

    "1 w\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n", // VerPattern

    "1 w\n"
    "4 0 m\n"
    "4 8 l\n"
    "0 4 m\n"
    "8 4 l\n"
    "S\n", // CrossPattern

    "1 w\n"
    "-1 5 m\n"
    "5 -1 l\n"
    "3 9 m\n"
    "9 3 l\n"
    "S\n", // BDiagPattern

    "1 w\n"
    "-1 3 m\n"
    "5 9 l\n"
    "3 -1 m\n"
    "9 5 l\n"
    "S\n", // FDiagPattern

    "1 w\n"
    "-1 3 m\n"
    "5 9 l\n"
    "3 -1 m\n"
    "9 5 l\n"
    "-1 5 m\n"
    "5 -1 l\n"
    "3 9 m\n"
    "9 3 l\n"
    "S\n", // DiagCrossPattern
};


QPdfBrush::QPdfBrush(const QString& id)
    : QPdfObject(), id_(id)
{
    type = QPdfObject::BRUSH;
    nobrush_ = true;
}

QPdfBrush::FixedPattern::FixedPattern(const QString& n, int idx, const QColor &col, const QMatrix& mat)
    : rgba(col), patternidx(idx)
{
    name = n;
    matrix = mat;
}

QPdfBrush::GradientPattern::GradientPattern(const QString& n, QPdfGradient* grad, const QMatrix& mat /* = QMatrix */)
    : shader(grad), mainobj_(-1)
{
    name = n;
    matrix = mat;
}

QPdfBrush::PixmapPattern::PixmapPattern(const QString& n, QPdfImage* im, const QMatrix& mat)
    :	image(im)
{
    name = n;
    matrix = mat;
}


QString QPdfBrush::Pattern::defBegin(int ptype, int w, int h)
{
    QString s;
    s += "<<\n";
    s += "/Type /Pattern\n";
    s += "/PatternType 1\n";
    s += QString("/PaintType %1\n").arg(ptype);
    s += "/TilingType 1\n";
    s += QString("/BBox [0 0 %1 %2]\n").arg(w).arg(h);
    s += QString("/XStep %1\n").arg(w);
    s += QString("/YStep %1\n").arg(h);
    //TODO bad tiling job of AR here with non-identity matrix
    s += QString("/Matrix [%1 %2 %3 %4 %5 %6]\n")
        .arg(matrix.m11(),0,'f',4)
        .arg(matrix.m12(),0,'f',4)
        .arg(matrix.m21(),0,'f',4)
        .arg(matrix.m22(),0,'f',4)
        .arg(matrix.dx(),0,'f',4)
        .arg(matrix.dy(),0,'f',4);
    s += "/Resources \n<<\n"; // open resource tree

    return s;
}

QString QPdfBrush::Pattern::getDefinition(const QString& res)
{
    QString s;
    s += ">>\n"; // close resource tree
    s += "/Length " + QString::number(res.length()) + "\n";
    s += ">>\n";
    s += "stream\n";
    s += res;
    s += "endstream\n";
    s += "endobj\n";
    return s;
}


QString QPdfBrush::FixedPattern::getDefinition()
{
    if (!isTruePattern())
        return "";

    QString s = defBegin(2, 8, 8);
    s += Pattern::getDefinition(pattern_for_brush[patternidx]);

    return s;
}



QString QPdfBrush::PixmapPattern::getDefinition(int objno)
{
    Q_ASSERT(image);
    QString s = defBegin(1, image->w(), image->h()) ;
    s += QString("/XObject <<%1 %2 0 R>>\n").arg(image->name).arg(objno);

    QMatrix m(image->w(),0,0,-image->h(),0,image->h());
    QString res = QString("%1 %2 %3 %4 %5 %6 cm\n")
                  .arg(m.m11(),0,'f',4)
                  .arg(m.m12(),0,'f',4)
                  .arg(m.m21(),0,'f',4)
                  .arg(m.m22(),0,'f',4)
                  .arg(m.dx(),0,'f',4)
                  .arg(m.dy(),0,'f',4);
    res += image->name + " Do\n";
    s += Pattern::getDefinition(res);

    return s;
}

void QPdfBrush::GradientPattern::setMainObj(int obj)
{
    mainobj_ = obj;
}

QString QPdfBrush::GradientPattern::getDefinition()
{
    Q_ASSERT(shader);

    QString s;
    s += "<<\n";
    s += "/Type /Pattern\n";
    s += "/PatternType 2\n";
    s += QString("/Matrix [%1 %2 %3 %4 %5 %6]\n")
         .arg(matrix.m11(),0,'f',6)
         .arg(matrix.m12(),0,'f',6)
         .arg(matrix.m21(),0,'f',6)
         .arg(matrix.m22(),0,'f',6)
         .arg(matrix.dx(),0,'f',6)
         .arg(matrix.dy(),0,'f',6);

    s += QString("/Shading %1 0 R\n").arg(shader->mainObject());
    s += ">>\n";
    s += "endobj\n";

    return s;
}

QPdfBrush::~QPdfBrush()
{
    int i;
    for (i=0; i!=pixmaps.size(); ++i)
        delete pixmaps[i].image;
    for (i=0; i!=gradients.size(); ++i)
        delete gradients[i].shader;
}

void QPdfBrush::streamText(QTextStream &s)
{
    if (streamstate_.empty())
        return;

    SUBTYPE sstate = streamstate_.first();
    streamstate_.pop_front();
    if (!alpha_.empty())
        alpha_.pop_front(); // clean-up only

    switch(sstate) {
    case FIXED: {
        if (fixeds.empty())
            break;
        FixedPattern p = fixeds.first();
        if (p.isSolid())
        {
            s << "/CSp cs\n"
              << pdfqreal(p.rgba.redF()) << ' '
              << pdfqreal(p.rgba.greenF()) << ' '
              << pdfqreal(p.rgba.blueF())
              << " scn\n";
        }
        else if (!p.isEmpty())
        {
            s << "/PCSp cs\n"
              << pdfqreal(p.rgba.redF()) << ' '
              << pdfqreal(p.rgba.greenF()) << ' '
              << pdfqreal(p.rgba.blueF()) << ' '
              << p.name
              << " scn\n";
        }
        fixeds.pop_front();
        break;
    }
    case GRADIENT: {
        if (gradients.empty())
            break;
        GradientPattern p = gradients.first();
        if (p.shader->hasSoftMask())
            s << p.shader->softMaskGraphicStateName() << " gs\n";
        s << "/PCSp cs\n"
          << p.name
          << " scn\n";
        gradients.pop_front();
        break;
    }
    case PIXMAP: {
        if (pixmaps.empty())
            break;
        PixmapPattern p = pixmaps.first();
        s << "/PCSp cs\n"
          << p.name
          << " scn\n";
        pixmaps.pop_front();
        break;
    }
    }
}

QPdfBrush* QPdfBrush::setFixed(Qt::BrushStyle style, const QColor &rgba, const QMatrix& mat)
{
    nobrush_ = (style == Qt::NoBrush);

    streamstate_.append(FIXED);
    QString s("/Pat");
    s += id_ + QString::number(fixeds.size());

    FixedPattern p(s, style, rgba, mat);

    fixeds.append(p);
    alpha_.append(p.isEmpty() ? -1.0 : rgba.alphaF());
    return this;
}

QPdfBrush* QPdfBrush::setGradient(const QColor &rgba, const QColor &gradrgba,
                                  qreal bx, qreal by, qreal ex, qreal ey,
                                  qreal bbox_xb, qreal bbox_xe, qreal bbox_yb, qreal bbox_ye, const QMatrix& mat)
{
    nobrush_ = false;
    streamstate_.append(GRADIENT);
    QString s("/PatLG");
    s += id_ + QString::number(gradients.size());

    QPdfGradient* grad = new QPdfGradient;
    grad->name = s;
    grad->setParameter(rgba, gradrgba, bx, by, ex, ey);
    grad->setSoftMaskRange(bbox_xb, bbox_xe, bbox_yb, bbox_ye);

    gradients.append(GradientPattern(s, grad, mat));
    alpha_.append(-1.0);
    return this;
}

QPdfBrush* QPdfBrush::setPixmap(const QPixmap& pm, const QMatrix& mat)
{
    nobrush_ = false;
    streamstate_.append(PIXMAP);
    QString s("/PatI");
    s += id_ + QString::number(pixmaps.size());

    QImage im = pm.toImage();
    QImage mask;
    QBitmap bmm = pm.mask();
    if (!bmm.isNull())
        mask = bmm.toImage();

    QPdfImage* img = new QPdfImage(im,mask);
    img->name = "/PImg" + id_ + QString::number(pixmaps.size());

    pixmaps.append(PixmapPattern(s,img,mat));
    alpha_.append(-1.0);
    return this;
}

qreal QPdfBrush::alpha() const
{
    if (alpha_.empty())
        return 1.0;
    return alpha_.last();
}

bool QPdfBrush::isGradient() const
{
    if (streamstate_.empty())
        return false;
    return (GRADIENT == streamstate_.last());
}

bool QPdfBrush::firstIsGradient() const
{
    if (streamstate_.empty())
        return false;
    return (GRADIENT == streamstate_.first());
}

QPdfPen::QPdfPen()
    : QPdfObject()
{
    type = QPdfObject::PEN;
}

void QPdfPen::streamText(QTextStream &s)
{
    if (streamstate_.empty())
        return;

    SUBTYPE sstate = streamstate_.first();
    streamstate_.pop_front();

    switch(sstate) {
    case LINEWIDTH:
        if (lw_.empty())
            break;
        s << pdfqreal(lw_.first()) << " w\n";
        lw_.pop_front();
        break;
    case LINECAP:
        if (lc_.empty())
            break;
        s << QString::number(lc_.first()) << " J\n";
        lc_.pop_front();
        break;
    case LINEJOIN:
        if (lj_.empty())
            break;
        s << QString::number(lj_.first()) << " j\n";
        lj_.pop_front();
        break;
    case MITERLIMIT:
        if (ml_.empty())
            break;
        s << pdfqreal(ml_.first()) << " M\n";
        ml_.pop_front();
        break;
    case COLOR:
        {
            if (col_.empty())
                break;
            QColor rgba = col_.first();
            s << "/CSp CS\n"
                << pdfqreal(rgba.redF()) << ' '
                << pdfqreal(rgba.greenF()) << ' '
                << pdfqreal(rgba.blueF())
                << " SCN\n";
            col_.pop_front();
        }
        break;
    case DASHARRAY:
        {
            if (da_.empty() || stroking_.empty())
                break;
            s << "[";

            for (int i=0; i!=da_.first().seq.size(); ++i) {
                    s << pdfqreal(da_.first().seq[i]);
                    if (i<da_.first().seq.size()-1)
                        s << ' ';
		}
            s << "] ";
            s << pdfqreal(da_.first().phase);
            s << " d\n";
            da_.pop_front();
            stroking_.pop_front();
            break;
        }
    default:
        break;
    }
}

QPdfPen* QPdfPen::setLineWidth(double v)
{
    streamstate_.append(LINEWIDTH);
    lw_.append((v<0) ? 0 : v);
    return this;
}

QPdfPen* QPdfPen::setLineCap(unsigned v)
{
    streamstate_.append(LINECAP);
    lc_.append((v>2) ? 0 : v);
    return this;
}

QPdfPen* QPdfPen::setLineJoin(unsigned v)
{
    streamstate_.append(LINEJOIN);
    lj_.append((v>2) ? 0 : v);
    return this;
}

QPdfPen* QPdfPen::setMiterLimit(double v)
{
    streamstate_.append(MITERLIMIT);
    ml_.append(v);
    return this;
}

QPdfPen* QPdfPen::setColor(const QColor &rgba)
{
    streamstate_.append(COLOR);
    col_.append(rgba);
    return this;
}

qreal QPdfPen::alpha() const
{
    if (!stroking())
        return -1.0;
    if (col_.empty())
        return 1.0;
    return col_.last().alphaF();
}

QPdfPen* QPdfPen::setDashArray(const QPen& pen, double phase)
{
    QVector<qreal> sequence;

    qreal capsize = 0;
    qreal lw = qMax(pen.width(),1);

    switch(pen.capStyle()) {
    case Qt::RoundCap:
    case Qt::SquareCap:
        capsize = qMax(lw,qreal(1));
        break;
    default:
        break;
    };

    bool stroke = true;
    switch (pen.style()) {
    case Qt::NoPen:
        stroke = false;
        break;
    case Qt::SolidLine:
        break;
    case Qt::DashLine:
        sequence.append(3*lw-capsize); // 3 full
        sequence.append(1*lw+capsize); // 1 gap
        break;
    case Qt::DotLine:
        sequence.append(qMax(1*lw-capsize,qreal(0))); // 1 full
        sequence.append(1*lw+capsize); // 1 gap
        break;
    case Qt::DashDotLine:
        sequence.append(3*lw-capsize); // 3 full
        sequence.append(1*lw+capsize); // 1 gap
        sequence.append(qMax(1*lw-capsize,qreal(0))); // 1 full
        sequence.append(1*lw+capsize); // 1 gap
        break;
    case Qt::DashDotDotLine:
        sequence.append(3*lw-capsize);
        sequence.append(1*lw+capsize);
        sequence.append(1*lw-capsize);
        sequence.append(1*lw+capsize);
        sequence.append(qMax(1*lw-capsize,qreal(0))); // safety
        sequence.append(1*lw+capsize);
        break;
    case Qt::MPenStyle:
        // avoid compiler warning
        break;
    }

    streamstate_.append(DASHARRAY);
    stroking_.append(stroke);
    da_.append(DashArray(sequence, phase));
    return this;
}

bool QPdfPen::stroking() const
{
    if (stroking_.empty())
        return true;
    return stroking_.last();
}

QPdfPath::QPdfPath(const QPdfPen* pen, const QPdfBrush* brush, int brushflags,  bool closed)
    : QPdfObject(), ca_(1.0), CA_(1.0), alphaobj_(-1), gradientstrokealpha_(false)
{
    type = QPdfObject::PATH;

    painttype = (closed) ? CLOSE : NONE;

    if (pen && pen->stroking())
        painttype |= STROKE;

    if (brush && !brush->noBrush())
        painttype |= brushflags;

    ca_ = (brush) ? brush->alpha() : -1.0;
    CA_ = (pen) ? pen->alpha() : -1.0;
    gradientstrokealpha_ = (brush) ? brush->isGradient() : false;
}

QString QPdfPath::paintOperator() const
{
    QString s;

    if (painttype & CLIPPING) {
        if (painttype & FILLNONZERO)
            s += "W n\n";
        else
            s += "W* n\n";
        return s;
    }

    if (painttype & STROKE) {
        bool nsa = !hasTrueStrokeAlpha(); // don't use PDF combined operators in this case
        if (painttype & CLOSE) {
            if (painttype & FILLNONZERO)
                s = (nsa) ? "b" : "f";
            else if (painttype & FILLEVENODD)
                s = (nsa) ? "b*" : "f*";
            else
                s = "s";
        }
        else if (painttype & FILLNONZERO)
            s = (nsa) ? "B" : "f";
        else if (painttype & FILLEVENODD)
            s = (nsa) ? "B*" : "f*";
        else
            s = "S";
    }
    else if (painttype & FILLNONZERO)
        s = "f";
    else if (painttype & FILLEVENODD)
        s = "f*";
    else
        return QString("n");

    return s;
}

QString QPdfPath::streamCoreText() const
{
    QString s;
    for (int i=0; i != subpaths.size(); ++i) {
        const SubPath& spath = subpaths[i];
        Q_ASSERT(spath.initialized);

        s += pdfqreal(spath.start.x);
        s += ' ';
        s += pdfqreal(spath.start.y);
        s += " m\n";
        for (int j=0; j!= spath.elements.size(); ++j) {
            switch (spath.elements[j].type) {
            case Element::LINE:
                s += pdfqreal(spath.elements[j].line.x);
                s += ' ';
                s += pdfqreal(spath.elements[j].line.y);
                s += " l\n";
                break;
            case Element::CURVE:
                s += pdfqreal(spath.elements[j].curve.x1);
                s += ' ';
                s +=  pdfqreal(spath.elements[j].curve.y1);
                s += ' ';
                s +=  pdfqreal(spath.elements[j].curve.x2);
                s += ' ';
                s +=  pdfqreal(spath.elements[j].curve.y2);
                s += ' ';
                s +=  pdfqreal(spath.elements[j].curve.xnew);
                s += ' ';
                s +=  pdfqreal(spath.elements[j].curve.ynew);
                s += " c\n";
                break;
            default:
                break;
            }
        }
        if (spath.closed)
            s+= "h\n";
    }
    return s;
}

void QPdfPath::streamText(QTextStream &s)
{
    QString paintop = paintOperator();
    Q_ASSERT(!paintop.isEmpty());

    if (!alphaname_.isEmpty())
        s << alphaname_ << " gs\n";
    else
        s << "/GSa gs\n";

    s << streamCoreText();
    s << ' ' << paintop << "\n";

    // 2nd run to get Qt's fill->stroke sequential behavior
    if (hasTrueStrokeAlpha()) {
        if (paintop != "S" && paintop != "s") {
            if (painttype & CLOSE)
                paintop = "s";
            else
                paintop = "S";
            s << streamCoreText();
            s << ' ' << paintop << "\n";
        }
    }
}

QString QPdfPath::getAlphaDefinition() const
{
    QString s;
    if (!hasTrueAlpha())
        return s;

    s += "<<\n";
    if (hasTrueStrokeAlpha())
        s += QString("/CA %1\n").arg(CA_);
    if (hasTrueNonStrokeAlpha())
        s += QString("/ca %1\n").arg(ca_);
    s += ">>\n";
    s += "endobj\n";

    return s;
}

QPdfPage::QPdfPage()
    : QPdfObject()
{
    width_ = 0;
    height_ = 0;
}

void QPdfPage::destroy()
{
    for (int i = 0; i != gobjects_.size(); ++i) {
        if (gobjects_[i])
            if (0 == --gobjects_[i]->appended)
                delete gobjects_[i];
    }
    delete this;
}

void QPdfPage::streamText(QTextStream &s)
{
    s << "q\n";
    QString cm,cp;

    QMatrix lm;
    bool grad = false;
    QString gradcmd;

    for (int i = 0; i != gobjects_.size(); ++i) {
        switch(gobjects_[i]->type) {
        case MATRIX:
            if (lm == ((QPdfMatrix*)gobjects_[i])->currentMatrix()) {
                gobjects_[i]->streamText(s);
                break;
            }
            lm = ((QPdfMatrix*)gobjects_[i])->currentMatrix();
            {
                QTextStream stream(&cm);
                gobjects_[i]->streamText(stream);
            }
            s << "Q q\n"; // retrieves original matrix, (PDF syntax has no means to handle matrices besides concatenating)
            s << cp << cm; // ... but this destroys also the actual clipping path, so set them again (_before_ the transformation)
            break;
        case PATH:
            if (((QPdfPath*)gobjects_[i])->painttype & QPdfPath::CLIPPING) {
                QTextStream stream(&cp);
                gobjects_[i]->streamText(stream);
                stream.flush();
                s << "Q q\n";
                s << cp << cm;
            } else {
                gobjects_[i]->streamText(s);
            }
            break;
        case IMAGE:
            gobjects_[i]->streamText(s);
            break;
        case BRUSH:
        case PEN: {
            if (!predType(i, BRUSH) && !predType(i,PEN))
                s << "Q\n";

            if (gobjects_[i]->type == BRUSH) {
                QPdfBrush* br = (QPdfBrush*)gobjects_[i];
                grad = br->firstIsGradient();
                if (grad) {
                    QTextStream stream(&gradcmd);
                    gobjects_[i]->streamText(stream);
                }
                else
                    gobjects_[i]->streamText(s);
            } else {
                gobjects_[i]->streamText(s);
            }

            if (!nextType(i, BRUSH) && !nextType(i,PEN)) {
                s << "q\n";
                if (nextType(i, PATH) || nextType(i,IMAGE) || nextType(i, MATRIX))
                {
                    s << cp << cm;
                    if (grad)
                        s << gradcmd;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    s << "Q\n";
}

bool QPdfPage::predType(int i, QPdfObject::TYPE t)
{
    return ((i>0 && gobjects_[i-1]->type == t)) ? true : false;
}
bool QPdfPage::nextType(int i, QPdfObject::TYPE t)
{
    return ((i+1<gobjects_.size() && gobjects_[i+1]->type == t)) ? true : false;
}

QPdfObject* QPdfPage::append(QPdfObject* val, bool protect)
{
    if (!val)
        return 0;
    if (protect)
        val->appended = -1;

    gobjects_.append(val);
    if (val->type == QPdfObject::IMAGE)
        images.append((QPdfImage*)val);
    else if (val->type == QPdfObject::PATH)
        paths.append((QPdfPath*)val);

    if (val->appended>=0)
        ++val->appended;

    return val;
}

QPdfEnginePrivate::QPdfEnginePrivate()
{
    objnumber_ = 1;
    width_ = 0;
    height_ = 0;
    options_ = 0;
    streampos_ = 0;
    landscape_ = false;

    curPage = new QPdfPage;

    curMatrix = new QPdfMatrix;
    curPen = new QPdfPen;
    curBrush = new QPdfBrush;

    stream_ = new QDataStream;
    pageOrder = QPrinter::FirstPageFirst;
    orientation = QPrinter::Portrait;
}

QPdfEnginePrivate::~QPdfEnginePrivate()
{
    curPage->destroy();
    delete curMatrix;
    delete curPen;
    delete curBrush;
    delete stream_;
}

// For strings up to 10000 bytes only !
int QPdfEnginePrivate::xprintf(const char* fmt, ...)
{
    if (!stream_)
        return 0;

    const int msize = 10000;
    static char buf[msize];

    va_list args;
    va_start(args, fmt);
    int bufsize = vsprintf(buf, fmt, args);

    Q_ASSERT(bufsize<msize);

    va_end(args);

    stream_->writeRawData(buf,bufsize);
    streampos_ += bufsize;
    return bufsize;
}

void QPdfEnginePrivate::setDevice(QIODevice* device)
{
    stream_->setDevice(device);
    streampos_ = 0;
}

void QPdfEnginePrivate::unsetDevice()
{
    stream_->unsetDevice();
}

void QPdfEnginePrivate::writeHeader()
{
    addxentry(0,false);

    xprintf("%%PDF-1.4\n");

    writeInfo();
    writeCatalog();

    gsobjnumber_ = requestObjNumber();
    pcsobjnumber_ = requestObjNumber();
    csobjnumber_ = requestObjNumber();
    csgobjnumber_ = requestObjNumber();
}

void QPdfEnginePrivate::writeInfo()
{
    time_t now;
    tm *newtime;

    time(&now);
    newtime = gmtime(&now);
    QString y;

    if (newtime && newtime->tm_year+1900 > 1992)
        y += QString::number(newtime->tm_year+1900);

    info_ = addxentry(-1);
    xprintf("<<\n"
            "/Title (%s)\n"
            "/Author (%s)\n"
            "/Creator (%s)\n"
            "/Producer (Qt %s (C) 1992-%s Trolltech AS)\n",
            title.toLocal8Bit().constData(), author.toLocal8Bit().constData(), creator.toLocal8Bit().constData(),
            qVersion(), y.toLocal8Bit().constData());

    if(!newtime){
        xprintf(">>\n"
                "endobj\n");
        return;
    }

    xprintf("/CreationDate (D:%d%02d%02d%02d%02d%02d)\n"
            ">>\n"
            "endobj\n",
            newtime->tm_year+1900,
            newtime->tm_mon+1,
            newtime->tm_mday,
            newtime->tm_hour,
            newtime->tm_min,
            newtime->tm_sec);
}


// Create catalog and page structure - 2nd and 3th PDF object

void QPdfEnginePrivate::writeCatalog()
{
    root_ = addxentry(-1);
    pagesobjnumber_ = requestObjNumber();
    xprintf("<<\n"
            "/Type /Catalog\n"
            "/Pages %d 0 R\n"
            ">>\n"
            "endobj\n",pagesobjnumber_);
}

void QPdfEnginePrivate::writePageRoot()
{
    addxentry(pagesobjnumber_);
    int viewport[4] = {0,0,width_,height_};

    xprintf("<<\n"
            "/Type /Pages\n"
            "/Kids \n");

    xprintf("[\n");
    int size = pageobjnumber_.size();
    for (int i = 0; i != size; ++i)
        xprintf("%d 0 R\n",pageobjnumber_[pageOrder == QPrinter::FirstPageFirst ? i : size-i-1][2]);
    xprintf("]\n");

    //xprintf("/Group <</S /Transparency /I true /K false>>\n");

    xprintf("/Count %d\n"
            "/MediaBox [%d %d %d %d]\n",
            pageobjnumber_.size(), viewport[0], viewport[1], viewport[2], viewport[3]);

    xprintf("/ProcSet [/PDF /Text /ImageB /ImageC]\n"
            ">>\n"
            "endobj\n"
        );

    // graphics state

    addxentry(gsobjnumber_);
    xprintf("<<\n"
            "/Type /ExtGState\n"
            "/SA true\n"
            "/SM 0.02\n"
            "/BG2 /Default\n"
            "/BM /Normal\n"
            "/UCR2 /Default\n"
            "/TR2 /Default\n"
            "/ca 1.0\n"
            "/CA 1.0\n"
            "/AIS false\n"
            ">>\n"
            "endobj\n");

    // color space for pattern

    addxentry(pcsobjnumber_);
    xprintf("[/Pattern %d 0 R]\n"
            "endobj\n", csobjnumber_);

    addxentry(csobjnumber_);
    xprintf("[ /CalRGB\n"
            "<<\n"
            "/WhitePoint [0.9505 1.0000 1.0890]\n"
            "/Gamma [1.8000 1.8000 1.8000]\n"
            "/Matrix [ 0.4497 0.2446 0.0252 0.3163 0.6720 0.1412 0.1845 0.0833 0.9227]\n"
            //"/Matrix [ 1 0 0 0 1 0 0 0 1]\n"
            ">>\n"
            "]\n"
            "endobj\n");

    addxentry(csgobjnumber_);
    xprintf("[ /CalGray\n"
            "<<\n"
            "/WhitePoint [0.9505 1.0000 1.0890]\n"
            ">>\n"
            "]\n"
            "endobj\n");
}

void QPdfEnginePrivate::newPage()
{
    flushPage();

    curPage->destroy();
    curPage = new QPdfPage;

    delete curMatrix; curMatrix = new QPdfMatrix;
    delete curPen; curPen = new QPdfPen;
    delete curBrush; curBrush = new QPdfBrush;

    curPage->append(curMatrix->setMatrix(QMatrix()),true);
    curPage->append(curPen->setColor(QColor()),true);
    curPage->append(curBrush->setFixed(Qt::NoBrush, QColor()),true);

    pageobjnumber_.append(QVector<uint>(4));
    pageobjnumber_.last()[0] = requestObjNumber();	// page stream object
    pageobjnumber_.last()[1] = requestObjNumber();	// stream length object
    pageobjnumber_.last()[2] = requestObjNumber();	// page object
    pageobjnumber_.last()[3] = requestObjNumber();	// page resources (pattern etc.)
}


void QPdfEnginePrivate::flushPage()
{
    if (pageobjnumber_.empty())
        return;

    QPdfStream cs;
    cs.setStream(*stream_);

    addxentry(pageobjnumber_.last()[2]);
    xprintf("<<\n"
            "/Type /Page\n"
            "/Parent %d 0 R\n"
            "/Contents %d 0 R\n"
            "/Resources %d 0 R\n"
            ">>\n"
            "endobj\n",
            pagesobjnumber_, pageobjnumber_.last()[0], pageobjnumber_.last()[3]);


    addxentry(pageobjnumber_.last()[3]);
    xprintf("<<\n"
            "/ColorSpace <<\n"
            "/PCSp %d 0 R\n"
            "/CSp %d 0 R\n"
            "/CSpg %d 0 R\n"
            "/DefaultRGB /CSp\n"
            "/DefaultGray /CSpg\n"
            ">>\n"
            "/ExtGState <<\n"
            "/GSa %d 0 R\n",
            pcsobjnumber_, csobjnumber_, csgobjnumber_, gsobjnumber_);

    int i;

    // Graphic states for const alphas < 1

    for (i=0; i<curPage->paths.size();++i) {
        QPdfPath* p = curPage->paths[i];

        if (p->hasTrueAlpha()) {
            p->setAlpha("/GStr", requestObjNumber());
            xprintf("%s %d 0 R\n", p->alphaName().toLocal8Bit().constData(), p->alphaObject());
        }
    }

    // Graphic states for transparent gradients

    for (i=0; i<curBrush->gradients.size();++i) {
        QPdfGradient* sh = curBrush->gradients[i].shader;
        sh->setColorSpaceObject(csobjnumber_);
        sh->setSoftMaskColorSpaceObject(csgobjnumber_);
        if (sh->hasSoftMask()) {
            sh->setSoftMaskObjects(requestObjNumber(),requestObjNumber(),requestObjNumber());
            xprintf("%s <</Type /ExtGState /SMask <</S /Alpha /G %d 0 R>> >>\n"
                    ,sh->softMaskGraphicStateName().toLocal8Bit().constData(),sh->softMaskFormObject());
        }
    }
    xprintf(">>\n");

    // brushes with fixed pattern

    xprintf("/Pattern <<\n");

    QVector<uint> fno;
    for (i=0; i<curBrush->fixeds.size();++i) {
        QPdfBrush::FixedPattern p = curBrush->fixeds[i];

        if (!p.isTruePattern())
            continue;

        fno.append(requestObjNumber());
        xprintf("%s %d 0 R\n",
                curBrush->fixeds[i].name.toLocal8Bit().constData(),fno.last());
    }

    // ... pixmap brushes

    QVector<uint> pno;
    for (i=0; i<curBrush->pixmaps.size();++i) {
        pno.append(requestObjNumber());
        xprintf("%s %d 0 R\n",
                curBrush->pixmaps[i].name.toLocal8Bit().constData(),pno.last());
    }

    // ... linear gradient brushes

    for (i=0; i<curBrush->gradients.size();++i) {
        int obj = requestObjNumber();
        curBrush->gradients[i].setMainObj(obj);
        xprintf("%s %d 0 R\n",
                curBrush->gradients[i].name.toLocal8Bit().constData(), obj);
    }

    xprintf(">>\n");


    // ... images

    xprintf("/XObject <<\n");
    QVector<int> iv;
    for (i=0; i<curPage->images.size();++i) {
        iv.append(requestObjNumber());
        xprintf("%s %d 0 R\n",
                curPage->images[i]->name.toLocal8Bit().constData(),iv.last());
    }
    xprintf(">>\n");

    // close resource dictionary

    xprintf(">>\n"
            "endobj\n");


    // write associated objects

    // Graphic states for const alphas < 1

    for (i=0; i<curPage->paths.size();++i) {
        QPdfPath* p = curPage->paths[i];

        if (p->hasTrueAlpha()) {
            addxentry(p->alphaObject());
            xprintf(p->getAlphaDefinition().toLocal8Bit());
        }
    }

    // fixed brushes

    int k = 0;
    for (i=0; i<curBrush->fixeds.size(); ++i) {
        QPdfBrush::FixedPattern p = curBrush->fixeds[i];

        if (!p.isTruePattern())
            continue;

        addxentry(fno[k++]);
        xprintf(p.getDefinition().toLocal8Bit().constData());
    }

    // gradient

    // write shader and associated function objects

    for (i=0; i<curBrush->gradients.size();++i) {
        QPdfBrush::GradientPattern p = curBrush->gradients[i];
        p.shader->setObjects(requestObjNumber(), requestObjNumber());

        addxentry(p.getMainObj());
        xprintf(p.getDefinition().toLocal8Bit().constData());

        addxentry(p.shader->mainObject());
        xprintf(p.shader->getMainDefinition().toLocal8Bit().constData());
        addxentry(p.shader->functionObject());
        xprintf(p.shader->getFuncDefinition().toLocal8Bit().constData());

        if (p.shader->hasSoftMask()) {
            addxentry(p.shader->softMaskFormObject());
            xprintf(p.shader->getSoftMaskFormDefinition().toLocal8Bit().constData());
            addxentry(p.shader->softMaskMainObject());
            xprintf(p.shader->getSoftMaskMainDefinition().toLocal8Bit().constData());
            addxentry(p.shader->softMaskFunctionObject());
            xprintf(p.shader->getSoftMaskFuncDefinition().toLocal8Bit().constData());
        }
    }

    // image

    // gather pattern images (add pixmap brushes to the remaining images)

    QVector<QPdfImage*> images = curPage->images;
    for (i=0; i<curBrush->pixmaps.size();++i) {
        QPdfBrush::PixmapPattern p = curBrush->pixmaps[i];
        addxentry(pno[i]);
        iv.append(requestObjNumber());
        xprintf(p.getDefinition(iv.last()).toLocal8Bit().constData());
        images.append(curBrush->pixmaps[i].image);
    }

    // write all image objects

    for (i=0; i<images.size();++i) {
        addxentry(iv[i]);
        QPdfImage* im = images[i];

        if (im->hasHardMask())
            im->setMaskObj(requestObjNumber());
        if (im->hasSoftMask())
            im->setSoftMaskObj(requestObjNumber());
        im->setLenObj(requestObjNumber());
        xprintf("%sstream\n", im->getDefinition().toLocal8Bit().constData());
        int len = streampos_;
        streampos_ += (int)cs.write(im->data(), im->rawLength());
        len = streampos_-len;
        xprintf("endstream\n"
                "endobj\n");
        addxentry(im->lenObj());
        xprintf("%d\n"
                "endobj\n", len);

        // ... image masks

        if (im->hasHardMask()) {
            addxentry(im->hardMaskObj());
            QPdfImage* im2 = im->stencil;
            im2->setLenObj(requestObjNumber());
            xprintf("%sstream\n", im2->getDefinition().toLocal8Bit().constData());
            len = streampos_;
            streampos_ += (int)cs.write(im2->data(), im2->rawLength());
            len = streampos_-len;
            xprintf("endstream\n"
                    "endobj\n");
            addxentry(im2->lenObj());
            xprintf("%d\n"
                    "endobj\n", len);
        }
        if (im->hasSoftMask()) {
            addxentry(im->softMaskObj());
            QPdfImage* im2 = im->softmask;
            im2->setLenObj(requestObjNumber());
            xprintf("%sstream\n", im2->getDefinition().toLocal8Bit().constData());
            len = streampos_;
            streampos_ += (int)cs.write(im2->data(), im2->rawLength());
            len = streampos_-len;
            xprintf("endstream\n"
                    "endobj\n");
            addxentry(im2->lenObj());
            xprintf("%d\n"
                    "endobj\n", len);
        }
    }


    // open page stream object

    addxentry(pageobjnumber_.last()[0]);
    xprintf("<<\n"
            "/Length %d 0 R\n",
            pageobjnumber_.last()[1]); // object number for stream length object
    if (do_compress)
        xprintf("/Filter /FlateDecode\n");

    xprintf(">>\n");
    xprintf("stream\n");
    QString s;
    QTextStream stream(&s);
    stream << "/GSa gs\n";
    stream << "/CSp cs /CSp CS\n";
    QMatrix tmp(1.0, 0.0, // m11, m12
                0.0, -1.0, // m21, m22
                0.0, height_); // dx, dy
    stream << QPdfMatrix::streamMatrix(tmp);
    curPage->streamText(stream);
    stream.flush();
    int len = streampos_;
    streampos_ += (int)cs.write(s.toLocal8Bit().constData(), s.size());
    len = streampos_-len;
    xprintf("endstream\n"
            "endobj\n");

    addxentry(pageobjnumber_.last()[1]);
    xprintf("%d\n"
            "endobj\n",len);
}

void QPdfEnginePrivate::writeTail()
{
    flushPage();
    writePageRoot();
    addxentry(xrefpos_.size(),false);
    xprintf("xref\n"
            "0 %d\n"
            "%010d 65535 f \n", xrefpos_.size()-1, xrefpos_[0]);

    for (int i=1; i!=xrefpos_.size()-1; ++i)
        xprintf("%010d 00000 n \n", xrefpos_[i]);

    xprintf("trailer\n"
            "<<\n"
            "/Size %d\n"
            "/Info %d 0 R\n"
            "/Root %d 0 R\n"
            ">>\n"
            "startxref\n%d\n"
            "%%%%EOF\n",
            xrefpos_.size()-1, info_, root_, xrefpos_.last());
}

int QPdfEnginePrivate::addxentry(int objnumber, bool printostr)
{
    if (objnumber < 0)
        objnumber = requestObjNumber();

    if (objnumber>=xrefpos_.size())
        xrefpos_.resize(objnumber+1);

    xrefpos_[objnumber] = streampos_;
    if (printostr)
        xprintf("%d 0 obj\n",objnumber);

    return objnumber;
}
