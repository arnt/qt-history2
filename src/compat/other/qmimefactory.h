/****************************************************************************
**
** Definition of QMimeFactory class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the other module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMIMEFACTORY_H
#define QMIMEFACTORY_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qstring.h"
#include "qmap.h"
#include "qpixmap.h"
#include "qpixmap.h"
#endif // QT_H

#ifndef QT_NO_MIMEFACTORY

class QStringList;
class QMimeSource;
class QMimeSourceFactoryData;

class Q_COMPAT_EXPORT QMimeSourceFactory {
public:
    QMimeSourceFactory();
    virtual ~QMimeSourceFactory();

    static QMimeSourceFactory* defaultFactory();
    static void setDefaultFactory(QMimeSourceFactory*);
    static QMimeSourceFactory* takeDefaultFactory();
    static void addFactory(QMimeSourceFactory *f);
    static void removeFactory(QMimeSourceFactory *f);

    virtual const QMimeSource* data(const QString& abs_name) const;
    virtual QString makeAbsolute(const QString& abs_or_rel_name, const QString& context) const;
    const QMimeSource* data(const QString& abs_or_rel_name, const QString& context) const;

    virtual void setText(const QString& abs_name, const QString& text);
    virtual void setImage(const QString& abs_name, const QImage& im);
    virtual void setPixmap(const QString& abs_name, const QPixmap& pm);
    virtual void setData(const QString& abs_name, QMimeSource* data);
    virtual void setFilePath(const QStringList&);
    virtual QStringList filePath() const;
    void addFilePath(const QString&);
    virtual void setExtensionType(const QString& ext, const char* mimetype);

private:
    QMimeSource *dataInternal(const QString& abs_name, const QMap<QString, QString> &extensions) const;
    QMimeSourceFactoryData* d;
};

Q_COMPAT_EXPORT QPixmap qPixmapFromMimeSource(const QString &abs_name);

Q_COMPAT_EXPORT QImage qImageFromMimeSource(const QString &abs_name);

#endif // QT_NO_MIMEFACTORY

#endif // QMIMEFACTORY_H
