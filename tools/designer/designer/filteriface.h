#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qplugininterface.h>

class FilterInterface : public QPlugInInterface
{
public:
    QCString queryPlugInInterface() const { return "FilterInterface"; }

    virtual QStringList import( const QString& filter, const QString& filename ) = 0;
};

#endif
