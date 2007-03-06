/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/qprintengine.h>

#include <qiodevice.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpainterpath.h>
#include <qpaintdevice.h>
#include <qfile.h>
#include <qdebug.h>
#include <qimagewriter.h>
#include <qbuffer.h>

#ifndef QT_NO_PRINTER
#include <time.h>
#include <limits.h>
#include <math.h>
#ifndef QT_NO_COMPRESS
#include <zlib.h>
#endif

#include "qprintengine_pdf_p.h"
#include "private/qdrawhelper_p.h"

extern qint64 qt_pixmap_id(const QPixmap &pixmap);
extern qint64 qt_image_id(const QImage &image);

//#define FONT_DUMP

// might be helpful for smooth transforms of images
// Can't use it though, as gs generates completely wrong images if this is true.
static const bool interpolateImages = false;

#ifdef QT_NO_COMPRESS
static const bool do_compress = false;
#else
static const bool do_compress = true;
#endif

QPdfPage::QPdfPage()
    : QPdf::ByteStream(&data)
{
}

void QPdfPage::streamImage(int w, int h, int object)
{
    *this << "/GSa gs " << w << "0 0 " << -h << "0 " << h << "cm /Im" << object << " Do\n";
    if (!images.contains(object))
        images.append(object);
}


inline QPaintEngine::PaintEngineFeatures qt_pdf_decide_features()
{
    QPaintEngine::PaintEngineFeatures f = QPaintEngine::AllFeatures;
    f &= ~(QPaintEngine::PorterDuff | QPaintEngine::PerspectiveTransform
#ifndef USE_NATIVE_GRADIENTS
           | QPaintEngine::LinearGradientFill
#endif
           | QPaintEngine::RadialGradientFill
           | QPaintEngine::ConicalGradientFill);
    return f;
}

QPdfEngine::QPdfEngine(QPrinter::PrinterMode m)
    : QPdfBaseEngine(*new QPdfEnginePrivate(m), qt_pdf_decide_features())
{
    state = QPrinter::Idle;
}

QPdfEngine::~QPdfEngine()
{
}

bool QPdfEngine::begin(QPaintDevice *pdev)
{
    Q_D(QPdfEngine);

    if(!QPdfBaseEngine::begin(pdev))
        return false;
    d->stream->setDevice(d->outDevice);

    d->streampos = 0;
    d->hasPen = true;
    d->hasBrush = false;
    d->clipEnabled = false;
    d->allClipped = false;

    d->xrefPositions.clear();
    d->pageRoot = 0;
    d->catalog = 0;
    d->info = 0;
    d->graphicsState = 0;
    d->patternColorSpace = 0;

    d->pages.clear();
    d->imageCache.clear();

    setActive(true);
    state = QPrinter::Active;
    d->writeHeader();
    newPage();

    return true;
}

bool QPdfEngine::end()
{
    Q_D(QPdfEngine);
    d->writeTail();

    d->stream->unsetDevice();
    QPdfBaseEngine::end();
    setActive(false);
    state = QPrinter::Idle;
    return true;
}


void QPdfEngine::drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr)
{
    if (sr.isEmpty() || rectangle.isEmpty() || pixmap.isNull())
        return;
    Q_D(QPdfEngine);

    QBrush b = d->brush;

    QRect sourceRect = sr.toRect();
    QPixmap pm = sourceRect != pixmap.rect() ? pixmap.copy(sourceRect) : pixmap;
    QImage image = pm.toImage();

    *d->currentPage << "q\n";
    *d->currentPage
        << QPdf::generateMatrix(QTransform(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                                           rectangle.x(), rectangle.y()) * (d->simplePen ? QTransform() : d->stroker.matrix));
    bool bitmap = true;
    int object = d->addImage(image, &bitmap, qt_pixmap_id(pm));
    if (bitmap) {
        // set current pen as d->brush
        d->brush = d->pen.brush();
        setBrush();
    }
    d->currentPage->streamImage(image.width(), image.height(), object);
    *d->currentPage << "Q\n";

    d->brush = b;
}

