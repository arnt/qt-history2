#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qapplicationinterface.h>
#include <qmap.h>
#include <qguardedptr.h>

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();
    
    QComponentInterface *requestInterface( const QCString &request );
    
private:
    QMap<QString, QGuardedPtr<QComponentInterface> > interfaces;
    
};

#endif
