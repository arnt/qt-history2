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

#include <private/qpainter_p.h>
#include <private/qpaintengine_p.h>
#include "qfile.h"
#include "qimage.h"
#include "qlist.h"
#include "qmap.h"
#include "qpaintdevicemetrics.h"
#include "qpaintengine_svg_p.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qregexp.h"
#include "qtextstream.h"

#include <math.h>

static const double deg2rad = 0.017453292519943295769;        // pi/180
static const char piData[] = "version=\"1.0\" standalone=\"no\"";
static const char publicId[] = "-//W3C//DTD SVG 20001102//EN";
static const char systemId[] = "http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd";

struct QM_EXPORT_SVG ImgElement {
    QDomElement element;
    QImage image;
    Q_DUMMY_COMPARISON_OPERATOR(ImgElement)
};

struct QM_EXPORT_SVG PixElement {
    QDomElement element;
    QPixmap pixmap;
    Q_DUMMY_COMPARISON_OPERATOR(PixElement)
};

struct QSVGPaintEngineState {
    int textx, texty; // current text position
    int textalign; // text alignment
    Q_DUMMY_COMPARISON_OPERATOR(QSVGPaintEngineState)
};

typedef QList<ImgElement> ImageList;
typedef QList<PixElement> PixmapList;
typedef QList<QSVGPaintEngineState> StateList;

enum ElementType {
    InvalidElement = 0,
    AnchorElement,
    CircleElement,
    ClipElement,
    CommentElement,
    DescElement,
    EllipseElement,
    GroupElement,
    ImageElement,
    LineElement,
    PolylineElement,
    PolygonElement,
    PathElement,
    RectElement,
    SvgElement,
    TextElement,
    TitleElement,
    TSpanElement
};

typedef QMap<QString,ElementType> QSvgTypeMap;
static QSvgTypeMap *qSvgTypeMap=0; // element types
static QMap<QString,QString> *qSvgColMap=0; // recognized color keyword names

class QSVGPaintEnginePrivate : public QPaintEnginePrivate, public QPaintCommands
{
    Q_DECLARE_PUBLIC(QSVGPaintEngine)

public:
    QSVGPaintEnginePrivate()
        : dirtyTransform(false), dirtyStyle(false), currentClip(0),
          dev(0), wwidth(0), wheight(0) {}
    void appendChild(QDomElement &e, PaintCommand c);
    void applyStyle(QDomElement *e, PaintCommand c) const;
    void applyTransform(QDomElement *e) const;
    double parseLen(const QString &str, bool *ok=0, bool horiz=true) const;
    int lenToInt(const QDomNamedNodeMap &map, const QString &attr, int def = 0) const;
    double lenToDouble(const QDomNamedNodeMap &map, const QString &attr, int def = 0) const;
    bool play(const QDomNode &node, QPainter *p);
    void setTransform(const QString &tr, QPainter *p);
    void restoreAttributes(QPainter *p);
    void saveAttributes(QPainter *p);
    void setStyle(const QString &s, QPainter *p);
    void setStyleProperty(const QString &prop, const QString &val, QPen *pen, QFont *font,
                          int *talign, QPainter *p);
    void drawPath(const QString &data, QPainter *p);
    QColor parseColor(const QString &col);


    bool dirtyTransform;
    bool dirtyStyle;
    QRect brect; // bounding rectangle
    QDomDocument doc; // document tree
    QDomNode current;

    ImageList images; // old private
    PixmapList pixmaps;
    StateList stack;
    int currentClip;

//     QPoint curPt;
    QSVGPaintEngineState *curr;
//     QPainter *pt; // only used by recursive play() et al
    QPen cpen;
    QBrush cbrush;
    QFont cfont;
    QWMatrix worldMatrix;
    const QPaintDevice *dev;
    int wwidth;
    int wheight;
};

#define d d_func()
#define q q_func()

QSVGPaintEngine::QSVGPaintEngine()
    : QPaintEngine(*(new QSVGPaintEnginePrivate))
{
    QDomImplementation domImpl;
    QDomDocumentType docType = domImpl.createDocumentType("svg", publicId, systemId);
    d->doc = domImpl.createDocument("http://www.w3.org/2000/svg", "svg", docType);
    d->doc.insertBefore(d->doc.createProcessingInstruction("xml", piData), d->doc.firstChild());
    d->current = d->doc.documentElement();
    d->images.clear();
    d->pixmaps.clear();
}

QSVGPaintEngine::QSVGPaintEngine(QSVGPaintEnginePrivate &dptr)
    : QPaintEngine(dptr)
{
    QDomImplementation domImpl;
    QDomDocumentType docType = domImpl.createDocumentType("svg", publicId, systemId);
    d->doc = domImpl.createDocument("http://www.w3.org/2000/svg", "svg", docType);
    d->doc.insertBefore(d->doc.createProcessingInstruction("xml", piData), d->doc.firstChild());
    d->current = d->doc.documentElement();
    d->images.clear();
    d->pixmaps.clear();
}

QSVGPaintEngine::~QSVGPaintEngine()
{
    delete qSvgTypeMap; qSvgTypeMap = 0;        // static
    delete qSvgColMap; qSvgColMap = 0;
}

bool QSVGPaintEngine::begin(QPaintDevice *pdev, QPainterState *, bool)
{
//     QDomImplementation domImpl;
//     QDomDocumentType docType = domImpl.createDocumentType("svg", publicId, systemId);
//     d->doc = domImpl.createDocument("http://www.w3.org/2000/svg", "svg", docType);
//     d->doc.insertBefore(d->doc.createProcessingInstruction("xml", piData), d->doc.firstChild());
//     d->current = d->doc.documentElement();
//     d->images.clear();
//     d->pixmaps.clear();
    d->dirtyTransform = d->dirtyStyle = false; // ### <- what are the #'s for??
    d->dev = pdev;
    setActive(true);
    return true;
}

bool QSVGPaintEngine::end()
{
    setActive(false);
    return true;
}

void QSVGPaintEngine::updatePen(QPainterState *ps)
{
    d->cpen = ps->pen;
    d->dirtyStyle = true;
}

void QSVGPaintEngine::updateBrush(QPainterState *ps)
{
    d->cbrush = ps->brush;
    d->dirtyStyle = true;
}

void QSVGPaintEngine::updateFont(QPainterState *ps)
{
    d->cfont = ps->font;
    d->dirtyStyle = true;
}

void QSVGPaintEngine::updateRasterOp(QPainterState *)
{
    d->dirtyStyle = true;
}

void QSVGPaintEngine::updateBackground(QPainterState *)
{
    d->dirtyStyle = true;
}

void QSVGPaintEngine::updateXForm(QPainterState *ps)
{
    d->dirtyTransform = true;
    d->worldMatrix = ps->worldMatrix;
    d->wwidth = ps->ww;
    d->wheight = ps->wh;
}

