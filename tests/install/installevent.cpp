#include "installevent.h"

InstallEvent::InstallEvent( int Type, QString name, int prog ) : QCustomEvent( eventID )
{
    fileName = name;
    progress = prog;
    eType = Type;
}