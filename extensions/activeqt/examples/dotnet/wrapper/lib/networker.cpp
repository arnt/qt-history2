#include "networker.h"
#include "worker.h"
#include "tools.h"

using namespace worker;

netWorker::netWorker()
{
    workerObject = new Worker();
}

netWorker::~netWorker()
{
    delete workerObject;
}

void netWorker::set_StatusString(String *string)
{
    workerObject->setStatusString(StringToQString(string));
}

String *netWorker::get_StatusString()
{
    return QStringToString(workerObject->statusString());
}
