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

#include <QtGui/qprintengine.h>

#ifndef QT_NO_PRINTER
#include <qiodevice.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpainterpath.h>
#include <qpaintdevice.h>
#include <qfile.h>
#include <qdebug.h>
#include <qendian.h>

#include <time.h>
#include <limits.h>
#include <math.h>
#ifndef QT_NO_COMPRESS
#include <zlib.h>
#endif

#include "qprintengine_pdf_p.h"
#include "private/qdrawhelper_p.h"

// might be helpful for smooth transforms of images
// Can't use it though, as gs generates completely wrong images if this is true.
static const bool interpolateImages = false;

#ifdef QT_NO_COMPRESS
static const bool do_compress = false;
#else
static const bool do_compress = true;
#endif

static const int resolution = 72;

static inline const char *toHex(ushort u, char *buffer)
{
    int i = 3;
    while (i >= 0) {
        ushort hex = (u & 0x000f);
        if (hex < 0x0a)
            buffer[i] = '0'+hex;
        else
            buffer[i] = 'A'+(hex-0x0a);
        u = u >> 4;
        i--;
    }
    buffer[4] = '\0';
    return buffer;
}



/* also adds a space at the end of the number */
const char *qt_real_to_string(qreal val, char *buf) {
    const char *ret = buf;
    if (val < 0) {
        *(buf++) = '-';
        val = -val;
    }
    int ival = (int) val;
    qreal frac = val - (qreal)ival;

    int ifrac = (int)(frac * 1000000);
    if (ifrac == 1000000) {
        ++ival;
        ifrac = 0;
    }
    char output[256];
    int i = 0;
    while (ival) {
        output[i] = '0' + (ival % 10);
        ++i;
        ival /= 10;
    }
    int fact = 100000;
    if (i == 0) {
        *(buf++) = '0';
    } else {
        while (i) {
            *(buf++) = output[--i];
            fact /= 10;
            ifrac /= 10;
        }
    }

    if (ifrac) {
        *(buf++) =  '.';
        while (fact) {
            *(buf++) = '0' + ((ifrac/fact) % 10);
            fact /= 10;
        }
    }
    *(buf++) = ' ';
    *buf = 0;
    return ret;
}

const char *qt_int_to_string(int val, char *buf) {
    const char *ret = buf;
    if (val < 0) {
        *(buf++) = '-';
        val = -val;
    }
    char output[256];
    int i = 0;
    while (val) {
        output[i] = '0' + (val % 10);
        ++i;
        val /= 10;
    }
    if (i == 0) {
        *(buf++) = '0';
    } else {
        while (i)
            *(buf++) = output[--i];
    }
    *(buf++) = ' ';
    *buf = 0;
    return ret;
}

#define QT_PATH_ELEMENT(elm)

QByteArray QPdf::generatePath(const QPainterPath &path, const QMatrix &matrix, PathFlags flags)
{
    QByteArray result;
    if (!path.elementCount())
        return result;

    ByteStream s(&result);

    int start = -1;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &elm = path.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (start >= 0
                && path.elementAt(start).x == path.elementAt(i-1).x
                && path.elementAt(start).y == path.elementAt(i-1).y)
                s << "h\n";
            s << matrix.map(QPointF(elm.x, elm.y)) << "m\n";
            start = i;
                break;
        case QPainterPath::LineToElement:
            s << matrix.map(QPointF(elm.x, elm.y)) << "l\n";
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(path.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(path.elementAt(i+2).type == QPainterPath::CurveToDataElement);
            s << matrix.map(QPointF(elm.x, elm.y))
              << matrix.map(QPointF(path.elementAt(i+1).x, path.elementAt(i+1).y))
              << matrix.map(QPointF(path.elementAt(i+2).x, path.elementAt(i+2).y))
              << "c\n";
            i += 2;
            break;
        default:
            qFatal("QPdf::generatePath(), unhandled type: %d", elm.type);
        }
    }
    if (start >= 0
        && path.elementAt(start).x == path.elementAt(path.elementCount()-1).x
        && path.elementAt(start).y == path.elementAt(path.elementCount()-1).y)
        s << "h\n";

    Qt::FillRule fillRule = path.fillRule();

    const char *op = 0;
    switch (flags) {
    case ClipPath:
        op = (fillRule == Qt::WindingFill) ? "W n\n" : "W* n\n";
        break;
    case FillPath:
        op = (fillRule == Qt::WindingFill) ? "f\n" : "f*\n";
        break;
    case StrokePath:
        op = "S\n";
        break;
    case FillAndStrokePath:
        op = (fillRule == Qt::WindingFill) ? "B\n" : "B*\n";
        break;
    }
    s << op;
    return result;
}

QByteArray QPdf::generateMatrix(const QMatrix &matrix)
{
    QByteArray result;
    ByteStream s(&result);
    s << matrix.m11()
      << matrix.m12()
      << matrix.m21()
      << matrix.m22()
      << matrix.dx()
      << matrix.dy()
      << "cm\n";
    return result;
}

