#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qcomponentinterface.h>

class FilterInterface : public QUnknownInterface
{
public:
    FilterInterface( QUnknownInterface *parent = 0 ) 
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "FilterInterface" ); }

    virtual QStringList featureList() const = 0;
    virtual QStringList import( const QString& filter, const QString& filename ) = 0;
};

#endif
