#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include <qapplication.h>

class QDualInterface : public QObject
{
    Q_OBJECT
signals:
    // Plugin:	    requests the value of a property
    void readProperty( const QCString&, QVariant& );

    // Plugin:	    demands to change a property
    // Application: answers an request
    void writeProperty( const QCString&, const QVariant& );

public slots:
    // Would be cool to have a makro defined 

    // Application: processes the readProperty request
    // PlugIn:	    sends a readProperty request
    virtual void requestProperty( const QCString&, QVariant& ) {}

    // Application: processes the writeProperty request
    // PlugIn:	    sends a writeProperty request
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

class PlugMainWindowInterface : public QApplicationInterface
{
public:
    void requestProperty( const QCString&, QVariant& );
    void requestSetProperty( const QCString&, const QVariant& );
};

#endif //QAPPLICATIONINTERFACES_H