void QPdfEngine::drawImage(const QRectF & rectangle, const QImage & image, const QRectF & sr, Qt::ImageConversionFlags)
{
    if (sr.isEmpty() || rectangle.isEmpty() || image.isNull())
        return;
    Q_D(QPdfEngine);

    QRect sourceRect = sr.toRect();
    QImage im = sourceRect != image.rect() ? image.copy(sr.toRect()) : image;

    *d->currentPage << "q\n";
    *d->currentPage
        << QPdf::generateMatrix(QTransform(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                                           rectangle.x(), rectangle.y()) * (d->simplePen ? QTransform() : d->stroker.matrix));
    bool bitmap = false;
    int object = d->addImage(im, &bitmap, qt_image_id(im));
    d->currentPage->streamImage(image.width(), image.height(), object);
    *d->currentPage << "Q\n";
}

void QPdfEngine::drawTiledPixmap (const QRectF &rectangle, const QPixmap &pixmap, const QPointF &point)
{
    Q_D(QPdfEngine);

    bool bitmap = (pixmap.depth() == 1);
    QBrush b = d->brush;
    QPointF bo = d->brushOrigin;
    bool hp = d->hasPen;
    d->hasPen = false;
    bool hb = d->hasBrush;
    d->hasBrush = true;

    d->brush = QBrush(pixmap);
    if (bitmap)
        // #### fix bitmap case where we have a brush pen
        d->brush.setColor(d->pen.color());

    d->brushOrigin = -point;
    *d->currentPage << "q\n";
    setBrush();

    drawRects(&rectangle, 1);
    *d->currentPage << "Q\n";

    d->hasPen = hp;
    d->hasBrush = hb;
    d->brush = b;
    d->brushOrigin = bo;
}


void QPdfEngine::setBrush()
{
    Q_D(QPdfEngine);
    Qt::BrushStyle style = d->brush.style();
    if (style == Qt::NoBrush)
        return;
    
    bool specifyColor;
    int gStateObject = 0;
    int patternObject = d->addBrushPattern(d->stroker.matrix, &specifyColor, &gStateObject);

    *d->currentPage << (patternObject ? "/PCSp cs " : "/CSp cs ");
    if (specifyColor) {
        QColor rgba = d->brush.color();
        *d->currentPage << rgba.redF()
                        << rgba.greenF()
                        << rgba.blueF();
    }
    if (patternObject)
        *d->currentPage << "/Pat" << patternObject;
    *d->currentPage << "scn\n";

    if (gStateObject)
        *d->currentPage << "/GState" << gStateObject << "gs\n";
    else
        *d->currentPage << "/GSa gs\n";
}

QPaintEngine::Type QPdfEngine::type() const
{
    return QPaintEngine::User;
}

bool QPdfEngine::newPage()
{
    Q_D(QPdfEngine);
    if (!isActive())
        return false;
    d->newPage();
    return QPdfBaseEngine::newPage();
}

QPdfEnginePrivate::QPdfEnginePrivate(QPrinter::PrinterMode m)
    : QPdfBaseEnginePrivate(m)
{
    streampos = 0;

    stream = new QDataStream;
    pageOrder = QPrinter::FirstPageFirst;
    orientation = QPrinter::Portrait;
    fullPage = false;
}

QPdfEnginePrivate::~QPdfEnginePrivate()
{
    delete stream;
}


