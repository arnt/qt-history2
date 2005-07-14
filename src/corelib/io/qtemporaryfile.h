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

#include <QtCore/qiodevice.h>
#include <QtCore/qfile.h>

#ifdef open
#error qtemporaryfile.h must be included before any system header that defines open
#endif

QT_MODULE(Core)

class QTemporaryFilePrivate;
class Q_CORE_EXPORT QTemporaryFile : public QFile
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
    Q_DECLARE_PRIVATE(QTemporaryFile)

public:
    QTemporaryFile();
    explicit QTemporaryFile(const QString &templateName);
#ifndef QT_NO_QOBJECT
    explicit QTemporaryFile(QObject *parent);
    QTemporaryFile(const QString &templateName, QObject *parent);
#endif
    ~QTemporaryFile();

    bool autoRemove() const;
    void setAutoRemove(bool b);

    // ### Hides open(flags)
    bool open() { return open(QIODevice::ReadWrite); }

    QString fileName() const;
    QString fileTemplate() const;
    void setFileTemplate(const QString &name);

    inline static QTemporaryFile *createLocalFile(const QString &fileName)
        { QFile file(fileName); return createLocalFile(file); }
    static QTemporaryFile *createLocalFile(QFile &file);

    virtual QFileEngine *fileEngine() const;

protected:
    bool open(OpenMode flags);

private:
    Q_DISABLE_COPY(QTemporaryFile)
};

#endif // QTEMPORARYFILE_H
