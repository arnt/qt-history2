#include "designerappiface.h"


DesignerApplicationInterface::DesignerApplicationInterface()
    : QApplicationInterface()
{
}


QComponentInterface * DesignerApplicationInterface::requestInterface( const QCString &request )
{
    return 0;
}
