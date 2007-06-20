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
#include "qplatformdefs.h"
#include <qdebug.h>
#include "qpdf_p.h"
#include <qfile.h>
#include <private/qmath_p.h>
#include "private/qcups_p.h"

extern int qt_defaultDpi();

#ifndef QT_NO_PRINTER

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

QByteArray QPdf::generatePath(const QPainterPath &path, const QTransform &matrix, PathFlags flags)
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

QByteArray QPdf::generateMatrix(const QTransform &matrix)
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
        off_min = qMin(off_min, qFloor(off));
        off_max = qMax(off_max, qCeil(off));
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
    if (!t->cosmeticPen)
        t->matrix.map(x, y, &x, &y);
    *t->stream << x << y << "m\n";
    t->first = false;
}

static void lineToHook(qfixed x, qfixed y, void *data)
{
    QPdf::Stroker *t = (QPdf::Stroker *)data;
    if (!t->cosmeticPen)
        t->matrix.map(x, y, &x, &y);
    *t->stream << x << y << "l\n";
}

static void cubicToHook(qfixed c1x, qfixed c1y,
                        qfixed c2x, qfixed c2y,
                        qfixed ex, qfixed ey,
                        void *data)
{
    QPdf::Stroker *t = (QPdf::Stroker *)data;
    if (!t->cosmeticPen) {
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
    cosmeticPen = true;
    basicStroker.setStrokeWidth(.1);
}

void QPdf::Stroker::setPen(const QPen &pen)
{
    if (pen.style() == Qt::NoPen) {
        stroker = 0;
        return;
    }
    qreal w = pen.widthF();
    bool zeroWidth = w < 0.0001;
    cosmeticPen = pen.isCosmetic();
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
        dashStroker.setDashOffset(pen.dashOffset());
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

    stroker->strokePath(path, this, cosmeticPen ? matrix : QTransform());
    *stream << "h f\n";
}

QByteArray QPdf::ascii85Encode(const QByteArray &input)
{
    int isize = input.size()/4*4;
    QByteArray output;
    output.resize(input.size()*5/4+7);
    char *out = output.data();
    const uchar *in = (const uchar *)input.constData();
    for (int i = 0; i < isize; i += 4) {
        uint val = (((uint)in[i])<<24) + (((uint)in[i+1])<<16) + (((uint)in[i+2])<<8) + (uint)in[i+3];
        if (val == 0) {
            *out = 'z';
            ++out;
        } else {
            char base[5];
            base[4] = val % 85;
            val /= 85;
            base[3] = val % 85;
            val /= 85;
            base[2] = val % 85;
            val /= 85;
            base[1] = val % 85;
            val /= 85;
            base[0] = val % 85;
            *(out++) = base[0] + '!';
            *(out++) = base[1] + '!';
            *(out++) = base[2] + '!';
            *(out++) = base[3] + '!';
            *(out++) = base[4] + '!';
        }
    }
    //write the last few bytes
    int remaining = input.size() - isize;
    if (remaining) {
        uint val = 0;
        for (int i = isize; i < input.size(); ++i)
            val = (val << 8) + in[i];
        val <<= 8*(4-remaining);
        char base[5];
        base[4] = val % 85;
        val /= 85;
        base[3] = val % 85;
        val /= 85;
        base[2] = val % 85;
        val /= 85;
        base[1] = val % 85;
        val /= 85;
        base[0] = val % 85;
        for (int i = 0; i < remaining+1; ++i)
            *(out++) = base[i] + '!';
    }
    *(out++) = '~';
    *(out++) = '>';
    output.resize(out-output.data());
    return output;
}

const char *QPdf::toHex(ushort u, char *buffer)
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

const char *QPdf::toHex(uchar u, char *buffer)
{
    int i = 1;
    while (i >= 0) {
        ushort hex = (u & 0x000f);
        if (hex < 0x0a)
            buffer[i] = '0'+hex;
        else
            buffer[i] = 'A'+(hex-0x0a);
        u = u >> 4;
        i--;
    }
    buffer[2] = '\0';
    return buffer;
}

#define Q_MM(n) int((n * 720 + 127) / 254)
#define Q_IN(n) int(n * 72)

static const QPdf::PaperSize paperSizes[QPrinter::NPageSize] =
{
    {  Q_MM(210), Q_MM(297) },      // A4
    {  Q_MM(176), Q_MM(250) },      // B5
    {  Q_IN(8.5), Q_IN(11) },       // Letter
    {  Q_IN(8.5), Q_IN(14) },       // Legal
    {  Q_IN(7.5), Q_IN(10) },       // Executive
    {  Q_MM(841), Q_MM(1189) },     // A0
    {  Q_MM(594), Q_MM(841) },      // A1
    {  Q_MM(420), Q_MM(594) },      // A2
    {  Q_MM(297), Q_MM(420) },      // A3
    {  Q_MM(148), Q_MM(210) },      // A5
    {  Q_MM(105), Q_MM(148) },      // A6
    {  Q_MM(74), Q_MM(105)},        // A7
    {  Q_MM(52), Q_MM(74) },        // A8
    {  Q_MM(37), Q_MM(52) },        // A9
    {  Q_MM(1000), Q_MM(1414) },    // B0
    {  Q_MM(707), Q_MM(1000) },     // B1
    {  Q_MM(31), Q_MM(44) },        // B10
    {  Q_MM(500), Q_MM(707) },      // B2
    {  Q_MM(353), Q_MM(500) },      // B3
    {  Q_MM(250), Q_MM(353) },      // B4
    {  Q_MM(125), Q_MM(176) },      // B6
    {  Q_MM(88), Q_MM(125) },       // B7
    {  Q_MM(62), Q_MM(88) },        // B8
    {  Q_MM(44), Q_MM(62) },        // B9
    {  Q_MM(162),    Q_MM(229) },   // C5E
    {  Q_IN(4.125),  Q_IN(9.5) },   // Comm10E
    {  Q_MM(110),    Q_MM(220) },   // DLE
    {  Q_IN(8.5),    Q_IN(13) },    // Folio
    {  Q_IN(17),     Q_IN(11) },    // Ledger
    {  Q_IN(11),     Q_IN(17) }     // Tabloid
};

static const char * const psToStr[QPrinter::NPageSize+1] =
{
    "A4", "B5", "Letter", "Legal", "Executive",
    "A0", "A1", "A2", "A3", "A5", "A6", "A7", "A8", "A9", "B0", "B1",
    "B10", "B2", "B3", "B4", "B6", "B7", "B8", "B9", "C5E", "Comm10E",
    "DLE", "Folio", "Ledger", "Tabloid", 0
};

QPdf::PaperSize QPdf::paperSize(QPrinter::PageSize pageSize)
{
    return paperSizes[pageSize];
}

const char *QPdf::paperSizeToString(QPrinter::PageSize pageSize)
{
    return psToStr[pageSize];
}


QByteArray QPdf::stripSpecialCharacters(const QByteArray &string)
{
    QByteArray s = string;
    s.replace(" ", "");
    s.replace("(", "");
    s.replace(")", "");
    s.replace("<", "");
    s.replace(">", "");
    s.replace("[", "");
    s.replace("]", "");
    s.replace("{", "");
    s.replace("}", "");
    s.replace("/", "");
    s.replace("%", "");
    return s;
}


// -------------------------- base engine, shared code between PS and PDF -----------------------

QPdfBaseEngine::QPdfBaseEngine(QPdfBaseEnginePrivate &dd, PaintEngineFeatures f)
    : QPaintEngine(dd, f)
{
    Q_D(QPdfBaseEngine);
#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
    if (QCUPSSupport::isAvailable()) {
        QCUPSSupport cups;
        const cups_dest_t* printers = cups.availablePrinters();
        int prnCount = cups.availablePrintersCount();

        for (int i = 0; i <  prnCount; ++i) {
            if (printers[i].is_default) {
                d->printerName = QString::fromLocal8Bit(printers[i].name);
                break;
            }
        }

    } else
#endif
    {
        d->printerName = QString::fromLocal8Bit(qgetenv("PRINTER"));
        if (d->printerName.isEmpty())
            d->printerName = QString::fromLocal8Bit(qgetenv("LPDEST"));
    }
}

void QPdfBaseEngine::drawPoints (const QPointF *points, int pointCount)
{
    Q_D(QPdfBaseEngine);
    if (!points || !d->hasPen)
        return;

    QPainterPath p;
    for (int i=0; i!=pointCount;++i) {
        p.moveTo(points[i]);
        p.lineTo(points[i] + QPointF(0, 0.001));
    }
    drawPath(p);
}

void QPdfBaseEngine::drawLines (const QLineF *lines, int lineCount)
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

void QPdfBaseEngine::drawRects (const QRectF *rects, int rectCount)
{
    if (!rects)
        return;

    Q_D(QPdfBaseEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    if (!d->hasPen && !d->hasBrush)
        return;

    QBrush penBrush = d->pen.brush();
    if (d->simplePen || !d->hasPen) {
        // draw strokes natively in this case for better output
        if(!d->simplePen && !d->stroker.matrix.isIdentity())
            *d->currentPage << "q\n" << QPdf::generateMatrix(d->stroker.matrix);
        for (int i = 0; i < rectCount; ++i)
            *d->currentPage << rects[i].x() << rects[i].y() << rects[i].width() << rects[i].height() << "re\n";
        *d->currentPage << (d->hasPen ? (d->hasBrush ? "B\n" : "S\n") : "f\n");
        if(!d->simplePen && !d->stroker.matrix.isIdentity())
            *d->currentPage << "Q\n";
    } else {
        QPainterPath p;
        for (int i=0; i!=rectCount; ++i)
            p.addRect(rects[i]);
        drawPath(p);
    }
}

void QPdfBaseEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    if (!points || !pointCount)
        return;
    Q_D(QPdfBaseEngine);

    bool hb = d->hasBrush;
    QPainterPath p;

    switch(mode) {
    case OddEvenMode:
        p.setFillRule(Qt::OddEvenFill);
        break;
    case ConvexMode:
    case WindingMode:
        p.setFillRule(Qt::WindingFill);
        break;
    case PolylineMode:
        d->hasBrush = false;
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

    d->hasBrush = hb;
}

void QPdfBaseEngine::drawPath (const QPainterPath &p)
{
    Q_D(QPdfBaseEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    if (!d->hasPen && !d->hasBrush)
        return;

    if (d->simplePen) {
        // draw strokes natively in this case for better output
        *d->currentPage << QPdf::generatePath(p, QTransform(), d->hasBrush ? QPdf::FillAndStrokePath : QPdf::StrokePath);
    } else {
        if (d->hasBrush)
            *d->currentPage << QPdf::generatePath(p, d->stroker.matrix, QPdf::FillPath);
        if (d->hasPen) {
            *d->currentPage << "q\n";
            QBrush b = d->brush;
            d->brush = d->pen.brush();
            setBrush();
            d->stroker.strokePath(p);
            *d->currentPage << "Q\n";
            d->brush = b;
        }
    }
}

void QPdfBaseEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QPdfBaseEngine);

    if (!d->hasPen || (d->clipEnabled && d->allClipped))
        return;

    *d->currentPage << "q\n";
    if(!d->simplePen)
        *d->currentPage << QPdf::generateMatrix(d->stroker.matrix);

    bool hp = d->hasPen;
    d->hasPen = false;
    QBrush b = d->brush;
    d->brush = d->pen.brush();
    setBrush();

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    Q_ASSERT(ti.fontEngine->type() != QFontEngine::Multi);
    d->drawTextItem(p, ti);
    d->hasPen = hp;
    d->brush = b;
    *d->currentPage << "Q\n";
}


void QPdfBaseEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QPdfBaseEngine);
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform)
        d->stroker.matrix = state.transform();

    if (flags & DirtyPen) {
        d->pen = state.pen();
        d->hasPen = d->pen.style() != Qt::NoPen;
        d->stroker.setPen(d->pen);
        QBrush penBrush = d->pen.brush();
        bool oldSimple = d->simplePen;
        d->simplePen = (d->hasPen && (penBrush.style() == Qt::SolidPattern) && penBrush.isOpaque());
        if (oldSimple != d->simplePen)
            flags |= DirtyTransform;
    }
    if (flags & DirtyBrush) {
        d->brush = state.brush();
        d->hasBrush = d->brush.style() != Qt::NoBrush;
    }
    if (flags & DirtyBrushOrigin) {
        d->brushOrigin = state.brushOrigin();
        flags |= DirtyBrush;
    }

    bool ce = d->clipEnabled;
    if (flags & DirtyClipPath) {
        d->clipEnabled = true;
        updateClipPath(state.clipPath(), state.clipOperation());
    } else if (flags & DirtyClipRegion) {
        d->clipEnabled = true;
        QPainterPath path;
        QVector<QRect> rects = state.clipRegion().rects();
        for (int i = 0; i < rects.size(); ++i)
            path.addRect(rects.at(i));
        updateClipPath(path, state.clipOperation());
        flags |= DirtyClipPath;
    } else if (flags & DirtyClipEnabled) {
        d->clipEnabled = state.isClipEnabled();
    }

    if (ce != d->clipEnabled)
        flags |= DirtyClipPath;
    else if (!d->clipEnabled)
        flags &= ~DirtyClipPath;

    setupGraphicsState(flags);
}

