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

#ifndef QTEMPORARYFILE_H
#define QTEMPORARYFILE_H

#include <qiodevice.h>
#include <qfile.h>

class QTemporaryFilePrivate;
class Q_CORE_EXPORT QTemporaryFile : public QFile
{
    Q_DECLARE_PRIVATE(QTemporaryFile)

public:
    QTemporaryFile();
    QTemporaryFile(const QString &templateName);
    ~QTemporaryFile();

    bool autoRemove() const;
    void setAutoRemove(bool b);

    bool open() { return QFile::open(QIODevice::ReadWrite); }

    QString fileName() const;
    QString fileTemplate() const;
    void setFileTemplate(const QString &name);

    inline static QTemporaryFile *createLocalFile(const QString &fileName)
        { QFile file(fileName); return createLocalFile(file); }
    static QTemporaryFile *createLocalFile(QFile &file);

    virtual QFileEngine *fileEngine() const;

private:
    Q_DISABLE_COPY(QTemporaryFile)
};

#endif // QTEMPORARYFILE_H
