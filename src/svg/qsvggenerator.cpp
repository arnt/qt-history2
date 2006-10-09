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

#include "qsvggenerator.h"

#include "qpainterpath.h"

#include "private/qpaintengine_p.h"
#include "private/qtextengine_p.h"

#include "qfile.h"
#include "qtextstream.h"
#include "qbuffer.h"

#include "qdebug.h"


static void translate_color(const QColor &color, QString *color_string,
                            QString *opacity_string)
{
    Q_ASSERT(color_string);
    Q_ASSERT(opacity_string);

    *color_string =
        QString("#%1%2%3")
        .arg(color.red(), 2, 16, QLatin1Char('0'))
        .arg(color.green(), 2, 16, QLatin1Char('0'))
        .arg(color.blue(), 2, 16, QLatin1Char('0'));
    *opacity_string = QString::number(color.alphaF());
}

class QSvgPaintEnginePrivate : public QPaintEnginePrivate
{
public:
    QSvgPaintEnginePrivate()
    {
        size = QSize(100, 100);
        outputDevice = 0;
        resolution = 72;

        attributes.document_title = "Qt Svg Document";
        attributes.document_description = "Generated with Qt";
        attributes.font_family = "serif";
        attributes.font_size = "10pt";
        attributes.font_style = "normal";
        attributes.font_weight = "normal";

        afterFirstUpdate = false;
        numGradients = 0;
    }

    QSize size;
    QIODevice *outputDevice;
    QTextStream *stream;
    int resolution;

    QString header;
    QString defs;
    QString body;
    bool    afterFirstUpdate;

    QBrush brush;
    QPen pen;
    QMatrix matrix;
    QFont font;

    QString generateGradientName() {
        ++numGradients;
        currentGradientName = QString("gradient%1").arg(numGradients);
        return currentGradientName;
    }

    QString currentGradientName;
    int numGradients;

    struct {
        QString document_title;
        QString document_description;
        QString font_weight;
        QString font_size;
        QString font_family;
        QString font_style;
        QString stroke, strokeOpacity;
        QString fill, fillOpacity;
    } attributes;
};

static inline QPaintEngine::PaintEngineFeatures svgEngineFeatures()
{
    return QPaintEngine::PaintEngineFeatures(
        QPaintEngine::AllFeatures
        & ~QPaintEngine::PerspectiveTransform
        & ~QPaintEngine::ConicalGradientFill
        & ~QPaintEngine::PorterDuff);

}

class QSvgPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QSvgPaintEngine);
public:

    QSvgPaintEngine()
        : QPaintEngine(*new QSvgPaintEnginePrivate,
                       svgEngineFeatures())
    {
    }

    bool begin(QPaintDevice *device);
    bool end();

    void updateState(const QPaintEngineState &state);
    void popGroup();

    void drawPath(const QPainterPath &path);
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawTextItem(const QPointF &pt, const QTextItem &item);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlag = Qt::AutoColor);

    QPaintEngine::Type type() const { return QPaintEngine::SVG; }

    QSize size() const { return d_func()->size; }
    void setSize(const QSize &size) {
        Q_ASSERT(!isActive());
        d_func()->size = size;
    }

    QIODevice *outputDevice() const { return d_func()->outputDevice; }
    void setOutputDevice(QIODevice *device) {
        Q_ASSERT(!isActive());
        d_func()->outputDevice = device;
    }

    int resolution() { return d_func()->resolution; }
    void setResolution(int resolution) {
        Q_ASSERT(!isActive());
        d_func()->resolution = resolution;
    }
    void saveLinearGradientBrush(const QGradient *g)
    {
        QTextStream str(&d_func()->defs, QIODevice::Append);
        str << "<linearGradient gradientUnits=\"userSpaceOnUse\" ";
        const QLinearGradient *grad = static_cast<const QLinearGradient*>(g);
        if (grad) {
            str << "x1 = \""<<grad->start().x()<<"\" "
                << "y1 = \""<<grad->start().y()<<"\" "
                << "x2 = \""<<grad->finalStop().x() <<"\" "
                << "y2 = \""<<grad->finalStop().y() <<"\" ";
        }

        str << "id=\""<< d_func()->generateGradientName() << "\">\n";
        saveGradientStops(str, g);
        str << "</linearGradient>" <<endl;
    }
    void saveRadialGradientBrush(const QGradient *g)
    {
        QTextStream str(&d_func()->defs, QIODevice::Append);
        str << "<radialGradient gradientUnits=\"userSpaceOnUse\"";
        const QRadialGradient *grad = static_cast<const QRadialGradient*>(g);
        if (grad) {
            str << "cx = \""<<grad->center().x()<<"\" "
                << "cy = \""<<grad->center().y()<<"\" "
                << "r = \""<<grad->radius() <<"\" "
                << "fx = \""<<grad->focalPoint().x() <<"\" "
                << "fy = \""<<grad->focalPoint().y() <<"\" ";
        }
        str << " xml:id=\""<<d_func()->generateGradientName()<< "\">\n";
        saveGradientStops(str, g);
        str << "</radialGradient>" << endl;
    }
    void saveConicalGradientBrush(const QGradient *)
    {
        qWarning("svg's don't support conical gradients!");
    }
    void saveTextureBrush(const QBrush &)
    {
        qWarning("texture brushes not yet supported");
    }
    void saveGradientStops(QTextStream &str, const QGradient *g) {
        QGradientStops stops = g->stops();
        foreach(QGradientStop stop, stops) {
            QString color =
                QString("#%1%2%3")
                .arg(stop.second.red(), 2, 16, QLatin1Char('0'))
                .arg(stop.second.green(), 2, 16, QLatin1Char('0'))
                .arg(stop.second.blue(), 2, 16, QLatin1Char('0'));
            str << "    <stop offset=\""<< stop.first << "\" "
                << "stop-color=\""<< color << "\" "
                << "stop-opacity=\"" << stop.second.alphaF() <<"\" />\n";
        }
    }
    void generateQtDefaults()
    {
        *d_func()->stream << "fill=\"none\" ";
        *d_func()->stream << "stroke=\"black\" ";
        *d_func()->stream << "vector-effect=\"non-scaling-stroke\" ";
        *d_func()->stream << "stroke-width=\"1\" ";
        *d_func()->stream << "fill-rule=\"evenodd\" ";
        *d_func()->stream << "stroke-linecap=\"square\" ";
        *d_func()->stream << "stroke-linejoin=\"bevel\" ";
        *d_func()->stream << ">\n";
    }
    inline QTextStream &stream()
    {
        return *d_func()->stream;
    }


    void qpenToSvg(const QPen &spen)
    {
        QString width;

        d_func()->pen = spen;

        switch (spen.style()) {
        case Qt::NoPen:
            stream() << "stroke=\"none\" ";

            d_func()->attributes.stroke = QLatin1String("none");
            d_func()->attributes.strokeOpacity = QString();
            return;
            break;
        case Qt::SolidLine: {
            QString color, colorOpacity;

            translate_color(spen.color(), &color,
                            &colorOpacity);
            d_func()->attributes.stroke = color;
            d_func()->attributes.strokeOpacity = colorOpacity;

            stream() <<"stroke=\""<<color<<"\" ";
            stream() <<"stroke-opacity=\""<<colorOpacity<<"\" ";
        }
            break;
        case Qt::DashLine:
        case Qt::DotLine:
        case Qt::DashDotLine:
        case Qt::DashDotDotLine:
        case Qt::CustomDashLine:
        default:
            qWarning("Unsupported pen style");
            break;
        }

        if (spen.widthF() == 0) {
            width = QLatin1String("1");
            stream() << "vector-effect=\"non-scaling-stroke\" ";
        }
        else
            width = QString::number(spen.widthF());
        stream() <<"stroke-width=\""<<width<<"\" ";

        switch (spen.capStyle()) {
        case Qt::FlatCap:
            stream() << "stroke-linecap=\"butt\" ";
            break;
        case Qt::SquareCap:
            stream() << "stroke-linecap=\"square\" ";
            break;
        case Qt::RoundCap:
            stream() << "stroke-linecap=\"round\" ";
            break;
        default:
            qWarning("Unhandled cap style");
        }
        switch (spen.joinStyle()) {
        case Qt::MiterJoin:
            stream() << "stroke-linejoin=\"miter\" ";
            stream() << "stroke-miterlimit=\""<<spen.miterLimit()<<"\" ";
            break;
        case Qt::BevelJoin:
            stream() << "stroke-linejoin=\"bevel\" ";
            break;
        case Qt::RoundJoin:
            stream() << "stroke-linejoin=\"round\" ";
            break;
        case Qt::SvgMiterJoin:
            stream() << "stroke-linejoin=\"miter\" ";
            stream() << "stroke-miterlimit=\""<<spen.miterLimit()<<"\" ";
            break;
        default:
            qWarning("Unhandled join style");
        }
    }
    void qbrushToSvg(const QBrush &sbrush)
    {
        d_func()->brush = sbrush;
        switch (sbrush.style()) {
        case Qt::SolidPattern: {
            QString color, colorOpacity;
            translate_color(sbrush.color(), &color, &colorOpacity);
            stream() << "fill=\"" << color << "\" ";
            stream() << "fill-opacity=\""
                     << colorOpacity << "\" ";
            d_func()->attributes.fill = color;
            d_func()->attributes.fillOpacity = colorOpacity;
        }
            break;
        case Qt::LinearGradientPattern:
            saveLinearGradientBrush(sbrush.gradient());
            d_func()->attributes.fill = QString("url(#%1)").arg(d_func()->currentGradientName);
            d_func()->attributes.fillOpacity = QString();
            stream() << "fill=\"url(#" << d_func()->currentGradientName << ")\" ";
            break;
        case Qt::RadialGradientPattern:
            saveRadialGradientBrush(sbrush.gradient());
            d_func()->attributes.fill = QString("url(#%1)").arg(d_func()->currentGradientName);
            d_func()->attributes.fillOpacity = QString();
            stream() << "fill=\"url(#" << d_func()->currentGradientName << ")\" ";
            break;
        case Qt::ConicalGradientPattern:
            saveConicalGradientBrush(sbrush.gradient());
            d_func()->attributes.fill = QString("url(#%1)").arg(d_func()->currentGradientName);
            d_func()->attributes.fillOpacity = QString();
            stream() << "fill=\"url(#" << d_func()->currentGradientName << ")\" ";
            break;
        case Qt::TexturePattern:
            saveTextureBrush(sbrush);
            break;
        case Qt::NoBrush:
            stream() << "fill=\"none\" ";
            d_func()->attributes.fill = QString("none");
            d_func()->attributes.fillOpacity = QString();
            return;
            break;
        default:
            qWarning("unhandled brush style");
            break;
        }
    }
    void qfontToSvg(const QFont &sfont)
    {
        Q_D(QSvgPaintEngine);
        
        d->font = sfont;
        d->attributes.font_size = QString::number(d->font.pointSize()) + "pt";
        d->attributes.font_weight = QString::number(d->font.weight() * 10);
        d->attributes.font_family = d->font.family();
        d->attributes.font_style = d->font.italic() ? "italic" : "normal";

        *d->stream << "font-family=\"" << d->attributes.font_family << "\" "
                   << "font-size=\"" << d->attributes.font_size << "\" "
                   << "font-weight=\"" << d->attributes.font_weight << "\" "
                   << "font-style=\"" << d->attributes.font_style << "\" "
                   << endl;
    }
};

