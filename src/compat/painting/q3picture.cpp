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

    if (format) {
#ifndef QT_NO_PICTUREIO
        QPictureIO io(dev, format);
        bool result = io.read();
        if (result) {
            operator=(io.picture());

        } else if (format)
#else
            bool result = false;
#endif
        {
            qWarning("Q3Picture::load: No such picture format: %s", format);
        }
        return result;
    }

    detach();
    QByteArray a = dev->readAll();

    d->pictb.setData(a);                        // set byte array in buffer
    return d->checkFormat();
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

    if(format) {
#ifndef QT_NO_PICTUREIO
        QPictureIO io(fileName, format);
        bool result = io.write();
        if (result) {
            operator=(io.picture());
        } else if (format)
#else
        bool result = false;
#endif
        {
            qWarning("Q3Picture::save: No such picture format: %s", format);
        }
        return result;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    return save(&f, format);
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

    if(format) {
#ifndef QT_NO_PICTUREIO
        QPictureIO io(dev, format);
        bool result = io.write();
        if (result) {
            operator=(io.picture());
        } else if (format)
#else
        bool result = false;
#endif
        {
            qWarning("Q3Picture::save: No such picture format: %s", format);
        }
        return result;
    }

    dev->write(d->pictb.buffer(), d->pictb.buffer().size());
    return true;
}
