#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>

class QAction;
class QObject;

// {BB206E09-84E5-4777-9FCE-706BABFAB931}
#ifndef IID_ActionInterface
#define IID_ActionInterface QUuid( 0xbb206e09, 0x84e5, 0x4777, 0x9f, 0xce, 0x70, 0x6b, 0xab, 0xfa, 0xb9, 0x31 )
#endif

class ActionInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QString group( const QString & ) const = 0;
    virtual void connectTo( QUnknownInterface *appInterface ) = 0;
};

#endif