void QPdfBaseEngine::setupGraphicsState(QPaintEngine::DirtyFlags flags)
{
    Q_D(QPdfBaseEngine);
    if (flags & DirtyClipPath)
        flags |= DirtyTransform|DirtyPen|DirtyBrush;

    if (flags & DirtyTransform) {
        *d->currentPage << "Q\n";
        flags |= DirtyPen|DirtyBrush;
    }

    if (flags & DirtyClipPath) {
        *d->currentPage << "Q q\n";

        d->allClipped = false;
        if (d->clipEnabled && !d->clips.isEmpty()) {
            for (int i = 0; i < d->clips.size(); ++i) {
                if (d->clips.at(i).isEmpty()) {
                    d->allClipped = true;
                    break;
                }
            }
            if (!d->allClipped) {
                for (int i = 0; i < d->clips.size(); ++i) {
                    *d->currentPage << QPdf::generatePath(d->clips.at(i), QTransform(), QPdf::ClipPath);
                }
            }
        }
    }

    if (flags & DirtyTransform) {
        *d->currentPage << "q\n";
        if (d->simplePen && !d->stroker.matrix.isIdentity())
            *d->currentPage << QPdf::generateMatrix(d->stroker.matrix);
    }
    if (flags & DirtyBrush)
        setBrush();
    if (d->simplePen && (flags & DirtyPen))
        setPen();
}

void QPdfBaseEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_D(QPdfBaseEngine);
    QPainterPath path = d->stroker.matrix.map(p);
    //qDebug() << "updateClipPath: " << d->stroker.matrix << p.boundingRect() << path.boundingRect() << op;

    if (op == Qt::NoClip) {
        d->clipEnabled = false;
        d->clips.clear();
    } else if (op == Qt::ReplaceClip) {
        d->clips.clear();
        d->clips.append(path);
    } else if (op == Qt::IntersectClip) {
        d->clips.append(path);
    } else { // UniteClip
        // ask the painter for the current clipping path. that's the easiest solution
        path = painter()->clipPath();
        path = d->stroker.matrix.map(path);
        d->clips.clear();
        d->clips.append(path);
    }
}

void QPdfBaseEngine::setPen()
{
    Q_D(QPdfBaseEngine);
    if (d->pen.style() == Qt::NoPen)
        return;
    QBrush b = d->pen.brush();
    Q_ASSERT(b.style() == Qt::SolidPattern && b.isOpaque());

    QColor rgba = b.color();
    *d->currentPage << rgba.redF()
                   << rgba.greenF()
                   << rgba.blueF()
                   << "SCN\n";

    *d->currentPage << d->pen.widthF() << "w ";

    int pdfCapStyle = 0;
    switch(d->pen.capStyle()) {
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
    switch(d->pen.joinStyle()) {
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

    *d->currentPage << QPdf::generateDashes(d->pen) << " 0 d\n";
}

bool QPdfBaseEngine::newPage()
{
    setupGraphicsState(DirtyBrush|DirtyPen|DirtyClipPath);
    return true;
}


int QPdfBaseEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const
{
    Q_D(const QPdfBaseEngine);
    int val;
    QRect r = d->fullPage ? d->paperRect() : d->pageRect();
    switch (metricType) {
    case QPaintDevice::PdmWidth:
        val = r.width();
        break;
    case QPaintDevice::PdmHeight:
        val = r.height();
        break;
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmDpiY:
        val = d->resolution;
        break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
        val = 1200;
        break;
    case QPaintDevice::PdmWidthMM:
        val = qRound(r.width()*25.4/d->resolution);
        break;
    case QPaintDevice::PdmHeightMM:
        val = qRound(r.height()*25.4/d->resolution);
        break;
    case QPaintDevice::PdmNumColors:
        val = INT_MAX;
        break;
    case QPaintDevice::PdmDepth:
        val = 32;
        break;
    default:
        qWarning("QPrinter::metric: Invalid metric command");
        return 0;
    }
    return val;
}

void QPdfBaseEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QPdfBaseEngine);
    switch (key) {
    case PPK_CollateCopies:
        d->collate = value.toBool();
        break;
    case PPK_ColorMode:
        d->colorMode = QPrinter::ColorMode(value.toInt());
        break;
    case PPK_Creator:
        d->creator = value.toString();
        break;
    case PPK_DocumentName:
        d->title = value.toString();
        break;
    case PPK_FullPage:
        d->fullPage = value.toBool();
        break;
    case PPK_NumberOfCopies:
        d->copies = value.toInt();
        break;
    case PPK_Orientation:
        d->orientation = QPrinter::Orientation(value.toInt());
        break;
    case PPK_OutputFileName:
        d->outputFileName = value.toString();
        break;
    case PPK_PageOrder:
        d->pageOrder = QPrinter::PageOrder(value.toInt());
        break;
    case PPK_PageSize:
        d->pageSize = QPrinter::PageSize(value.toInt());
        break;
    case PPK_PaperSource:
        d->paperSource = QPrinter::PaperSource(value.toInt());
        break;
    case PPK_PrinterName:
        d->printerName = value.toString();
        break;
    case PPK_PrinterProgram:
        d->printProgram = value.toString();
        break;
    case PPK_Resolution:
        d->resolution = value.toInt();
        break;
    case PPK_SelectionOption:
        d->selectionOption = value.toString();
        break;
    case PPK_FontEmbedding:
        d->embedFonts = value.toBool();
        break;
    case PPK_Duplex:
        d->duplex = value.toBool();
        break;
    case PPK_CupsPageRect:
        d->cupsPageRect = value.toRect();
        break;
    case PPK_CupsPaperRect:
        d->cupsPaperRect = value.toRect();
        break;
    case PPK_CupsOptions:
        d->cupsOptions = value.toStringList();
        break;
    case PPK_CupsStringPageSize:
        d->cupsStringPageSize = value.toString();
        break;
    default:
        break;
    }
}

