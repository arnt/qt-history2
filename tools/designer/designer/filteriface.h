#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qcomponentinterface.h>

class FilterInterface : public QUnknownInterface
{
public:
    FilterInterface( QUnknownInterface *parent = 0, const char *name = 0 ) 
	: QUnknownInterface( parent, name ) {}

    virtual QStringList featureList() const = 0;

    virtual QStringList import( const QString& filter, const QString& filename ) = 0;

    QString interfaceID() const { return createID( QUnknownInterface::interfaceID(), "FilterInterface" ); }
};

#endif