void QSVGPaintEngine::updateClipRegion(QPainterState *ps)
{
    if (!ps->clipEnabled)
        return;

    QDomElement e;
    d->currentClip++;
    e = d->doc.createElement("clipPath");
    e.setAttribute("id", QString("clip%1").arg(d->currentClip));
    QRect br = ps->clipRegion.boundingRect();
    QDomElement ce;
    if (ps->clipRegion.rects().count() == 1) {
        // Then it's just a rect, boundingRect() will do
        ce = d->doc.createElement("rect");
        ce.setAttribute("x", br.x());
        ce.setAttribute("y", br.y());
        ce.setAttribute("width", br.width());
        ce.setAttribute("height", br.height());
    } else {
        // It's an ellipse, calculate the ellipse
        // from the boundingRect()
        ce = d->doc.createElement("ellipse");
        double cx = br.x() + (br.width() / 2.0);
        double cy = br.y() + (br.height() / 2.0);
        ce.setAttribute("cx", cx);
        ce.setAttribute("cy", cy);
        ce.setAttribute("rx", cx - br.x());
        ce.setAttribute("ry", cy - br.y());
    }
    e.appendChild(ce);
    d->appendChild(e, PdcSetClipRegion);
}

void QSVGPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    QDomElement e;

    e = d->doc.createElement("line");
    e.setAttribute("x1", p1.x());
    e.setAttribute("y1", p1.y());
    e.setAttribute("x2", p2.x());
    e.setAttribute("y2", p2.y());
    d->appendChild(e, PdcDrawLine);
}

void QSVGPaintEngine::drawRect(const QRect &r)
{
    QDomElement e;
    int x, y, width, height;

    e = d->doc.createElement("rect");
    x = r.x();
    y = r.y();
    width = r.width();
    height = r.height();

    e.setAttribute("x", x);
    e.setAttribute("y", y);
    e.setAttribute("width", width);
    e.setAttribute("height", height);
    d->appendChild(e, PdcDrawRect);
}

void QSVGPaintEngine::drawPoint(const QPoint &p)
{
    drawLine(p, p);
}

void QSVGPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    for (int i = index; i < npoints; ++i)
        drawLine(pa[i], pa[i]); // should be drawPoint(), but saves one fu call
}

void QSVGPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    QDomElement e;
    int x, y, width, height;

    // ### code dup - maybe operate with a current dom element?
    e = d->doc.createElement("rect");
    x = r.x();
    y = r.y();
    width = r.width();
    height = r.height();

    e.setAttribute("x", x);
    e.setAttribute("y", y);
    e.setAttribute("width", width);
    e.setAttribute("height", height);
    e.setAttribute("rx", (xRnd*r.width())/200);
    e.setAttribute("ry", (yRnd*r.height())/200);
    d->appendChild(e, PdcDrawRoundRect);
}

void QSVGPaintEngine::drawEllipse(const QRect &r)
{
    QDomElement e;

    if (r.width() == r.height()) {
        e = d->doc.createElement("circle");
        double cx = r.x() + (r.width() / 2.0);
        double cy = r.y() + (r.height() / 2.0);
        e.setAttribute("cx", cx);
        e.setAttribute("cy", cy);
        e.setAttribute("r", cx - r.x());
    } else {
        e = d->doc.createElement("ellipse");
        double cx = r.x() + (r.width() / 2.0);
        double cy = r.y() + (r.height() / 2.0);
        e.setAttribute("cx", cx);
        e.setAttribute("cy", cy);
        e.setAttribute("rx", cx - r.x());
        e.setAttribute("ry", cy - r.y());
    }
    d->appendChild(e, PdcDrawEllipse);
}

void QSVGPaintEngine::drawArc(const QRect &r, int _a, int alen)
{
    double a = (double) _a / 16.0 * deg2rad;
    double al = (double) alen / 16.0 * deg2rad;
    double rx = r.width() / 2.0;
    double ry = r.height() / 2.0;
    double x0 = (double) r.x() + rx;
    double y0 = (double) r.y() + ry;
    double x1 = x0 + rx*cos(a);
    double y1 = y0 - ry*sin(a);
    double x2 = x0 + rx*cos(a + al);
    double y2 = y0 - ry*sin(a + al);
    int large = QABS(al) > (180.0 * deg2rad) ? 1 : 0;
    int sweep = al < 0.0 ? 1 : 0;
    QDomElement e;
    QString str;

    str = QString("M %1 %2 ").arg(x1).arg(y1);
    str += QString("A %1 %2 %3 %4 %5 %6 %7")
           .arg(rx).arg(ry).arg(a / deg2rad). arg(large).arg(sweep)
           .arg(x2).arg(y2);
    e = d->doc.createElement("path");
    e.setAttribute("d", str);
    d->appendChild(e, PdcDrawArc);
}

void QSVGPaintEngine::drawPie(const QRect &r, int _a, int alen)
{
    double a = (double) _a / 16.0 * deg2rad;
    double al = (double) alen / 16.0 * deg2rad;
    double rx = r.width() / 2.0;
    double ry = r.height() / 2.0;
    double x0 = (double) r.x() + rx;
    double y0 = (double) r.y() + ry;
    double x1 = x0 + rx*cos(a);
    double y1 = y0 - ry*sin(a);
    double x2 = x0 + rx*cos(a + al);
    double y2 = y0 - ry*sin(a + al);
    int large = QABS(al) > (180.0 * deg2rad) ? 1 : 0;
    int sweep = al < 0.0 ? 1 : 0;
    QDomElement e;
    QString str;

    str = QString("M %1 %2 L %3 %4 ").arg(x0).arg(y0).arg(x1).arg(y1);
    str += QString("A %1 %2 %3 %4 %5 %6 %7z")
           .arg(rx).arg(ry).arg(a / deg2rad).arg(large).arg(sweep).arg(x2).arg(y2);
    e = d->doc.createElement("path");
    e.setAttribute("d", str);
    d->appendChild(e, PdcDrawPie);
}

void QSVGPaintEngine::drawChord(const QRect &r, int _a, int alen)
{
    double a = (double) _a / 16.0 * deg2rad;
    double al = (double) alen / 16.0 * deg2rad;
    double rx = r.width() / 2.0;
    double ry = r.height() / 2.0;
    double x0 = (double) r.x() + rx;
    double y0 = (double) r.y() + ry;
    double x1 = x0 + rx*cos(a);
    double y1 = y0 - ry*sin(a);
    double x2 = x0 + rx*cos(a + al);
    double y2 = y0 - ry*sin(a + al);
    int large = QABS(al) > (180.0 * deg2rad) ? 1 : 0;
    int sweep = al < 0.0 ? 1 : 0;
    QDomElement e;
    QString str;

    str = QString("M %1 %2 ").arg(x1).arg(y1);
    str += QString("A %1 %2 %3 %4 %5 %6 %7z")
           .arg(rx).arg(ry).arg(a / deg2rad). arg(large).arg(sweep)
           .arg(x2).arg(y2);
    e = d->doc.createElement("path");
    e.setAttribute("d", str);
    d->appendChild(e, PdcDrawChord);
}