QVariant QPdfBaseEngine::property(PrintEnginePropertyKey key) const
{
    Q_D(const QPdfBaseEngine);

    QVariant ret;
    switch (key) {
    case PPK_CollateCopies:
        ret = d->collate;
        break;
    case PPK_ColorMode:
        ret = d->colorMode;
        break;
    case PPK_Creator:
        ret = d->creator;
        break;
    case PPK_DocumentName:
        ret = d->title;
        break;
    case PPK_FullPage:
        ret = d->fullPage;
        break;
    case PPK_NumberOfCopies:
#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
        if (QCUPSSupport::isAvailable())
            ret = 1;
        else
#endif
            ret = d->copies;
        break;
    case PPK_Orientation:
        ret = d->orientation;
        break;
    case PPK_OutputFileName:
        ret = d->outputFileName;
        break;
    case PPK_PageOrder:
        ret = d->pageOrder;
        break;
    case PPK_PageSize:
        ret = d->pageSize;
        break;
    case PPK_PaperSource:
        ret = d->paperSource;
        break;
    case PPK_PrinterName:
        ret = d->printerName;
        break;
    case PPK_PrinterProgram:
        ret = d->printProgram;
        break;
    case PPK_Resolution:
        ret = d->resolution;
        break;
    case PPK_SupportedResolutions:
        ret = QList<QVariant>() << 72;
        break;
    case PPK_PaperRect:
        ret = d->paperRect();
        break;
    case PPK_PageRect:
        ret = d->pageRect();
        break;
    case PPK_SelectionOption:
        ret = d->selectionOption;
        break;
    case PPK_FontEmbedding:
        ret = d->embedFonts;
        break;
    case PPK_Duplex:
        ret = d->duplex;
        break;
    case PPK_CupsPageRect:
        ret = d->cupsPageRect;
        break;
    case PPK_CupsPaperRect:
        ret = d->cupsPaperRect;
        break;
    case PPK_CupsOptions:
        ret = d->cupsOptions;
        break;
    case PPK_CupsStringPageSize:
        ret = d->cupsStringPageSize;
        break;
    default:
        break;
    }
    return ret;
}

