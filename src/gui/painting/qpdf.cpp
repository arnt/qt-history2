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
#include "qpdf_p.h"

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


// -------------------------- base engine, shared code between PS and PDF -----------------------

QPdfBaseEngine::QPdfBaseEngine(QPdfBaseEnginePrivate &d, PaintEngineFeatures f)
    : QPaintEngine(d, f)
{
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

    QPainterPath p;
    for (int i=0; i!=rectCount; ++i) {
        p.addRect(rects[i]);
    }
    drawPath(p);
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

    QBrush penBrush = d->pen.brush();
    if (d->hasPen && penBrush == Qt::SolidPattern && penBrush.isOpaque()) {
        // draw strokes natively in this case for better output
        *d->currentPage << "q\n";
        setPen();
        *d->currentPage << QPdf::generateMatrix(d->stroker.matrix);
        *d->currentPage << QPdf::generatePath(p, QMatrix(), d->hasBrush ? QPdf::FillAndStrokePath : QPdf::StrokePath);
        *d->currentPage << "Q\n";
    } else {
        if (d->hasBrush) {
            *d->currentPage << QPdf::generatePath(p, d->stroker.matrix, QPdf::FillPath);
        }
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

    *d->currentPage << "q " << QPdf::generateMatrix(d->stroker.matrix);

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
        d->stroker.matrix = state.matrix();

    if (flags & DirtyPen) {
        d->pen = state.pen();
        d->hasPen = d->pen != Qt::NoPen;
        d->stroker.setPen(d->pen);
    }
    if (flags & DirtyBrush) {
        d->brush = state.brush();
        d->hasBrush = d->brush != Qt::NoBrush;
    }
    if (flags & DirtyBrushOrigin) {
        d->brushOrigin = state.brushOrigin();
        flags |= DirtyBrush;
    }

    if (flags & DirtyBackground)
        d->backgroundBrush = state.backgroundBrush();
    if (flags & DirtyBackgroundMode)
        d->backgroundMode = state.backgroundMode();

    bool ce = d->clipEnabled;
    if (flags & DirtyClipEnabled)
        d->clipEnabled = state.isClipEnabled();
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

    if (ce != d->clipEnabled)
        flags |= DirtyClipPath;
    else if (!d->clipEnabled)
        flags &= ~DirtyClipPath;

    if (flags & DirtyClipPath) {
        *d->currentPage << "Q q\n";
        flags |= DirtyPen|DirtyBrush;
    }

    if (flags & DirtyClipPath) {
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
                    *d->currentPage << QPdf::generatePath(d->clips.at(i), QMatrix(), QPdf::ClipPath);
                }
            }
        }
    }

    if (flags & DirtyBrush)
        setBrush();
}

void QPdfBaseEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_D(QPdfBaseEngine);
    QPainterPath path = d->stroker.matrix.map(p);
    //qDebug() << "updateClipPath: " << matrix << p.boundingRect() << path.boundingRect();

    if (op == Qt::NoClip) {
        d->clipEnabled = false;
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

QPdfBaseEnginePrivate::QPdfBaseEnginePrivate()
{
    postscript = false;
    currentObject = 1;

    currentPage = new QPdfPage;
    stroker.stream = currentPage;

    backgroundMode = Qt::TransparentMode;
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

    glyph_t *glyphs = ti.deviceGlyphs;
    QFixedPoint *positions = ti.deviceGlyphPositions;

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
    for (int i = 0; i < ti.num_deviceGlyphs; ++i) {
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
        for (int i = 0; i < ti.num_deviceGlyphs; ++i) {
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

#endif