QByteArray QPdf::generateDashes(const QPen &pen)
{
    QByteArray result;
    ByteStream s(&result);
    s << "[";

    QVector<qreal> dasharray = pen.dashPattern();
    qreal w = pen.widthF();
    if (w < 0.001)
        w = 1;
    for (int i = 0; i < dasharray.size(); ++i) {
        qreal dw = dasharray.at(i)*w;
        if (dw < 0.0001) dw = 0.0001;
        s << dw;
    }
    s << "]";
    //qDebug() << "dasharray: pen has" << dasharray;
    //qDebug() << "  => " << result;
    return result;
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

QByteArray QPdf::patternForBrush(const QBrush &b)
{
    int style = b.style();
    if (style > Qt::DiagCrossPattern)
        return QByteArray();
    return pattern_for_brush[style];
}

#ifdef USE_NATIVE_GRADIENTS
static void writeTriangleLine(uchar *&data, int xpos, int ypos, int xoff, int yoff, uint rgb, uchar flag, bool alpha)
{
    data[0] =  flag;
    data[1] = (uchar)(xpos >> 16);
    data[2] = (uchar)(xpos >> 8);
    data[3] = (uchar)(xpos >> 0);
    data[4] = (uchar)(ypos >> 16);
    data[5] = (uchar)(ypos >> 8);
    data[6] = (uchar)(ypos >> 0);
    data += 7;
    if (alpha) {
        *data++ = (uchar)qAlpha(rgb);
    } else {
        *data++ = (uchar)qRed(rgb);
        *data++ = (uchar)qGreen(rgb);
        *data++ = (uchar)qBlue(rgb);
    }
    xpos += xoff;
    ypos += yoff;
    data[0] =  flag;
    data[1] = (uchar)(xpos >> 16);
    data[2] = (uchar)(xpos >> 8);
    data[3] = (uchar)(xpos >> 0);
    data[4] = (uchar)(ypos >> 16);
    data[5] = (uchar)(ypos >> 8);
    data[6] = (uchar)(ypos >> 0);
    data += 7;
    if (alpha) {
        *data++ = (uchar)qAlpha(rgb);
    } else {
        *data++ = (uchar)qRed(rgb);
        *data++ = (uchar)qGreen(rgb);
        *data++ = (uchar)qBlue(rgb);
    }
}


QByteArray QPdf::generateLinearGradientShader(const QLinearGradient *gradient, const QPointF *page_rect, bool alpha)
{
    // generate list of triangles with colors
    QPointF start = gradient->start();
    QPointF stop = gradient->finalStop();
    QGradientStops stops = gradient->stops();
    QPointF offset = stop - start;
    QGradient::Spread spread = gradient->spread();

    if (gradient->spread() == QGradient::ReflectSpread) {
        offset *= 2;
        for (int i = stops.size() - 2; i >= 0; --i) {
            QGradientStop stop = stops.at(i);
            stop.first = 2. - stop.first;
            stops.append(stop);
        }
        for (int i = 0 ; i < stops.size(); ++i)
            stops[i].first /= 2.;
    }

    QPointF orthogonal(offset.y(), -offset.x());
    qreal length = offset.x()*offset.x() + offset.y()*offset.y();

    // find the max and min values in offset and orth direction that are needed to cover
    // the whole page
    int off_min = INT_MAX;
    int off_max = INT_MIN;
    qreal ort_min = INT_MAX;
    qreal ort_max = INT_MIN;
    for (int i = 0; i < 4; ++i) {
        qreal off = ((page_rect[i].x() - start.x()) * offset.x() + (page_rect[i].y() - start.y()) * offset.y())/length;
        qreal ort = ((page_rect[i].x() - start.x()) * orthogonal.x() + (page_rect[i].y() - start.y()) * orthogonal.y())/length;
        off_min = qMin(off_min, (int)floor(off));
        off_max = qMax(off_max, (int)ceil(off));
        ort_min = qMin(ort_min, ort);
        ort_max = qMax(ort_max, ort);
    }
    ort_min -= 1;
    ort_max += 1;

    start += off_min * offset + ort_min * orthogonal;
    orthogonal *= (ort_max - ort_min);
    int num = off_max - off_min;

    QPointF gradient_rect[4] = { start,
                                 start + orthogonal,
                                 start + num*offset,
                                 start + num*offset + orthogonal };
    qreal xmin = gradient_rect[0].x();
    qreal xmax = gradient_rect[0].x();
    qreal ymin = gradient_rect[0].y();
    qreal ymax = gradient_rect[0].y();
    for (int i = 1; i < 4; ++i) {
        xmin = qMin(xmin, gradient_rect[i].x());
        xmax = qMax(xmax, gradient_rect[i].x());
        ymin = qMin(ymin, gradient_rect[i].y());
        ymax = qMax(ymax, gradient_rect[i].y());
    }
    xmin -= 1000;
    xmax += 1000;
    ymin -= 1000;
    ymax += 1000;
    start -= QPointF(xmin, ymin);
    qreal factor_x = qreal(1<<24)/(xmax - xmin);
    qreal factor_y = qreal(1<<24)/(ymax - ymin);
    int xoff = (int)(orthogonal.x()*factor_x);
    int yoff = (int)(orthogonal.y()*factor_y);

    QByteArray triangles;
    triangles.resize(spread == QGradient::PadSpread ? 20*(stops.size()+2) : 20*num*stops.size());
    uchar *data = (uchar *) triangles.data();
    if (spread == QGradient::PadSpread) {
        if (off_min > 0 || off_max < 1) {
            // linear gradient outside of page
            const QGradientStop &current_stop = off_min > 0 ? stops.at(stops.size()-1) : stops.at(0);
            uint rgb = current_stop.second.rgba();
            int xpos = (int)(start.x()*factor_x);
            int ypos = (int)(start.y()*factor_y);
            writeTriangleLine(data, xpos, ypos, xoff, yoff, rgb, 0, alpha);
            start += num*offset;
            xpos = (int)(start.x()*factor_x);
            ypos = (int)(start.y()*factor_y);
            writeTriangleLine(data, xpos, ypos, xoff, yoff, rgb, 1, alpha);
        } else {
            int flag = 0;
            if (off_min < 0) {
                uint rgb = stops.at(0).second.rgba();
                int xpos = (int)(start.x()*factor_x);
                int ypos = (int)(start.y()*factor_y);
                writeTriangleLine(data, xpos, ypos, xoff, yoff, rgb, flag, alpha);
                start -= off_min*offset;
                flag = 1;
            }
            for (int s = 0; s < stops.size(); ++s) {
                const QGradientStop &current_stop = stops.at(s);
                uint rgb = current_stop.second.rgba();
                int xpos = (int)(start.x()*factor_x);
                int ypos = (int)(start.y()*factor_y);
                writeTriangleLine(data, xpos, ypos, xoff, yoff, rgb, flag, alpha);
                if (s < stops.size()-1)
                    start += offset*(stops.at(s+1).first - stops.at(s).first);
                flag = 1;
            }
            if (off_max > 1) {
                start += (off_max - 1)*offset;
                uint rgb = stops.at(stops.size()-1).second.rgba();
                int xpos = (int)(start.x()*factor_x);
                int ypos = (int)(start.y()*factor_y);
                writeTriangleLine(data, xpos, ypos, xoff, yoff, rgb, flag, alpha);
            }
        }
    } else {
        for (int i = 0; i < num; ++i) {
            uchar flag = 0;
            for (int s = 0; s < stops.size(); ++s) {
                uint rgb = stops.at(s).second.rgba();
                int xpos = (int)(start.x()*factor_x);
                int ypos = (int)(start.y()*factor_y);
                writeTriangleLine(data, xpos, ypos, xoff, yoff, rgb, flag, alpha);
                if (s < stops.size()-1)
                    start += offset*(stops.at(s+1).first - stops.at(s).first);
                flag = 1;
            }
        }
    }
    triangles.resize((char *)data - triangles.constData());

    QByteArray shader;
    QPdf::ByteStream s(&shader);
    s << "<<\n"
        "/ShadingType 4\n"
        "/ColorSpace " << (alpha ? "/DeviceGray\n" : "/DeviceRGB\n") <<
        "/AntiAlias true\n"
        "/BitsPerCoordinate 24\n"
        "/BitsPerComponent 8\n"
        "/BitsPerFlag 8\n"
        "/Decode [" << xmin << xmax << ymin << ymax << (alpha ? "0 1]\n" : "0 1 0 1 0 1]\n") <<
        "/AntiAlias true\n"
        "/Length " << triangles.length() << "\n"
        ">>\n"
        "stream\n" << triangles << "endstream\n"
        "endobj\n";
    return shader;
}
#endif

static void moveToHook(qfixed x, qfixed y, void *data)
{
    QPdf::Stroker *t = (QPdf::Stroker *)data;
    if (!t->first)
        *t->stream << "h\n";
    if (!t->zeroWidth)
        t->matrix.map(x, y, &x, &y);
    *t->stream << x << y << "m\n";
    t->first = false;
}

static void lineToHook(qfixed x, qfixed y, void *data)
{
    QPdf::Stroker *t = (QPdf::Stroker *)data;
    if (!t->zeroWidth)
        t->matrix.map(x, y, &x, &y);
    *t->stream << x << y << "l\n";
}

static void cubicToHook(qfixed c1x, qfixed c1y,
                        qfixed c2x, qfixed c2y,
                        qfixed ex, qfixed ey,
                        void *data)
{
    QPdf::Stroker *t = (QPdf::Stroker *)data;
    if (!t->zeroWidth) {
        t->matrix.map(c1x, c1y, &c1x, &c1y);
        t->matrix.map(c2x, c2y, &c2x, &c2y);
        t->matrix.map(ex, ey, &ex, &ey);
    }
    *t->stream << c1x << c1y
               << c2x << c2y
               << ex << ey
               << "c\n";
}

QPdf::Stroker::Stroker()
    : dashStroker(&basicStroker)
{
    stroker = &basicStroker;
    basicStroker.setMoveToHook(moveToHook);
    basicStroker.setLineToHook(lineToHook);
    basicStroker.setCubicToHook(cubicToHook);
    zeroWidth = true;
    basicStroker.setStrokeWidth(.1);
}

void QPdf::Stroker::setPen(const QPen &pen)
{
    if (pen.style() == Qt::NoPen) {
        stroker = 0;
        return;
    }
    qreal w = pen.widthF();
    zeroWidth = (w < 0.0001);
    if (zeroWidth)
        w = .1;

    basicStroker.setStrokeWidth(w);
    basicStroker.setCapStyle(pen.capStyle());
    basicStroker.setJoinStyle(pen.joinStyle());
    basicStroker.setMiterLimit(pen.miterLimit());

    QVector<qreal> dashpattern = pen.dashPattern();
    if (zeroWidth) {
        for (int i = 0; i < dashpattern.size(); ++i)
            dashpattern[i] *= 10.;
    }
    if (!dashpattern.isEmpty()) {
        dashStroker.setDashPattern(dashpattern);
        stroker = &dashStroker;
    } else {
        stroker = &basicStroker;
    }
}

void QPdf::Stroker::strokePath(const QPainterPath &path)
{
    if (!stroker)
        return;
    first = true;
    stroker->strokePath(path, this, zeroWidth ? matrix : QMatrix());
    *stream << "h f\n";
}

// ------------------------------ Truetype embedding support

typedef qint16 F2DOT14;
typedef quint32 Tag;
typedef quint16 GlyphID;
typedef quint16 Offset;


class QTtfStream {
public:
    QTtfStream(QByteArray &ba) : data((uchar *)ba.data()) { start = data; }
    QTtfStream &operator <<(quint8 v) { *data = v; ++data; return *this; }
    QTtfStream &operator <<(quint16 v) { qToBigEndian(v, data); data += sizeof(v); return *this; }
    QTtfStream &operator <<(quint32 v) { qToBigEndian(v, data); data += sizeof(v); return *this; }
    QTtfStream &operator <<(qint8 v) { *data = quint8(v); ++data; return *this; }
    QTtfStream &operator <<(qint16 v) { qToBigEndian(v, data); data += sizeof(v); return *this; }
    QTtfStream &operator <<(qint32 v) { qToBigEndian(v, data); data += sizeof(v); return *this; }
    QTtfStream &operator <<(qint64 v) { qToBigEndian(v, data); data += sizeof(v); return *this; }

    int offset() const { return data - start; }
    void setOffset(int o) { data = start + o; }
    void align4() { while (offset() & 3) { *data = '\0'; ++data; } }
private:
    uchar *data;
    uchar *start;
};

#define MAKE_TAG( str )                      \
    ( ( (quint32)str[0] << 24 ) |           \
      ( (quint32)str[1] << 16 ) |           \
      ( (quint32)str[2] <<  8 ) |           \
      (quint32)str[3]         )

struct QTtfTable {
    Tag tag;
    QByteArray data;
};
Q_DECLARE_TYPEINFO(QTtfTable, Q_MOVABLE_TYPE);


struct qttf_head_table {
    qint32 font_revision;
    quint16 flags;
    qint64 created;
    qint64 modified;
    qint16 xMin;
    qint16 yMin;
    qint16 xMax;
    qint16 yMax;
    quint16 macStyle;
    qint16 indexToLocFormat;
};


struct qttf_hhea_table {
    qint16 ascender;
    qint16 descender;
    qint16 lineGap;
    quint16 maxAdvanceWidth;
    qint16 minLeftSideBearing;
    qint16 minRightSideBearing;
    qint16 xMaxExtent;
    quint16 numberOfHMetrics;
};


struct qttf_maxp_table {
    quint16 numGlyphs;
    quint16 maxPoints;
    quint16 maxContours;
    quint16 maxCompositePoints;
    quint16 maxCompositeContours;
    quint16 maxComponentElements;
    quint16 maxComponentDepth;
};

struct qttf_name_table {
    QString copyright;
    QString family;
    QString subfamily;
    QString postscript_name;
};


static QTtfTable generateHead(const qttf_head_table &head);
static QTtfTable generateHhea(const qttf_hhea_table &hhea);
static QTtfTable generateMaxp(const qttf_maxp_table &maxp);
static QTtfTable generateName(const qttf_name_table &name);

struct qttf_font_tables
{
    qttf_head_table head;
    qttf_hhea_table hhea;
    qttf_maxp_table maxp;
};


struct QTtfGlyph {
    quint16 index;
    qint16 xMin;
    qint16 xMax;
    qint16 yMin;
    qint16 yMax;
    quint16 advanceWidth;
    qint16 lsb;
    quint16 numContours;
    quint16 numPoints;
    QByteArray data;
};
Q_DECLARE_TYPEINFO(QTtfGlyph, Q_MOVABLE_TYPE);

static QTtfGlyph generateGlyph(int index, const QPainterPath &path, qreal advance, qreal lsb, qreal ppem);
// generates glyf, loca and hmtx
static QList<QTtfTable> generateGlyphTables(qttf_font_tables &tables, const QList<QTtfGlyph> &_glyphs);

static QByteArray bindFont(const QList<QTtfTable>& _tables);


static quint32 checksum(const QByteArray &table)
{
    quint32 sum = 0;
    int offset = 0;
    const uchar *d = (uchar *)table.constData();
    while (offset <= table.size()-3) {
        sum += qFromBigEndian<quint32>(d + offset);
        offset += 4;
    }
    int shift = 24;
    quint32 x = 0;
    while (offset < table.size()) {
        x |= ((quint32)d[offset]) << shift;
        ++offset;
        shift -= 8;
    }
    sum += x;

    return sum;
}

static QTtfTable generateHead(const qttf_head_table &head)
{
    const int head_size = 54;
    QTtfTable t;
    t.tag = MAKE_TAG("head");
    t.data.resize(head_size);

    QTtfStream s(t.data);

// qint32  Table version number  0x00010000 for version 1.0.
// qint32  fontRevision  Set by font manufacturer.
    s << qint32(0x00010000)
      << head.font_revision
// quint32  checkSumAdjustment  To compute: set it to 0, sum the entire font as quint32, then store 0xB1B0AFBA - sum.
      << quint32(0)
// quint32  magicNumber  Set to 0x5F0F3CF5.
      << quint32(0x5F0F3CF5)
// quint16  flags  Bit 0: Baseline for font at y=0;
// Bit 1: Left sidebearing point at x=0;
// Bit 2: Instructions may depend on point size;
// Bit 3: Force ppem to integer values for all internal scaler math; may use fractional ppem sizes if this bit is clear;
// Bit 4: Instructions may alter advance width (the advance widths might not scale linearly);
// Bits 5-10: These should be set according to  Apple's specification . However, they are not implemented in OpenType.
// Bit 11: Font data is 'lossless,' as a result of having been compressed and decompressed with the Agfa MicroType Express engine.
// Bit 12: Font converted (produce compatible metrics)
// Bit 13: Font optimised for ClearType
// Bit 14: Reserved, set to 0
// Bit 15: Reserved, set to 0
      << quint16(0)

// quint16  unitsPerEm  Valid range is from 16 to 16384. This value should be a power of 2 for fonts that have TrueType outlines.
      << quint16(2048)
// qint64  created  Number of seconds since 12:00 midnight, January 1, 1904. 64-bit integer
      << head.created
// qint64  modified  Number of seconds since 12:00 midnight, January 1, 1904. 64-bit integer
      << head.modified
// qint16  xMin  For all glyph bounding boxes.
// qint16  yMin  For all glyph bounding boxes.
// qint16  xMax  For all glyph bounding boxes.
// qint16  yMax  For all glyph bounding boxes.
      << head.xMin
      << head.yMin
      << head.xMax
      << head.yMax
// quint16  macStyle  Bit 0: Bold (if set to 1);
// Bit 1: Italic (if set to 1)
// Bit 2: Underline (if set to 1)
// Bit 3: Outline (if set to 1)
// Bit 4: Shadow (if set to 1)
// Bit 5: Condensed (if set to 1)
// Bit 6: Extended (if set to 1)
// Bits 7-15: Reserved (set to 0).
      << head.macStyle
// quint16  lowestRecPPEM  Smallest readable size in pixels.
      << quint16(6) // just a wild guess
// qint16  fontDirectionHint   0: Fully mixed directional glyphs;
      << qint16(0)
// 1: Only strongly left to right;
// 2: Like 1 but also contains neutrals;
// -1: Only strongly right to left;
// -2: Like -1 but also contains neutrals. 1
// qint16  indexToLocFormat  0 for short offsets, 1 for long.
      << head.indexToLocFormat
// qint16  glyphDataFormat  0 for current format.
      << qint16(0);

    Q_ASSERT(s.offset() == head_size);
    return t;
}


static QTtfTable generateHhea(const qttf_hhea_table &hhea)
{
    const int hhea_size = 36;
    QTtfTable t;
    t.tag = MAKE_TAG("hhea");
    t.data.resize(hhea_size);

    QTtfStream s(t.data);
// qint32  Table version number  0x00010000 for version 1.0.
    s << qint32(0x00010000)
// qint16  Ascender  Typographic ascent.  (Distance from baseline of highest ascender)
      << hhea.ascender
// qint16  Descender  Typographic descent.  (Distance from baseline of lowest descender)
      << hhea.descender
// qint16  LineGap  Typographic line gap.
// Negative LineGap values are treated as zero
// in Windows 3.1, System 6, and
// System 7.
      << hhea.lineGap
// quint16  advanceWidthMax  Maximum advance width value in 'hmtx' table.
      << hhea.maxAdvanceWidth
// qint16  minLeftSideBearing  Minimum left sidebearing value in 'hmtx' table.
      << hhea.minLeftSideBearing
// qint16  minRightSideBearing  Minimum right sidebearing value; calculated as Min(aw - lsb - (xMax - xMin)).
      << hhea.minRightSideBearing
// qint16  xMaxExtent  Max(lsb + (xMax - xMin)).
      << hhea.xMaxExtent
// qint16  caretSlopeRise  Used to calculate the slope of the cursor (rise/run); 1 for vertical.
      << qint16(1)
// qint16  caretSlopeRun  0 for vertical.
      << qint16(0)
// qint16  caretOffset  The amount by which a slanted highlight on a glyph needs to be shifted to produce the best appearance. Set to 0 for non-slanted fonts
      << qint16(0)
// qint16  (reserved)  set to 0
      << qint16(0)
// qint16  (reserved)  set to 0
      << qint16(0)
// qint16  (reserved)  set to 0
      << qint16(0)
// qint16  (reserved)  set to 0
      << qint16(0)
// qint16  metricDataFormat  0 for current format.
      << qint16(0)
// quint16  numberOfHMetrics  Number of hMetric entries in 'hmtx' table
      << hhea.numberOfHMetrics;

    Q_ASSERT(s.offset() == hhea_size);
    return t;
}


static QTtfTable generateMaxp(const qttf_maxp_table &maxp)
{
    const int maxp_size = 32;
    QTtfTable t;
    t.tag = MAKE_TAG("maxp");
    t.data.resize(maxp_size);

    QTtfStream s(t.data);

// qint32  Table version number  0x00010000 for version 1.0.
    s << qint32(0x00010000)
// quint16  numGlyphs  The number of glyphs in the font.
      << maxp.numGlyphs
// quint16  maxPoints  Maximum points in a non-composite glyph.
      << maxp.maxPoints
// quint16  maxContours  Maximum contours in a non-composite glyph.
      << maxp.maxContours
// quint16  maxCompositePoints  Maximum points in a composite glyph.
      << maxp.maxCompositePoints
// quint16  maxCompositeContours  Maximum contours in a composite glyph.
      << maxp.maxCompositeContours
// quint16  maxZones  1 if instructions do not use the twilight zone (Z0), or 2 if instructions do use Z0; should be set to 2 in most cases.
      << quint16(1) // we do not embed instructions
// quint16  maxTwilightPoints  Maximum points used in Z0.
      << quint16(0)
// quint16  maxStorage  Number of Storage Area locations.
      << quint16(0)
// quint16  maxFunctionDefs  Number of FDEFs.
      << quint16(0)
// quint16  maxInstructionDefs  Number of IDEFs.
      << quint16(0)
// quint16  maxStackElements  Maximum stack depth2.
      << quint16(0)
// quint16  maxSizeOfInstructions  Maximum byte count for glyph instructions.
      << quint16(0)
// quint16  maxComponentElements  Maximum number of components referenced at "top level" for any composite glyph.
      << maxp.maxComponentElements
// quint16  maxComponentDepth  Maximum levels of recursion; 1 for simple components.
      << maxp.maxComponentDepth;

    Q_ASSERT(s.offset() == maxp_size);
    return t;
}

struct NameRecord {
    quint16 nameId;
    QString value;
};

static QTtfTable generateName(const QList<NameRecord> &name);

static QTtfTable generateName(const qttf_name_table &name)
{
    QList<NameRecord> list;
    NameRecord rec;
    rec.nameId = 0;
    rec.value = name.copyright;
    list.append(rec);
    rec.nameId = 1;
    rec.value = name.family;
    list.append(rec);
    rec.nameId = 2;
    rec.value = name.subfamily;
    list.append(rec);
    rec.nameId = 4;
    rec.value = name.family;
    if (name.subfamily != "Regular")
        rec.value += " " + name.subfamily;
    list.append(rec);
    rec.nameId = 6;
    rec.value = name.postscript_name;
    list.append(rec);

    return generateName(list);
}

// ####### should probably generate Macintosh/Roman name entries as well
static QTtfTable generateName(const QList<NameRecord> &name)
{
    const int char_size = 2;

    QTtfTable t;
    t.tag = MAKE_TAG("name");

    const int name_size = 6 + 12*name.size();
    int string_size = 0;
    for (int i = 0; i < name.size(); ++i) {
        string_size += name.at(i).value.length()*char_size;
    }
    t.data.resize(name_size + string_size);

    QTtfStream s(t.data);
// quint16  format  Format selector (=0).
    s << quint16(0)
// quint16  count  Number of name records.
      << quint16(name.size())
// quint16  stringOffset  Offset to start of string storage (from start of table).
      << quint16(name_size);
// NameRecord  nameRecord[count]  The name records where count is the number of records.
// (Variable)

    int off = 0;
    for (int i = 0; i < name.size(); ++i) {
        int len = name.at(i).value.length()*char_size;
// quint16  platformID  Platform ID.
// quint16  encodingID  Platform-specific encoding ID.
// quint16  languageID  Language ID.
        s << quint16(3)
          << quint16(1)
          << quint16(0x0409) // en_US
// quint16  nameId  Name ID.
          << name.at(i).nameId
// quint16  length  String length (in bytes).
          << quint16(len)
// quint16  offset  String offset from start of storage area (in bytes).
          << quint16(off);
        off += len;
    }
    for (int i = 0; i < name.size(); ++i) {
        const QString &n = name.at(i).value;
        const ushort *uc = n.utf16();
        for (int i = 0; i < n.length(); ++i) {
            s << quint16(*uc);
            ++uc;
        }
    }
    return t;
}


enum Flags {
    OffCurve = 0,
    OnCurve = (1 << 0),
    XShortVector = (1 << 1),
    YShortVector = (1 << 2),
    Repeat = (1 << 3),
    XSame = (1 << 4),
    XShortPositive = (1 << 4),
    YSame = (1 << 5),
    YShortPositive = (1 << 5)
};
struct TTF_POINT {
    qint16 x;
    qint16 y;
    quint8 flags;
};
Q_DECLARE_TYPEINFO(TTF_POINT, Q_PRIMITIVE_TYPE);

static void convertPath(const QPainterPath &path, QList<TTF_POINT> *points, QList<int> *endPoints, qreal ppem)
{
    int numElements = path.elementCount();
    for (int i = 0; i < numElements - 1; ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        TTF_POINT p;
        p.x = qRound(e.x * 2048. / ppem);
        p.y = qRound(-e.y * 2048. / ppem);

        switch(e.type) {
        case QPainterPath::MoveToElement:
            if (i != 0) {
                // see if start and end points of the last contour agree
                int start = endPoints->size() ? endPoints->at(endPoints->size()-1) - 1 : 0;
                int end = points->size() - 1;
                if (points->at(end).x == points->at(start).x
                    && points->at(end).y == points->at(start).y)
                    points->takeLast();
                endPoints->append(points->size() - 1);
            }
            // fall through
        case QPainterPath::LineToElement:
            p.flags = OnCurve;
            break;
        case QPainterPath::CurveToElement: {
            // cubic bezier curve, we need to reduce to a list of quadratic curves
            TTF_POINT list[3*16 + 4]; // we need max 16 subdivisions
            list[3] = points->at(points->size() - 1);
            list[2] = p;
            const QPainterPath::Element &e2 = path.elementAt(++i);
            list[1].x = qRound(e2.x * 2048. / ppem);
            list[1].y = qRound(-e2.y * 2048. / ppem);
            const QPainterPath::Element &e3 = path.elementAt(++i);
            list[0].x = qRound(e3.x * 2048. / ppem);
            list[0].y = qRound(-e3.y * 2048. / ppem);

            TTF_POINT *base = list;

            bool try_reduce = points->size() > 1
                              && points->at(points->size() - 1).flags == OnCurve
                              && points->at(points->size() - 2).flags == OffCurve;
//             qDebug("generating beziers:");
            while (base >= list) {
                const int split_limit = 3;
//                 {
//                     qDebug("iteration:");
//                     TTF_POINT *x = list;
//                     while (x <= base + 3) {
//                         qDebug() << "    " << QPoint(x->x, x->y);
//                         ++x;
//                     }
//                 }
                Q_ASSERT(base - list < 3*16 + 1);
                // first see if we can easily reduce the cubic to a quadratic bezier curve
                int i1_x = base[1].x + ((base[1].x - base[0].x) >> 1);
                int i1_y = base[1].y + ((base[1].y - base[0].y) >> 1);
                int i2_x = base[2].x + ((base[2].x - base[3].x) >> 1);
                int i2_y = base[2].y + ((base[2].y - base[3].y) >> 1);
//                 qDebug() << "checking: i1=" << QPoint(i1_x, i1_y) << " i2=" << QPoint(i2_x, i2_y);
                if (qAbs(i1_x - i2_x) <= split_limit && qAbs(i1_y - i2_y) <= split_limit) {
                    // got a quadratic bezier curve
                    TTF_POINT np;
                    np.x = (i1_x + i2_x) >> 1;
                    np.y = (i1_y + i2_y) >> 1;
                    if (try_reduce) {
                        // see if we can optimise out the last onCurve point
                        int mx = (points->at(points->size() - 2).x + base[2].x) >> 1;
                        int my = (points->at(points->size() - 2).y + base[2].y) >> 1;
                        if (qAbs(mx - base[3].x) <= split_limit && qAbs(my = base[3].y) <= split_limit)
                            points->takeLast();
                        try_reduce = false;
                    }
                    np.flags = OffCurve;
                    points->append(np);
//                     qDebug() << "   appending offcurve point " << QPoint(np.x, np.y);
                    base -= 3;
                } else {
                    // need to split
//                     qDebug() << "  -> splitting";
                    qint16 a, b, c, d;
                    base[6].x = base[3].x;
                    c = base[1].x;
                    d = base[2].x;
                    base[1].x = a = ( base[0].x + c ) >> 1;
                    base[5].x = b = ( base[3].x + d ) >> 1;
                    c = ( c + d ) >> 1;
                    base[2].x = a = ( a + c ) >> 1;
                    base[4].x = b = ( b + c ) >> 1;
                    base[3].x = ( a + b ) >> 1;

                    base[6].y = base[3].y;
                    c = base[1].y;
                    d = base[2].y;
                    base[1].y = a = ( base[0].y + c ) >> 1;
                    base[5].y = b = ( base[3].y + d ) >> 1;
                    c = ( c + d ) >> 1;
                    base[2].y = a = ( a + c ) >> 1;
                    base[4].y = b = ( b + c ) >> 1;
                    base[3].y = ( a + b ) >> 1;
                    base += 3;
                }
            }
            p = list[0];
            p.flags = OnCurve;
            break;
        }
        case QPainterPath::CurveToDataElement:
            Q_ASSERT(false);
            break;
        }
//         qDebug() << "   appending oncurve point " << QPoint(p.x, p.y);
        points->append(p);
    }
    int start = endPoints->size() ? endPoints->at(endPoints->size()-1) + 1 : 0;
    int end = points->size() - 1;
    if (points->at(end).x == points->at(start).x
        && points->at(end).y == points->at(start).y)
        points->takeLast();
    endPoints->append(points->size() - 1);
}

static void getBounds(const QList<TTF_POINT> &points, qint16 *xmin, qint16 *xmax, qint16 *ymin, qint16 *ymax)
{
    *xmin = points.at(0).x;
    *xmax = *xmin;
    *ymin = points.at(0).y;
    *ymax = *ymin;

    for (int i = 1; i < points.size(); ++i) {
        *xmin = qMin(*xmin, points.at(i).x);
        *xmax = qMax(*xmax, points.at(i).x);
        *ymin = qMin(*ymin, points.at(i).y);
        *ymax = qMax(*ymax, points.at(i).y);
    }
}

static int convertToRelative(QList<TTF_POINT> *points)
{
    // convert points to relative and setup flags
//     qDebug() << "relative points:";
    qint16 prev_x = 0;
    qint16 prev_y = 0;
    int point_array_size = 0;
    for (int i = 0; i < points->size(); ++i) {
        const int x = points->at(i).x;
        const int y = points->at(i).y;
        TTF_POINT rel;
        rel.x = x - prev_x;
        rel.y = y - prev_y;
        rel.flags = points->at(i).flags;
        Q_ASSERT(rel.flags < 2);
        if (!rel.x) {
            rel.flags |= XSame;
        } else if (rel.x > 0 && rel.x < 256) {
            rel.flags |= XShortVector|XShortPositive;
            point_array_size++;
        } else if (rel.x < 0 && rel.x > -256) {
            rel.flags |= XShortVector;
            rel.x = -rel.x;
            point_array_size++;
        } else {
            point_array_size += 2;
        }
        if (!rel.y) {
            rel.flags |= YSame;
        } else if (rel.y > 0 && rel.y < 256) {
            rel.flags |= YShortVector|YShortPositive;
            point_array_size++;
        } else if (rel.y < 0 && rel.y > -256) {
            rel.flags |= YShortVector;
            rel.y = -rel.y;
            point_array_size++;
        } else {
            point_array_size += 2;
        }
        (*points)[i] = rel;
// #define toString(x) ((rel.flags & x) ? #x : "")
//         qDebug() << "    " << QPoint(rel.x, rel.y) << "flags="
//                  << toString(OnCurve) << toString(XShortVector)
//                  << (rel.flags & XShortVector ? toString(XShortPositive) : toString(XSame))
//                  << toString(YShortVector)
//                  << (rel.flags & YShortVector ? toString(YShortPositive) : toString(YSame));

        prev_x = x;
        prev_y = y;
    }
    return point_array_size;
}

static void getGlyphData(QTtfGlyph *glyph, const QList<TTF_POINT> &points, const QList<int> &endPoints, int point_array_size)
{
    const int max_size = 5*sizeof(qint16) // header
                         + endPoints.size()*sizeof(quint16) // end points of contours
                         + sizeof(quint16) // instruction length == 0
                         + points.size()*(1) // flags
                         + point_array_size; // coordinates

    glyph->data.resize(max_size);

    QTtfStream s(glyph->data);
    s << qint16(endPoints.size())
      << glyph->xMin << glyph->yMin << glyph->xMax << glyph->yMax;

    for (int i = 0; i < endPoints.size(); ++i)
        s << quint16(endPoints.at(i));
    s << quint16(0); // instruction length

    // emit flags
    for (int i = 0; i < points.size(); ++i)
        s << quint8(points.at(i).flags);
    // emit points
    for (int i = 0; i < points.size(); ++i) {
        quint8 flags = points.at(i).flags;
        qint16 x = points.at(i).x;

        if (flags & XShortVector)
            s << quint8(x);
        else if (!(flags & XSame))
            s << qint16(x);
    }
    for (int i = 0; i < points.size(); ++i) {
        quint8 flags = points.at(i).flags;
        qint16 y = points.at(i).y;

        if (flags & YShortVector)
            s << quint8(y);
        else if (!(flags & YSame))
            s << qint16(y);
    }

//     qDebug() << "offset=" << s.offset() << "max_size=" << max_size << "point_array_size=" << point_array_size;
    Q_ASSERT(s.offset() == max_size);

    glyph->numContours = endPoints.size();
    glyph->numPoints = points.size();
}

static QTtfGlyph generateGlyph(int index, const QPainterPath &path, qreal advance, qreal lsb, qreal ppem)
{
    QList<TTF_POINT> points;
    QList<int> endPoints;
    QTtfGlyph glyph;
    glyph.index = index;
    glyph.advanceWidth = qRound(advance * 2048. / ppem);
    glyph.lsb = qRound(lsb * 2048. / ppem);

    if (!path.elementCount()) {
        //qDebug("glyph %d is empty", index);
        lsb = 0;
        glyph.xMin = glyph.xMax = glyph.yMin = glyph.yMax = 0;
        return glyph;
    }

    convertPath(path, &points, &endPoints, ppem);

//     qDebug() << "number of contours=" << endPoints.size();
//     for (int i = 0; i < points.size(); ++i)
//         qDebug() << "  point[" << i << "] = " << QPoint(points.at(i).x, points.at(i).y) << " flags=" << points.at(i).flags;
//     qDebug() << "endPoints:";
//     for (int i = 0; i < endPoints.size(); ++i)
//         qDebug() << endPoints.at(i);

    getBounds(points, &glyph.xMin, &glyph.xMax, &glyph.yMin, &glyph.yMax);
    int point_array_size = convertToRelative(&points);
    getGlyphData(&glyph, points, endPoints, point_array_size);
    return glyph;
}

static bool operator <(const QTtfGlyph &g1, const QTtfGlyph &g2)
{
    return g1.index < g2.index;
}

static QList<QTtfTable> generateGlyphTables(qttf_font_tables &tables, const QList<QTtfGlyph> &_glyphs)
{
    const int max_size_small = 65536*2;
    QList<QTtfGlyph> glyphs = _glyphs;
    qSort(glyphs);

    Q_ASSERT(tables.maxp.numGlyphs == glyphs.at(glyphs.size()-1).index + 1);
    int nGlyphs = tables.maxp.numGlyphs;

    int glyf_size = 0;
    for (int i = 0; i < glyphs.size(); ++i)
        glyf_size += (glyphs.at(i).data.size() + 3) & ~3;

    tables.head.indexToLocFormat = glyf_size < max_size_small ? 0 : 1;
    tables.hhea.numberOfHMetrics = nGlyphs;

    QTtfTable glyf;
    glyf.tag = MAKE_TAG("glyf");

    QTtfTable loca;
    loca.tag = MAKE_TAG("loca");
    loca.data.resize(glyf_size < max_size_small ? (nGlyphs+1)*sizeof(quint16) : (nGlyphs+1)*sizeof(quint32));
    QTtfStream ls(loca.data);

    QTtfTable hmtx;
    hmtx.tag = MAKE_TAG("hmtx");
    hmtx.data.resize(nGlyphs*4);
    QTtfStream hs(hmtx.data);

    int pos = 0;
    for (int i = 0; i < nGlyphs; ++i) {
        int gpos = glyf.data.size();
        quint16 advance = 0;
        qint16 lsb = 0;

        if (glyphs[pos].index == i) {
            // emit glyph
//             qDebug("emitting glyph %d: size=%d", i, glyphs.at(i).data.size());
            glyf.data += glyphs.at(pos).data;
            while (glyf.data.size() & 1)
                glyf.data.append('\0');
            advance = glyphs.at(pos).advanceWidth;
            lsb = glyphs.at(pos).lsb;
            ++pos;
        }
        if (glyf_size < max_size_small) {
            // use short loca format
            ls << quint16(gpos>>1);
        } else {
            // use long loca format
            ls << quint32(gpos);
        }
        hs << advance
           << lsb;
    }
    if (glyf_size < max_size_small) {
        // use short loca format
        ls << quint16(glyf.data.size()>>1);
    } else {
        // use long loca format
        ls << quint32(glyf.data.size());
    }

    Q_ASSERT(loca.data.size() == ls.offset());
    Q_ASSERT(hmtx.data.size() == hs.offset());

    QList<QTtfTable> list;
    list.append(glyf);
    list.append(loca);
    list.append(hmtx);
    return list;
}

static bool operator <(const QTtfTable &t1, const QTtfTable &t2)
{
    return t1.tag < t2.tag;
}

static QByteArray bindFont(const QList<QTtfTable>& _tables)
{
    QList<QTtfTable> tables = _tables;

    qSort(tables);

    QByteArray font;
    const int header_size = sizeof(qint32) + 4*sizeof(quint16);
    const int directory_size = 4*sizeof(quint32)*tables.size();
    font.resize(header_size + directory_size);

    int log2 = 0;
    int pow = 1;
    int n = tables.size() >> 1;
    while (n) {
        ++log2;
        pow <<= 1;
        n >>= 1;
    }

    quint32 head_offset = 0;
    {
        QTtfStream f(font);
// Offset Table
// Type  Name  Description
//   qint32  sfnt version  0x00010000 for version 1.0.
//   quint16   numTables  Number of tables.
//   quint16   searchRange  (Maximum power of 2 <= numTables) x 16.
//   quint16   entrySelector  Log2(maximum power of 2 <= numTables).
//   quint16   rangeShift  NumTables x 16-searchRange.
        f << qint32(0x00010000)
          << quint16(tables.size())
          << quint16(16*pow)
          << quint16(log2)
          << quint16(16*(tables.size() - pow));

// Table Directory
// Type  Name  Description
//   quint32  tag  4 -byte identifier.
//   quint32  checkSum  CheckSum for this table.
//   quint32  offset  Offset from beginning of TrueType font file.
//   quint32  length  Length of this table.
        quint32 table_offset = header_size + directory_size;
        for (int i = 0; i < tables.size(); ++i) {
            const QTtfTable &t = tables.at(i);
            const quint32 size = (t.data.size() + 3) & ~3;
            if (t.tag == MAKE_TAG("head"))
                head_offset = table_offset;
            f << t.tag
              << checksum(t.data)
              << table_offset
              << t.data.size();
            table_offset += size;
#define TAG(x) char(t.tag >> 24) << char((t.tag >> 16) & 0xff) << char((t.tag >> 8) & 0xff) << char(t.tag & 0xff)
            //qDebug() << "table " << TAG(t.tag) << "has size " << t.data.size() << "stream at " << f.offset();
        }
    }
    for (int i = 0; i < tables.size(); ++i) {
        const QByteArray &t = tables.at(i).data;
        font += t;
        int s = t.size();
        while (s & 3) { font += '\0'; ++s; }
    }

    if (!head_offset) {
        qWarning("Font misses 'head' table");
        return QByteArray();
    }

    // calculate the fonts checksum and qToBigEndian into 'head's checksum_adjust
    quint32 checksum_adjust = 0xB1B0AFBA - checksum(font);
    qToBigEndian(checksum_adjust, (uchar *)font.data() + head_offset + 8);

    return font;
}



/*
  PDF requires the following tables:

  head, hhea, loca, maxp, cvt , prep, glyf, hmtx, fpgm

  This means we don't have to add a os/2, post or name table. cvt , prep and fpgm could be empty
  if really required.
*/

QByteArray QPdf::Font::toTruetype() const
{
    qttf_font_tables font;
    memset(&font, 0, sizeof(qttf_font_tables));

    qreal ppem = fontEngine->fontDef.pixelSize;
#define TO_TTF(x) qRound(x * 2048. / ppem)
    QList<QTtfGlyph> glyphs;

    QFontEngine::Properties properties = fontEngine->properties();
    // initialize some stuff needed in createWidthArray
    emSquare = 2048;
    widths.resize(nGlyphs);

    // head table
    font.head.font_revision = 0x00010000;
    font.head.flags = (1 << 2) | (1 << 4);
    font.head.created = 0; // ###
    font.head.modified = 0; // ###
    font.head.xMin = SHRT_MAX;
    font.head.xMax = SHRT_MIN;
    font.head.yMin = SHRT_MAX;
    font.head.yMax = SHRT_MIN;
    font.head.macStyle = (fontEngine->fontDef.weight > QFont::Normal) ? 1 : 0;
    font.head.macStyle |= (fontEngine->fontDef.styleHint != QFont::StyleNormal) ? 1 : 0;

    // hhea table
    font.hhea.ascender = qRound(properties.ascent);
    font.hhea.descender = -qRound(properties.descent);
    font.hhea.lineGap = qRound(properties.leading);
    font.hhea.maxAdvanceWidth = TO_TTF(fontEngine->maxCharWidth());
    font.hhea.minLeftSideBearing = TO_TTF(fontEngine->minLeftBearing());
    font.hhea.minRightSideBearing = TO_TTF(fontEngine->minRightBearing());
    font.hhea.xMaxExtent = SHRT_MIN;

    font.maxp.numGlyphs = 0;
    font.maxp.maxPoints = 0;
    font.maxp.maxContours = 0;
    font.maxp.maxCompositePoints = 0;
    font.maxp.maxCompositeContours = 0;
    font.maxp.maxComponentElements = 0;
    font.maxp.maxComponentDepth = 0;


    // name
    qttf_name_table name;
    name.copyright = QLatin1String("Converted to TTF by Qt");
    name.family = fontEngine->fontDef.family;
    name.subfamily = QLatin1String("Regular"); // ######
    name.postscript_name = fontEngine->fontDef.family;

    uint sumAdvances = 0;
    for (int i = 0; i < glyph_indices.size(); ++i) {
        glyph_t g = glyph_indices.at(i);
        QPainterPath path;
        glyph_metrics_t metric;
        fontEngine->getUnscaledGlyph(g, &path, &metric);
        QTtfGlyph glyph = generateGlyph(g, path, metric.xoff.toReal(), metric.x.toReal(), properties.emSquare.toReal());

        font.head.xMin = qMin(font.head.xMin, glyph.xMin);
        font.head.xMax = qMax(font.head.xMax, glyph.xMax);
        font.head.yMin = qMin(font.head.yMin, glyph.yMin);
        font.head.yMax = qMax(font.head.yMax, glyph.yMax);

        font.hhea.xMaxExtent = qMax(font.hhea.xMaxExtent, (qint16)(glyph.lsb + glyph.xMax - glyph.xMin));

        font.maxp.numGlyphs = qMax(font.maxp.numGlyphs, quint16(g + 1));
        font.maxp.maxPoints = qMax(font.maxp.maxPoints, glyph.numPoints);
        font.maxp.maxContours = qMax(font.maxp.maxContours, glyph.numContours);

        if (glyph.xMax > glyph.xMin)
            sumAdvances += glyph.xMax - glyph.xMin;

//         qDebug("adding glyph %d size=%d", glyph.index, glyph.data.size());
        glyphs.append(glyph);
        widths[g] = glyph.advanceWidth;
    }


    QList<QTtfTable> tables = generateGlyphTables(font, glyphs);
    tables.append(generateHead(font.head));
    tables.append(generateHhea(font.hhea));
    tables.append(generateMaxp(font.maxp));
    tables.append(generateName(name));

    return bindFont(tables);
}


QByteArray QPdf::Font::widthArray() const
{
    Q_ASSERT(!widths.isEmpty());

    QFontEngine::Properties properties = fontEngine->properties();

    QByteArray width;
    QPdf::ByteStream s(&width);
    QFixed scale = QFixed(1000)/emSquare;

    QFixed defWidth = widths[0];
    //qDebug("defWidth=%d, scale=%f", defWidth.toInt(), scale.toReal());
    for (int i = 0; i < nGlyphs; ++i) {
        if (defWidth != widths[i])
            defWidth = 0;
    }
    if (defWidth > 0) {
        s << "/DW " << (defWidth*scale).toInt();
    } else {
        s << "/W [";
        for (int g = 0; g < nGlyphs;) {
            QFixed w = widths[g];
            int start = g;
            int startLinear = 0;
            ++g;
            while (g < nGlyphs) {
                QFixed nw = widths[g];
                if (nw == w) {
                if (!startLinear)
                    startLinear = g - 1;
                } else {
                    if (startLinear > 0 && g - startLinear >= 10)
                        break;
                    startLinear = 0;
                }
                w = nw;
                ++g;
            }
            // qDebug("start=%x startLinear=%x g-1=%x",start,startLinear,g-1);
            if (g - startLinear < 10)
                startLinear = 0;
            int endnonlinear = startLinear ? startLinear : g;
            // qDebug("    startLinear=%x endnonlinear=%x", startLinear,endnonlinear);
            if (endnonlinear > start) {
                s << start << "[";
                for (int i = start; i < endnonlinear; ++i)
                    s << (widths[i]*scale).toInt();
                s << "]\n";
            }
            if (startLinear)
                s << startLinear << g - 1 << (widths[startLinear]*scale).toInt() << "\n";
        }
        s << "]\n";
    }
    return width;
}

static void checkRanges(QPdf::ByteStream &ts, QByteArray &ranges, int &nranges)
{
    if (++nranges > 100) {
        ts << nranges << "beginbfrange\n"
           << ranges << "endbfrange\n";
        ranges = QByteArray();
        nranges = 0;
    }
}

QByteArray QPdf::Font::createToUnicodeMap() const
{
    QVector<int> reverseMap;
    reverseMap.resize(nGlyphs);
    QGlyphLayout glyphs[10];
    for (uint uc = 0; uc < 0x10000; ++uc) {
        QChar ch(uc);
        int nglyphs = 10;
        fontEngine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
        if (glyphs[0].glyph < (uint)nGlyphs && !reverseMap.at(glyphs[0].glyph))
            reverseMap[glyphs[0].glyph] = uc;
    }

    QByteArray touc;
    QPdf::ByteStream ts(&touc);
    ts << "/CIDInit /ProcSet findresource begin\n"
        "12 dict begin\n"
        "begincmap\n"
        "/CIDSystemInfo << /Registry (Adobe) /Ordering (UCS) /Supplement 0 >> def\n"
        "/CMapName /Adobe-Identity-UCS def\n"
        "/CMapType 2 def\n"
        "1 begincodespacerange\n"
        "<0000> <FFFF>\n"
        "endcodespacerange\n";

    int nranges = 1;
    QByteArray ranges = "<0000> <0000> <0000>\n";
    QPdf::ByteStream s(&ranges);

    char buf[5];
    for (int g = 1; g < nGlyphs; ) {
        int uc0 = reverseMap.at(g);
        if (!uc0) {
            ++g;
            continue;
        }
        int start = g;
        int startLinear = 0;
        ++g;
        while (g < nGlyphs) {
            int uc = reverseMap[g];
            // cmaps can't have the high byte changing within one range, so we need to break on that as well
            if (!uc || (g>>8) != (start >> 8))
                break;
            if (uc == uc0 + 1) {
                if (!startLinear)
                    startLinear = g - 1;
            } else {
                if (startLinear > 0 && g - startLinear >= 10)
                    break;
                startLinear = 0;
            }
            uc0 = uc;
            ++g;
        }
        // qDebug("start=%x startLinear=%x g-1=%x",start,startLinear,g-1);
        if (g - startLinear < 10)
            startLinear = 0;
        int endnonlinear = startLinear ? startLinear : g;
        // qDebug("    startLinear=%x endnonlinear=%x", startLinear,endnonlinear);
        if (endnonlinear > start) {
            s << "<" << toHex((ushort)start, buf) << "> <";
            s << toHex((ushort)(endnonlinear - 1), buf) << "> ";
            if (endnonlinear == start + 1) {
                s << "<" << toHex((ushort)reverseMap[start], buf) << ">\n";
            } else {
                s << "[";
                for (int i = start; i < endnonlinear; ++i) {
                    s << "<" << toHex((ushort)reverseMap[i], buf) << "> ";
                }
                s << "]\n";
            }
            checkRanges(ts, ranges, nranges);
        }
        if (startLinear) {
            while (startLinear < g) {
                int len = g - startLinear;
                int uc_start = reverseMap[startLinear];
                int uc_end = uc_start + len - 1;
                if ((uc_end >> 8) != (uc_start >> 8))
                    len = 256 - (uc_start & 0xff);
                s << "<" << toHex((ushort)startLinear, buf) << "> <";
                s << toHex((ushort)(startLinear + len - 1), buf) << "> ";
                s << "<" << toHex((ushort)reverseMap[startLinear], buf) << ">\n";
                checkRanges(ts, ranges, nranges);
                startLinear += len;
            }
        }
    }
    if (nranges) {
        ts << nranges << "beginbfrange\n"
           << ranges << "endbfrange\n";
    }
    ts << "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end\n";

    return touc;
}


// ----------------------------------- End of trueType embedding code ---------------------------------



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
    f &= ~(QPaintEngine::PorterDuff
#ifndef USE_NATIVE_GRADIENTS
           | QPaintEngine::LinearGradientFill
#endif
           | QPaintEngine::RadialGradientFill
           | QPaintEngine::ConicalGradientFill);
    return f;
}

