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

#ifndef BLOCKINGPROCESS_H
#define BLOCKINGPROCESS_H

#include <qprocess.h>
#include <qbytearray.h>

class QEventLoop;

class BlockingProcess : public QProcess
{
    Q_OBJECT

public:
    BlockingProcess();

    virtual bool start(QStringList *env=0);

public slots:
    void readOut();
    void readErr();
    void exited();

public:
    QEventLoop *eventLoop;
    QByteArray out;
    QByteArray err;
    int outUsed;
    int errUsed;
};

#endif // BLOCKINGPROCESS_H