#ifdef USE_NATIVE_GRADIENTS
int QPdfEnginePrivate::gradientBrush(const QBrush &b, const QMatrix &matrix, int *gStateObject)
{
    const QGradient *gradient = b.gradient();
    if (!gradient)
        return 0;

    QTransform inv = matrix.inverted();
    QPointF page_rect[4] = { inv.map(QPointF(0, 0)),
                             inv.map(QPointF(width_, 0)),
                             inv.map(QPointF(0, height_)),
                             inv.map(QPointF(width_, height_)) };

    bool opaque = b.isOpaque();

    QByteArray shader;
    QByteArray alphaShader;
    if (gradient->type() == QGradient::LinearGradient) {
        const QLinearGradient *lg = static_cast<const QLinearGradient *>(gradient);
        shader = QPdf::generateLinearGradientShader(lg, page_rect);
        if (!opaque)
            alphaShader = QPdf::generateLinearGradientShader(lg, page_rect, true);
    } else {
        // #############
        return 0;
    }
    int shaderObject = addXrefEntry(-1);
    write(shader);

    QByteArray str;
    QPdf::ByteStream s(&str);
    s << "<<\n"
        "/Type /Pattern\n"
        "/PatternType 2\n"
        "/Shading " << shaderObject << "0 R\n"
        "/Matrix ["
      << matrix.m11()
      << matrix.m12()
      << matrix.m21()
      << matrix.m22()
      << matrix.dx()
      << matrix.dy() << "]\n";
    s << ">>\n"
        "endobj\n";

    int patternObj = addXrefEntry(-1);
    write(str);
    currentPage->patterns.append(patternObj);

    if (!opaque) {
        bool ca = true;
        QGradientStops stops = gradient->stops();
        int a = stops.at(0).second.alpha();
        for (int i = 1; i < stops.size(); ++i) {
            if (stops.at(i).second.alpha() != a) {
                ca = false;
                break;
            }
        }
        if (ca) {
            *gStateObject = addConstantAlphaObject(stops.at(0).second.alpha());
        } else {
            int alphaShaderObject = addXrefEntry(-1);
            write(alphaShader);

            QByteArray content;
            QPdf::ByteStream c(&content);
            c << "/Shader" << alphaShaderObject << "sh\n";

            QByteArray form;
            QPdf::ByteStream f(&form);
            f << "<<\n"
                "/Type /XObject\n"
                "/Subtype /Form\n"
                "/BBox [0 0 " << width_ << height_ << "]\n"
                "/Group <</S /Transparency >>\n"
                "/Resources <<\n"
                "/Shading << /Shader" << alphaShaderObject << alphaShaderObject << "0 R >>\n"
                ">>\n";

            f << "/Length " << content.length() << "\n"
                ">>\n"
                "stream\n"
              << content
              << "endstream\n"
                "endobj\n";

            int softMaskFormObject = addXrefEntry(-1);
            write(form);
            *gStateObject = addXrefEntry(-1);
            xprintf("<< /SMask << /S /Alpha /G %d 0 R >> >>\n"
                    "endobj\n", softMaskFormObject);
            currentPage->graphicStates.append(*gStateObject);
        }
    }

    return patternObj;
}
#endif

int QPdfEnginePrivate::addConstantAlphaObject(int alpha)
{
    if (alpha == 255)
        return 0;
    int object = alphaCache.value(alpha, 0);
    if (!object) {
        object = addXrefEntry(-1);
        QByteArray alphaDef;
        QPdf::ByteStream s(&alphaDef);
        s << "<< /ca " << (alpha/255.) << ">>\n";
        xprintf(alphaDef.constData());
        xprintf("endobj\n");
    }
    currentPage->graphicStates.append(object);
    return object;
}