QPdfEngine::QPdfEngine()
    : QPaintEngine(qt_pdf_decide_features()), outFile_(new QFile)
{
    device_ = 0;
    backgroundMode = Qt::TransparentMode;

    d = new QPdfEnginePrivate;

    pagesize_ = QPrinter::A4;
    QRect r = paperRect();
    d->setDimensions(r.width(),r.height());
}

QPdfEngine::~QPdfEngine()
{
    delete d;

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
        break;
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
    case PPK_FullPage:
        d->fullPage = value.toBool();
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
        return d->fullPage;
    case PPK_NumberOfCopies:
        return 1;
    case PPK_Orientation:
        return d->orientation;
    case PPK_OutputFileName:
        return outFile_->fileName();
    case PPK_PageRect:
        return pageRect();
    case PPK_PageSize:
        return pagesize_;
    case PPK_PaperRect:
        return paperRect();
    case PPK_PaperSource:
        return QPrinter::Auto;
    case PPK_Resolution:
        return 600;
    case PPK_SupportedResolutions:
        return QList<QVariant>() << resolution;
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

QRect QPdfEngine::pageRect() const
{
    QRect r = paperRect();
    if (d->fullPage)
        return r;
    // would be nice to get better margins than this.
    return QRect(resolution/3, resolution/3, r.width()-2*resolution/3, r.height()-2*resolution/3);
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

    hasPen = true;
    hasBrush = false;
    clipEnabled = false;
    allClipped = false;

    d->unsetDevice();
    d->setDevice(device_);
    setActive(true);
    d->writeHeader();
    newPage();

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
    if (!points || !hasPen)
        return;

    QPainterPath p;
    for (int i=0; i!=pointCount;++i) {
        p.moveTo(points[i]);
        p.lineTo(points[i] + QPointF(0, 0.001));
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

    bool hb = hasBrush;

    switch(mode) {
    case OddEvenMode:
        p.setFillRule(Qt::OddEvenFill);
        break;
    case ConvexMode:
    case WindingMode:
        p.setFillRule(Qt::WindingFill);
        break;
    case PolylineMode:
        hasBrush = false;
        break;
    default:
        break;
    }

    p.moveTo(points[0]);
    for (int i = 1; i < pointCount; ++i)
        p.lineTo(points[i]);

    if (mode != PolylineMode)
        p.closeSubpath();
    drawPath(p);

    hasBrush = hb;
}

void QPdfEngine::drawPath (const QPainterPath &p)
{
    if (clipEnabled && allClipped)
        return;
    QBrush penBrush = pen.brush();
    if (hasPen && penBrush == Qt::SolidPattern && penBrush.isOpaque()) {
        // draw strokes natively in this case for better output
        *d->currentPage << "q\n";
        setPen();
        *d->currentPage << QPdf::generateMatrix(d->stroker.matrix);
        *d->currentPage << QPdf::generatePath(p, QMatrix(), hasBrush ? QPdf::FillAndStrokePath : QPdf::StrokePath);
        *d->currentPage << "Q\n";
    } else {
        if (hasBrush) {
            *d->currentPage << QPdf::generatePath(p, d->stroker.matrix, QPdf::FillPath);
        }
        if (hasPen) {
            *d->currentPage << "q\n";
            QBrush b = brush;
            brush = pen.brush();
            setBrush();
            d->stroker.strokePath(p);
            *d->currentPage << "Q\n";
            brush = b;
        }
    }
}

void QPdfEngine::drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr)
{
    if (sr.isEmpty() || rectangle.isEmpty() || pixmap.isNull())
        return;
    QBrush b = brush;

    QPixmap pm = pixmap.copy(sr.toRect());
    QImage image = pm.toImage();

    *d->currentPage << "q\n";
    *d->currentPage
        << QPdf::generateMatrix(QMatrix(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                                        rectangle.x(), rectangle.y()) * d->stroker.matrix);
    bool bitmap = true;
    int object = d->addImage(image, &bitmap);
    if (bitmap) {
        if (backgroundMode == Qt::OpaqueMode) {
            // draw background
            brush = backgroundBrush;
            setBrush();
            *d->currentPage << "0 0 " << rectangle.width() << rectangle.height() << "re f\n";
        }
        // set current pen as brush
        brush = pen.brush();
        setBrush();
    }
    d->currentPage->streamImage(image.width(), image.height(), object);
    *d->currentPage << "Q\n";

    brush = b;
}

void QPdfEngine::drawImage(const QRectF & rectangle, const QImage & image, const QRectF & sr, Qt::ImageConversionFlags)
{
    if (sr.isEmpty() || rectangle.isEmpty() || image.isNull())
        return;

    QImage im = image.copy(sr.toRect());

    *d->currentPage << "q\n";
    *d->currentPage
        << QPdf::generateMatrix(QMatrix(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                                        rectangle.x(), rectangle.y()) * d->stroker.matrix);
    bool bitmap = false;
    int object = d->addImage(im, &bitmap);
    d->currentPage->streamImage(image.width(), image.height(), object);
    *d->currentPage << "Q\n";
}

void QPdfEngine::drawTiledPixmap (const QRectF &rectangle, const QPixmap &pixmap, const QPointF &point)
{
    bool bitmap = (pixmap.depth() == 1);
    QBrush b = brush;
    QPointF bo = brushOrigin;
    bool hp = hasPen;
    hasPen = false;
    bool hb = hasBrush;
    hasBrush = true;

    if (bitmap) {
        if (backgroundMode == Qt::OpaqueMode) {
            // draw background
            brush = backgroundBrush;
            setBrush();
            *d->currentPage << "q\n";
            *d->currentPage
                << QPdf::generateMatrix(d->stroker.matrix);
            *d->currentPage << rectangle.x() << rectangle.y() << rectangle.width() << rectangle.height() << "re f Q\n";
        }
    }
    brush = QBrush(pixmap);
    if (bitmap)
        // #### fix bitmap case where we have a brush pen
        brush.setColor(pen.color());

    brushOrigin = -point;
    *d->currentPage << "q\n";
    setBrush();

    drawRects(&rectangle, 1);
    *d->currentPage << "Q\n";

    hasPen = hp;
    hasBrush = hb;
    brush = b;
    brushOrigin = bo;
}

void QPdfEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    *d->currentPage << "q " << QPdf::generateMatrix(d->stroker.matrix);

    bool hp = hasPen;
    hasPen = false;
    QBrush b = brush;
    brush = pen.brush();
    setBrush();

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    if (ti.fontEngine->type() == QFontEngine::Multi) {
        QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
        QGlyphLayout *glyphs = ti.glyphs;
        int which = glyphs[0].glyph >> 24;

        qreal x = p.x();
        qreal y = p.y();

        int start = 0;
        int end, i;
        for (end = 0; end < ti.num_glyphs; ++end) {
            const int e = glyphs[end].glyph >> 24;
            if (e == which)
                continue;

            // set the high byte to zero
            for (i = start; i < end; ++i)
                glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

            // draw the text
            QTextItemInt ti2 = ti;
            ti2.glyphs = ti.glyphs + start;
            ti2.num_glyphs = end - start;
            ti2.fontEngine = multi->engine(which);
            ti2.f = ti.f;
            d->drawTextItem(this, QPointF(x, y), ti2);

            QFixed xadd;
            // reset the high byte for all glyphs and advance to the next sub-string
            const int hi = which << 24;
            for (i = start; i < end; ++i) {
                glyphs[i].glyph = hi | glyphs[i].glyph;
                xadd += glyphs[i].advance.x;
            }
            x += xadd.toReal();

            // change engine
            start = end;
            which = e;
        }

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        d->drawTextItem(this, QPointF(x,y), ti2);

        // reset the high byte for all glyphs
        const int hi = which << 24;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;
    } else {
        d->drawTextItem(this, p, ti);
    }
    hasPen = hp;
    brush = b;
    *d->currentPage << "Q\n";
}

void QPdfEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform)
        d->stroker.matrix = state.matrix();

    if (flags & DirtyPen) {
        pen = state.pen();
        hasPen = pen != Qt::NoPen;
        d->stroker.setPen(pen);
    }
    if (flags & DirtyBrush) {
        brush = state.brush();
        hasBrush = brush != Qt::NoBrush;
    }
    if (flags & DirtyBrushOrigin) {
        brushOrigin = state.brushOrigin();
        flags |= DirtyBrush;
    }

    if (flags & DirtyBackground)
        backgroundBrush = state.backgroundBrush();
    if (flags & DirtyBackgroundMode)
        backgroundMode = state.backgroundMode();

    bool ce = clipEnabled;
    if (flags & DirtyClipEnabled)
        clipEnabled = state.isClipEnabled();
    if (flags & DirtyClipPath)
        updateClipPath(state.clipPath(), state.clipOperation());
    if (flags & DirtyClipRegion) {
        QPainterPath path;
        QVector<QRect> rects = state.clipRegion().rects();
        for (int i = 0; i < rects.size(); ++i)
            path.addRect(rects.at(i));
        updateClipPath(path, state.clipOperation());
        flags |= DirtyClipPath;
    }

    if (ce != clipEnabled)
        flags |= DirtyClipPath;
    else if (!clipEnabled)
        flags &= ~DirtyClipPath;

    if (flags & DirtyClipPath) {
        *d->currentPage << "Q q\n";
        flags |= DirtyPen|DirtyBrush;
    }

    if (flags & DirtyClipPath) {
        allClipped = false;
        if (clipEnabled && !clips.isEmpty()) {
            for (int i = 0; i < clips.size(); ++i) {
                if (clips.at(i).isEmpty()) {
                    allClipped = true;
                    break;
                }
            }
            if (!allClipped) {
                for (int i = 0; i < clips.size(); ++i) {
                    *d->currentPage << QPdf::generatePath(clips.at(i), QMatrix(), QPdf::ClipPath);
                }
            }
        }
    }

    if (flags & DirtyBrush)
        setBrush();
}

void QPdfEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    QPainterPath path = d->stroker.matrix.map(p);
    //qDebug() << "updateClipPath: " << matrix << p.boundingRect() << path.boundingRect();

    if (op == Qt::NoClip) {
        clipEnabled = false;
    } else if (op == Qt::ReplaceClip) {
        clips.clear();
        clips.append(path);
    } else if (op == Qt::IntersectClip) {
        clips.append(path);
    } else { // UniteClip
        // ask the painter for the current clipping path. that's the easiest solution
        path = painter()->clipPath();
        path = d->stroker.matrix.map(path);
        clips.clear();
        clips.append(path);
    }
}

void QPdfEngine::setPen()
{
    QBrush b = pen.brush();
    Q_ASSERT(b.style() == Qt::SolidPattern && b.isOpaque());

    QColor rgba = b.color();
    *d->currentPage << rgba.redF()
                    << rgba.greenF()
                    << rgba.blueF()
                    << "SCN\n";

    *d->currentPage << pen.widthF() << "w ";

    int pdfCapStyle = 0;
    switch(pen.capStyle()) {
    case Qt::FlatCap:
        pdfCapStyle = 0;
        break;
    case Qt::SquareCap:
        pdfCapStyle = 2;
        break;
    case Qt::RoundCap:
        pdfCapStyle = 1;
        break;
    default:
        break;
    }
    *d->currentPage << pdfCapStyle << "J ";

    int pdfJoinStyle = 0;
    switch(pen.joinStyle()) {
    case Qt::MiterJoin:
        pdfJoinStyle = 0;
        break;
    case Qt::BevelJoin:
        pdfJoinStyle = 2;
        break;
    case Qt::RoundJoin:
        pdfJoinStyle = 1;
        break;
    default:
        break;
    }
    *d->currentPage << pdfJoinStyle << "j ";

    *d->currentPage << QPdf::generateDashes(pen) << " 0 d\n";

    int penObj = d->addPenGState(pen);
    if (penObj)
        *d->currentPage << "/GState" << penObj << "gs\n";
}

