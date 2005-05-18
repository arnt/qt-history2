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

#include "worker.h"
#include "tools.h"

Worker::Worker()
{
    status = "Idle";
}

void Worker::setStatusString(const QString &string)
{
    status = string;
    emit statusStringChanged(status);
}

QString Worker::statusString() const
{
    return status;
}
