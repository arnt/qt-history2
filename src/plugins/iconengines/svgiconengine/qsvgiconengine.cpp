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
#include "qsvgiconengine.h"

#include "qpainter.h"
#include "qpixmap.h"
#include "qsvgrenderer.h"
#include "qpixmapcache.h"
#include "qstyle.h"
#include "qapplication.h"
#include "qstyleoption.h"
#include "qfileinfo.h"
#include "qdebug.h"


class QSvgIconEnginePrivate : public QSharedData
{
public:
    explicit QSvgIconEnginePrivate()
    {
        render = new QSvgRenderer;
    }
    ~QSvgIconEnginePrivate()
    {
        delete render;
        render = 0;
    }

    QSvgRenderer *render;
    QMap<int, QPixmap> svgCache;
};
static inline int area(const QSize &s) { return s.width() * s.height(); }

static inline int createKey(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return ((((((size.width()<<11)|size.height())<<11)|mode)<<4)|state);
}

QSvgIconEngine::QSvgIconEngine()
    : d(new QSvgIconEnginePrivate)
{

}


QSvgIconEngine::~QSvgIconEngine()
{
}


QSize QSvgIconEngine::actualSize(const QSize &size, QIcon::Mode,
                                 QIcon::State )
{
    return size;
}


QPixmap QSvgIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                               QIcon::State state)
{
    int index = createKey(size, mode, state);
    if (d->svgCache.contains(index))
        return d->svgCache[index];
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0x00000000);
    QPainter p(&img);
    d->render->render(&p);
    p.end();
    QPixmap pm = QPixmap::fromImage(img);
    QStyleOption opt(0);
    opt.palette = QApplication::palette();
    QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
    if (!generated.isNull())
        pm = generated;

    d->svgCache.insert(index, pm);

    return pm;
}


void QSvgIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                               QIcon::State state)
{
    int index = createKey(pixmap.size(), mode, state);
    d->svgCache.insert(index, pixmap);
}


void QSvgIconEngine::addFile(const QString &fileName, const QSize &,
                             QIcon::Mode, QIcon::State)
{
    if (!fileName.isEmpty()) {
        QString abs = fileName;
        if (fileName.at(0) != QLatin1Char(':'))
            abs = QFileInfo(fileName).absoluteFilePath();
        d->render->load(abs);
        //qDebug()<<"loaded "<<abs<<", isOK = "<<d->render->isValid();
    }
}

void QSvgIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}
