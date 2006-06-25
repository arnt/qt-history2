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

#ifndef QRESOURCE_H
#define QRESOURCE_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QResourcePrivate;

class Q_CORE_EXPORT QResource
{
public:
    QResource(const QString &file=QString());

    void setFile(const QString &file);
    QString filePath() const;

    QString canonicalFilePath() const;

    bool exists() const;

    inline bool isFile() const { return !isDir(); }
    bool isCompressed() const;
    qint64 size() const;
    const uchar *data() const;

    bool isDir() const;
    QStringList children() const;

    static void addSearchPath(const QString &path);
    static QStringList searchPaths();

    static bool registerResource(const QString &rccFilename);
    static bool unregisterResource(const QString &rccFilename);

protected:
    QResourcePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QResource)
};

QT_END_HEADER

#endif // QRESOURCE_H
