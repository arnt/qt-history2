#ifndef INTERPRETERINTERFACE_H
#define INTERPRETERINTERFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>

class QObject;

// {11cad9ec-4e3c-418b-8e90-e1b8c0c1f48f}
Q_UUID( IID_InterpreterInterface,
	0x11cad9ec, 0x4e3c, 0x418b, 0x8e, 0x90, 0xe1, 0xb8, 0xc0, 0xc1, 0xf4, 0x8f );

struct InterpreterInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0;
    virtual bool exec( QObject *obj, const QString &code ) = 0;
};



#endif