void QSVGPaintEngine::drawLineSegments(const QPointArray &pa, int /* index */, int /* nlines */)
{
    QDomElement e;
    uint end = pa.size() / 2; // ### use index and nlines instead - they are verified by QPainter

    for (uint i = 0; i < end; i++) {
        e = d->doc.createElement("line");
        e.setAttribute("x1", pa[int(2*i)].x());
        e.setAttribute("y1", pa[int(2*i)].y());
        e.setAttribute("x2", pa[int(2*i+1)].x());
        e.setAttribute("y2", pa[int(2*i+1)].y());
        d->appendChild(e, PdcDrawLineSegments);
    }
}

void QSVGPaintEngine::drawPolyline(const QPointArray &a, int index, int npoints)
{
    QString str;
    QDomElement e = d->doc.createElement("polyline");
    for (int i = index; i < npoints; ++i) {
        QString tmp;
        tmp.sprintf("%d %d ", a[i].x(), a[i].y());
        str += tmp;
    }
    e.setAttribute("points", str.trimmed());
    d->appendChild(e, PdcDrawPolyline);
}

void QSVGPaintEngine::drawPolygon(const QPointArray &a, bool, int index, int npoints)
{
    QString str;
    QDomElement e = d->doc.createElement("polygon");
    for (int i = index; i < npoints; ++i) {
        QString tmp;
        tmp.sprintf("%d %d ", a[i].x(), a[i].y());
        str += tmp;
    }
    e.setAttribute("points", str.trimmed());
    d->appendChild(e, PdcDrawPolygon);
}

void QSVGPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    drawPolygon(pa, false, index, npoints);
}

void QSVGPaintEngine::drawCubicBezier(const QPointArray &a, int /* index */)
{
#ifndef QT_NO_BEZIER
    QString str;
    QDomElement e = d->doc.createElement("path");
    str.sprintf("M %d %d C %d %d %d %d %d %d", a[0].x(), a[0].y(),
                a[1].x(), a[1].y(), a[2].x(), a[2].y(),
                a[3].x(), a[3].y());
    e.setAttribute("d", str);
    d->appendChild(e, PdcDrawCubicBezier);
#endif
}

void QSVGPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect & /* sr */,
                                 QPainter::PixmapDrawingMode mode)
{
    QDomElement e = d->doc.createElement("image");
    e.setAttribute("x", r.x());
    e.setAttribute("y", r.y());
    e.setAttribute("width", r.width());
    e.setAttribute("height", r.height());

    // ### fix the image drawing - converting to pixmaps first is going to
    // slow things down considerably
//     if (c == PdcDrawImage) {
//         ImgElement ie;
//         ie.element = e;
//         ie.image = *p[1].image;
//         d->images.append(ie);
//     } else {
        PixElement pe;
        pe.element = e;
        pe.pixmap = pm;
        d->pixmaps.append(pe);
//     }
    // saving to disk and setting the xlink:href attribute will be
    // done later in save() once we now the svg document name.
    d->appendChild(e, PdcDrawPixmap);
}

void QSVGPaintEngine::drawTiledPixmap(const QRect & /* r */, const QPixmap & /* pixmap */,
                                      const QPoint & /* s */)
{
}

void QSVGPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{
    QDomElement e = d->doc.createElement("text");
    int x, y;
    const QRect r(p.x(), p.y(), ti.width, ti.ascent + ti.descent);
    // horizontal text alignment
    if ((textflags & Qt::AlignHCenter) != 0) {
        x = r.x() + r.width() / 2;
        e.setAttribute("text-anchor", "middle");
    } else if ((textflags & Qt::AlignRight) != 0) {
        x = r.right();
        e.setAttribute("text-anchor", "end");
    } else {
        x = r.x();
    }
    // vertical text alignment
    if ((textflags & Qt::AlignVCenter) != 0)
        y = r.y() + (r.height() + ti.ascent) / 2;
    else if ((textflags & Qt::AlignBottom) != 0)
        y = r.bottom();
    else
        y = r.y() + ti.ascent;
    if (x)
        e.setAttribute("x", x);
    if (y)
        e.setAttribute("y", y);
    e.appendChild(d->doc.createTextNode(QString(ti.chars, ti.num_chars)));
}

/*!
    Returns the SVG as a single string of XML.
*/

QString QSVGPaintEngine::toString() const
{
    if (d->doc.isNull())
        return QString();

    return d->doc.toString();
}

/*!
    Saves the SVG to \a fileName.
*/

bool QSVGPaintEngine::save(const QString &fileName)
{
    // guess svg id from fileName
    QString svgName = fileName.endsWith(".svg") ?
                      fileName.left(fileName.length()-4) : fileName;

    // now we have the info about name and dimensions available
    QDomElement root = d->doc.documentElement();
    root.setAttribute("id", svgName);
    // the standard doesn't take respect x and y. But we want a
    // proper bounding rect. We make width and height bigger when
    // writing out and subtract x and y when reading in.
    root.setAttribute("x", d->brect.x());
    root.setAttribute("y", d->brect.y());
    root.setAttribute("width", d->brect.width() + d->brect.x());
    root.setAttribute("height", d->brect.height() + d->brect.y());

    // ... and know how to name any image files to be written out
    int icount = 0;
    ImageList::Iterator iit = d->images.begin();
    for (; iit != d->images.end(); ++iit) {
        QString href = QString("%1_%2.png").arg(svgName).arg(icount);
        (*iit).image.save(href, "PNG");
        (*iit).element.setAttribute("xlink:href", href);
        icount++;
    }
    PixmapList::Iterator pit = d->pixmaps.begin();
    for (; pit != d->pixmaps.end(); ++pit) {
        QString href = QString("%1_%2.png").arg(svgName).arg(icount);
        (*pit).pixmap.save(href, "PNG");
        (*pit).element.setAttribute("xlink:href", href);
        icount++;
    }

    QFile f(fileName);
    if (!f.open (IO_WriteOnly))
        return false;
    QTextStream s(&f);
    s.setEncoding(QTextStream::UnicodeUTF8);
    s << d->doc;

    return true;
}

/*!
    \overload

    \a dev is the device to use for saving.
*/

bool QSVGPaintEngine::save(QIODevice *dev)
{
#if defined(CHECK_RANGE)
    if (!d->images.isEmpty() || !d->pixmaps.isEmpty())
        qWarning("QSVGPaintEngine::save: skipping external images");
#endif

    QTextStream s(dev);
    s.setEncoding(QTextStream::UnicodeUTF8);
    s << d->doc;

    return true;
}

/*!
    Sets the bounding rectangle of the SVG to rectangle \a r.
*/

void QSVGPaintEngine::setBoundingRect(const QRect &r)
{
    d->brect = r;
}

