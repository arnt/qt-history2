#include "customaction.h"

CustomAction::CustomAction( QObject* parent )
: QAction( "CustomAction", "&Custom action", 0, parent )
{
    setStatusTip("This is a custom action, added by a run-time loaded plugin");
    connect( this, SIGNAL(activated()), this, SLOT(onActivate()));
}

void CustomAction::onActivate()
{
    qDebug("Custom Action fired!");
}
