#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

class QPlugInInterface
{
public:
    QPlugInInterface() {}
    virtual ~QPlugInInterface() {}

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }
};

#endif