int QPdfEnginePrivate::addBrushPattern(const QTransform &m, bool *specifyColor, int *gStateObject)
{
    int paintType = 2; // Uncolored tiling
    int w = 8;
    int h = 8;

    *specifyColor = true;
    *gStateObject = 0;

    QTransform matrix = m;
    matrix.translate(brushOrigin.x(), brushOrigin.y());
    matrix = matrix * pageMatrix();
    //qDebug() << brushOrigin << matrix;

    Qt::BrushStyle style = brush.style();
    if (style == Qt::LinearGradientPattern) {// && style <= Qt::ConicalGradientPattern) {
#ifdef USE_NATIVE_GRADIENTS
        *specifyColor = false;
        return gradientBrush(b, matrix, gStateObject);
#else
        return 0;
#endif
    }

    if (!brush.isOpaque() && brush.style() < Qt::LinearGradientPattern) 
        *gStateObject = addConstantAlphaObject(brush.color().alpha());

    int imageObject = 0;
    QByteArray pattern = QPdf::patternForBrush(brush);
    if (pattern.isEmpty()) {
        if (brush.style() != Qt::TexturePattern)
            return 0;
        QImage image = brush.texture().toImage();
        bool bitmap = true;
        imageObject = addImage(image, &bitmap, qt_pixmap_id(brush.texture()));
        QImage::Format f = image.format();
        if (f != QImage::Format_MonoLSB && f != QImage::Format_Mono) {
            paintType = 1; // Colored tiling
            *specifyColor = false;
        }
        w = image.width();
        h = image.height();
        QTransform m(w, 0, 0, -h, 0, h);
        QPdf::ByteStream s(&pattern);
        s << QPdf::generateMatrix(m);
        s << "/Im" << imageObject << " Do\n";
    }

    QByteArray str;
    QPdf::ByteStream s(&str);
    s << "<<\n"
        "/Type /Pattern\n"
        "/PatternType 1\n"
        "/PaintType " << paintType << "\n"
        "/TilingType 1\n"
        "/BBox [0 0 " << w << h << "]\n"
        "/XStep " << w << "\n"
        "/YStep " << h << "\n"
        "/Matrix ["
      << matrix.m11()
      << matrix.m12()
      << matrix.m21()
      << matrix.m22()
      << matrix.dx()
      << matrix.dy() << "]\n"
        "/Resources \n<< "; // open resource tree
    if (imageObject) {
        s << "/XObject << /Im" << imageObject << ' ' << imageObject << "0 R >> ";
    }
    s << ">>\n"
        "/Length " << pattern.length() << "\n"
        ">>\n"
        "stream\n"
      << pattern
      << "endstream\n"
        "endobj\n";

    int patternObj = addXrefEntry(-1);
    write(str);
    currentPage->patterns.append(patternObj);
    return patternObj;
}

int QPdfEnginePrivate::addImage(const QImage &img, bool *bitmap, qint64 serial_no)
{
    if (img.isNull())
        return -1;

    int object = imageCache.value(serial_no);
    if(object)
        return object;

    QImage image = img;
    QImage::Format format = image.format();
    if (image.depth() == 1 && *bitmap) {
        if (format == QImage::Format_MonoLSB)
            image = image.convertToFormat(QImage::Format_Mono);
    } else {
        *bitmap = false;
        if (format != QImage::Format_RGB32 && format != QImage::Format_ARGB32)
            image = image.convertToFormat(QImage::Format_ARGB32);
    }

    int w = image.width();
    int h = image.height();
    int d = image.depth();

    if (d == 1) {
        int bytesPerLine = (w + 7) >> 3;
        QByteArray data;
        data.resize(bytesPerLine * h);
        char *rawdata = data.data();
        for (int y = 0; y < h; ++y) {
            memcpy(rawdata, image.scanLine(y), bytesPerLine);
            rawdata += bytesPerLine;
        }
        object = writeImage(data, w, h, d, 0, 0);
    } else {
        QByteArray softMaskData;
        softMaskData.resize(w * h);
        bool dct = false;
        QByteArray imageData;
        bool hasAlpha = false;
        bool hasMask = false;

        if (QImageWriter::supportedImageFormats().contains("jpeg")) {
            QBuffer buffer(&imageData);
            QImageWriter writer(&buffer, "jpeg");
            writer.setQuality(94);
            writer.write(img);

            uchar *sdata = (uchar *)softMaskData.data();
            for (int y = 0; y < h; ++y) {
                const QRgb *rgb = (const QRgb *)image.scanLine(y);
                for (int x = 0; x < w; ++x) {
                    uchar alpha = qAlpha(*rgb);
                    *sdata++ = alpha;
                    hasMask |= (alpha < 255);
                    hasAlpha |= (alpha != 0 && alpha != 255);
                    ++rgb;
                }
            }
            dct = true;
        } else {
            imageData.resize(3 * w * h);
            uchar *data = (uchar *)imageData.data();
            uchar *sdata = (uchar *)softMaskData.data();
            for (int y = 0; y < h; ++y) {
                const QRgb *rgb = (const QRgb *)image.scanLine(y);
                for (int x = 0; x < w; ++x) {
                    *(data++) = qRed(*rgb);
                    *(data++) = qGreen(*rgb);
                    *(data++) = qBlue(*rgb);
                    uchar alpha = qAlpha(*rgb);
                    *sdata++ = alpha;
                    hasMask |= (alpha < 255);
                    hasAlpha |= (alpha != 0 && alpha != 255);
                    ++rgb;
                }
            }
        }
        int maskObject = 0;
        int softMaskObject = 0;
        if (hasAlpha) {
            softMaskObject = writeImage(softMaskData, w, h, 8, 0, 0);
        } else if (hasMask) {
            // dither the soft mask to 1bit and add it. This also helps PDF viewers
            // without transparency support
            int bytesPerLine = (w + 7) >> 3;
            QByteArray mask(bytesPerLine * h, 0);
            uchar *mdata = (uchar *)mask.data();
            const uchar *sdata = (const uchar *)softMaskData.constData();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (*sdata)
                        mdata[x>>3] |= (0x80 >> (x&7));
                    ++sdata;
                }
                mdata += bytesPerLine;
            }
            maskObject = writeImage(mask, w, h, 1, 0, 0);
        }
        object = writeImage(imageData, w, h, 32, maskObject, softMaskObject, dct);
    }
    imageCache.insert(serial_no, object);
    return object;
}

