/****************************************************************************
**
** Definition of ???.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIMAGEFORMATPLUGIN_H
#define QIMAGEFORMATPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#endif // QT_H

#ifndef QT_NO_IMAGEFORMATPLUGIN
class QImageFormat;
class QImageFormatPluginPrivate;
class QImage;
class QStringList;
class QString;

class Q_GUI_EXPORT QImageFormatPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QImageFormatPlugin();
    ~QImageFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadImage(const QString &format, const QString &filename, QImage *image);
    virtual bool saveImage(const QString &format, const QString &filename, const QImage &image);
    virtual bool installIOHandler(const QString &format) = 0;

private:
    QImageFormatPluginPrivate *d;
};
#endif // QT_NO_IMAGEFORMATPLUGIN
#endif // QIMAGEFORMATPLUGIN_H
