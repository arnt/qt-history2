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
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>
#include <qtextbrowser.h>

// set by the mime source factory in Qt3Compat
QTextImageHandler::ExternalImageLoaderFunction QTextImageHandler::externalLoader = 0;

static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format)
{
    QPixmap pm;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());

    QString name = format.name();
    if (browser)
        name = browser->source().resolved(name).toString();

    QString key = QString("$qt_rt_%1_%2_%3").arg(name).arg(width).arg(height);

    if (!QPixmapCache::find(key, pm)) {
        QImage img;
        const QString name = format.name();

        const QVariant data = doc->loadResource(QTextDocument::ImageResource, name);
            if (data.type() == QVariant::Pixmap) {
                pm = qvariant_cast<QPixmap>(data);
                QPixmapCache::insert(key, pm);
                return pm;
            } else if (data.type() == QVariant::Image) {
                img = qvariant_cast<QImage>(data);
            } else if (data.type() == QVariant::ByteArray) {
                img.loadFromData(data.toByteArray());
            }

        QString context;
        if (browser)
            context = browser->source().toString();

        if (img.isNull() && QTextImageHandler::externalLoader)
            img = QTextImageHandler::externalLoader(name, context);

        if (img.isNull()) // try direct loading
            if (name.isEmpty() || !img.load(name))
                return pm;

        QSize size = img.size();
        if (hasWidth)
            size.setWidth(width);
        if (hasHeight)
            size.setHeight(height);
        if (size.isValid() && img.size() != size)
            img = img.scaled(size);

        pm = QPixmap::fromImage(img);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSizeF QTextImageHandler::intrinsicSize(QTextDocument *doc, const QTextFormat &format)
{
    const QTextImageFormat imageFormat = format.toImageFormat();

    return getPixmap(doc, imageFormat).size();
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, const QTextFormat &format)
{
    const QTextImageFormat imageFormat = format.toImageFormat();
    const QPixmap pixmap = getPixmap(doc, imageFormat);

    p->drawPixmap(rect, pixmap, pixmap.rect());
}

