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
#ifndef __QTEMPORARYFILE_H__
#define __QTEMPORARYFILE_H__

#include <qiodevice.h>
#include <qfile.h>

class QTemporaryFilePrivate;
class Q_CORE_EXPORT QTemporaryFile : public QIODevice
{
    Q_DECLARE_PRIVATE(QTemporaryFile)

public:
    QTemporaryFile();
    QTemporaryFile(const QString &templateName);
    ~QTemporaryFile();

    bool autoRemove() const;
    void setAutoRemove(bool b);
    bool remove();

    int handle() const;

    inline bool open() { return QIODevice::open(IO_ReadWrite); }

    QString fileName() const;
    QString fileTemplate() const;
    void setFileTemplate(const QString &name);

    virtual QIOEngine *ioEngine() const;

    inline static QTemporaryFile *createLocalFile(const QString &fileName)
        { QFile file(fileName); return createLocalFile(file); }
    static QTemporaryFile *createLocalFile(QFile &file);

private:
#if defined(Q_DISABLE_COPY)
    QTemporaryFile(const QTemporaryFile &);
    QTemporaryFile &operator=(const QTemporaryFile &);
#endif
};

#endif /* __QTEMPORARYFILE_H__ */