/*!
    Returns the SVG's bounding rectangle.
*/

QRect QSVGPaintEngine::boundingRect() const
{
    return d->brect;
}

/*!
    Loads and parses a SVG from \a dev into the device. Returns true
    on success (i.e. loaded and parsed without error); otherwise
    returns false.
*/

bool QSVGPaintEngine::load(QIODevice *dev)
{
    return d->doc.setContent(dev);
}

void QSVGPaintEnginePrivate::appendChild(QDomElement &e, PaintCommand c)
{
    if (!e.isNull()) {
        d->current.appendChild(e);
        if (c == PdcSave)
            d->current = e;
        // ### optimize application of attributes utilizing <g>
        if (c == PdcSetClipRegion) {
            QDomElement ne;
            ne = d->doc.createElement("g");
            ne.setAttribute("style", QString("clip-path:url(#clip%1)").arg(d->currentClip));
            d->current.appendChild(ne);
            d->current = ne;
        } else {
            if (d->dirtyStyle)                // only reset when entering
                applyStyle(&e, c);        // or leaving a <g> tag
            if (d->dirtyTransform && e.tagName() != "g") {
                // same as above but not for <g> tags
                applyTransform(&e);
                if (c == PdcSave)
                    d->dirtyTransform = false;
            }
        }
    }
}

void QSVGPaintEnginePrivate::applyStyle(QDomElement *e, PaintCommand c) const
{
    // ### do not write every attribute each time
    QColor pcol = d->cpen.color();
    QColor bcol = d->cbrush.color();
    QString s;
    if (c == PdcDrawText2 || c == PdcDrawText2Formatted) {
        // QPainter has a reversed understanding of pen/stroke vs.
        // brush/fill for text
        s += QString("fill:rgb(%1,%2,%3);").arg(pcol.red()).arg(pcol.green()).arg(pcol.blue());
        s += QString("stroke-width:0;");
        QFont f = d->cfont;
        QFontInfo fi(f);
        s += QString("font-size:%1;").arg(fi.pointSize());
        s += QString("font-style:%1;").arg(f.italic() ? "italic" : "normal");
        // not a very scientific distribution
        QString fw;
        if (f.weight() <= QFont::Light)
            fw = "100";
        else if (f.weight() <= QFont::Normal)
            fw = "400";
        else if (f.weight() <= QFont::DemiBold)
            fw = "600";
        else if (f.weight() <= QFont::Bold)
            fw = "700";
        else if (f.weight() <= QFont::Black)
            fw = "800";
        else
            fw = "900";
        s += QString("font-weight:%1;").arg(fw);
        s += QString("font-family:%1;").arg(f.family());
    } else {
        s += QString("stroke:rgb(%1,%2,%3);").arg(pcol.red()).arg(pcol.green()).arg(pcol.blue());
        double pw = d->cpen.width();
        if (pw == 0 && d->cpen.style() != Qt::NoPen)
            pw = 0.9;
        if (c == PdcDrawLine)
            pw /= (QABS(d->worldMatrix.m11()) + QABS(d->worldMatrix.m22())) / 2.0;
        s += QString("stroke-width:%1;").arg(pw);
        if (d->cpen.style() == Qt::DashLine)
            s+= QString("stroke-dasharray:18,6;");
        else if (d->cpen.style() == Qt::DotLine)
            s+= QString("stroke-dasharray:3;");
        else if (d->cpen.style() == Qt::DashDotLine)
            s+= QString("stroke-dasharray:9,6,3,6;");
        else if (d->cpen.style() == Qt::DashDotDotLine)
            s+= QString("stroke-dasharray:9,3,3;");
        if (d->cbrush.style() == Qt::NoBrush || c == PdcDrawPolyline || c == PdcDrawCubicBezier)
            s += "fill:none;"; // Qt polylines use no brush, neither do Beziers
        else
            s += QString("fill:rgb(%1,%2,%3);").arg(bcol.red()).arg(bcol.green()).arg(bcol.blue());
    }
    e->setAttribute("style", s);
}

void QSVGPaintEnginePrivate::applyTransform(QDomElement *e) const
{
    QWMatrix m = d->worldMatrix;

    QString s;
    bool rot = (m.m11() != 1.0 || m.m12() != 0.0 ||
                 m.m21() != 0.0 || m.m22() != 1.0);
    if (!rot && (m.dx() != 0.0 || m.dy() != 0.0)) {
        s = QString("translate(%1,%2)").arg(m.dx()).arg(m.dy());
    } else if (rot) {
        if (m.m12() == 0.0 && m.m21() == 0.0 &&
             m.dx() == 0.0 && m.dy() == 0.0)
            s = QString("scale(%1,%2)").arg(m.m11()).arg(m.m22());
        else
            s = QString("matrix(%1,%2,%3,%4,%5,%6)")
                .arg(m.m11()).arg(m.m12())
                .arg(m.m21()).arg(m.m22())
                .arg(m.dx()).arg(m.dy());
    } else {
        return;
    }
    e->setAttribute("transform", s);
}

bool QSVGPaintEngine::play(QPainter *pt)
{
    if (!pt) {
        Q_ASSERT(pt);
        return false;
    }
    d->wwidth = pt->window().width();
    d->wheight = pt->window().height();

    pt->setPen(Qt::NoPen); // SVG default pen and brush
    pt->setBrush(Qt::black);
    if (d->doc.isNull()) {
        qWarning("QSVGPaintEngine::play: No SVG data set.");
        return false;
    }

    QDomNode svg = d->doc.namedItem("svg");
    if (svg.isNull() || !svg.isElement()) {
        qWarning("QSVGPaintEngine::play: Couldn't find any svg element.");
        return false;
    }

    // force transform to be activated in case our sequences
    // are replayed later with a transformed painter
    pt->setWorldXForm(true);

    QDomNamedNodeMap attr = svg.attributes();
    int x = d->lenToInt(attr, "x");
    int y = d->lenToInt(attr, "y");
    d->brect.setX(x);
    d->brect.setY(y);
    QString wstr = attr.contains("width")
                   ? attr.namedItem("width").nodeValue() : QString("100%");
    QString hstr = attr.contains("height")
                   ? attr.namedItem("height").nodeValue() : QString("100%");
    double width = d->parseLen(wstr, 0, true);
    double height = d->parseLen(hstr, 0, false);
    // SVG doesn't respect x and y. But we want a proper bounding rect.
    d->brect.setWidth(int(width) - x);
    d->brect.setHeight(int(height) - y);
    pt->setClipRect(d->brect);

    if (attr.contains("viewBox")) {
        QRegExp re(QString::fromLatin1("\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*,?"
                                       "\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*"));
        if (re.indexIn(attr.namedItem("viewBox").nodeValue()) < 0) {
            qWarning("QSVGPaintEngine::play: Invalid viewBox attribute.");
            return false;
        } else {
            double x = re.cap(1).toDouble();
            double y = re.cap(2).toDouble();
            double w = re.cap(3).toDouble();
            double h = re.cap(4).toDouble();
            if (w < 0 || h < 0) {
                qWarning("QSVGPaintEngine::play: Invalid viewBox dimension.");
                return false;
            } else if (w == 0 || h == 0) {
                return true;
            }
            pt->translate(-x, -y);
            pt->scale(width/w, height/h);
        }
    }

    const struct ElementTable {
        const char *name;
        ElementType type;
    } etab[] = {
        {"a",        AnchorElement  },
        {"#comment", CommentElement },
        {"circle",   CircleElement  },
        {"clipPath", ClipElement    },
        {"desc",     DescElement    },
        {"ellipse",  EllipseElement },
        {"g",        GroupElement   },
        {"image",    ImageElement   },
        {"line",     LineElement    },
        {"polyline", PolylineElement},
        {"polygon",  PolygonElement },
        {"path",     PathElement    },
        {"rect",     RectElement    },
        {"svg",      SvgElement     },
        {"text",     TextElement    },
        {"tspan",    TSpanElement   },
        {"title",    TitleElement   },
        {0,          InvalidElement }
    };
    // initialize only once
    if (!qSvgTypeMap) {
        qSvgTypeMap = new QSvgTypeMap;
        const ElementTable *t = etab;
        while (t->name) {
            qSvgTypeMap->insert(t->name, t->type);
            t++;
        }
    }

    // initial state
    QSVGPaintEngineState st;
    st.textx = st.texty = 0;
    st.textalign = Qt::AlignLeft;
    d->stack.append(st);
    d->curr = &d->stack.last();
    // 'play' all elements recursively starting with 'svg' as root
    bool b = d->play(svg, pt);
    d->stack.removeFirst();
    return b;
}

