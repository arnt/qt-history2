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

#include "private/qpicture_p.h"
#include "QtCore/qfile.h"
#include "QtGui/qpainter.h"

#include "q3picture.h"
#include "q3paintengine_svg_p.h"

class Q3SvgDevice : public QPaintDevice
{
public:
    Q3SvgDevice() : QPaintDevice(QInternal::ExternalDevice) {}
    bool load(QIODevice *dev) { return svgEngine.load(dev); }
    bool save(const QString &fileName) { return svgEngine.save(fileName); }
    bool save(QIODevice *dev) { return svgEngine.save(dev); }
    void setBoundingRect(const QRect &rect) { svgEngine.setBoundingRect(rect); }
    QRect boundingRect() const { return svgEngine.boundingRect(); }
    QPaintEngine *paintEngine() const { return (QPaintEngine *)&svgEngine; }
    bool play(QPainter *p) { return svgEngine.play(p); }

    int metric(PaintDeviceMetric m) const {
        int val;
        QRect br = svgEngine.boundingRect();
        switch (m) {
        case PdmWidth:
            val = br.width();
            break;
        case PdmHeight:
            val = br.height();
            break;
        case PdmWidthMM:
            val = int(25.4/72.0*br.width());
            break;
        case PdmHeightMM:
            val = int(25.4/72.0*br.height());
            break;
        case PdmDpiX:
            val = 72;
            break;
        case PdmDpiY:
            val = 72;
            break;
        case PdmNumColors:
            val = 16777216;
            break;
        case PdmDepth:
            val = 24;
            break;
        default:
            val = 0;
            qWarning("Q3SvgDevice::metric: Invalid metric command");
        }
        return val;
    }

private:
    Q3SVGPaintEngine svgEngine;
};

#define d d_func()
#define q q_func()

bool Q3Picture::load(const QString &fileName, const char *format)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    return load(&f, format);
}

bool Q3Picture::load(QIODevice *dev, const char *format)
{
    if (qstrcmp(format, "svg" ) == 0) {
	Q3SvgDevice svg;
	if (!svg.load(dev))
	    return FALSE;
        QPainter p(this);
	bool b = svg.play(&p);
	d->brect = svg.boundingRect();
	return b;
    }
    return QPicture::load(dev, format);
}

bool Q3Picture::save(const QString &fileName, const char *format)
{
    if (paintingActive()) {
        qWarning("Q3Picture::save: still being painted on. "
                  "Call QPainter::end() first");
        return false;
    }

    // identical to QIODevice* code below but the file name
    // makes a difference when it comes to saving pixmaps
    if (qstricmp( format, "svg") == 0) {
	Q3SvgDevice svg;
	QPainter p(&svg);
	if (!play(&p))
	    return FALSE;
	svg.setBoundingRect(boundingRect());
	return svg.save(fileName);
    }

    return QPicture::save(fileName, format);
}

bool Q3Picture::save(QIODevice *dev, const char *format)
{
    if (paintingActive()) {
        qWarning("Q3Picture::save: still being painted on. "
                  "Call QPainter::end() first");
        return false;
    }

    if (qstricmp(format, "svg") == 0) {
	Q3SvgDevice svg;
	QPainter p(&svg);
	if (!play(&p))
	    return FALSE;
	svg.setBoundingRect(boundingRect());
	return svg.save(dev);
    }

    return QPicture::save(dev, format);
}
