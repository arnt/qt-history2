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


#include "qtextimagehandler_p.h"

#include <qtextformat.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qdragobject.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>

QTextImageHandler::ExternalImageLoaderFunction QTextImageHandler::externalLoader = 0;

static QPixmap getPixmap(const QTextDocument *doc, const QTextImageFormat &format, QObject *layout)
{
    QPixmap pm;

    QSize size;

    if (format.hasProperty(QTextFormat::ImageWidth))
        size.setWidth(format.width());

    if (format.hasProperty(QTextFormat::ImageHeight))
        size.setHeight(format.height());

    QString key = QString("$qt_rt_%1_%2_%3").arg(format.name()).arg(size.width()).arg(size.height());
    if (!QPixmapCache::find(key, pm)) {
        QImage img;
        const QString name = format.name();

        if (QTextImageHandler::externalLoader) {
            QString context;

            // ###
            /*
            if ((QTextBrowser *browser = qt_cast<doc->parent()))
                context = browser->source();
            */
            img = QTextImageHandler::externalLoader(name, context);
        } else if (layout && layout->parent() && layout->parent()->parent()) { // ### temporary, until Q4TextBrowser and friends are in main
            QTextDocumentLoaderInterface *loader = qt_cast<QTextDocumentLoaderInterface *>(layout->parent()->parent());
            if (loader)
                img = loader->image(name);
        }

        if (img.isNull()) // try direct loading
            if (!img.load(name))
                return pm;

        if (size.isValid() && img.size() != size)
            img = img.smoothScale(size);
        pm.fromImage(img);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSize QTextImageHandler::intrinsicSize(const QTextDocument *doc, const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();

    QPixmap pixmap = getPixmap(doc, imageFormat, parent());
    return pixmap.size();
}

void QTextImageHandler::drawObject(QPainter *p, const QRect &rect, const QTextDocument *doc, const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();
    QPixmap pixmap = getPixmap(doc, imageFormat, parent());

    p->drawPixmap(rect, pixmap);
}

