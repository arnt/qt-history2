#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qcomponentinterface.h>

class FilterInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;

    virtual QStringList import( const QString& filter, const QString& filename ) = 0;

    QString interfaceID() const { return "FilterInterface_QtDesigner_Trolltech_05102000_0516"; }
};

#endif
