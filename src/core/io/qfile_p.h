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
#ifndef __QFILE_P_H__
#define __QFILE_P_H__

#include <private/qiodevice_p.h>
#include <private/qinternal_p.h>
#include <qfileengine.h>

class QFilePrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QFile)

protected:
    QFilePrivate();
    ~QFilePrivate();

    bool openExternalFile(int flags, int fd);

    QCircularBuffer buffer;
    QString fileName;
    mutable QFileEngine *fileEngine;

private:
    inline static QByteArray locale_encode(const QString &f)
           { return f.toLocal8Bit(); }
    static QFile::EncoderFn encoder;
    inline static QString locale_decode(const QByteArray &f)
           { return QString::fromLocal8Bit(f); }
    static QFile::DecoderFn decoder;
};

#endif /* __QFILE_P_H__ */