bool QSVGPaintEnginePrivate::play(const QDomNode &node, QPainter *pt)
{
    saveAttributes(pt);

    ElementType t = (*qSvgTypeMap)[node.nodeName()];

    if (t == LineElement && pt->pen().style() == Qt::NoPen) {
        QPen p = pt->pen();
        p.setStyle(Qt::SolidLine);
        pt->setPen(p);
    }
    QDomNamedNodeMap attr = node.attributes();
    if (attr.contains("style"))
        setStyle(attr.namedItem("style").nodeValue(), pt);
    // ### might have to exclude more elements from transform
    if (t != SvgElement && attr.contains("transform"))
        setTransform(attr.namedItem("transform").nodeValue(), pt);
    uint i = attr.length();
    if (i > 0) {
        QPen pen = pt->pen();
        QFont font = pt->font();
        while (i--) {
            QDomNode n = attr.item(i);
            QString a = n.nodeName();
            QString val = n.nodeValue().toLower().trimmed();
            setStyleProperty(a, val, &pen, &font, &d->curr->textalign, pt);
        }
        pt->setPen(pen);
        pt->setFont(font);
    }

    int x1, y1, x2, y2, rx, ry, w, h;
    double cx1, cy1, crx, cry;
    switch (t) {
    case CommentElement:
        // ignore
        break;
    case RectElement:
        rx = ry = 0;
        x1 = lenToInt(attr, "x");
        y1 = lenToInt(attr, "y");
        w = lenToInt(attr, "width");
        h = lenToInt(attr, "height");
        if (w == 0 || h == 0) // prevent div by zero below
            break;
        x2 = (int) attr.contains("rx"); // tiny abuse of x2 and y2
        y2 = (int) attr.contains("ry");
        if (x2)
            rx = lenToInt(attr, "rx");
        if (y2)
            ry = lenToInt(attr, "ry");
        if (x2 && !y2)
            ry = rx;
        else if (!x2 && y2)
            rx = ry;
        rx = int(200.0*double(rx) / double(w));
        ry = int(200.0*double(ry) / double(h));
        pt->drawRoundRect(x1, y1, w, h, rx, ry);
        break;
    case CircleElement:
        cx1 = lenToDouble(attr, "cx") + 0.5;
        cy1 = lenToDouble(attr, "cy") + 0.5;
        crx = lenToDouble(attr, "r");
        pt->drawEllipse((int)(cx1-crx), (int)(cy1-crx), (int)(2*crx), (int)(2*crx));
        break;
    case EllipseElement:
        cx1 = lenToDouble(attr, "cx") + 0.5;
        cy1 = lenToDouble(attr, "cy") + 0.5;
        crx = lenToDouble(attr, "rx");
        cry = lenToDouble(attr, "ry");
        pt->drawEllipse((int)(cx1-crx), (int)(cy1-cry), (int)(2*crx), (int)(2*cry));
        break;
    case LineElement:
        {
            x1 = lenToInt(attr, "x1");
            x2 = lenToInt(attr, "x2");
            y1 = lenToInt(attr, "y1");
            y2 = lenToInt(attr, "y2");
            QPen p = pt->pen();
            w = p.width();
            p.setWidth((unsigned int)(w * (QABS(pt->worldMatrix().m11()) + QABS(pt->worldMatrix().m22())) / 2));
            pt->setPen(p);
            pt->drawLine(x1, y1, x2, y2);
            p.setWidth(w);
            pt->setPen(p);
        }
        break;
    case PolylineElement:
    case PolygonElement:
        {
            QString pts = attr.namedItem("points").nodeValue();
            pts = pts.simplified();
            QStringList sl = pts.split(QRegExp(QString::fromLatin1("[,]")));
            QPointArray ptarr((uint) sl.count() / 2);
            for (int i = 0; i < (int) sl.count() / 2; i++) {
                double dx = sl[2*i].toDouble();
                double dy = sl[2*i+1].toDouble();
                ptarr.setPoint(i, int(dx), int(dy));
            }
            if (t == PolylineElement) {
                if (pt->brush().style() != Qt::NoBrush) {
                    QPen pn = pt->pen();
                    pt->setPen(Qt::NoPen);
                    pt->drawPolygon(ptarr);
                    pt->setPen(pn);
                }
                pt->drawPolyline(ptarr); // ### closes when filled. bug ?
            } else {
                pt->drawPolygon(ptarr);
            }
        }
        break;
    case SvgElement:
    case GroupElement:
    case AnchorElement:
        {
            QDomNode child = node.firstChild();
            while (!child.isNull()) {
                play(child, pt);
                child = child.nextSibling();
            }
        }
        break;
    case PathElement:
        drawPath(attr.namedItem("d").nodeValue(), pt);
        break;
    case TSpanElement:
    case TextElement:
        {
            if (attr.contains("x"))
                 d->curr->textx = lenToInt(attr, "x");
            if (attr.contains("y"))
                 d->curr->texty = lenToInt(attr, "y");
            if (t == TSpanElement) {
                d->curr->textx += lenToInt(attr, "dx");
                d->curr->texty += lenToInt(attr, "dy");
            }
            // backup old colors
            QPen pn = pt->pen();
            QColor pcolor = pn.color();
            QColor bcolor = pt->brush().color();
            QDomNode c = node.firstChild();
            while (!c.isNull()) {
                if (c.isText()) {
                    // we have pen and brush reversed for text drawing
                    pn.setColor(bcolor);
                    pt->setPen(pn);
                    QString text = c.toText().nodeValue();
                    text = text.simplified(); // ### 'preserve'
                    w = pt->fontMetrics().width(text);
                    if (d->curr->textalign == Qt::AlignHCenter)
                        d->curr->textx -= w / 2;
                    else if (d->curr->textalign == Qt::AlignRight)
                        d->curr->textx -= w;
                    pt->drawText(d->curr->textx, d->curr->texty, text);
                    // restore pen
                    pn.setColor(pcolor);
                    pt->setPen(pn);
                    d->curr->textx += w;
                } else if (c.isElement() && c.toElement().tagName() == "tspan") {
                    play(c, pt);

                }
                c = c.nextSibling();
            }
            if (t == TSpanElement) {
                // move current text position in parent text element
                StateList::Iterator it = --(--d->stack.end());
                (*it).textx = d->curr->textx;
                (*it).texty = d->curr->texty;
            }
        }
        break;
    case ImageElement:
        {
            x1 = lenToInt(attr, "x");
            y1 = lenToInt(attr, "y");
            w = lenToInt(attr, "width");
            h = lenToInt(attr, "height");
            QString href = attr.namedItem("xlink:href").nodeValue();
            // ### catch references to embedded .svg files
            QPixmap pix;
            if (!pix.load(href)){
                qWarning("QSVGPaintEngine::play: Couldn't load image %s",href.latin1());
                break;
            }
            pt->drawPixmap(QRect(x1, y1, w, h), pix);
        }
        break;
    case DescElement:
    case TitleElement:
        // ignored for now
        break;
    case ClipElement:
        {
            restoreAttributes(pt); // To ensure the clip rect is saved, we need to restore now
            QDomNode child = node.firstChild();
            QDomNamedNodeMap childAttr = child.attributes();
            if (child.nodeName() == "rect") {
                QRect r;
                r.setX(lenToInt(childAttr, "x"));
                r.setY(lenToInt(childAttr, "y"));
                r.setWidth(lenToInt(childAttr, "width"));
                r.setHeight(lenToInt(childAttr, "height"));
                pt->setClipRect(r);
            } else if (child.nodeName() == "ellipse") {
                QRect r;
                int x = lenToInt(childAttr, "cx");
                int y = lenToInt(childAttr, "cy");
                int width = lenToInt(childAttr, "rx");
                int height = lenToInt(childAttr, "ry");
                r.setX(x - width);
                r.setY(y - height);
                r.setWidth(width * 2);
                r.setHeight(height * 2);
                QRegion rgn(r, QRegion::Ellipse);
                pt->setClipRegion(rgn);
            }
            break;
        }
    case InvalidElement:
        qWarning("QSVGPaintEngine::play: unknown element type %s", node.nodeName().latin1());
        break;
    }

    if (t != ClipElement)
        restoreAttributes(pt);

    return true;
}

