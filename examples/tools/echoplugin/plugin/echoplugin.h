#ifndef ECHOPLUGIN_H
#define ECHOPLUGIN_H

#include <QObject>
#include "echoplugin.h"
#include "echointerface.h"

class EchoPlugin : public QObject, EchoInterface
{
    Q_OBJECT
    Q_INTERFACES(EchoInterface)

public:
    QString echo(const QString &message); 
};

#endif
