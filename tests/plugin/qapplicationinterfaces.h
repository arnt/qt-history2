#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include <qobject.h>

class QDualInterface : public QObject
{
    Q_OBJECT
signals:
    void readProperty( const QCString&, QVariant& );
    void writeProperty( const QCString&, const QVariant& );

public slots:
    virtual void requestProperty( const QCString&, QVariant& ) {}
    virtual void requestSetProperty( const QCString&, const QVariant& ) {}
};

class QClientInterface : public QDualInterface
{
public:
    void requestProperty( const QCString& p, QVariant& v ) 
    {
	emit readProperty( p, v );
    }
    void requestSetProperty( const QCString& p, const QVariant& v )
    {
	emit writeProperty( p, v );
    }
};

class QApplicationInterface : public QDualInterface
{
public:
    void requestProperty( const QCString&, QVariant& ) = 0;
    void requestSetProperty( const QCString&, const QVariant& ) = 0;
};

#endif //QAPPLICATIONINTERFACES_H