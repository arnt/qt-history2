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

#ifndef QBUFFEREDFSFILEENGINE_P_H
#define QBUFFEREDFSFILEENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qplatformdefs.h>
#include <qiodevice.h>

#include "qfileengine.h"
#include <private/qfsfileengine_p.h>

class QBufferedFSFileEnginePrivate;
class QBufferedFSFileEngine : public QFSFileEngine
{
    Q_DECLARE_PRIVATE(QBufferedFSFileEngine)
public:
    QBufferedFSFileEngine();

    enum BufferedFSFileEngineType {
        BufferedFSFileEngine = MaxUser + 1
    };

    Type type() const;
    bool open(int flags);
    bool open(int flags, FILE *fh);
    bool close();
    void flush();
    qint64 at() const;
    bool seek(qint64);
    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);
};

class QBufferedFSFileEnginePrivate : public QFSFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QBufferedFSFileEngine)

public:
    inline QBufferedFSFileEnginePrivate()
    {
        fh = 0;
        lastIOCommand = IOFlushCommand;
    }

    FILE *fh;
    
    enum LastIOCommand
    {
        IOFlushCommand,
        IOReadCommand,
        IOWriteCommand
    };
    LastIOCommand  lastIOCommand;

};

#endif
