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