QTransform QPdfEnginePrivate::pageMatrix() const
{
    qreal scale = 72./resolution;
    QTransform tmp(scale, 0.0, 0.0, -scale, 0.0, height());
    if (!fullPage)
        tmp.translate(resolution/3, resolution/3);
    return tmp;
}

void QPdfEnginePrivate::newPage()
{
    writePage();

    delete currentPage;
    currentPage = new QPdfPage;
    stroker.stream = currentPage;
    pages.append(requestObject());

    *currentPage << "/GSa gs /CSp cs /CSp CS\n"
                 << QPdf::generateMatrix(pageMatrix())
                 << "q q\n";
}


// For strings up to 10000 bytes only !
void QPdfEnginePrivate::xprintf(const char* fmt, ...)
{
    if (!stream)
        return;

    const int msize = 10000;
    char buf[msize];

    va_list args;
    va_start(args, fmt);
    int bufsize = qvsnprintf(buf, msize, fmt, args);

    Q_ASSERT(bufsize<msize);

    va_end(args);

    stream->writeRawData(buf, bufsize);
    streampos += bufsize;
}

int QPdfEnginePrivate::writeCompressed(const char *src, int len)
{
#ifndef QT_NO_COMPRESS
    if(do_compress) {
        uLongf destLen = len + len/100 + 13; // zlib requirement
        Bytef* dest = new Bytef[destLen];
        if (Z_OK == ::compress(dest, &destLen, (const Bytef*) src, (uLongf)len)) {
            stream->writeRawData((const char*)dest, destLen);
        } else {
            qWarning("QPdfStream::writeCompressed: Error in compress()");
            destLen = 0;
        }
        delete [] dest;
        len = destLen;
    } else
#endif
    {
        stream->writeRawData(src,len);
    }
    streampos += len;
    return len;
}