QPdfBaseEnginePrivate::QPdfBaseEnginePrivate(QPrinter::PrinterMode m)
    : clipEnabled(false), allClipped(false), hasPen(true), hasBrush(false), simplePen(false),
      outDevice(0), fd(-1),
      duplex(false), collate(false), fullPage(false), embedFonts(true), copies(1),
      pageOrder(QPrinter::FirstPageFirst), orientation(QPrinter::Portrait),
      pageSize(QPrinter::A4), colorMode(QPrinter::Color), paperSource(QPrinter::Auto)

{
    resolution = 72;
    if (m == QPrinter::HighResolution)
        resolution = 1200;
    else if (m == QPrinter::ScreenResolution)
        resolution = qt_defaultDpi();

    postscript = false;
    currentObject = 1;
    currentPage = 0;
    stroker.stream = 0;
}

bool QPdfBaseEngine::begin(QPaintDevice *pdev)
{
    Q_D(QPdfBaseEngine);
    d->pdev = pdev;

    d->postscript = false;
    d->currentObject = 1;

    d->currentPage = new QPdfPage;
    d->stroker.stream = d->currentPage;

    return d->openPrintDevice();
}

bool QPdfBaseEngine::end()
{
    Q_D(QPdfBaseEngine);
    qDeleteAll(d->fonts);
    d->fonts.clear();
    delete d->currentPage;
    d->currentPage = 0;

    d->closePrintDevice();
    return true;
}