class QSvgGeneratorPrivate
{
public:
    QSvgPaintEngine *engine;

    uint owns_iodevice : 1;
};

QSvgGenerator::QSvgGenerator()
    : d_ptr(new QSvgGeneratorPrivate)
{
    Q_D(QSvgGenerator);

    d->engine = new QSvgPaintEngine;
    d->owns_iodevice = false;
}

QSvgGenerator::~QSvgGenerator()
{
    Q_D(QSvgGenerator);
    if (d->owns_iodevice)
        delete d->engine->outputDevice();
    delete d->engine;
}

/*!
  Returns the size of the generated svg.
*/
QSize QSvgGenerator::size() const
{
    return d_func()->engine->size();
}

/*!
  Sets the size of the generated svg to \a size.

  It is not possible to set the size while the svg is being generated
*/
void QSvgGenerator::setSize(const QSize &size)
{
    Q_D(QSvgGenerator);
    if (d->engine->isActive()) {
        qWarning("QSvgGenerator::setSize(), cannot set size while svg is being generated");
        return;
    }
    d->engine->setSize(size);
}

/*!
  Sets the target filename for generated svgs to \a fileName.

  \sa setOutputDevice()
*/
void QSvgGenerator::setFileName(const QString &fileName)
{
    Q_D(QSvgGenerator);
    if (d->engine->isActive()) {
        qWarning("QSvgGenerator::setFileName(), cannot set fileName svg is being generated");
        return;
    }

    if (d->owns_iodevice)
        delete d->engine->outputDevice();

    d->owns_iodevice = true;

    QFile *file = new QFile(fileName);
    d->engine->setOutputDevice(file);
}