/*!
    \internal

    Parse a <length> datatype consisting of a number followed by an
    optional unit specifier. Can be used for type <coordinate> as
    well. For relative units the value of \a horiz will determine
    whether the horizontal or vertical dimension will be used.
*/
double QSVGPaintEnginePrivate::parseLen(const QString &str, bool *ok, bool horiz) const
{
    QRegExp reg(QString::fromLatin1("([+-]?\\d*\\.*\\d*[Ee]?[+-]?\\d*)(em|ex|px|%|pt|pc|cm|mm|in|)$"));
    if (reg.indexIn(str) == -1) {
        qWarning("QSVGPaintEngine::parseLen: couldn't parse %s", str.latin1());
        if (ok)
            *ok = false;
        return 0.0;
    }

    double dbl = reg.cap(1).toDouble();
    QString u = reg.cap(2);
    if (!u.isEmpty() && u != "px") {
        QPaintDeviceMetrics m(d->dev);//pt->device()); // ### NB! this is the metrics from QPicture - was the same as the old QSvgDevice...
        if (u == "em") {
            QFontInfo fi(d->cfont);
            dbl *= fi.pixelSize();
        } else if (u == "ex") {
            QFontInfo fi(d->cfont);
            dbl *= 0.5 * fi.pixelSize();
        } else if (u == "%")
            dbl *= (horiz ? d->wwidth : d->wheight)/100.0;
        else if (u == "cm")
            dbl *= m.logicalDpiX() / 2.54;
        else if (u == "mm")
            dbl *= m.logicalDpiX() / 25.4;
        else if (u == "in")
            dbl *= m.logicalDpiX();
        else if (u == "pt")
            dbl *= m.logicalDpiX() / 72.0;
        else if (u == "pc")
            dbl *= m.logicalDpiX() / 6.0;
        else
            qWarning("QSVGPaintEngine::parseLen: Unknown unit %s", u.latin1());
    }
    if (ok)
        *ok = true;
    return dbl;
}

/*!
    \internal

    Returns the length specified in attribute \a attr in \a map. If
    the specified attribute doesn't exist or can't be parsed \a def is
    returned.
*/

int QSVGPaintEnginePrivate::lenToInt(const QDomNamedNodeMap &map, const QString &attr, int def) const
{
    if (map.contains(attr)) {
        bool ok;
        double dbl = parseLen(map.namedItem(attr).nodeValue(), &ok);
        if (ok)
            return qRound(dbl);
    }
    return def;
}

double QSVGPaintEnginePrivate::lenToDouble(const QDomNamedNodeMap &map, const QString &attr,
                                    int def) const
{
    if (map.contains(attr)) {
        bool ok;
        double x = parseLen(map.namedItem(attr).nodeValue(), &ok);
        if (ok) return x;
    }
    return static_cast<double>(def);
}

void QSVGPaintEnginePrivate::setTransform(const QString &tr, QPainter *pt)
{
    QString t = tr.simplified();

    QRegExp reg(QString::fromLatin1("\\s*([\\w]+)\\s*\\(([^\\(]*)\\)"));
    int index = 0;
    while ((index = reg.indexIn(t, index)) >= 0) {
        QString command = reg.cap(1);
        QString params = reg.cap(2);
        QStringList plist = params.split(QRegExp(QString::fromLatin1("[,\\s]")));
        if (command == "translate") {
            double tx = 0, ty = 0;
            tx = plist[0].toDouble();
            if (plist.count() >= 2)
                ty = plist[1].toDouble();
            pt->translate(tx, ty);
        } else if (command == "rotate") {
            pt->rotate(plist[0].toDouble());
        } else if (command == "scale") {
            double sx, sy;
            sx = sy = plist[0].toDouble();
            if (plist.count() >= 2)
                sy = plist[1].toDouble();
            pt->scale(sx, sy);
        } else if (command == "matrix" && plist.count() >= 6) {
            double m[6];
            for (int i = 0; i < 6; i++)
                m[i] = plist[i].toDouble();
            QWMatrix wm(m[0], m[1], m[2],
                         m[3], m[4], m[5]);
            pt->setWorldMatrix(wm, true);
        } else if (command == "skewX") {
            pt->shear(0.0, tan(plist[0].toDouble() * deg2rad));
        } else if (command == "skewY") {
            pt->shear(tan(plist[0].toDouble() * deg2rad), 0.0);
        }

        // move on to next command
        index += reg.matchedLength();
    }
}
/*!
    \internal

    Push the current drawing attributes on a stack.

    \sa restoreAttributes()
*/