#ifndef QT_NO_LPR
static void closeAllOpenFds()
{
    // hack time... getting the maximum number of open
    // files, if possible.  if not we assume it's the
    // larger of 256 and the fd we got
    int i;
#if defined(_SC_OPEN_MAX)
    i = (int)sysconf(_SC_OPEN_MAX);
#elif defined(_POSIX_OPEN_MAX)
    i = (int)_POSIX_OPEN_MAX;
#elif defined(OPEN_MAX)
    i = (int)OPEN_MAX;
#else
    i = 256;
#endif
    // leave stdin/out/err untouched
    while(--i > 2)
        ::close(i);
}
#endif

bool QPdfBaseEnginePrivate::openPrintDevice()
{
    if(outDevice)
        return false;

    if (!outputFileName.isEmpty()) {
        QFile *file = new QFile(outputFileName);
        outDevice = file;
        file->open(QFile::WriteOnly|QFile::Truncate);
#ifndef QT_NO_LPR
    } else {
        QString pr;
        if (!printerName.isEmpty())
            pr = printerName;
        int fds[2];
        if (pipe(fds) != 0) {
            qWarning("QPSPrinter: Could not open pipe to print");
            return false;
        }

        pid_t pid = fork();
        if (pid == 0) {       // child process
            // if possible, exit quickly, so the actual lp/lpr
            // becomes a child of init, and ::waitpid() is
            // guaranteed not to wait.
            if (fork() > 0) {
                closeAllOpenFds();

                // try to replace this process with "true" - this prevents
                // global destructors from being called (that could possibly
                // do wrong things to the parent process)
                (void)execlp("true", "true", (char *)0);
                (void)execl("/bin/true", "true", (char *)0);
                (void)execl("/usr/bin/true", "true", (char *)0);
                ::exit(0);
            }
            dup2(fds[0], 0);

            closeAllOpenFds();

            if (!printProgram.isEmpty()) {
                if (!selectionOption.isEmpty())
                    pr.prepend(selectionOption);
                else
                    pr.prepend(QLatin1String("-P"));
                (void)execlp(printProgram.toLocal8Bit().data(), printProgram.toLocal8Bit().data(),
                             pr.toLocal8Bit().data(), (char *)0);
#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
            } else if (QCUPSSupport::isAvailable()) {

                QList<QByteArray> cupsArgList;

                cupsArgList << "lpr";

                if (!printerName.isEmpty()) {
                    cupsArgList << "-P";
                    cupsArgList << printerName.toLocal8Bit();
                }

                if (!cupsStringPageSize.isEmpty()) {
                    cupsArgList << "-o";
                    cupsArgList << QByteArray("media=") + cupsStringPageSize.toLocal8Bit();
                }

                if (copies > 1) {
                    cupsArgList << "-#";
                    cupsArgList << QByteArray::number(copies);
                }

                if (collate) {
                    cupsArgList << "-o";
                    cupsArgList << "Collate=True";
                }

                if (duplex) {
                    cupsArgList << "-o";
                    if (orientation == QPrinter::Portrait)
                        cupsArgList << "sides=two-sided-long-edge";
                    else
                        cupsArgList << "sides=two-sided-short-edge";
                }

                if (QCUPSSupport::cupsVersion() >= 10200 && orientation == QPrinter::Landscape) {
                    cupsArgList << "-o";
                    cupsArgList << "landscape";
                }

                if (!title.isEmpty()) {
                    cupsArgList << "-J";
                    cupsArgList << title.toLocal8Bit();
                }

                QStringList::const_iterator it = cupsOptions.constBegin();
                while (it != cupsOptions.constEnd()) {
                    cupsArgList << "-o";
                    cupsArgList << (*it).toLocal8Bit() + "=" + (*(it+1)).toLocal8Bit();
                    it += 2;
                }

                char** lprargs = new char*[cupsArgList.count() + 1];
                int i;
                for (i = 0; i < cupsArgList.count(); ++i) {
                    lprargs[i] = cupsArgList[i].data();
                }
                lprargs[i] = 0;

                // if the CUPS is available we expect lpr around
                (void)execvp( "lpr", lprargs );
                (void)execv( "/bin/lpr", lprargs);
                (void)execv( "/usr/bin/lpr", lprargs);

#endif
            } else {
                // if no print program has been specified, be smart
                // about the option string too.
                QList<QByteArray> lprhack;
                QList<QByteArray> lphack;
                QByteArray media;
                if (!pr.isEmpty() || !selectionOption.isEmpty()) {
                    if (!selectionOption.isEmpty()) {
                        QStringList list = selectionOption.split(QLatin1Char(' '));
                        for (int i = 0; i < list.size(); ++i)
                            lprhack.append(list.at(i).toLocal8Bit());
                        lphack = lprhack;
                    } else {
                        lprhack.append("-P");
                        lphack.append("-d");
                    }
                    lprhack.append(pr.toLocal8Bit());
                    lphack.append(pr.toLocal8Bit());
                }
                char ** lpargs = new char *[lphack.size()+6];
                lpargs[0] = "lp";
                int i;
                for (i = 0; i < lphack.size(); ++i)
                    lpargs[i+1] = (char *)lphack.at(i).constData();
#ifndef Q_OS_OSF
                if (QPdf::paperSizeToString(pageSize)) {
                    lpargs[++i] = "-o";
                    lpargs[++i] = (char *)QPdf::paperSizeToString(pageSize);
                    lpargs[++i] = "-o";
                    media = "media=";
                    media += QPdf::paperSizeToString(pageSize);
                    lpargs[++i] = (char *)media.constData();
                }
#endif
                lpargs[++i] = 0;
                char **lprargs = new char *[lprhack.size()+2];
                lprargs[0] = "lpr";
                for (int i = 0; i < lprhack.size(); ++i)
                    lprargs[i+1] = (char *)lprhack[i].constData();
                lprargs[lprhack.size() + 1] = 0;
                (void)execvp("lp", lpargs);
                (void)execvp("lpr", lprargs);
                (void)execv("/bin/lp", lpargs);
                (void)execv("/bin/lpr", lprargs);
                (void)execv("/usr/bin/lp", lpargs);
                (void)execv("/usr/bin/lpr", lprargs);
            }
            // if we couldn't exec anything, close the fd,
            // wait for a second so the parent process (the
            // child of the GUI process) has exited.  then
            // exit.
            ::close(0);
            (void)::sleep(1);
            ::exit(0);
        }
        // parent process
        ::close(fds[0]);
        fd = fds[1];
        (void)::waitpid(pid, 0, 0);

        if (fd < 0)
            return false;

        outDevice = new QFile();
        static_cast<QFile *>(outDevice)->open(fd, QIODevice::WriteOnly);
#endif
    }

    return true;
}