void QPdfEngine::setBrush()
{
    bool specifyColor;
    int gStateObject = 0;
    int patternObject = d->addBrushPattern(brush, d->stroker.matrix, brushOrigin, &specifyColor, &gStateObject);

    *d->currentPage << (patternObject ? "/PCSp cs " : "/CSp cs ");
    if (specifyColor) {
        QColor rgba = brush.color();
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


int QPdfEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const
{
    int val;
    QRect r = d->fullPage ? paperRect() : pageRect();
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
    if (!isActive())
        return false;
    d->newPage();
    return true;
}

QPdfEnginePrivate::QPdfEnginePrivate()
{
    currentObject = 1;
    width_ = 0;
    height_ = 0;
    streampos = 0;

    currentPage = new QPdfPage;
    stroker.stream = currentPage;

    stream = new QDataStream;
    pageOrder = QPrinter::FirstPageFirst;
    orientation = QPrinter::Portrait;
    fullPage = false;
}

QPdfEnginePrivate::~QPdfEnginePrivate()
{
    delete currentPage;
    delete stream;
}


int QPdfEnginePrivate::addPenGState(const QPen &pen)
{
    QColor rgba = pen.color();
    if (rgba.alphaF() == 1.)
        return 0;
    QByteArray penDef;
    QPdf::ByteStream s(&penDef);
    s << "<<";
    if (rgba.alphaF() < 1.)
        s << "/CA " << rgba.alphaF();
    s << ">>\n";

    int penObj = addXrefEntry(-1);
    xprintf(penDef.constData());
    xprintf("endobj\n");

    currentPage->graphicStates.append(penObj);
    return penObj;
}

void QPdfEnginePrivate::drawTextItem(QPdfEngine *q, const QPointF &p, const QTextItemInt &ti)
{
    QFontEngine *fe = ti.fontEngine;

    QFontEngine::FaceId face_id = fe->faceId();
    if (face_id.filename.isEmpty()) {
        *currentPage << "Q\n";
        q->QPaintEngine::drawTextItem(p, ti);
        *currentPage << "q\n";
        return;
    }

    QPdf::Font *font = fonts.value(face_id, 0);
    if (!font)
        font = new QPdf::Font(fe, requestObject());
    fonts.insert(face_id, font);

    if (!currentPage->fonts.contains(font->object_id))
        currentPage->fonts.append(font->object_id);

    qreal size;
#ifdef Q_WS_WIN
    size = ti.fontEngine->tm.w.tmHeight;
#else
    size = ti.fontEngine->fontDef.pixelSize;
#endif

    QVarLengthArray<glyph_t> glyphs;
    QVarLengthArray<QFixedPoint> positions;
    QMatrix m;
    m.translate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, m, ti.flags,
                                     glyphs, positions);
    int synthesized = ti.fontEngine->synthesized();
    qreal stretch = synthesized & QFontEngine::SynthesizedStretch ? ti.fontEngine->fontDef.stretch/100. : 1.;

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut|QTextItem::Overline)) {
        qreal lw = fe->lineThickness().toReal();
        if (ti.flags & (QTextItem::Underline))
            *currentPage << p.x() << (p.y() + fe->underlinePosition().toReal())
                           << ti.width.toReal() << lw << "re ";
        if (ti.flags & (QTextItem::StrikeOut))
            *currentPage  << p.x() << (p.y() - fe->ascent().toReal()/3.)
                            << ti.width.toReal() << lw << "re ";
        if (ti.flags & (QTextItem::Overline))
            *currentPage  << p.x() << (p.y() - fe->ascent().toReal())
                            << ti.width.toReal() << lw << "re ";
        *currentPage << "f\n";
    }
    *currentPage << "BT\n"
                 << "/F" << font->object_id << size << "Tf "
                 << stretch << (synthesized & QFontEngine::SynthesizedItalic
                                ? "0 .3 -1 0 0 Tm\n"
                                : "0 0 -1 0 0 Tm\n");
                     