/*!
  Returns the target output device for generated svgs.
*/
QIODevice *QSvgGenerator::outputDevice() const
{
    Q_D(const QSvgGenerator);
    return d->engine->outputDevice();
}

/*!
  Sets the output device for generated svgs to \a outputDevice.

  If both output device and file name are specified, the output device
  will have precedence.
*/
void QSvgGenerator::setOutputDevice(QIODevice *outputDevice)
{
    Q_D(QSvgGenerator);
    if (d->engine->isActive()) {
        qWarning("QSvgGenerator::setOutputDevice(), cannot set output device svg is being generated");
        return;
    }
    d->owns_iodevice = false;
    d->engine->setOutputDevice(outputDevice);
}

QPaintEngine *QSvgGenerator::paintEngine() const
{
    Q_D(const QSvgGenerator);
    return d->engine;
}

int QSvgGenerator::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    Q_D(const QSvgGenerator);
    switch (metric) {
    case QPaintDevice::PdmDepth:
        return 32;
    case QPaintDevice::PdmWidth:
        return d->engine->size().width();
    case QPaintDevice::PdmHeight:
        return d->engine->size().height();
    case QPaintDevice::PdmDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmDpiY:
        return d->engine->resolution();
    case QPaintDevice::PdmHeightMM:
        return int(d->engine->size().height() / d->engine->resolution() * 2.54);
    case QPaintDevice::PdmWidthMM:
        return int(d->engine->size().width() / d->engine->resolution() * 2.54);
    case QPaintDevice::PdmNumColors:
        return 0xffffffff;
    case QPaintDevice::PdmPhysicalDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmPhysicalDpiY:
        return d->engine->resolution();
    default:
        qWarning("QSvgGenerator::metric(), unhandled metric %d\n", metric);
        break;
    }
    return 0;
}

/*****************************************************************************
 * class QSvgPaintEngine
 */

bool QSvgPaintEngine::begin(QPaintDevice *)
{
    Q_D(QSvgPaintEngine);
    if (!d->outputDevice) {
        qWarning("QSvgPaintEngine::begin(), no output device");
        return false;
    }

    if (!d->outputDevice->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("QSvgPaintEngine::begin(), could not open output device: '%s'",
                 qPrintable(d->outputDevice->errorString()));
        return false;
    }

    d->stream = new QTextStream(&d->header);

    int w = d->size.width();
    int h = d->size.height();

    qreal wmm = w * 2.54 / d->resolution;
    qreal hmm = h * 2.54 / d->resolution;

    // stream out the header...
    *d->stream << "<?xml version=\"1.0\" standalone=\"no\"?>" << endl;
    *d->stream << "<svg width=\"" << wmm << "cm\" height=\"" << hmm << "cm\"" << endl;
    *d->stream << " viewBox=\"0 0 " << w << " " << h << "\"" << endl;
    *d->stream << " xmlns=\"http://www.w3.org/2000/svg\""
               << " xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
               << " version=\"1.2\" baseProfile=\"tiny\">" << endl;

    if (!d->attributes.document_title.isEmpty()) {
        *d->stream << "<title>" << d->attributes.document_title << "</title>" << endl;
    }

    if (!d->attributes.document_description.isEmpty()) {
        *d->stream << "<desc>" << d->attributes.document_description << "</desc>" << endl;
    }

    d->stream->setString(&d->defs);
    *d->stream << "<defs>\n";

    d->stream->setString(&d->body);
    // Start the initial graphics state...
    *d->stream << "<g ";
    generateQtDefaults();
    *d->stream << endl;

    return true;
}

bool QSvgPaintEngine::end()
{
    Q_D(QSvgPaintEngine);


    d->stream->setString(&d->defs);
    *d->stream << "</defs>\n";

    d->stream->setDevice(d->outputDevice);
    *d->stream << d->header;
    *d->stream << d->defs;
    *d->stream << d->body;
    if (d->afterFirstUpdate)
        *d->stream << "</g>" << endl; // close the updateState

    *d->stream << "</g>" << endl // close the Qt defaults
               << "</svg>" << endl;

    delete d->stream;

    return true;
}

void QSvgPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm,
                                 const QRectF &sr)
{
    drawImage(r, pm.toImage(), sr);
}

