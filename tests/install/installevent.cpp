#include "installevent.h"

InstallEvent::InstallEvent( QString name, int prog ) : QCustomEvent( eventID )
{
    fileName = name;
    progress = prog;
}