void QPdfBaseEnginePrivate::closePrintDevice()
{
    if (!outDevice)
        return;
    outDevice->close();
    if (fd >= 0)
#if defined(Q_OS_WIN) && defined(_MSC_VER) && _MSC_VER >= 1400
        ::_close(fd);
#else
        ::close(fd);
#endif
    fd = -1;
    delete outDevice;
    outDevice = 0;
}

QPdfBaseEnginePrivate::~QPdfBaseEnginePrivate()
{
    qDeleteAll(fonts);
    delete currentPage;
}

void QPdfBaseEnginePrivate::drawTextItem(const QPointF &p, const QTextItemInt &ti)
{
    Q_Q(QPdfBaseEngine);

    QFontEngine *fe = ti.fontEngine;

    QFontEngine::FaceId face_id = fe->faceId();
    bool noEmbed = false;
    if (face_id.filename.isEmpty()
        || (!postscript && ((fe->fsType & 0x200) /* bitmap embedding only */
                            || (fe->fsType == 2) /* no embedding allowed */))) {
        *currentPage << "Q\n";
        q->QPaintEngine::drawTextItem(p, ti);
        *currentPage << "q\n";
        if (face_id.filename.isEmpty())
            return;
        noEmbed = true;
    }

    QFontSubset *font = fonts.value(face_id, 0);
    if (!font) {
        font = new QFontSubset(fe, requestObject());
        font->noEmbed = noEmbed;
    }
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
    QTransform m;
    m.translate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, m, ti.flags,
                                     glyphs, positions);
    if (glyphs.size() == 0)
        return;
    int synthesized = ti.fontEngine->synthesized();
    qreal stretch = synthesized & QFontEngine::SynthesizedStretch ? ti.fontEngine->fontDef.stretch/100. : 1.;

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
        int g = font->addGlyph(glyphs[i]);
        *currentPage << x - last_x << last_y - y << "Td <"
                     << QPdf::toHex((ushort)g, buf) << "> Tj\n";
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
            int g = font->addGlyph(glyphs[i]);
            *currentPage << x - last_x << last_y - y << "Td <"
                        << QPdf::toHex((ushort)g, buf) << "> Tj\n";
            last_x = x;
            last_y = y;
        }
        *currentPage << "EMC\n";
    }