void QSVGPaintEnginePrivate::saveAttributes(QPainter *pt)
{
    pt->save();
    // copy old state
    QSVGPaintEngineState st(*d->curr);
    d->stack.append(st);
    d->curr = &d->stack.last();
}

/*!
    \internal

    Pop the current drawing attributes off the stack.

    \sa saveAttributes()
*/

void QSVGPaintEnginePrivate::restoreAttributes(QPainter *pt)
{
    pt->restore();
    Q_ASSERT(d->stack.count() > 1);
    d->stack.removeLast();
    d->curr = &d->stack.last();
}

void QSVGPaintEnginePrivate::setStyle(const QString &s, QPainter *pt)
{
    QStringList rules = s.split(QChar(';'));

    QPen pen = pt->pen();
    QFont font = pt->font();

    QStringList::ConstIterator it = rules.begin();
    for (; it != rules.end(); it++) {
        int col = (*it).indexOf(':');
        if (col > 0) {
            QString prop = (*it).left(col).simplified();
            QString val = (*it).right((*it).length() - col - 1);
            val = val.toLower().trimmed();
            setStyleProperty(prop, val, &pen, &font, &d->curr->textalign, pt);
        }
    }

    pt->setPen(pen);
    pt->setFont(font);
}

void QSVGPaintEnginePrivate::setStyleProperty(const QString &prop, const QString &val, QPen *pen,
                                       QFont *font, int *talign, QPainter *pt)
{
    if (prop == "stroke") {
        if (val == "none") {
            pen->setStyle(Qt::NoPen);
        } else {
            pen->setColor(parseColor(val));
            if (pen->style() == Qt::NoPen)
                pen->setStyle(Qt::SolidLine);
            if (pen->width() == 0)
                pen->setWidth(1);
        }
    } else if (prop == "stroke-width") {
        double w = parseLen(val);
        if (w > 0.0001)
            pen->setWidth(int(w));
        else
            pen->setStyle(Qt::NoPen);
    } else if (prop == "stroke-linecap") {
        if (val == "butt")
            pen->setCapStyle(Qt::FlatCap);
        else if (val == "round")
            pen->setCapStyle(Qt::RoundCap);
        else if (val == "square")
            pen->setCapStyle(Qt::SquareCap);
    } else if (prop == "stroke-linejoin") {
        if (val == "miter")
            pen->setJoinStyle(Qt::MiterJoin);
        else if (val == "round")
            pen->setJoinStyle(Qt::RoundJoin);
        else if (val == "bevel")
            pen->setJoinStyle(Qt::BevelJoin);
    } else if (prop == "stroke-dasharray") {
        if (val == "18,6")
            pen->setStyle(Qt::DashLine);
        else if (val == "3")
            pen->setStyle(Qt::DotLine);
        else if (val == "9,6,3,6")
            pen->setStyle(Qt::DashDotLine);
        else if (val == "9,3,3")
            pen->setStyle(Qt::DashDotDotLine);
    } else if (prop == "fill") {
        if (val == "none")
            pt->setBrush(Qt::NoBrush);
        else
            pt->setBrush(parseColor(val));
    } else if (prop == "font-size") {
        font->setPointSizeFloat(float(parseLen(val)));
    } else if (prop == "font-family") {
        font->setFamily(val);
    } else if (prop == "font-style") {
        if (val == "normal")
            font->setItalic(false);
        else if (val == "italic")
            font->setItalic(true);
        else
            qWarning("QSvgDevice::setStyleProperty: unhandled font-style: %s", val.latin1());
    } else if (prop == "font-weight") {
        int w = font->weight();
        // no exact equivalents so we have to "round" a little bit
        if (val == "100" || val == "200")
            w = QFont::Light;
        if (val == "300" || val == "400" || val == "normal")
            w = QFont::Normal;
        else if (val == "500" || val == "600")
            w = QFont::DemiBold;
        else if (val == "700" || val == "bold" || val == "800")
            w = QFont::Bold;
        else if (val == "900")
            w = QFont::Black;
        font->setWeight(w);
    } else if (prop == "text-anchor") {
        if (val == "middle")
            *talign = Qt::AlignHCenter;
        else if (val == "end")
            *talign = Qt::AlignRight;
        else
            *talign = Qt::AlignLeft;
    }
}