#if 0
    // #### implement actual text for complex languages
    const unsigned short *logClusters = ti.logClusters;
    int pos = 0;
    do {
        int end = pos + 1;
        while (end < ti.num_chars && logClusters[end] == logClusters[pos])
            ++end;
        *currentPage << "/Span << /ActualText <FEFF";
        for (int i = pos; i < end; ++i) {
            s << toHex((ushort)ti.chars[i].unicode(), buf);
        }
        *currentPage << "> >>\n"
            "BDC\n"
            "<";
        int ge = end == ti.num_chars ? ti.num_glyphs : logClusters[end];
        for (int gs = logClusters[pos]; gs < ge; ++gs)
            *currentPage << toHex((ushort)ti.glyphs[gs].glyph, buf);
        *currentPage << "> Tj\n"
            "EMC\n";
        pos = end;
    } while (pos < ti.num_chars);
#else
    qreal last_x = 0.;
    qreal last_y = 0.;
    for (int i = 0; i < glyphs.size(); ++i) {
        qreal x = positions[i].x.toReal();
        qreal y = positions[i].y.toReal();
        if (synthesized & QFontEngine::SynthesizedItalic)
            x += .3*y;
        x /= stretch;
        char buf[5];
        *currentPage << x - last_x << last_y - y << "Td <"
                     << toHex((ushort)glyphs[i], buf) << "> Tj\n";
        font->addGlyph(glyphs[i]);
        last_x = x;
        last_y = y;
    }
    if (synthesized & QFontEngine::SynthesizedBold) {
        *currentPage << stretch << (synthesized & QFontEngine::SynthesizedItalic
                            ? "0 .3 -1 0 0 Tm\n"
                            : "0 0 -1 0 0 Tm\n");
        *currentPage << "/Span << /ActualText <> >> BDC\n";
        last_x = 0.5*fe->lineThickness().toReal();
        last_y = 0.;
        for (int i = 0; i < glyphs.size(); ++i) {
            qreal x = positions[i].x.toReal();
            qreal y = positions[i].y.toReal();
            if (synthesized & QFontEngine::SynthesizedItalic)
                x += .3*y;
            x /= stretch;
            char buf[5];
            *currentPage << x - last_x << last_y - y << "Td <"
                        << toHex((ushort)glyphs[i], buf) << "> Tj\n";
            font->addGlyph(glyphs[i]);
            last_x = x;
            last_y = y;
        }
        *currentPage << "EMC\n";
    }
