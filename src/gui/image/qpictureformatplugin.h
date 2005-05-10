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

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#include "QtCore/qplugin.h"
#include "QtCore/qfactoryinterface.h"

class QPicture;
class QImage;
class QString;
class QStringList;

struct Q_GUI_EXPORT QPictureFormatInterface : public QFactoryInterface
{
    virtual bool loadPicture(const QString &format, const QString &filename, QPicture *) = 0;
    virtual bool savePicture(const QString &format, const QString &filename, const QPicture &) = 0;

    virtual bool installIOHandler(const QString &) = 0;
};

Q_DECLARE_INTERFACE(QPictureFormatInterface, "com.trolltech.Qt.QPictureFormatInterface")


#ifndef QT_NO_PICTUREFORMATPLUGIN
class Q_GUI_EXPORT QPictureFormatPlugin : public QObject, public QPictureFormatInterface
{
    Q_OBJECT
    Q_INTERFACES(QPictureFormatInterface:QFactoryInterface)
public:
    explicit QPictureFormatPlugin(QObject *parent = 0);
    ~QPictureFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadPicture(const QString &format, const QString &filename, QPicture *pic);
    virtual bool savePicture(const QString &format, const QString &filename, const QPicture &pic);
    virtual bool installIOHandler(const QString &format) = 0;

};
#endif // QT_NO_PICTUREFORMATPLUGIN

#endif // QPICTUREFORMATPLUGIN_H
