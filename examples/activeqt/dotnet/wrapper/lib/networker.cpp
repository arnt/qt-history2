/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "networker.h"
#include "worker.h"
#include "tools.h"

netWorker::netWorker()
{
    workerObject = new Worker();
}

netWorker::~netWorker()
{
    delete workerObject;
}

String *netWorker::get_StatusString()
{
    return QStringToString(workerObject->statusString());
}

void netWorker::set_StatusString(String *string)
{
    workerObject->setStatusString(StringToQString(string));
    __raise statusStringChanged(string);
}
