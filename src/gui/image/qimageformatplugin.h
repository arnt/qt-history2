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

#ifndef QIMAGEFORMATPLUGIN_H
#define QIMAGEFORMATPLUGIN_H

#include "QtCore/qplugin.h"
#include "QtCore/qfactoryinterface.h"

class QImageFormat;
class QImage;
class QStringList;
class QString;

struct Q_GUI_EXPORT QImageFormatInterface : public QFactoryInterface
{
    virtual bool loadImage(const QString &format, const QString &filename, QImage *) = 0;
    virtual bool saveImage(const QString &format, const QString &filename, const QImage &) = 0;

    virtual bool installIOHandler(const QString &) = 0;
};

Q_DECLARE_INTERFACE(QImageFormatInterface, "http://trolltech.com/Qt/QImageFormatInterface")

#ifndef QT_NO_IMAGEFORMATPLUGIN
class Q_GUI_EXPORT QImageFormatPlugin : public QObject, public QImageFormatInterface
{
    Q_OBJECT
    Q_INTERFACES(QImageFormatInterface:QFactoryInterface)
public:
    QImageFormatPlugin(QObject *parent = 0);
    ~QImageFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadImage(const QString &format, const QString &filename, QImage *image);
    virtual bool saveImage(const QString &format, const QString &filename, const QImage &image);
    virtual bool installIOHandler(const QString &format) = 0;

};
#endif // QT_NO_IMAGEFORMATPLUGIN
#endif // QIMAGEFORMATPLUGIN_H