void QSVGPaintEnginePrivate::drawPath(const QString &data, QPainter *pt)
{
    double x0 = 0, y0 = 0;                // starting point
    double x = 0, y = 0;                // current point
    double controlX = 0, controlY = 0;        // last control point for curves
    QPointArray path(500);                // resulting path
    QList<int> subIndex;                // start indices for subpaths
    QPointArray quad(4), bezier;        // for curve calculations
    int pcount = 0;                        // current point array index
    int idx = 0;                        // current data position
    int mode = 0, lastMode = 0;                // parser state
    bool relative = false;                // e.g. 'h' vs. 'H'
    QString commands("MZLHVCSQTA");        // recognized commands
    int cmdArgs[] = { 2, 0, 2, 1, 1, 6, 4, 4, 2, 7 };        // no of arguments
    QRegExp reg(QString::fromLatin1("\\s*,?\\s*([+-]?\\d*\\.?\\d*)"));        // floating point

    subIndex.append(0);
    // detect next command
    while (idx < data.length()) {
        QChar ch = data[(int)idx++];
        if (ch.isSpace())
            continue;
        QChar chUp = ch.toUpper();
        int cmd = commands.indexOf(chUp);
        if (cmd >= 0) {
            // switch to new command mode
            mode = cmd;
            relative = (ch != chUp);                // e.g. 'm' instead of 'M'
        } else {
            if (mode && !ch.isLetter()) {
                cmd = mode;                        // continue in previous mode
                idx--;
            } else {
                qWarning("QSVGPaintEngine::drawPath: Unknown command");
                return;
            }
        }

        // read in the required number of arguments
        const int maxArgs = 7;
        double arg[maxArgs];
        int numArgs = cmdArgs[cmd];
        for (int i = 0; i < numArgs; i++) {
            int pos = reg.indexIn(data, idx);
            if (pos == -1) {
                qWarning("QSVGPaintEngine::drawPath: Error parsing arguments");
                return;
            }
            arg[i] = reg.cap(1).toDouble();
            idx = pos + reg.matchedLength();
        };

        // process command
        double offsetX = relative ? x : 0;        // correction offsets
        double offsetY = relative ? y : 0;        // for relative commands
        switch (mode) {
        case 0:                                        // 'M' move to
            if (x != x0 || y != y0)
                path.setPoint(pcount++, int(x0), int(y0));
            x = x0 = arg[0] + offsetX;
            y = y0 = arg[1] + offsetY;
            subIndex.append(pcount);
            path.setPoint(pcount++, int(x0), int(y0));
            mode = 2;                                // -> 'L'
            break;
        case 1:                                        // 'Z' close path
            path.setPoint(pcount++, int(x0), int(y0));
            x = x0;
            y = y0;
            mode = 0;
            break;
        case 2:                                        // 'L' line to
            x = arg[0] + offsetX;
            y = arg[1] + offsetY;
            path.setPoint(pcount++, int(x), int(y));
            break;
        case 3:                                        // 'H' horizontal line
            x = arg[0] + offsetX;
            path.setPoint(pcount++, int(x), int(y));
            break;
        case 4:                                        // 'V' vertical line
            y = arg[0] + offsetY;
            path.setPoint(pcount++, int(x), int(y));
            break;
#ifndef QT_NO_BEZIER
        case 5:                                        // 'C' cubic bezier curveto
        case 6:                                        // 'S' smooth shorthand
        case 7:                                        // 'Q' quadratic bezier curves
        case 8: {                                // 'T' smooth shorthand
            quad.setPoint(0, int(x), int(y));
            // if possible, reflect last control point if smooth shorthand
            if (mode == 6 || mode == 8) {         // smooth 'S' and 'T'
                bool cont = mode == lastMode ||
                     mode == 6 && lastMode == 5 ||         // 'S' and 'C'
                     mode == 8 && lastMode == 7;        // 'T' and 'Q'
                x = cont ? 2*x-controlX : x;
                y = cont ? 2*y-controlY : y;
                quad.setPoint(1, int(x), int(y));
                quad.setPoint(2, int(x), int(y));
            }
            for (int j = 0; j < numArgs/2; j++) {
                x = arg[2*j  ] + offsetX;
                y = arg[2*j+1] + offsetY;
                quad.setPoint(j+4-numArgs/2, int(x), int(y));
            }
            // remember last control point for next shorthand
            controlX = quad[2].x();
            controlY = quad[2].y();
            // transform quadratic into cubic Bezier
            if (mode == 7 || mode == 8) {        // cubic 'Q' and 'T'
                int x31 = quad[0].x()+int(2.0*(quad[2].x()-quad[0].x())/3.0);
                int y31 = quad[0].y()+int(2.0*(quad[2].y()-quad[0].y())/3.0);
                int x32 = quad[2].x()+int(2.0*(quad[3].x()-quad[2].x())/3.0);
                int y32 = quad[2].y()+int(2.0*(quad[3].y()-quad[2].y())/3.0);
                quad.setPoint(1, x31, y31);
                quad.setPoint(2, x32, y32);
            }
            // calculate points on curve
            bezier = quad.cubicBezier();
            // reserve more space if needed
            if (bezier.size() > path.size() - pcount)
                path.resize(path.size() - pcount + bezier.size());
            // copy
            for (int k = 0; k < (int)bezier.size(); k ++)
                path.setPoint(pcount++, bezier[k]);
            break;
        }
#endif // QT_NO_BEZIER
        case 9:                                        // 'A' elliptical arc curve
            // ### just a straight line
            x = arg[5] + offsetX;
            y = arg[6] + offsetY;
            path.setPoint(pcount++, int(x), int(y));
            break;
        };
        lastMode = mode;
        // array almost full ? expand for next loop
        if (pcount >= (int)path.size() - 4)
            path.resize(2 * path.size());
    }

    subIndex.append(pcount);                        // dummy marking the end
    if (pt->brush().style() != Qt::NoBrush) {
        // fill the area without stroke first
        if (x != x0 || y != y0)
            path.setPoint(pcount++, int(x0), int(y0));
        QPen pen = pt->pen();
        pt->setPen(Qt::NoPen);
        pt->drawPolygon(path, false, 0, pcount);
        pt->setPen(pen);
    }
    // draw each subpath stroke seperately
    QList<int>::ConstIterator it = subIndex.begin();
    QList<int>::ConstIterator end = --subIndex.end();
    int start = 0;
    while (it != end) {
        int next = *++it;
        // ### always joins ends if first and last point coincide.
        // ### 'Z' can't have the desired effect
        pt->drawPolyline(path, start, next-start);
        start = next;
    }
}

/*!
    \internal

    Parses a CSS2-compatible color specification. Either a keyword or
    a numerical RGB specification like #ff00ff or rgb(255,0,50%).
*/

QColor QSVGPaintEnginePrivate::parseColor(const QString &col)
{
    static const struct ColorTable {
        const char *name;
        const char *rgb;
    } coltab[] = {
        { "black",   "#000000" },
        { "silver",  "#c0c0c0" },
        { "gray",    "#808080" },
        { "white",   "#ffffff" },
        { "maroon",  "#800000" },
        { "red",     "#ff0000" },
        { "purple",  "#800080" },
        { "fuchsia", "#ff00ff" },
        { "green",   "#008000" },
        { "lime",    "#00ff00" },
        { "olive",   "#808000" },
        { "yellow",  "#ffff00" },
        { "navy",    "#000080" },
        { "blue",    "#0000ff" },
        { "teal",    "#008080" },
        { "aqua",    "#00ffff" },
        // ### the latest spec has more
        { 0,         0         }
    };

    // initialize color map on first use
    if (!qSvgColMap) {
        qSvgColMap = new QMap<QString,QString>;
        const struct ColorTable *t = coltab;
        while (t->name) {
            qSvgColMap->insert(t->name, t->rgb);
            ++t;
        }
    }

    // a keyword?
    if (qSvgColMap->contains(col))
        return QColor((*qSvgColMap)[col]);
    // in rgb(r,g,b) form ?
    QString c = col;
    c.replace(QRegExp(QString::fromLatin1("\\s*")), "");
    QRegExp reg(QString::fromLatin1("^rgb\\((\\d+)(%?),(\\d+)(%?),(\\d+)(%?)\\)$"));
    if (reg.indexIn(c) >= 0) {
        int comp[3];
        for (int i = 0; i < 3; i++) {
            comp[i] = reg.cap(2*i+1).toInt();
            if (!reg.cap(2*i+2).isEmpty())                // percentage ?
                comp[i] = int((double(255*comp[i])/100.0));
        }
        return QColor(comp[0], comp[1], comp[2]);
    }

    // check for predefined Qt color objects, #RRGGBB and #RGB
    return QColor(col);
}
