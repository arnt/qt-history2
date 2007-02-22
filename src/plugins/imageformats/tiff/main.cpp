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

#include <qimageiohandler.h>
#include <qdebug.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_TIFF
#undef QT_NO_IMAGEFORMAT_TIFF
#endif
#include "qtiffhandler.h"

class QTiffPlugin : public QImageIOPlugin
{
public:
    Capabilities capabilities(QIODevice * device, const QByteArray & format) const;
    QImageIOHandler * create(QIODevice * device, const QByteArray & format = QByteArray()) const;
    QStringList keys() const;
};

QImageIOPlugin::Capabilities QTiffPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "tiff" || format == "tif")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && QTiffHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler* QTiffPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *tiffHandler = new QTiffHandler();
    tiffHandler->setDevice(device);
    tiffHandler->setFormat(format);
    return tiffHandler;
}

QStringList QTiffPlugin::keys() const
{
    return QStringList() << "tiff" << "tif";
}

Q_EXPORT_STATIC_PLUGIN(QTiffPlugin)
Q_EXPORT_PLUGIN2(qtiff, QTiffPlugin)

#endif /* QT_NO_IMAGEFORMATPLUGIN */
