#ifndef EVENTINTERFACED_H
#define EVENTINTERFACED_H

#include <qcomponentinterface.h>
#include <qstringlist.h>
#include <qobject.h>

// {9958cfbc-64f9-44ce-a65e-2c6c11969a7b}
#ifndef IID_EventInterface
#define IID_EventInterface QUuid( 0x9958cfbc, 0x64f9, 0x44ce, 0xa6, 0x5e, 0x2c, 0x6c, 0x11, 0x96, 0x9a, 0x7b )
#endif

class EventInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual QStringList events( QObject *obj ) const = 0;
    virtual void setEventHandler( QObject *obj, const QString &event, const QString &function ) = 0;

};

#endif