#endif

    *currentPage << "ET\n";
}

QRect QPdfBaseEnginePrivate::paperRect() const
{
    int w;
    int h;
#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
    if (QCUPSSupport::isAvailable() && !cupsPaperRect.isNull()) {
        QRect r = cupsPaperRect;
        w = r.width();
        h = r.height();
    } else
#endif
    {
        QPdf::PaperSize s = QPdf::paperSize(pageSize);
        w = s.width;
        h = s.height;
    }
    w = qRound(w*resolution/72.);
    h = qRound(h*resolution/72.);
    if (orientation == QPrinter::Portrait)
        return QRect(0, 0, w, h);
    else
        return QRect(0, 0, h, w);
}

QRect QPdfBaseEnginePrivate::pageRect() const
{
    if(fullPage)
        return paperRect();

    QRect r;

#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
    if (QCUPSSupport::isAvailable() && !cupsPageRect.isNull()) {
        r = cupsPageRect;
        if (r == cupsPaperRect)
            // if cups doesn't define any margins, give it at least approx 3.5 mm
            r = QRect(10, 10, r.width() - 20, r.height() - 20);
    } else
#endif
    {
        QPdf::PaperSize s = QPdf::paperSize(pageSize);
        r = QRect(72/3, 72/3, s.width - 2*72/3, s.height - 2*72/3);
    }
    int x = qRound(r.left()*resolution/72.);
    int y = qRound(r.top()*resolution/72.);
    int w = qRound(r.width()*resolution/72.);
    int h = qRound(r.height()*resolution/72.);
    if (orientation == QPrinter::Portrait)
        return QRect(x, y, w, h);
    else
        return QRect(y, x, h, w);
}


#endif