int QPdfEnginePrivate::writeImage(const QByteArray &data, int width, int height, int depth,
                                  int maskObject, int softMaskObject, bool dct)
{
    int image = addXrefEntry(-1);
    xprintf("<<\n"
            "/Type /XObject\n"
            "/Subtype /Image\n"
            "/Width %d\n"
            "/Height %d\n", width, height);

    if (depth == 1) {
        xprintf("/ImageMask true\n"
                "/Decode [1 0]\n");
    } else {
        xprintf("/BitsPerComponent 8\n"
                "/ColorSpace %s\n", (depth == 32) ? "/DeviceRGB" : "/DeviceGray");
    }
    if (maskObject > 0)
        xprintf("/Mask %d 0 R\n", maskObject);
    if (softMaskObject > 0)
        xprintf("/SMask %d 0 R\n", softMaskObject);

    int lenobj = requestObject();
    xprintf("/Length %d 0 R\n", lenobj);
    if (interpolateImages)
        xprintf("/Interpolate true\n");
    int len = 0;
    if (dct) {
        //qDebug() << "DCT";
        xprintf("/Filter /DCTDecode\n>>\nstream\n");
        write(data);
        len = data.length();
    } else {
        if (do_compress)
            xprintf("/Filter /FlateDecode\n>>\nstream\n");
        else
            xprintf(">>\nstream\n");
        len = writeCompressed(data);
    }
    xprintf("endstream\n"
            "endobj\n");
    addXrefEntry(lenobj);
    xprintf("%d\n"
            "endobj\n", len);
    return image;
}


void QPdfEnginePrivate::writeHeader()
{
    addXrefEntry(0,false);

    xprintf("%%PDF-1.4\n");

    writeInfo();

    catalog = addXrefEntry(-1);
    pageRoot = requestObject();
    xprintf("<<\n"
            "/Type /Catalog\n"
            "/Pages %d 0 R\n"
            ">>\n"
            "endobj\n", pageRoot);

    // graphics state
    graphicsState = addXrefEntry(-1);
    xprintf("<<\n"
            "/Type /ExtGState\n"
            "/SA true\n"
            "/SM 0.02\n"
            "/ca 1.0\n"
            "/CA 1.0\n"
            "/AIS false\n"
            "/SMask /None"
            ">>\n"
            "endobj\n");

    // color space for pattern
    patternColorSpace = addXrefEntry(-1);
    xprintf("[/Pattern /DeviceRGB]\n"
            "endobj\n");
}

void QPdfEnginePrivate::writeInfo()
{
    tm *newtime;

#if defined(Q_OS_WIN) && defined(_MSC_VER) && _MSC_VER >= 1400
    __time32_t now;
    _time32(&now);
    tm buffer;
    _gmtime32_s(&buffer, &now);
    newtime = &buffer;
#else
    time_t now;
    time(&now);
    newtime = gmtime(&now);
#endif
    QByteArray y;

    if (newtime && newtime->tm_year+1900 > 1992)
        y += QByteArray::number(newtime->tm_year+1900);

    info = addXrefEntry(-1);
    xprintf("<<\n"
            "/Title (%s)\n"
//            "/Author (%s)\n"
            "/Creator (%s)\n"
            "/Producer (Qt %s (C) 1992-%s $TROLLTECH$)\n",
            title.toUtf8().constData(),
//            author.toUtf8().constData(),
            creator.toUtf8().constData(),
            qVersion(), y.constData());

    if (newtime) {
        xprintf("/CreationDate (D:%d%02d%02d%02d%02d%02d)\n",
            newtime->tm_year+1900,
            newtime->tm_mon+1,
            newtime->tm_mday,
            newtime->tm_hour,
            newtime->tm_min,
            newtime->tm_sec);
    }
    xprintf(">>\n"
            "endobj\n");
}

void QPdfEnginePrivate::writePageRoot()
{
    addXrefEntry(pageRoot);

    xprintf("<<\n"
            "/Type /Pages\n"
            "/Kids \n"
            "[\n");
    int size = pages.size();
    for (int i = 0; i < size; ++i)
        xprintf("%d 0 R\n", pages[pageOrder == QPrinter::FirstPageFirst ? i : size-i-1]);
    xprintf("]\n");

    //xprintf("/Group <</S /Transparency /I true /K false>>\n");

    xprintf("/Count %d\n"
            "/MediaBox [%d %d %d %d]\n",
            pages.size(), 0, 0, width(), height());

    xprintf("/ProcSet [/PDF /Text /ImageB /ImageC]\n"
            ">>\n"
            "endobj\n");
}