void QSvgPaintEngine::drawImage(const QRectF &r, const QImage &image,
                                const QRectF &sr,
                                Qt::ImageConversionFlag flags)
{
    //Q_D(QSvgPaintEngine);

    Q_UNUSED(sr);
    Q_UNUSED(flags);
    stream() << "<image ";
    stream() << "x=\""<<r.x()<<"\" ";
    stream() << "y=\""<<r.y()<<"\" ";
    stream() << "width=\""<<r.width()<<"\" ";
    stream() << "height=\""<<r.height()<<"\" ";

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadWrite);
    image.save(&buffer, "PNG");
    buffer.close();
    stream() << "xlink:href=\"data:image/png;base64,"
             << data.toBase64()
             <<"\" ";
    stream() << "/>\n";
}

void QSvgPaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QSvgPaintEngine);
    QPaintEngine::DirtyFlags flags = state.state();

    // always stream full gstate, which is not required, but...
    flags |= QPaintEngine::AllDirty;

    // close old state and start a new one...
    if (d->afterFirstUpdate)
        *d->stream << "</g>\n\n";

    *d->stream << "<g ";

    if (flags & QPaintEngine::DirtyBrush) {
        qbrushToSvg(state.brush());
    }

    if (flags & QPaintEngine::DirtyPen) {
        qpenToSvg(state.pen());
    }

    if (flags & QPaintEngine::DirtyTransform) {
        d->matrix = state.matrix();
        *d->stream << "transform=\"matrix(" << d->matrix.m11() << ","
                   << d->matrix.m12() << ","
                   << d->matrix.m21() << "," << d->matrix.m22() << ","
                   << d->matrix.dx() << "," << d->matrix.dy()
                   << ")\""
                   << endl;
    }

    if (flags & QPaintEngine::DirtyFont) {
        qfontToSvg(state.font());
    }

    if (flags & QPaintEngine::DirtyOpacity) {
        if (!qFuzzyCompare(state.opacity(), 1))
            stream() << "opacity=\""<<state.opacity()<<"\" ";
    }

    *d->stream << ">" << endl;

    d->afterFirstUpdate = true;
}

void QSvgPaintEngine::drawPath(const QPainterPath &p)
{
    Q_D(QSvgPaintEngine);

    *d->stream << "<path ";


    *d->stream << "fill-rule=";
    if (p.fillRule() == Qt::OddEvenFill)
        *d->stream << "\"evenodd\" ";
    else
        *d->stream << "\"nonzero\" ";

    *d->stream << "d=\"";

    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &e = p.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            *d->stream << "M" << e.x << "," << e.y;
            break;
        case QPainterPath::LineToElement:
            *d->stream << "L" << e.x << "," << e.y;
            break;
        case QPainterPath::CurveToElement:
            *d->stream << "C" << e.x << "," << e.y;
            ++i;
            while (i < p.elementCount()) {
                const QPainterPath::Element &e = p.elementAt(i);
                if (e.type != QPainterPath::CurveToDataElement) {
                    --i;
                    break;
                } else
                    *d->stream << " ";
                *d->stream << e.x << "," << e.y;
                ++i;
            }
            break;
        default:
            break;
        }
        if (i != p.elementCount() - 1) {
            *d->stream << " ";
        }
    }

    *d->stream << "\"/>" << endl;
}

void QSvgPaintEngine::drawPolygon(const QPointF *points, int pointCount,
                                  PolygonDrawMode mode)
{
    Q_ASSERT(pointCount >= 2);

    //Q_D(QSvgPaintEngine);

    QPainterPath path(points[0]);
    for (int i=1; i<pointCount; ++i)
        path.lineTo(points[i]);

    if (mode == PolylineMode) {
        stream() << "<polyline points=\"";
        for (int i = 0; i < pointCount; ++i) {
            const QPointF &pt = points[i];
            stream() << pt.x() << "," << pt.y() << " ";
        }
        stream() << "\" />" <<endl;
    } else {
        path.closeSubpath();
        drawPath(path);
    }
}

void QSvgPaintEngine::drawTextItem(const QPointF &pt, const QTextItem &textItem)
{
    Q_D(QSvgPaintEngine);
    if (d->pen.style() == Qt::NoPen)
        return;

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QString s = QString::fromRawData(ti.chars, ti.num_chars);

    *d->stream << "<text "
               << "fill=\"" << d->attributes.stroke << "\" "
               << "fill-opacity=\"" << d->attributes.strokeOpacity << "\" "
               << "stroke=\"none\" "
               << "x=\"" << pt.x() << "\" y=\"" << pt.y() << "\" ";
    qfontToSvg(textItem.font());
    *d->stream << " >"
               << Qt::escape(s)
               << "</text>"
               << endl;
}
