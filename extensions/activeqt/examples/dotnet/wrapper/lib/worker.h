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

#ifndef WORKER_H
#define WORKER_H

#include <qobject.h>

// native Qt/C++ class
class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusString READ statusString WRITE setStatusString)
public:
    Worker();

    QString statusString() const;

public slots:
    void setStatusString(const QString &string);

signals:
    void statusStringChanged(const QString &string);

private:
    QString status;
};

#endif // WORKER_H