void QPdfEnginePrivate::embedFont(QFontSubset *font)
{
    //qDebug() << "embedFont" << font->object_id;
    int fontObject = font->object_id;
    QByteArray fontData = font->toTruetype();
#ifdef FONT_DUMP
    static int i = 0;
    QString fileName("font%1.ttf");
    fileName = fileName.arg(i++);
    QFile ff(fileName);
    ff.open(QFile::WriteOnly);
    ff.write(fontData);
    ff.close();
#endif

    int fontDescriptor = requestObject();
    int fontstream = requestObject();
    int cidfont = requestObject();
    int toUnicode = requestObject();

    QFontEngine::Properties properties = font->fontEngine->properties();

    {
        qreal scale = 1000/properties.emSquare.toReal();
        addXrefEntry(fontDescriptor);
        QByteArray descriptor;
        QPdf::ByteStream s(&descriptor);
        s << "<< /Type /FontDescriptor\n"
            "/FontName /Q";
        int tag = fontDescriptor;
        for (int i = 0; i < 5; ++i) {
            s << (char)('A' + (tag % 26));
            tag /= 26;
        }
        s <<  "+" << properties.postscriptName << "\n"
            "/Flags " << 4 << "\n"
            "/FontBBox ["
          << properties.boundingBox.x()*scale
          << -(properties.boundingBox.y() + properties.boundingBox.height())*scale
          << (properties.boundingBox.x() + properties.boundingBox.width())*scale
          << -properties.boundingBox.y()*scale  << "]\n"
            "/ItalicAngle " << properties.italicAngle.toReal() << "\n"
            "/Ascent " << properties.ascent.toReal()*scale << "\n"
            "/Descent " << -properties.descent.toReal()*scale << "\n"
            "/CapHeight " << properties.capHeight.toReal()*scale << "\n"
            "/StemV " << properties.lineWidth.toReal()*scale << "\n"
            "/FontFile2 " << fontstream << "0 R\n"
            ">> endobj\n";
        write(descriptor);
    }
    {
        addXrefEntry(fontstream);
        QByteArray header;
        QPdf::ByteStream s(&header);

        int length_object = requestObject();
        s << "<<\n"
            "/Length1 " << fontData.size() << "\n"
            "/Length " << length_object << "0 R\n";
        if (do_compress)
            s << "/Filter /FlateDecode\n";
        s << ">>\n"
            "stream\n";
        write(header);
        int len = writeCompressed(fontData);
        write("endstream\n"
              "endobj\n");
        addXrefEntry(length_object);
        xprintf("%d\n"
                "endobj\n", len);
    }
    {
        addXrefEntry(cidfont);
        QByteArray cid;
        QPdf::ByteStream s(&cid);
        s << "<< /Type /Font\n"
            "/Subtype /CIDFontType2\n"
            "/BaseFont /" << properties.postscriptName << "\n"
            "/CIDSystemInfo << /Registry (Adobe) /Ordering (Identity) /Supplement 0 >>\n"
            "/FontDescriptor " << fontDescriptor << "0 R\n"
            "/CIDToGIDMap /Identity\n"
          << font->widthArray() <<
            ">>\n"
            "endobj\n";
        write(cid);
    }
    {
        addXrefEntry(toUnicode);
        QByteArray touc = font->createToUnicodeMap();
        xprintf("<< /Length %d >>\n"
                "stream\n", touc.length());
        write(touc);
        write("endstream\n"
              "endobj\n");
    }
    {
        addXrefEntry(fontObject);
        QByteArray font;
        QPdf::ByteStream s(&font);
        s << "<< /Type /Font\n"
            "/Subtype /Type0\n"
            "/BaseFont /" << properties.postscriptName << "\n"
            "/Encoding /Identity-H\n"
            "/DescendantFonts [" << cidfont << "0 R]\n"
            "/ToUnicode " << toUnicode << "0 R"
            ">>\n"
            "endobj\n";
        write(font);
    }
}


