#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qcomponentinterface.h>

// {EA8CB381-59B5-44a8-BAE5-9BEA8295762A}
Q_GUID(IID_FilterInterface, 
0xea8cb381, 0x59b5, 0x44a8, 0xba, 0xe5, 0x9b, 0xea, 0x82, 0x95, 0x76, 0x2a);

class FilterInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual QStringList import( const QString& filter, const QString& filename ) = 0;
};

#endif
