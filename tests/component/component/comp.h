#ifndef COMP_H
#define COMP_H

#include "../interfaces/printinterface.h"

class Component : public PrintInterface
{
public:
    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;
};

#endif // COMP_H