void QPdfEnginePrivate::writeFonts()
{
    for (QHash<QFontEngine::FaceId, QFontSubset *>::iterator it = fonts.begin(); it != fonts.end(); ++it) {
        embedFont(*it);
        delete *it;
    }
    fonts.clear();
}

void QPdfEnginePrivate::writePage()
{
    if (pages.empty())
        return;

    *currentPage << "Q Q\n";

    uint pageStream = requestObject();
    uint pageStreamLength = requestObject();
    uint resources = requestObject();

    addXrefEntry(pages.last());
    xprintf("<<\n"
            "/Type /Page\n"
            "/Parent %d 0 R\n"
            "/Contents %d 0 R\n"
            "/Resources %d 0 R\n"
            ">>\n"
            "endobj\n",
            pageRoot, pageStream, resources);


    addXrefEntry(resources);
    xprintf("<<\n"
            "/ColorSpace <<\n"
            "/PCSp %d 0 R\n"
            "/CSp /DeviceRGB\n"
            "/CSpg /DeviceGray\n"
            ">>\n"
            "/ExtGState <<\n"
            "/GSa %d 0 R\n",
            patternColorSpace, graphicsState);

    for (int i = 0; i < currentPage->graphicStates.size(); ++i)
        xprintf("/GState%d %d 0 R\n", currentPage->graphicStates.at(i), currentPage->graphicStates.at(i));
    xprintf(">>\n");

    xprintf("/Pattern <<\n");
    for (int i = 0; i < currentPage->patterns.size(); ++i)
        xprintf("/Pat%d %d 0 R\n", currentPage->patterns.at(i), currentPage->patterns.at(i));
    xprintf(">>\n");

    xprintf("/Font <<\n");
    for (int i = 0; i < currentPage->fonts.size();++i)
        xprintf("/F%d %d 0 R\n", currentPage->fonts[i], currentPage->fonts[i]);
    xprintf(">>\n");

    xprintf("/XObject <<\n");
    for (int i = 0; i<currentPage->images.size(); ++i) {
        xprintf("/Im%d %d 0 R\n", currentPage->images.at(i), currentPage->images.at(i));
    }
    xprintf(">>\n");

    xprintf(">>\n"
            "endobj\n");

    addXrefEntry(pageStream);
    xprintf("<<\n"
            "/Length %d 0 R\n", pageStreamLength); // object number for stream length object
    if (do_compress)
        xprintf("/Filter /FlateDecode\n");

    xprintf(">>\n");
    xprintf("stream\n");
    QByteArray content = currentPage->content();
    int len = writeCompressed(content);
    xprintf("endstream\n"
            "endobj\n");

    addXrefEntry(pageStreamLength);
    xprintf("%d\nendobj\n",len);
}

void QPdfEnginePrivate::writeTail()
{
    writePage();
    writeFonts();
    writePageRoot();
    addXrefEntry(xrefPositions.size(),false);
    xprintf("xref\n"
            "0 %d\n"
            "%010d 65535 f \n", xrefPositions.size()-1, xrefPositions[0]);

    for (int i = 1; i < xrefPositions.size()-1; ++i)
        xprintf("%010d 00000 n \n", xrefPositions[i]);

    xprintf("trailer\n"
            "<<\n"
            "/Size %d\n"
            "/Info %d 0 R\n"
            "/Root %d 0 R\n"
            ">>\n"
            "startxref\n%d\n"
            "%%%%EOF\n",
            xrefPositions.size()-1, info, catalog, xrefPositions.last());
}

int QPdfEnginePrivate::addXrefEntry(int object, bool printostr)
{
    if (object < 0)
        object = requestObject();

    if (object>=xrefPositions.size())
        xrefPositions.resize(object+1);

    xrefPositions[object] = streampos;
    if (printostr)
        xprintf("%d 0 obj\n",object);

    return object;
}

#endif // QT_NO_PRINTER