#endif

    *currentPage << "ET\n";
}


#ifdef USE_NATIVE_GRADIENTS
int QPdfEnginePrivate::gradientBrush(const QBrush &b, const QMatrix &matrix, int *gStateObject)
{
    const QGradient *gradient = b.gradient();
    if (!gradient)
        return 0;

    QMatrix inv = matrix.inverted();
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
            *gStateObject = addXrefEntry(-1);
            xprintf("<< /ca %f >>\n"
                    "endobj\n", stops.at(0).second.alphaF());

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
        }
        currentPage->graphicStates.append(*gStateObject);
    }

    return patternObj;
}
#endif

int QPdfEnginePrivate::addBrushPattern(const QBrush &b, const QMatrix &m, const QPointF &brushOrigin,
                                       bool *specifyColor, int *gStateObject)
{
    int paintType = 2; // Uncolored tiling
    int w = 8;
    int h = 8;

    *specifyColor = true;
    *gStateObject = 0;

    QMatrix matrix = m;
    matrix.translate(brushOrigin.x(), brushOrigin.y());
    matrix = matrix * QMatrix(1, 0, 0, -1, 0, height_);
    //qDebug() << brushOrigin << matrix;

    Qt::BrushStyle style = b.style();
    if (style == Qt::LinearGradientPattern) {// && style <= Qt::ConicalGradientPattern) {
#ifdef USE_NATIVE_GRADIENTS
        *specifyColor = false;
        return gradientBrush(b, matrix, gStateObject);
#else
        return 0;
#endif
    }

    if (!b.isOpaque() && b.style() < Qt::LinearGradientPattern) {
        QByteArray brushDef;
        QPdf::ByteStream s(&brushDef);
        s << "<<";
        QColor rgba = b.color();
        s << "/ca " << rgba.alphaF();
        s << ">>\n";

        *gStateObject = addXrefEntry(-1);
        xprintf(brushDef.constData());
        xprintf("endobj\n");

        currentPage->graphicStates.append(*gStateObject);
    }

    int imageObject = 0;
    QByteArray pattern = QPdf::patternForBrush(b);
    if (pattern.isEmpty()) {
        if (b.style() != Qt::TexturePattern)
            return 0;
        QImage image = b.texture().toImage();
        bool bitmap = true;
        imageObject = addImage(image, &bitmap);
        QImage::Format f = image.format();
        if (f != QImage::Format_MonoLSB && f != QImage::Format_Mono) {
            paintType = 1; // Colored tiling
            *specifyColor = false;
        }
        w = image.width();
        h = image.height();
        QMatrix m(w, 0, 0, -h, 0, h);
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

int QPdfEnginePrivate::addImage(const QImage &img, bool *bitmap)
{
    if (img.isNull())
        return -1;

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
            memcpy(rawdata, image.scanLine(y), bytesPerLine * sizeof(char));
            rawdata += bytesPerLine;
        }
        return writeImage(data, w, h, d, 0, 0);
    }

    QByteArray imageData;
    QByteArray softMaskData;

    imageData.resize(3 * w * h);
    softMaskData.resize(w * h);

    uchar *data = (uchar *)imageData.data();
    uchar *sdata = (uchar *)softMaskData.data();
    bool hasAlpha = false;
    bool hasMask = false;
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

    return writeImage(imageData, w, h, 32, maskObject, softMaskObject);
}


void QPdfEnginePrivate::newPage()
{
    writePage();

    delete currentPage;
    currentPage = new QPdfPage;
    stroker.stream = currentPage;
    pages.append(requestObject());

    *currentPage << "/GSa gs /CSp cs /CSp CS\n";
    QMatrix tmp(1.0, 0.0, 0.0, -1.0, 0.0, height_);
    if (!fullPage)
        tmp.translate(resolution/3, resolution/3);
    *currentPage << QPdf::generateMatrix(tmp) << "q\n";
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
    int bufsize = vsprintf(buf, fmt, args);

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
            qWarning("QPdfStream::writeCompressed(): compress error");
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


void QPdfEnginePrivate::setDevice(QIODevice* device)
{
    stream->setDevice(device);
    streampos = 0;
}

void QPdfEnginePrivate::unsetDevice()
{
    stream->unsetDevice();
}

int QPdfEnginePrivate::writeImage(const QByteArray &data, int width, int height, int depth,
                                  int maskObject, int softMaskObject)
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
    if (do_compress)
        xprintf("/Filter /FlateDecode\n");
    xprintf(">>\nstream\n");
    int len = writeCompressed(data);
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
    time_t now;
    tm *newtime;

    time(&now);
    newtime = gmtime(&now);
    QByteArray y;

    if (newtime && newtime->tm_year+1900 > 1992)
        y += QByteArray::number(newtime->tm_year+1900);

    info = addXrefEntry(-1);
    xprintf("<<\n"
            "/Title (%s)\n"
            "/Author (%s)\n"
            "/Creator (%s)\n"
            "/Producer (Qt %s (C) 1992-%s Trolltech AS)\n",
            title.toUtf8().constData(), author.toUtf8().constData(), creator.toUtf8().constData(),
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
            pages.size(), 0, 0, width_, height_);

    xprintf("/ProcSet [/PDF /Text /ImageB /ImageC]\n"
            ">>\n"
            "endobj\n");
}


void QPdfEnginePrivate::embedFont(QPdf::Font *font)
{
    //qDebug() << "embedFont" << font->object_id;
    int fontObject = font->object_id;
    QByteArray fontData = font->toTruetype();
//     QFile ff("font.ttf");
//     ff.open(QFile::WriteOnly);
//     ff.write(fontData);
//     ff.close();

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
            "/FontName /" << properties.postscriptName << "\n"
            "/Flags " << 4 << "\n"
            "/FontBBox ["
          << properties.boundingBox.x()*scale
          << -(properties.boundingBox.y() + properties.boundingBox.height())*scale
          << (properties.boundingBox.x() + properties.boundingBox.width())*scale
          << -properties.boundingBox.y()*scale  << "]\n"
            "/ItalicAngle " << properties.italicAngle.toReal() << "\n"
            "/Ascent " << properties.ascent.toReal()*scale << "\n"
            "/Descent -" << properties.descent.toReal()*scale << "\n"
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
    for (QHash<QFontEngine::FaceId, QPdf::Font *>::iterator it = fonts.begin(); it != fonts.constEnd(); ++it) {
        embedFont(*it);
        delete *it;
    }
    fonts.clear();
}

void QPdfEnginePrivate::writePage()
{
    if (pages.empty())
        return;

    *currentPage << "Q\n";

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
