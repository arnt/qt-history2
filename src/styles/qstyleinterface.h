#ifndef QSTYLEINTERFACE_H
#define QSTYLEINTERFACE_H

#ifndef QT_H
#include "qcomponentinterface.h"
#endif // QT_H

class QStyle;

class Q_EXPORT QStyleInterface : public QUnknownInterface
{
public:
    QStyleInterface( QUnknownInterface *parent = 0 ) 
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "QStyleInterface" ); }
    
    virtual QStringList featureList() const = 0; 
    virtual QStyle* create( const QString& style ) = 0;
};

#endif //QSTYLEINTERFACE_H
