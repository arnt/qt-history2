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

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#endif // QT_H

#ifndef QT_NO_PICTUREFORMATPLUGIN
class QPictureFormatPluginPrivate;
class QPicture;
class QImage;
class QString;
class QStringList;

class Q_GUI_EXPORT QPictureFormatPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QPictureFormatPlugin();
    ~QPictureFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadPicture( const QString &format, const QString &filename, QPicture *pic );
    virtual bool savePicture( const QString &format, const QString &filename, const QPicture &pic );
    virtual bool installIOHandler( const QString &format ) = 0;

private:
    QPictureFormatPluginPrivate *d;
};
#endif // QT_NO_IMAGEFORMATPLUGIN
#endif // QIMAGEFORMATPLUGIN_H
